#define _XOPEN_SOURCE 700

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <stdatomic.h>



#include "constants.h"
#include "io.h"
#include "operations.h"
#include "parser.h"
#include "pthread.h"
#include "src/common/constants.h"
#include "subscriptions.h"

struct SharedData {
  DIR *dir;
  char *dir_name;
  pthread_mutex_t directory_mutex;
};

typedef struct {
    char client_init_message[MAX_CLIENT_MESSAGE_SIZE];
} SessionRequest;

typedef struct {
    SessionRequest buffer[MAX_SESSION_COUNT];
    size_t head;
    size_t tail;
    sem_t empty_slots; 
    sem_t filled_slots; 
    pthread_mutex_t mutex;
} ProducerConsumerBuffer;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t n_current_backups_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t client_info_array_mutex = PTHREAD_MUTEX_INITIALIZER;


size_t active_backups = 0; // Number of active backups
size_t max_backups;        // Maximum allowed simultaneous backups
size_t max_threads;        // Maximum allowed simultaneous threads
char *jobs_directory = NULL;
pthread_t* worker_threads;
size_t active_worker_threads;

ProducerConsumerBuffer session_buffer = {
    .head = 0,
    .tail = 0,
    .empty_slots = {{0}},
    .filled_slots = {{0}},
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

pthread_t session_threads[MAX_SESSION_COUNT];
ClientInfo* client_info_array[MAX_SESSION_COUNT] = {NULL};


int process_client_message(char* client_message, ClientInfo* client_info);
ClientInfo* handle_connect(char* client_message, int thread_id);
int handle_disconnect(ClientInfo* client_info);
int handle_subscribe(char* client_message, ClientInfo* client_info);
int handle_unsubscribe(char* client_message, ClientInfo* client_info);
void* session_manager_task(void* id);


//signals 
atomic_int global_cleanup_flag = 0;
void sigusr1_handler(int sig) {
    (void)sig;
    int saved_errno = errno;  // Save errno
    atomic_store(&global_cleanup_flag, 1); // Set the flag atomically
    fprintf(stderr, "SIGUSR1 received: initiating cleanup...\n");
    errno = saved_errno;      // Restore errno
}


void block_sigusr1() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    // Block SIGUSR1 for this thread
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

void cleanup_all_client_fifos() {
  printf("VAMOS LA LIMPAR TUDO\n");
    // Acquire the global array lock to safely iterate over clients.
    pthread_mutex_lock(&client_info_array_mutex);
    for (int i = 0; i < MAX_SESSION_COUNT; i++) {
        ClientInfo* ci = client_info_array[i];
        if (ci != NULL) {
            // Lock the per-client mutex to safely close its FIFOs.
            pthread_mutex_lock(&ci->lock);

            // Remove subscriptions based on the `notif_fd` directly.
            if (ci->notif_fd != -1) {
                remove_subscriptions_for_fd(ci->notif_fd);
            }

            // Close FIFOs if they are open.
            if (ci->req_fd != -1) {
                close(ci->req_fd);
                ci->req_fd = -1;
            }
            if (ci->resp_fd != -1) {
                close(ci->resp_fd);
                ci->resp_fd = -1;
            }
            if (ci->notif_fd != -1) {
                close(ci->notif_fd);
                ci->notif_fd = -1;
            }

            pthread_mutex_unlock(&ci->lock);

            // Nullify the client info in the array.
            client_info_array[i] = NULL;
        }
    }
    pthread_mutex_unlock(&client_info_array_mutex);
}


ClientInfo* create_client_info(const char* client_id, int req_fd, int resp_fd, int notif_fd, int thread_id) {
    ClientInfo* client_info = malloc(sizeof(ClientInfo));
    if (!client_info) {
        perror("Failed to allocate ClientInfo");
        return NULL;
    }
    client_info->notif_fd = notif_fd;
    client_info->resp_fd = resp_fd;
    client_info->req_fd = req_fd;
    strncpy(client_info->client_id, client_id, MAX_STRING_SIZE - 1);
    client_info->client_id[MAX_STRING_SIZE - 1] = '\0'; // Ensure null termination
    client_info->thread_id = thread_id;
    if (pthread_mutex_init(&client_info->lock, NULL) != 0) {
        free(client_info);
        return NULL;
    }
    printf("%d is client %s notif fifo\n", notif_fd, client_id);
    return client_info;
}

int filter_job_files(const struct dirent *entry) {
  const char *dot = strrchr(entry->d_name, '.');
  if (dot != NULL && strcmp(dot, ".job") == 0) {
    return 1; // Keep this file (it has the .job extension)
  }
  return 0;
}

static int entry_files(const char *dir, struct dirent *entry, char *in_path,
                       char *out_path) {
  const char *dot = strrchr(entry->d_name, '.');
  if (dot == NULL || dot == entry->d_name || strlen(dot) != 4 ||
      strcmp(dot, ".job")) {
    return 1;
  }

  if (strlen(entry->d_name) + strlen(dir) + 2 > MAX_JOB_FILE_NAME_SIZE) {
    fprintf(stderr, "%s/%s\n", dir, entry->d_name);
    return 1;
  }

  strcpy(in_path, dir);
  strcat(in_path, "/");
  strcat(in_path, entry->d_name);

  strcpy(out_path, in_path);
  strcpy(strrchr(out_path, '.'), ".out");

  return 0;
}

static int run_job(int in_fd, int out_fd, char *filename) {
  size_t file_backups = 0;
  while (1) {
    char keys[MAX_WRITE_SIZE][MAX_STRING_SIZE] = {0};
    char values[MAX_WRITE_SIZE][MAX_STRING_SIZE] = {0};
    unsigned int delay;
    size_t num_pairs;

    switch (get_next(in_fd)) {
    case CMD_WRITE:
      num_pairs =
          parse_write(in_fd, keys, values, MAX_WRITE_SIZE, MAX_STRING_SIZE);
      if (num_pairs == 0) {
        write_str(STDERR_FILENO, "Invalid command. See HELP for usage\n");
        continue;
      }

      if (kvs_write(num_pairs, keys, values)) {
        write_str(STDERR_FILENO, "Failed to write pair\n");
      }
      break;

    case CMD_READ:
      num_pairs =
          parse_read_delete(in_fd, keys, MAX_WRITE_SIZE, MAX_STRING_SIZE);

      if (num_pairs == 0) {
        write_str(STDERR_FILENO, "Invalid command. See HELP for usage\n");
        continue;
      }

      if (kvs_read(num_pairs, keys, out_fd)) {
        write_str(STDERR_FILENO, "Failed to read pair\n");
      }
      break;

    case CMD_DELETE:
      num_pairs =
          parse_read_delete(in_fd, keys, MAX_WRITE_SIZE, MAX_STRING_SIZE);

      if (num_pairs == 0) {
        write_str(STDERR_FILENO, "Invalid command. See HELP for usage\n");
        continue;
      }

      if (kvs_delete(num_pairs, keys, out_fd)) {
        write_str(STDERR_FILENO, "Failed to delete pair\n");
      }
      break;

    case CMD_SHOW:
      kvs_show(out_fd);
      break;

    case CMD_WAIT:
      if (parse_wait(in_fd, &delay, NULL) == -1) {
        write_str(STDERR_FILENO, "Invalid command. See HELP for usage\n");
        continue;
      }

      if (delay > 0) {
        printf("Waiting %d seconds\n", delay / 1000);
        kvs_wait(delay);
      }
      break;

    case CMD_BACKUP:
      pthread_mutex_lock(&n_current_backups_lock);
      if (active_backups >= max_backups) {
        wait(NULL);
      } else {
        active_backups++;
      }
      pthread_mutex_unlock(&n_current_backups_lock);
      int aux = kvs_backup(++file_backups, filename, jobs_directory);

      if (aux < 0) {
        write_str(STDERR_FILENO, "Failed to do backup\n");
      } else if (aux == 1) {
        return 1;
      }
      break;

    case CMD_INVALID:
      write_str(STDERR_FILENO, "Invalid command. See HELP for usage\n");
      break;

    case CMD_HELP:
      write_str(STDOUT_FILENO,
                "Available commands:\n"
                "  WRITE [(key,value)(key2,value2),...]\n"
                "  READ [key,key2,...]\n"
                "  DELETE [key,key2,...]\n"
                "  SHOW\n"
                "  WAIT <delay_ms>\n"
                "  BACKUP\n" // Not implemented
                "  HELP\n");

      break;

    case CMD_EMPTY:
      break;

    case EOC:
      printf("EOF\n");
      return 0;
    }
  }
}

// frees arguments
static void *get_file(void *arguments) {
    block_sigusr1();
    struct SharedData *thread_data = (struct SharedData *)arguments;
    DIR *dir = thread_data->dir;
    char *dir_name = thread_data->dir_name;

  while (1) {
    if (pthread_mutex_lock(&thread_data->directory_mutex) != 0) {
        fprintf(stderr, "Thread failed to lock directory_mutex\n");
        pthread_exit(NULL);
    }

    struct dirent *entry = readdir(dir);

    if (pthread_mutex_unlock(&thread_data->directory_mutex) != 0) {
        fprintf(stderr, "Thread failed to unlock directory_mutex\n");
        pthread_exit(NULL);
    }

    if (entry == NULL) {
        break;
    }

    char in_path[MAX_JOB_FILE_NAME_SIZE];
    char out_path[MAX_JOB_FILE_NAME_SIZE];
    if (entry_files(dir_name, entry, in_path, out_path) != 0) {
        continue;
    }

    int in_fd = open(in_path, O_RDONLY);
    if (in_fd == -1) {
        fprintf(stderr, "[Thread %lu] Failed to open input: %s\n",
                (unsigned long)pthread_self(), in_path);
        continue;
    }

    int out_fd = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (out_fd == -1) {
        fprintf(stderr, "[Thread %lu] Failed to open output: %s\n",
                (unsigned long)pthread_self(), out_path);
        close(in_fd);
        continue;
    }

    run_job(in_fd, out_fd, entry->d_name);

    close(in_fd);
    close(out_fd);
  }

  pthread_exit(NULL);
}



void dispatch_threads(struct SharedData* thread_data) {
    worker_threads = malloc(max_threads * sizeof(pthread_t));
    if (worker_threads == NULL) {
        fprintf(stderr, "Failed to allocate memory for threads\n");
        return;
    }

    for (size_t i = 0; i < max_threads; i++) {
        if (pthread_create(&worker_threads[i], NULL, get_file,
                           (void*)thread_data) != 0) {
            fprintf(stderr, "Failed to create thread %zu\n", i);
            continue;
        }
        active_worker_threads++;
    }
}

void shutdown_threads() {
    for (size_t i = 0; i < active_worker_threads; i++) {
        if (pthread_join(worker_threads[i], NULL) != 0) {
            fprintf(stderr, "Failed to join thread %zu\n", i);
        }
    }

    free(worker_threads);
    worker_threads = NULL;
    active_worker_threads = 0;
}


int process_client_message(char* client_message, ClientInfo* client_info){
  char op_code = client_message[0];

  switch (op_code) {       
        case '2': // Disconnect
            return handle_disconnect(client_info);
        
        case '3': // Subscribe
            return handle_subscribe(client_message, client_info);
        
        case '4': // Unsubscribe
            return handle_unsubscribe(client_message, client_info);
        
        default:
            fprintf(stderr, "Invalid operation code: %c\n", op_code);
            return 1;
    }
}

void handle_client_session(ClientInfo* client_info){
  char buffer[MAX_CLIENT_MESSAGE_SIZE];
  int req_fd = client_info->req_fd;
  while (1) {
        
        ssize_t bytes_read = read(req_fd, buffer, sizeof(buffer));
        
        if (bytes_read == 0) {
            fprintf(stdout, "FIFO closed for client %s, ending session.\n", client_info->client_id);
            break;
        } else if (bytes_read < 0) {
            if (errno == EINTR && atomic_load(&global_cleanup_flag)) {
                fprintf(stdout, "Read interrupted by signal for client %s.\n", client_info->client_id);
                break;
            }
            fprintf(stderr, "Error reading from request FIFO for client %s\n", client_info->client_id);
            break;
        }
        
        int result = process_client_message(buffer, client_info);
        if (result != 0) {
            if (result == 2) {
                fprintf(stdout, "Client %s requested disconnect. Ending session.\n", client_info->client_id);
                break;
            } else {
                fprintf(stderr, "Error processing client message for client %s. Reading next message.\n", client_info->client_id);
            }
        }
        
        if (atomic_load(&global_cleanup_flag)) {
            fprintf(stdout, "Global cleanup flag set for client %s.\n", client_info->client_id);
            break;
        }
    }

    pthread_mutex_lock(&client_info->lock);
    if (client_info->req_fd != -1) { close(client_info->req_fd); client_info->req_fd = -1; }
    if (client_info->resp_fd != -1) { close(client_info->resp_fd); client_info->resp_fd = -1; }
    if (client_info->notif_fd != -1) { close(client_info->notif_fd); client_info->notif_fd = -1; }
    pthread_mutex_unlock(&client_info->lock);
    pthread_mutex_destroy(&client_info->lock);
    free(client_info);
}

ClientInfo* handle_connect(char* client_init_message, int thread_id) {
    char req_fifo_path[MAX_PIPE_PATH_LENGTH] = {0};
    char resp_fifo_path[MAX_PIPE_PATH_LENGTH] = {0};
    char notif_fifo_path[MAX_PIPE_PATH_LENGTH] = {0};

    strncpy(req_fifo_path, &client_init_message[1], MAX_PIPE_PATH_LENGTH - 1);
    strncpy(resp_fifo_path, &client_init_message[MAX_PIPE_PATH_LENGTH + 1], MAX_PIPE_PATH_LENGTH - 1);
    strncpy(notif_fifo_path, &client_init_message[MAX_PIPE_PATH_LENGTH * 2 + 1], MAX_PIPE_PATH_LENGTH - 1);

    if (req_fifo_path[0] == '\0' || resp_fifo_path[0] == '\0' || notif_fifo_path[0] == '\0') {
        fprintf(stderr, "Invalid FIFO paths\n");
        return NULL;
    }

    // Extract the client ID
    const char *id_start = strstr(req_fifo_path, "/tmp/req");
    if (id_start == NULL) {
        fprintf(stderr, "Invalid request FIFO path: %s\n", req_fifo_path);
        return NULL;
    }

    id_start += strlen("/tmp/req"); // Move past the "/tmp/req" prefix
    char client_id[MAX_STRING_SIZE] = {0};
    strncpy(client_id, id_start, MAX_STRING_SIZE - 1); // Copy the ID

    // Open FIFOs
    int resp_fd = open(resp_fifo_path, O_WRONLY);
    if (resp_fd < 0) {
        fprintf(stderr, "Failed to open response pipe: %s\n", resp_fifo_path);
        return NULL;
    }

    char response_message[2] = { '1', '0' };
    if (write(resp_fd, response_message, 2) < 0) {
        fprintf(stderr, "Server failed to write response to client\n");
        close(resp_fd);
        return NULL;
    }

    int req_fd = open(req_fifo_path, O_RDONLY);
    if (req_fd < 0) {
        fprintf(stderr, "Failed to open request pipe (server side)\n");
        close(resp_fd);
        return NULL;
    }

    int notif_fd = open(notif_fifo_path, O_WRONLY);
    if (notif_fd < 0) {
        fprintf(stderr, "Failed to open notification pipe (server side)\n");
        close(resp_fd);
        close(req_fd);
        return NULL;
    }
    ClientInfo* client_info = create_client_info(req_fifo_path + 8, req_fd, resp_fd, notif_fd, thread_id);
    if (!client_info) {
        fprintf(stderr, "Failed to initialize client info\n");
        return NULL;
    }
    fprintf(stdout, "Client with ID '%s' session started.\n", client_info->client_id);
    return client_info;
}

int handle_disconnect(ClientInfo* client_info) {
    char response[2] = { '2', '0' };

    if (remove_subscriptions_for_fd(client_info->notif_fd) != 0) {
        fprintf(stderr, "Failed to remove subscriptions for client\n");
        response[1] = '1'; 
    }
    if (write(client_info->resp_fd, response, sizeof(response)) != sizeof(response)) {
        fprintf(stderr, "Error writing disconnect confirmation to client\n");
        return 1;
    }
    return 2; // Signal session termination
}


int handle_subscribe(char* client_message, ClientInfo* client_info) {
    char buffer[MAX_STRING_SIZE + 1] = {0};
    char response[2];
    response[0] = '3';

    strncpy(buffer, client_message + 1, MAX_STRING_SIZE);
    buffer[MAX_STRING_SIZE] = '\0';
    if (!kvs_key_exists(buffer)) { 
        response[1] = '1';
    } else {
        if (add_subscription(buffer, client_info->notif_fd) != 0) {
            fprintf(stderr, "Failed to add subscription for key: %s\n", buffer);
            response[1] = '1';
        } else {
            response[1] = '0';
        }
    }

    ssize_t bytes_written = write(client_info->resp_fd, response, sizeof(response));
    if (bytes_written <= 0) {
        fprintf(stderr, "Failed to respond to client subscription\n");
        return 1;
    }

    return 0;
}

int handle_unsubscribe(char* client_message,ClientInfo* client_info) {
    char buffer[MAX_STRING_SIZE + 1] = {0};
    char response[2];
    response[0] = '4';

    strncpy(buffer, client_message + 1, MAX_STRING_SIZE);
    buffer[MAX_STRING_SIZE] = '\0';

    if (!kvs_key_exists(buffer)) {
        fprintf(stderr, "Key '%s' does not exist in the KVS table\n", buffer);
        response[1] = '1';
    } else {
        if (remove_subscription(buffer, client_info->notif_fd) == 0) {
            response[1] = '0';
        } else {
            response[1] = '1';
        }
    }

    ssize_t bytes_written = write(client_info->resp_fd, response, sizeof(response));
    if (bytes_written < 0) {
        fprintf(stderr, "Failed to send unsubscribe response for key '%s'\n", buffer);
        return 1;
    }

    return 0;
}


int main(int argc, char **argv) {
  signal(SIGPIPE, SIG_IGN);
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sigusr1_handler;
  sigemptyset(&sa.sa_mask);  // Don't block additional signals during handler
  sa.sa_flags = 0;           // No special flags; adjust SA_RESTART if desired

  if (sigaction(SIGUSR1, &sa, NULL) == -1) {
      perror("sigaction");
      exit(EXIT_FAILURE);
  }
  if (argc < 5) {
    write_str(STDERR_FILENO, "Usage: ");
    write_str(STDERR_FILENO, argv[0]);
    write_str(STDERR_FILENO, " <jobs_dir>");
    write_str(STDERR_FILENO, " <max_threads>");
    write_str(STDERR_FILENO, " <max_backups> \n");
    write_str(STDERR_FILENO, " <register_fifo_pathname> \n");
    return 1;
  }

  jobs_directory = argv[1];

  char *endptr;
  max_backups = strtoul(argv[3], &endptr, 10);

  if (*endptr != '\0') {
    fprintf(stderr, "Invalid max_proc value\n");
    return 1;
  }

  max_threads = strtoul(argv[2], &endptr, 10);

  if (*endptr != '\0') {
    fprintf(stderr, "Invalid max_threads value\n");
    return 1;
  }

  if (max_backups <= 0) {
    write_str(STDERR_FILENO, "Invalid number of backups\n");
    return 0;
  }

  if (max_threads <= 0) {
    write_str(STDERR_FILENO, "Invalid number of threads\n");
    return 0;
  }

  if (kvs_init()) {
    write_str(STDERR_FILENO, "Failed to initialize KVS\n");
    return 1;
  }

  DIR *dir = opendir(argv[1]);
  if (dir == NULL) {
    fprintf(stderr, "Failed to open directory: %s\n", argv[1]);
    return 0;
  }

  
  struct SharedData thread_data;
  thread_data.dir = dir;
  thread_data.dir_name = jobs_directory;
  pthread_mutex_init(&thread_data.directory_mutex, NULL);  
  dispatch_threads(&thread_data);


  // Session manager
  sem_init(&session_buffer.empty_slots, 0, MAX_SESSION_COUNT);
  sem_init(&session_buffer.filled_slots, 0, 0);

  for (int i = 0; i < MAX_SESSION_COUNT; i++) {
      int* a = malloc(sizeof(int));
      *a = i;
      if (pthread_create(&session_threads[i], NULL, session_manager_task, a) != 0) {
          fprintf(stderr, "Failed to create session manager thread %d\n", i);
          exit(EXIT_FAILURE);
      }
  }

  // Getting the register_fifo pathname to create it.
  const char *fifo_path = argv[4];

  unlink(fifo_path);
  if (mkfifo(fifo_path, 0777) < 0){
    fprintf(stderr, "Failed to create server register FIFO\n");
    exit(EXIT_FAILURE);
  }

  fprintf(stdout, "Server is listening on %s\n", fifo_path);

  while (1) {
    if (atomic_load(&global_cleanup_flag)) {
        fprintf(stderr, "Global cleanup flag detected before read.\n");
        cleanup_all_client_fifos();
        atomic_store(&global_cleanup_flag, 0);
    }

        int fserv;
          fserv = open(fifo_path, O_RDONLY);
          if (fserv < 0 && errno == EINTR) {
              // Log and retry on EINTR
              fprintf(stderr, "open() interrupted by signal, retrying...\n");
              continue;
          } else if (fserv < 0) {
              // Handle other errors
              fprintf(stderr, "Could not open server register FIFO: %s\n", strerror(errno));
              exit(EXIT_FAILURE);
          }
    SessionRequest session_request;
    ssize_t bytes_read;

    do {
        bytes_read = read(fserv, 
                          session_request.client_init_message, 
                          sizeof(session_request.client_init_message));

        if (bytes_read < 0 && errno == EINTR && atomic_load(&global_cleanup_flag)) {
            break;
        }
    } while (bytes_read < 0 && errno == EINTR);

    close(fserv);

    if ((bytes_read < 0 && errno == EINTR && atomic_load(&global_cleanup_flag))) {
        fprintf(stderr, "Read interrupted by signal; handling cleanup after read.\n");
        cleanup_all_client_fifos();
        atomic_store(&global_cleanup_flag, 0);
        continue;
    }

    if (bytes_read == 0) {
        fprintf(stderr, "Client opened then closed FIFO, no data\n");
        continue;
    } else if (bytes_read < 0) {
        fprintf(stderr, "Error reading from register FIFO\n");
        continue;
    }
    sem_wait(&session_buffer.empty_slots);
    pthread_mutex_lock(&session_buffer.mutex);

    session_buffer.buffer[session_buffer.tail] = session_request;
    session_buffer.tail = (session_buffer.tail + 1) % MAX_SESSION_COUNT;

    pthread_mutex_unlock(&session_buffer.mutex);
    sem_post(&session_buffer.filled_slots);
}

  sem_destroy(&session_buffer.empty_slots);
  sem_destroy(&session_buffer.filled_slots);

  for (int i = 0; i < MAX_SESSION_COUNT; i++) {
      pthread_join(session_threads[i], NULL);
  }
  
  shutdown_threads();
  if (closedir(dir) == -1) {
    fprintf(stderr, "Failed to close directory\n");
    return 0;
  }
  if (pthread_mutex_destroy(&thread_data.directory_mutex) != 0) {
        fprintf(stderr, "Failed to destroy directory_mutex\n");
    }
  

  while (active_backups > 0) {
    wait(NULL);
    active_backups--;
  }

  kvs_terminate();

  return 0;
}



void* session_manager_task(void* id) {
    block_sigusr1();
    int* thread_id_ptr = (int*)id;
    int thread_id = *thread_id_ptr;
    free(thread_id_ptr);
    while (1) {
        // Wait for a filled slot in the buffer
        sem_wait(&session_buffer.filled_slots);

        // Lock the buffer to safely extract the session request
        pthread_mutex_lock(&session_buffer.mutex);

        // Extract session request from the buffer
        SessionRequest session_request = session_buffer.buffer[session_buffer.head];
        session_buffer.head = (session_buffer.head + 1) % MAX_SESSION_COUNT;

        // Unlock the buffer
        pthread_mutex_unlock(&session_buffer.mutex);

        // Signal an empty slot is now available
        sem_post(&session_buffer.empty_slots);

        // Process the session request (start a session)
        char* client_init_message = session_request.client_init_message;

        ClientInfo* client_info = handle_connect(client_init_message, thread_id);
        if (client_info == NULL) {
            fprintf(stderr, "Failed to handle connect for thread %d.\n", thread_id);
            continue;
        }
        pthread_mutex_lock(&client_info_array_mutex);
        client_info_array[thread_id] = client_info;
        pthread_mutex_unlock(&client_info_array_mutex);

        handle_client_session(client_info);

        pthread_mutex_lock(&client_info_array_mutex);
        client_info_array[thread_id] = NULL;  
        pthread_mutex_unlock(&client_info_array_mutex);
    } 
    return NULL;
}
