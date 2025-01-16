#define _XOPEN_SOURCE 700
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>  
#include "common/constants.h"
#include "common/io.h"
#include "operations.h"

typedef  struct{
  int session_id;
  char request_pipe_path[CLIENT_PATH_LENGTH + PIPE_NAME_SIZE-1];
  char response_pipe_path[CLIENT_PATH_LENGTH + PIPE_NAME_SIZE-1];
} Session_info;

//global vector of sessions
Session_info sessions_vector[MAX_SESSION_COUNT];
int num_sessions = 0;
int id_prod = 0;
int id_cons = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;

//signal flag
int signal_flag = 0;  

//signal handler function
void signal_handler(){
  signal_flag = 1;
}

void *processComands(){
  int freq, fres;
  size_t result;
  //signals BLOCK
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0) {
    perror("pthread_sigmask");
    result = 1;
    pthread_exit((void *) result);
  }
  while(1){
    pthread_mutex_lock(&mutex);
    while (num_sessions == 0){
      pthread_cond_wait(&full, &mutex);
    }
    Session_info info;
    info.session_id = sessions_vector[id_cons].session_id;
    strcpy(info.request_pipe_path, sessions_vector[id_cons].request_pipe_path);
    strcpy(info.response_pipe_path, sessions_vector[id_cons].response_pipe_path);
    id_cons++;
    if (id_cons == MAX_SESSION_COUNT){
      id_cons = 0;
    }
    num_sessions--;
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);

    if((freq = open(info.request_pipe_path, O_RDONLY)) == -1){
      fprintf(stderr, "Could not open request fifo\n");
      result = 1;
      pthread_exit((void *) result);
    }
    if ((fres = open(info.response_pipe_path, O_WRONLY)) == -1){
      fprintf(stderr, "Could not open response fifo\n");
      result = 1;
      pthread_exit((void *) result);
    }
    if (write(fres, &num_sessions, sizeof(int)) == -1){
      fprintf(stderr, "Could not write to server fifo\n");
      result = 1;
      pthread_exit((void *) result);
    }

    ssize_t bytes_lidos;
    char op_code_char;
    int op_code_int;
    int flag = 0;

    //common variables
    int resp_create;
    int session_id;
    unsigned int event_id;

    while(1){
      bytes_lidos = read(freq, &op_code_char, sizeof(char));
      op_code_int = op_code_char - '0';
      if (bytes_lidos == 0){
        close(freq);
        close(fres);
        break;
      }
      else if(bytes_lidos == -1){
        fprintf(stderr, "Failed to read bytes\n");
        result = 1;
        pthread_exit((void *) result);
      }
      else{
        switch (op_code_int){
          case 2:
            printf("QUIT\n");
            unlink(info.request_pipe_path);
            unlink(info.response_pipe_path);
            flag = 1;
            break;
          case 3:
            printf("CREATE\n");
            size_t num_rows_create, num_cols_create;
            if (read(freq, &session_id, sizeof(int)) == -1){
              fprintf(stderr, "Could read from request fifo\n");
              result = 1;
              pthread_exit((void *) result);
            }
            if (read(freq, &event_id, sizeof(unsigned int)) == -1){
              fprintf(stderr, "Could read from request fifo\n");
              result = 1;
              pthread_exit((void *) result);
            }
            if (read(freq, &num_rows_create, sizeof(ssize_t)) == -1){
              fprintf(stderr, "Could read from request fifo\n");
              result = 1;
              pthread_exit((void *) result);
            }
            if (read(freq, &num_cols_create, sizeof(ssize_t)) == -1){
              fprintf(stderr, "Could read from request fifo\n");
              result = 1;
              pthread_exit((void *) result);
            }
            resp_create = ems_create(event_id, num_rows_create, num_cols_create);
            
            if (write(fres, &resp_create, sizeof(int)) == -1){
                fprintf(stderr, "Could not write to response fifo\n");
              result = 1;
              pthread_exit((void *) result);
            }
            break;
          case 4:
            printf("RESERVE\n");
            size_t num_seats;
            size_t* xs = (size_t*)malloc(MAX_RESERVATION_SIZE * sizeof(size_t));
            size_t* ys = (size_t*)malloc(MAX_RESERVATION_SIZE * sizeof(size_t));

            if (xs == NULL || ys == NULL) {
              fprintf(stderr, "Memory allocation failed for xs or ys\n");
              result = 1;
              pthread_exit((void *) result);
            }

            if (read(freq, &session_id, sizeof(int)) == -1){
              fprintf(stderr, "Could read from request fifo\n");
              result = 1;
              pthread_exit((void *) result);
            }
            if (read(freq, &event_id, sizeof(unsigned int)) == -1){
              fprintf(stderr, "Could read from request fifo\n");
              result = 1;
              pthread_exit((void *) result);
            }
            if (read(freq, &num_seats, sizeof(size_t)) == -1){
              fprintf(stderr, "Could read from request fifo\n");
              result = 1;
              pthread_exit((void *) result);
            }
            if (read(freq, xs, sizeof(size_t)*MAX_RESERVATION_SIZE) == -1){
              fprintf(stderr, "Could read from request fifo\n");
              result = 1;
              pthread_exit((void *) result);
            }
            if (read(freq, ys, sizeof(size_t)*MAX_RESERVATION_SIZE) == -1){
              fprintf(stderr, "Could read from request fifo\n");
              result = 1;
              pthread_exit((void *) result);
            }
            
            resp_create = ems_reserve(event_id, num_seats, xs, ys);

            if (write(fres, &resp_create, sizeof(int)) == -1){
              fprintf(stderr, "Could not write to response fifo\n");
              free(xs);
              free(ys);
              result = 1;
              pthread_exit((void *) result);
            }
            free(xs);
            free(ys);
            break;
          case 5:
            printf("SHOW\n");
            unsigned int* seats = NULL;
            size_t num_rows_show, num_cols_show;
            if (read(freq, &session_id, sizeof(int)) == -1){
              fprintf(stderr, "Could read from request fifo\n");
              result = 1;
              pthread_exit((void *) result);
            }
            if (read(freq, &event_id, sizeof(unsigned int)) == -1){
              fprintf(stderr, "Could read from request fifo\n");
              result = 1;
              pthread_exit((void *) result);
            }

            resp_create = ems_show(event_id, &num_rows_show, &num_cols_show, &seats);
            if (write(fres, &resp_create, sizeof(int)) == -1){
              fprintf(stderr, "Could not write to response fifo\n");
              free(seats);
              result = 1;
              pthread_exit((void *) result);
            }
            if (write(fres, &num_rows_show, sizeof(size_t)) == -1){
              fprintf(stderr, "Could not write to response fifo\n");
              free(seats);
              result = 1;
              pthread_exit((void *) result);
            }
            if (write(fres, &num_cols_show, sizeof(size_t)) == -1){
              fprintf(stderr, "Could not write to response fifo\n");
              free(seats);
              result = 1;
              pthread_exit((void *) result);
            }
            if (write(fres, seats, sizeof(unsigned int) * (num_rows_show*num_cols_show)) == -1){
              fprintf(stderr, "Could not write to response fifo\n");
              free(seats);
              result = 1;
              pthread_exit((void *) result);
            }
            free(seats);
          break;
          case 6:
            printf("LIST\n");
            unsigned int* ids = NULL;
            size_t num_events = 0;
            if (read(freq, &session_id, sizeof(int)) == -1){
              fprintf(stderr, "Could read from request fifo\n");
              result = 1;
              pthread_exit((void *) result);
            }
            resp_create = ems_list_events(&num_events, &ids);
            if (write(fres, &resp_create, sizeof(int)) == -1){
              fprintf(stderr, "Could not write to response fifo\n");
              free(ids);
              result = 1;
              pthread_exit((void *) result);
            }
            if (write(fres, &num_events, sizeof(size_t)) == -1){
              fprintf(stderr, "Could not write to response fifo\n");
              free(ids);
              result = 1;
              pthread_exit((void *) result);
            }
            if (write(fres, ids, sizeof(unsigned int) * num_events) == -1){
              fprintf(stderr, "Could not write to response fifo\n");
              free(ids);
              result = 1;
              pthread_exit((void *) result);
            }
            free(ids);
            break;
        }
      }
      if (flag){
        break;
      }
    }
  }
  result = 0;
  pthread_exit((void *) result);
}

