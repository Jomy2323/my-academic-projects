#include "api.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>


#include "src/client/globals.h"
#include "src/common/constants.h"
#include "src/common/protocol.h"

void* notif_listener(void* arg) {
    int fd = *(int*)arg;
    char buffer[MAX_STRING_SIZE * 2 + 2] = {0};  // Buffer for incoming notification
    char key[MAX_STRING_SIZE + 1] = {0};        // Null-terminated key
    char value[MAX_STRING_SIZE + 1] = {0};      // Null-terminated value
    ssize_t total_read = 0;

    while (1) {
        size_t remaining_bytes = sizeof(buffer) - (size_t)total_read;
        ssize_t bytes_read = read(fd, buffer + total_read, remaining_bytes);
        if (bytes_read > 0) {
            total_read += bytes_read;

            if (total_read == sizeof(buffer)) {
                strncpy(key, buffer, MAX_STRING_SIZE);
                key[MAX_STRING_SIZE] = '\0';
                strncpy(value, buffer + MAX_STRING_SIZE, MAX_STRING_SIZE);
                value[MAX_STRING_SIZE] = '\0';

                fprintf(stdout, "(%s,%s)\n", key, value);

                total_read = 0;
            }
        } else if (bytes_read == 0) {
            fprintf(stderr, "Notification FIFO closed by server. Exiting thread.\n");
            pthread_mutex_lock(&server_disconnected_mutex);
            server_disconnected = 1;
            pthread_mutex_unlock(&server_disconnected_mutex);
            break;
        } else if (bytes_read < 0) {
            fprintf(stderr,"Error reading from notification FIFO\n");
            break;
        }
    }

    return NULL;
}


