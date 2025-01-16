#include "operations.h"

#include <limits.h>  // For PATH_MAX
#include <sys/stat.h>  // For stat, S_ISDIR
#include <fcntl.h>  // For open
#include <unistd.h>  // For fork, _exit, write
#include <pthread.h>  // For pthread_rwlock
#include <stdio.h>  // For snprintf
#include <string.h>  // For strrchr
#include <stdlib.h>  // For exit
#include <sys/wait.h>




#include "constants.h"
#include "io.h"
#include "kvs.h"
#include "subscriptions.h"
#include <linux/limits.h>

static struct HashTable *kvs_table = NULL;

/// Calculates a timespec from a delay in milliseconds.
/// @param delay_ms Delay in milliseconds.
/// @return Timespec with the given delay.
static struct timespec delay_to_timespec(unsigned int delay_ms) {
  return (struct timespec){delay_ms / 1000, (delay_ms % 1000) * 1000000};
}

int kvs_init() {
  if (kvs_table != NULL) {
    fprintf(stderr, "KVS state has already been initialized\n");
    return 1;
  }

  kvs_table = create_hash_table();
  return kvs_table == NULL;
}

int kvs_terminate() {
  if (kvs_table == NULL) {
    fprintf(stderr, "KVS state must be initialized\n");
    return 1;
  }

  free_table(kvs_table);
  kvs_table = NULL;
  return 0;
}

int kvs_write(size_t num_pairs, char keys[][MAX_STRING_SIZE],
              char values[][MAX_STRING_SIZE]) {
  if (kvs_table == NULL) {
    fprintf(stderr, "KVS state must be initialized\n");
    return 1;
  }

  pthread_rwlock_wrlock(&kvs_table->tablelock);

  for (size_t i = 0; i < num_pairs; i++) {
    if (write_pair(kvs_table, keys[i], values[i]) != 0) {
      fprintf(stderr, "Failed to write key pair (%s,%s)\n", keys[i], values[i]);
    } else {
      printf("[WRITE] Key: '%s', Value: '%s'\n", keys[i], values[i]);
      // Notify subscribers about the key-value update
      notify_subscribers(keys[i], values[i], 0); // `is_deleted = 0` indicates an update
    }
  }

  pthread_rwlock_unlock(&kvs_table->tablelock);
  return 0;
}



int kvs_read(size_t num_pairs, char keys[][MAX_STRING_SIZE], int fd) {
  if (kvs_table == NULL) {
    fprintf(stderr, "KVS state must be initialized\n");
    return 1;
  }

  pthread_rwlock_rdlock(&kvs_table->tablelock);

  write_str(fd, "[");
  for (size_t i = 0; i < num_pairs; i++) {
    char *result = read_pair(kvs_table, keys[i]);
    char aux[MAX_STRING_SIZE];
    if (result == NULL) {
      snprintf(aux, MAX_STRING_SIZE, "(%s,KVSERROR)", keys[i]);
    } else {
      snprintf(aux, MAX_STRING_SIZE, "(%s,%s)", keys[i], result);
    }
    write_str(fd, aux);
    free(result);
  }
  write_str(fd, "]\n");

  pthread_rwlock_unlock(&kvs_table->tablelock);
  return 0;
}

int kvs_delete(size_t num_pairs, char keys[][MAX_STRING_SIZE], int fd) {
  if (kvs_table == NULL) {
    fprintf(stderr, "KVS state must be initialized\n");
    return 1;
  }

  pthread_rwlock_wrlock(&kvs_table->tablelock);

  int aux = 0;
  for (size_t i = 0; i < num_pairs; i++) {
    if (delete_pair(kvs_table, keys[i]) != 0) {
      // Key not found, log missing key
      if (!aux) {
        write_str(fd, "[");
        aux = 1;
      }
      char str[MAX_STRING_SIZE];
      snprintf(str, MAX_STRING_SIZE, "(%s,KVSMISSING)", keys[i]);
      write_str(fd, str);
    } else {
      // Key found and deleted, notify subscribers
      printf("[DELETE] Key: '%s' successfully deleted\n", keys[i]);
      notify_subscribers(keys[i], NULL, 1); // `is_deleted = 1` indicates deletion
    }
  }
  
  if (aux) {
    write_str(fd, "]\n");
  }

  pthread_rwlock_unlock(&kvs_table->tablelock);
  return 0;
}



