#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>


#include "api.h"
#include "../common/constants.h"
#include "../common/io.h"

int response_id, request_id;


int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path, int* session_id) {  
  
  //creating request pipe
  if (mkfifo(req_pipe_path, 0777) == -1){
    if (errno != EEXIST){
      fprintf(stderr, "Could not create request fifo\n");
      return 1;
    }
  }

  //creating response pipe
  if (mkfifo(resp_pipe_path, 0777) == -1){
    if (errno != EEXIST){
      fprintf(stderr, "Could not create response fifo\n");
      return 1;
    }
  }

  //sending session message
  int server_id = open(server_pipe_path, O_WRONLY);
  if (server_id == -1){
    fprintf(stderr, "Could not open server fifo\n");
    return 1;
  }

  //envio
  char op_code = '1';
  char init_msg[PIPE_NAME_SIZE * 2 + sizeof(char)];

  memset(init_msg, '\0', sizeof(init_msg));
  init_msg[0] = op_code;

  strncpy(init_msg + sizeof(char), req_pipe_path, PIPE_NAME_SIZE);

  strncpy(init_msg + sizeof(char) + PIPE_NAME_SIZE, resp_pipe_path, PIPE_NAME_SIZE);


  if (write(server_id, init_msg, sizeof(init_msg)) == -1){
    fprintf(stderr, "Could not write to server fifo\n");
    return 1;
  }

  //abrir pipe de resposta
  response_id = open(resp_pipe_path, O_RDONLY);
  if (response_id == -1){
    fprintf(stderr, "Could not open response fifo\n");
    return 1;
  }

  //resposta
  if (read(response_id, session_id, sizeof(int)) == -1){
    fprintf(stderr, "Could read from response fifo\n");
    return 1;
  }

  //abrir pipe de request
  request_id = open(req_pipe_path, O_WRONLY);
  if (request_id == -1){
    fprintf(stderr, "Could not open request fifo\n");
    return 1;
  }
  return 0;
}

int ems_quit(int session_id) {
  printf("QUIT\n");
  char op_code = '2';
  
  if(write(request_id, &op_code, sizeof(char)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }
  if(write(request_id, &session_id, sizeof(int)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  close(request_id);
  close(response_id);
  return 1;
}


int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols, int session_id) {
  printf("CREATE\n");
  char op_code = '3';

  if(write(request_id, &op_code, sizeof(char)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  if(write(request_id, &session_id, sizeof(int)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  if(write(request_id, &event_id, sizeof(unsigned int)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  if(write(request_id, &num_rows, sizeof(size_t)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  if(write(request_id, &num_cols, sizeof(size_t)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  //resposta
  ssize_t bytes_lidos;
  int status;
  if((bytes_lidos = read(response_id, &status, sizeof(int))) == -1){
    fprintf(stderr, "Could not read from server fifo\n");
    return 1;
  }
  
  if (status == 1){
    return 1;
  }
  return 0;
}

int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys, int session_id) {
  printf("RESERVE\n");
  char op_code = '4';    
  if(write(request_id, &op_code, sizeof(char)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  if(write(request_id, &session_id, sizeof(int)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  if(write(request_id, &event_id, sizeof(unsigned int)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  if(write(request_id, &num_seats, sizeof(size_t)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  if(write(request_id, xs, sizeof(size_t) * MAX_RESERVATION_SIZE) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  if(write(request_id, ys, sizeof(size_t) * MAX_RESERVATION_SIZE) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }
  //resposta
  ssize_t bytes_lidos;
  int status;
  if((bytes_lidos = read(response_id, &status, sizeof(int))) == -1){
    fprintf(stderr, "Could not read from server fifo\n");
    return 1;
  }
  if (status == 1){
    return 1;
  }
  return 0;
}

int ems_show(int out_fd, unsigned int event_id, int session_id) {
  printf("SHOW\n");
  char op_code = '5'; 
  if(write(request_id, &op_code, sizeof(char)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  if(write(request_id, &session_id, sizeof(int)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  if(write(request_id, &event_id, sizeof(unsigned int)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  //resposta
  ssize_t bytes_lidos;
  size_t num_rows, num_cols;
  int status;
  if((bytes_lidos = read(response_id, &status, sizeof(int))) == -1){
    fprintf(stderr, "Could not read from server fifo\n");
    return 1;
  }
  if (status == 1){
    return 1;
  }
  if((bytes_lidos = read(response_id, &num_rows, sizeof(size_t))) == -1){
    fprintf(stderr, "Could not read from server fifo\n");
    return 1;
  }
  if((bytes_lidos = read(response_id, &num_cols, sizeof(size_t))) == -1){
    fprintf(stderr, "Could not read from server fifo\n");
    return 1;
  }

  unsigned int* seats = (unsigned int*)malloc(sizeof(unsigned int)*(size_t)(num_cols*num_rows));

  if((bytes_lidos = read(response_id, seats, sizeof(unsigned int) * (size_t)(num_cols*num_rows))) == -1){
    fprintf(stderr, "Could not read from server fifo\n");
    free(seats);
    return 1;
  }

  for(size_t i = 1; i <= num_rows; i++){
    for(size_t j = 1; j <= num_cols; j++){
      char buffer[16];
      sprintf(buffer, "%u", seats[(i - 1) * num_cols + j - 1]);

      if (print_str(out_fd, buffer)) {
        perror("Error writing to file descriptor");
        free(seats);
        return 1;
      }

      if (j < num_cols) {
        if (print_str(out_fd, " ")) {
          perror("Error writing to file descriptor");
          free(seats);
          return 1;
        }
      }
    }
    if (print_str(out_fd, "\n")) {
      perror("Error writing to file descriptor");
      free(seats);
      return 1;
    }
  }
  free(seats);
  return 0;
}

int ems_list_events(int out_fd, int session_id) {
  printf("LIST\n");
  char op_code = '6';
  if(write(request_id, &op_code, sizeof(char)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  if(write(request_id, &session_id, sizeof(int)) == -1){
    fprintf(stderr, "Could not write to request fifo\n");
    return 1;
  }

  //resposta
  ssize_t bytes_lidos;
  size_t num_events;
  int status;
  if((bytes_lidos = read(response_id, &status, sizeof(int))) == -1){
    fprintf(stderr, "Could not read from server fifo\n");
    return 1;
  }
  if (status == 1){
    return 1;
  }
  if((bytes_lidos = read(response_id, &num_events, sizeof(size_t))) == -1){
    fprintf(stderr, "Could not read from server fifo\n");
    return 1;
  }

  unsigned int* ids = (unsigned int*)malloc(sizeof(unsigned int)*num_events);

  if((bytes_lidos = read(response_id, ids, sizeof(unsigned int)*num_events)) == -1){
    fprintf(stderr, "Could not read from server fifo\n");
    free(ids);
    return 1;
  }
  
  if(num_events == 0){
    char buff[] = "No events\n";
    if (print_str(out_fd, buff)) {
      perror("Error writing to file descriptor");
      free(ids);
      return 1;
    }
  }
  else{
    for(size_t i = 0; i < num_events; i++){
      char buff[] = "Event: ";
      if (print_str(out_fd, buff)) {
        perror("Error writing to file descriptor");
        free(ids);
        return 1;
      }
      char id[16];
      sprintf(id, "%u\n", ids[i]);
      if (print_str(out_fd, id)) {
        perror("Error writing to file descriptor");
        free(ids);
        return 1;
      }
    }
  }
  free(ids);
  return 0;
}