int kvs_connect(KVSConnection* client_connection) {

    // Validate path lengths
    if (strlen(client_connection->req_fifo_path) >= MAX_PIPE_PATH_LENGTH || 
        strlen(client_connection->resp_fifo_path) >= MAX_PIPE_PATH_LENGTH || 
        strlen(client_connection->notif_fifo_path) >= MAX_PIPE_PATH_LENGTH) {
        fprintf(stderr, "FIFO path exceeds maximum length for client %s\n", client_connection->client_id);
        return 1;
    }

    // Unlink and handle errors
    if ((unlink(client_connection->req_fifo_path) < 0 && errno != ENOENT) ||
        (unlink(client_connection->resp_fifo_path) < 0 && errno != ENOENT) ||
        (unlink(client_connection->notif_fifo_path) < 0 && errno != ENOENT)) {
        fprintf(stderr, "Failed to unlink one or more FIFOs for client %s\n", client_connection->client_id);
        return 1;
    }

    // Create FIFOs
    if (mkfifo(client_connection->req_fifo_path, 0666) < 0) {
        fprintf(stderr, "Failed to create request FIFO for client %s\n", client_connection->client_id);
        return 1;
    }

    if (mkfifo(client_connection->resp_fifo_path, 0666) < 0) {
        fprintf(stderr, "Failed to create response FIFO for client %s\n", client_connection->client_id);
        unlink(client_connection->req_fifo_path);
        return 1;
    }

    if (mkfifo(client_connection->notif_fifo_path, 0666) < 0) {
        fprintf(stderr, "Failed to create notification FIFO for client %s\n", client_connection->client_id);
        unlink(client_connection->req_fifo_path);
        unlink(client_connection->resp_fifo_path);
        return 1;
    }

    // Construct client message
    char client_message[MAX_CLIENT_MESSAGE_SIZE] = {0};
    client_message[0] = '1'; // OP_CODE for connect
    strncpy(&client_message[1], client_connection->req_fifo_path, MAX_PIPE_PATH_LENGTH - 1);
    client_message[MAX_PIPE_PATH_LENGTH] = '\0';
    strncpy(&client_message[MAX_PIPE_PATH_LENGTH + 1], client_connection->resp_fifo_path, MAX_PIPE_PATH_LENGTH - 1);
    client_message[MAX_PIPE_PATH_LENGTH * 2] = '\0';
    strncpy(&client_message[MAX_PIPE_PATH_LENGTH * 2 + 1], client_connection->notif_fifo_path, MAX_PIPE_PATH_LENGTH - 1);
    client_message[MAX_PIPE_PATH_LENGTH * 3] = '\0';

    // Open server FIFO and write client message
    int fserv = open(client_connection->server_fifo_path, O_WRONLY);
    if (fserv < 0) {
        fprintf(stderr, "Could not open server register FIFO\n");
        unlink(client_connection->req_fifo_path);
        unlink(client_connection->resp_fifo_path);
        unlink(client_connection->notif_fifo_path);
        return 1;
    }

    if (write(fserv, client_message, MAX_CLIENT_MESSAGE_SIZE) != MAX_CLIENT_MESSAGE_SIZE) {
        fprintf(stderr, "Failed to write client message to server FIFO\n");
        close(fserv);
        unlink(client_connection->req_fifo_path);
        unlink(client_connection->resp_fifo_path);
        unlink(client_connection->notif_fifo_path);
        return 1;
    }
    close(fserv);

    // Open response FIFO and read server response
    client_connection->resp_fd = open(client_connection->resp_fifo_path, O_RDONLY);
    if (client_connection->resp_fd < 0) {
        fprintf(stderr, "Could not open response FIFO for client %s\n", client_connection->client_id);
        unlink(client_connection->req_fifo_path);
        unlink(client_connection->resp_fifo_path);
        unlink(client_connection->notif_fifo_path);
        return 1;
    }

    char response[2];
    if (read(client_connection->resp_fd, response, sizeof(response)) != sizeof(response)) {
        fprintf(stderr, "Error reading server response for client %s\n", client_connection->client_id);
        close(client_connection->resp_fd);
        unlink(client_connection->req_fifo_path);
        unlink(client_connection->resp_fifo_path);
        unlink(client_connection->notif_fifo_path);
        return 1;
    }
    fprintf(stdout, "Server returned %c for operation: connect\n", response[1]);
    char op_code = response[0];
    char result = response[1];
    if (op_code != '1' || result != '0') {
        fprintf(stderr, "Server connection failed for client %s with result code: %c\n", client_connection->client_id, result);
        close(client_connection->resp_fd);
        unlink(client_connection->req_fifo_path);
        unlink(client_connection->resp_fifo_path);
        unlink(client_connection->notif_fifo_path);
        return 1;
    }

    // Open request and notification FIFOs
    client_connection->req_fd = open(client_connection->req_fifo_path, O_WRONLY);
    if (client_connection->req_fd < 0) {
        fprintf(stderr, "Could not open request FIFO for client %s\n", client_connection->client_id);
        close(client_connection->resp_fd);
        unlink(client_connection->req_fifo_path);
        unlink(client_connection->resp_fifo_path);
        unlink(client_connection->notif_fifo_path);
        return 1;
    }

    client_connection->notif_fd = open(client_connection->notif_fifo_path, O_RDONLY);
    if (client_connection->notif_fd < 0) {
        fprintf(stderr, "Could not open notification FIFO for client %s\n", client_connection->client_id);
        close(client_connection->req_fd);
        close(client_connection->resp_fd);
        unlink(client_connection->req_fifo_path);
        unlink(client_connection->resp_fifo_path);
        unlink(client_connection->notif_fifo_path);
        return 1;
    }
    
    if (pthread_create(&client_connection->notif_thread, NULL, notif_listener, (void*)&client_connection->notif_fd) != 0){
        fprintf(stderr, "Error creating client notification thread\n");
        return 1;
    }


    return 0;
}



