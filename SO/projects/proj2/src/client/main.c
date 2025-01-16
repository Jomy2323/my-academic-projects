#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parser.h"
#include "src/client/api.h"
#include "src/common/constants.h"
#include "src/common/io.h"
#include <errno.h>
#include <signal.h>
#include "src/client/globals.c"


int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <client_unique_id> <register_pipe_path>\n",
            argv[0]);
    return 1;
  }

  char req_pipe_path[256] = "/tmp/req";
  char resp_pipe_path[256] = "/tmp/resp";
  char notif_pipe_path[256] = "/tmp/notif";
  
  char keys[MAX_NUMBER_SUB][MAX_STRING_SIZE] = {0};
  unsigned int delay_ms;
  size_t num;

  strncat(req_pipe_path, argv[1], strlen(argv[1]) * sizeof(char));
  strncat(resp_pipe_path, argv[1], strlen(argv[1]) * sizeof(char));
  strncat(notif_pipe_path, argv[1], strlen(argv[1]) * sizeof(char));

  KVSConnection* client_connection = malloc(sizeof(KVSConnection));
  if (!client_connection) {
    fprintf(stderr,"Failed to allocate memory for client connection\n");
    return 1;
  }
  strncpy(client_connection->req_fifo_path, req_pipe_path, MAX_STRING_SIZE);
  strncpy(client_connection->resp_fifo_path, resp_pipe_path, MAX_STRING_SIZE);
  strncpy(client_connection->notif_fifo_path, notif_pipe_path, MAX_STRING_SIZE);
  strncpy(client_connection->server_fifo_path, argv[2], MAX_STRING_SIZE);
  client_connection->client_id = argv[1];
  client_connection->req_fd = -1;
  client_connection->resp_fd = -1;
  client_connection->notif_fd = -1;

  if (kvs_connect(client_connection)){
    fprintf(stderr, "Client %s could not connect to server\n", client_connection->client_id);
    exit(EXIT_FAILURE);
  }

  while (1) {
    pthread_mutex_lock(&server_disconnected_mutex);
    if (server_disconnected){
      
      if (pthread_join(client_connection->notif_thread, NULL) != 0){
        fprintf(stderr, "Error joining client notification thread\n");
        pthread_mutex_unlock(&server_disconnected_mutex);
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
      free(client_connection);
      fprintf(stderr, "Server has disconnected unexpectedly. Exiting.\n");
      pthread_mutex_unlock(&server_disconnected_mutex);
      return 0;      
    }
    pthread_mutex_unlock(&server_disconnected_mutex);

    switch (get_next(STDIN_FILENO)) {
    case CMD_DISCONNECT:
      if (kvs_disconnect(client_connection) != 0) {
        fprintf(stderr, "Failed to disconnect to the server\n");
        free(client_connection);
        return 1;
      }
      free(client_connection);
      return 0;

    case CMD_SUBSCRIBE:
      num = parse_list(STDIN_FILENO, keys, 1, MAX_STRING_SIZE);
      if (num == 0) {
        fprintf(stderr, "Invalid command. See HELP for usage\n");
        continue;
      }

      if (kvs_subscribe(client_connection, keys[0])) {
        fprintf(stderr, "Command subscribe failed\n");
      }

      break;

    case CMD_UNSUBSCRIBE:
      num = parse_list(STDIN_FILENO, keys, 1, MAX_STRING_SIZE);
      if (num == 0) {
        fprintf(stderr, "Invalid command. See HELP for usage\n");
        continue;
      }
      if (kvs_unsubscribe(client_connection, keys[0])) {
        fprintf(stderr, "Command subscribe failed\n");
      }

      break;

    case CMD_DELAY:
      if (parse_delay(STDIN_FILENO, &delay_ms) == -1) {
        fprintf(stderr, "Invalid command. See HELP for usage\n");
        continue;
      }

      if (delay_ms > 0) {
        printf("Waiting...\n");
        delay(delay_ms);
      }
      break;

    case CMD_INVALID:
      fprintf(stderr, "Invalid command. See HELP for usage\n");
      break;

    case CMD_EMPTY:
      break;

    case EOC:
      // input should end in a disconnect, or it will loop here forever
      break;
    }
  }
  return 0;
}