int main(int argc, char* argv[]) {
  int fserv;
  char buf_init[INIT_MSG_SIZE];
  
  if (signal(SIGUSR1, signal_handler) == SIG_ERR) {
    perror("signal");
    return 1;
  }

  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s\n <pipe_path> [delay]\n", argv[0]);
    return 1;
  }

  char* endptr;
  unsigned int state_access_delay_us = STATE_ACCESS_DELAY_US;
  if (argc == 3) {
    unsigned long int delay = strtoul(argv[2], &endptr, 10);

    if (*endptr != '\0' || delay > UINT_MAX) {
      fprintf(stderr, "Invalid delay value or value too large\n");
      return 1;
    }

    state_access_delay_us = (unsigned int)delay;
  }

  //create named pipe
  if (mkfifo(argv[1], 0777) == -1){
    if (errno != EEXIST){
      fprintf(stderr, "Error: Could not create fifo file\n");
      return 1;
    }
  }

  if (ems_init(state_access_delay_us)) {
    fprintf(stderr, "Failed to initialize EMS\n");
    return 1;
  }

  //open server named_pipe
  if((fserv = open(argv[1], O_RDWR)) < 0){
    fprintf(stderr, "Failed to open pipe\n");
    return 1;
  }

  //threads
  pthread_t thread_ids[MAX_SESSION_COUNT];
  for(int i=0; i < MAX_SESSION_COUNT; i++){
    if(pthread_create(&thread_ids[i], NULL, processComands, NULL) != 0){
      fprintf(stderr, "Failed to create a thread\n");
      return 1;
    } 
  }

  char request_pipe[CLIENT_PATH_LENGTH + PIPE_NAME_SIZE];
  char response_pipe[CLIENT_PATH_LENGTH + PIPE_NAME_SIZE];
  char client_path[CLIENT_PATH_LENGTH] = "../client/";
  

  //session requests by clients
  while (1) {
    if (signal_flag){
      printf("HELO\n");
      int resp_create;
      unsigned int* ids = NULL;
      size_t num_events;
      unsigned int* seats = NULL;
      size_t num_rows_show, num_cols_show;
      resp_create = ems_list_events(&num_events, &ids);
      if (resp_create == 1){
        fprintf(stderr, "Failed doing event list\n");
        return 1;
      }
      else{
        for (size_t i = 0; i < num_events; i++){
          printf("Evento %d\n", ids[i]);
          resp_create = ems_show(ids[i], &num_rows_show, &num_cols_show, &seats);
          if (resp_create == 1){
            fprintf(stderr, "Failed doing event list\n");
            return 1;
          }
          else{
            for (size_t j = 0; j < num_rows_show; j++){
              for (size_t k = 0; k < num_rows_show; k++){
                char buffer[16];
                sprintf(buffer, "%u", seats[(j - 1) * num_cols_show + k - 1]);

                if (print_str(STDOUT_FILENO, buffer)) {
                  perror("Error writing to file descriptor");
                  free(seats);
                  free(ids);
                  return 1;
                }

                if (j < num_cols_show) {
                  if (print_str(STDOUT_FILENO, " ")) {
                    perror("Error writing to file descriptor");
                    free(seats);
                    free(ids);
                    return 1;
                  }
                }
              }
              if (print_str(STDOUT_FILENO, "\n")) {
                perror("Error writing to file descriptor");
                free(seats);
                free(ids);
                return 1;
              }
              free(seats);
            }
            free(ids);
          }
        }
      }
      signal_flag = 0;
    }
    ssize_t bytes_lidos = read(fserv, buf_init, INIT_MSG_SIZE);
    if (bytes_lidos > 0){
      pthread_mutex_lock(&mutex);
      while(num_sessions == MAX_SESSION_COUNT){
        pthread_cond_wait(&empty, &mutex);
      }
      Session_info new_session;
      strncpy(request_pipe, client_path, CLIENT_PATH_LENGTH);
      strncpy(request_pipe + CLIENT_PATH_LENGTH-1, buf_init + sizeof(char), PIPE_NAME_SIZE);
      strncpy(response_pipe, client_path, CLIENT_PATH_LENGTH);
      strncpy(response_pipe + CLIENT_PATH_LENGTH-1, buf_init + sizeof(char) + PIPE_NAME_SIZE, PIPE_NAME_SIZE);
      new_session.session_id = num_sessions;
      strcpy(new_session.request_pipe_path, request_pipe);
      strcpy(new_session.response_pipe_path, response_pipe);
      sessions_vector[id_prod] = new_session;
      id_prod++;
      if (id_prod == MAX_SESSION_COUNT){
        id_prod = 0;
      }
      num_sessions++;
      pthread_cond_signal(&full);
      pthread_mutex_unlock(&mutex);
    }
    else if(bytes_lidos == 0){  
      break;
    }
    else{
      if (errno == EINTR) {
        continue; 
      }
      fprintf(stderr, "Could not read from server fifo\n");
      return 1;
    }
  }

  //conclusão main
  void* ret_val = NULL;
  for(int i=0; i < MAX_SESSION_COUNT; i++){
    if(pthread_join(thread_ids[i], &ret_val) != 0){
      fprintf(stderr, "Failed to join thread\n");
      return 1;
    }
  }

  close(fserv);
  unlink(argv[1]);
  ems_terminate();
  return 0;
}