int kvs_disconnect(KVSConnection* client_connection) {
    char disconnect_msg[1] = {'2'};

    if (write(client_connection->req_fd, disconnect_msg, 1) < 0) {
        fprintf(stderr,"Failed to send disconnect message\n");
        return 1;
    }

    char response[2];
    ssize_t bytes_read = read(client_connection->resp_fd, response, 2);
    if (bytes_read < 0) {
        fprintf(stderr,"Failed to read disconnect response\n");
        return 1;
    } else if (bytes_read < 2) {
        fprintf(stderr, "Incomplete disconnect response\n");
        return 1;
    }
    fprintf(stdout, "Server returned %c for operation: disconnect\n", response[1]);

    if (response[0] != '2' || response[1] != '0') {
        fprintf(stderr, "Server reported an error on disconnect (op=%c, result=%c)\n",
                response[0], response[1]);
        return 1;
    }

    if (pthread_cancel(client_connection->notif_thread) != 0) {
        fprintf(stderr, "Failed to cancel notification thread\n");
        return 1;
    }
    if (pthread_join(client_connection->notif_thread, NULL) != 0){
        fprintf(stderr, "Error joining client notification thread\n");
        return 1;
    }
    close(client_connection->req_fd);
    close(client_connection->resp_fd);
    close(client_connection->notif_fd);

    if (unlink(client_connection->req_fifo_path) < 0 && errno != ENOENT) {
        perror("Failed to unlink request FIFO");
    }
    if (unlink(client_connection->resp_fifo_path) < 0 && errno != ENOENT) {
        perror("Failed to unlink response FIFO");
    }
    if (unlink(client_connection->notif_fifo_path) < 0 && errno != ENOENT) {
        perror("Failed to unlink notification FIFO");
    }
    return 0;
}

int kvs_subscribe(KVSConnection* client_connection, const char *key) {
    if (strlen(key) > MAX_STRING_SIZE){
        fprintf(stderr, "Key length is bigger than 40 bytes\n");
        return 1;
    }

  char message[MAX_STRING_SIZE + 2] = {0}; // 1 for opcode, 1 for \0
  message[0] = '3';
  strncpy(&message[1], key, MAX_STRING_SIZE);
  message[MAX_STRING_SIZE + 1] = '\0';

  if (write(client_connection->req_fd, message, sizeof(message)) < 0){
    fprintf(stderr, "Failed to send subscription message\n");
    return 1;
  }

  char response[2];
  ssize_t bytes_read = read(client_connection->resp_fd, response, 2);
  if (bytes_read < 0) {
        fprintf(stderr,"Failed to read subscribe response\n");
        return 1;
    } else if (bytes_read < 2) {
        fprintf(stderr, "Incomplete subscribe response\n");
        return 1;
    }

    fprintf(stdout, "Server returned %c for operation: subscribe\n", response[1]);
    return 0;
}

int kvs_unsubscribe(KVSConnection* client_connection, const char *key) {
    if (strlen(key) > MAX_STRING_SIZE){
        fprintf(stderr, "Key length is bigger than 40 bytes\n");
        return 1;
    }

    char message[MAX_STRING_SIZE + 2] = {0}; // 1 for opcode, 1 for \0
    message[0] = '4';
    strncpy(&message[1], key, MAX_STRING_SIZE);
    message[MAX_STRING_SIZE + 1] = '\0';

    if (write(client_connection->req_fd, message, sizeof(message)) < 0){
        fprintf(stderr, "Failed to send subscription message\n");
        return 1;
    }
    char response[2];
    ssize_t bytes_read = read(client_connection->resp_fd, response, 2);
    if (bytes_read < 0) {
        fprintf(stderr,"Failed to read unsubscribe response\n");
        return 1;
    } else if (bytes_read < 2) {
        fprintf(stderr, "Incomplete unsubscribe response\n");
        return 1;
    }

    fprintf(stdout, "Server returned %c for operation: unsubscribe\n", response[1]);

    return 0;
}