void kvs_show(int fd) {
  if (kvs_table == NULL) {
    fprintf(stderr, "KVS state must be initialized\n");
    return;
  }

  pthread_rwlock_rdlock(&kvs_table->tablelock);
  char aux[MAX_STRING_SIZE];

  for (int i = 0; i < TABLE_SIZE; i++) {
    KeyNode *keyNode = kvs_table->table[i]; // Get the next list head
    while (keyNode != NULL) {
      snprintf(aux, MAX_STRING_SIZE, "(%s, %s)\n", keyNode->key,
               keyNode->value);
      write_str(fd, aux);
      keyNode = keyNode->next; // Move to the next node of the list
    }
  }

  pthread_rwlock_unlock(&kvs_table->tablelock);
}

int kvs_backup(size_t num_backup, char *job_filename, char *directory) {
    pid_t pid;
    char bck_name[PATH_MAX];

    // Validate directory
    if (!directory || strlen(directory) == 0) {
        fprintf(stderr, "Invalid directory specified for backup\n");
        return -1;
    }

    // Extract base filename (without extension)
    const char *dot = strrchr(job_filename, '.');
    if (!dot || dot == job_filename) {
        fprintf(stderr, "Invalid job filename: %s\n", job_filename);
        return -1;
    }

    snprintf(bck_name, sizeof(bck_name), "%s/%.*s-%ld.bck", directory,
             (int)(dot - job_filename), job_filename, num_backup);


    // Ensure the path is not a directory
    struct stat sb;
    if (stat(bck_name, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        fprintf(stderr, "Backup path is a directory: %s\n", bck_name);
        return -1;
    }

    pthread_rwlock_rdlock(&kvs_table->tablelock);
    pid = fork();
    pthread_rwlock_unlock(&kvs_table->tablelock);

    if (pid == 0) { // Child process
        // Open backup file
        int fd = open(bck_name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd < 0) {
            perror("Failed to open backup file");
            _exit(1);
        }

        // Write KVS data
        for (int i = 0; i < TABLE_SIZE; i++) {
            KeyNode *keyNode = kvs_table->table[i];
            while (keyNode != NULL) {
            // Ensure aux can hold the formatted string
            char aux[2 * MAX_STRING_SIZE + 10]; // "(key, value)\n" format + safety margin
            
            int num_bytes_copied = snprintf(aux, sizeof(aux), "(%s, %s)\n",
                                            keyNode->key, keyNode->value);
            
            if (num_bytes_copied < 0) {
                // snprintf failed
                fprintf(stderr, "Error formatting string for key: %s\n", keyNode->key);
            } else if ((size_t)num_bytes_copied >= sizeof(aux)) {
                // Truncation occurred
                fprintf(stderr, "Formatted string for key: %s was truncated\n", keyNode->key);
            } else {
                // Write the formatted string to the file descriptor
                if (write(fd, aux, (size_t)num_bytes_copied) < 0) {
                    perror("Failed to write to file descriptor");
                }
        }

        keyNode = keyNode->next;
    }
}


        close(fd);
        _exit(0);
    } else if (pid < 0) { // Fork failed
        perror("Failed to fork for backup");
        return -1;
    }

    int status;
    waitpid(pid, &status, 0);

    return 0;
}



void kvs_wait(unsigned int delay_ms) {
  struct timespec delay = delay_to_timespec(delay_ms);
  nanosleep(&delay, NULL);
}

int kvs_key_exists(char* key){
  if (kvs_table == NULL){
    fprintf(stderr, "KVS table needs to be initialized\n");
    return 0;
  }
  if (key == NULL || strlen(key) == 0) {
    fprintf(stderr, "Invalid key provided\n");
    return 0;
  }

  int index = hash(key);
  pthread_rwlock_rdlock(&kvs_table->tablelock);
  KeyNode* current = kvs_table->table[index];
  while(current != NULL){
    if (strcmp(key, current->key) == 0){
      pthread_rwlock_unlock(&kvs_table->tablelock);
      return 1;
    }
    current = current->next;
  }
  pthread_rwlock_unlock(&kvs_table->tablelock);
  return 0;
}
