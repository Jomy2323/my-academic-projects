#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "common/io.h"
#include "eventlist.h"

static struct EventList* event_list = NULL;
static unsigned int state_access_delay_us = 0;

/// Gets the event with the given ID from the state.
/// @note Will wait to simulate a real system accessing a costly memory resource.
/// @param event_id The ID of the event to get.
/// @param from First node to be searched.
/// @param to Last node to be searched.
/// @return Pointer to the event if found, NULL otherwise.
static struct Event* get_event_with_delay(unsigned int event_id, struct ListNode* from, struct ListNode* to) {
  struct timespec delay = {0, state_access_delay_us * 1000};
  nanosleep(&delay, NULL);  // Should not be removed

  return get_event(event_list, event_id, from, to);
}

/// Gets the index of a seat.
/// @note This function assumes that the seat exists.
/// @param event Event to get the seat index from.
/// @param row Row of the seat.
/// @param col Column of the seat.
/// @return Index of the seat.
static size_t seat_index(struct Event* event, size_t row, size_t col) { return (row - 1) * event->cols + col - 1; }

int ems_init(unsigned int delay_us) {
  if (event_list != NULL) {
    fprintf(stderr, "EMS state has already been initialized\n");
    return 1;
  }

  event_list = create_list();
  state_access_delay_us = delay_us;

  return event_list == NULL;
}

int ems_terminate() {
  if (event_list == NULL) {
    fprintf(stderr, "EMS state must be initialized\n");
    return 1;
  }

  if (pthread_rwlock_wrlock(&event_list->rwl) != 0) {
    fprintf(stderr, "Error locking list rwl\n");
    return 1;
  }

  pthread_rwlock_unlock(&event_list->rwl);
  free_list(event_list);

  return 0;
}

int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols) {
  if (event_list == NULL) {
    fprintf(stderr, "EMS state must be initialized\n");
    return 1;
  }

  if (pthread_rwlock_wrlock(&event_list->rwl) != 0) {
    fprintf(stderr, "Error locking list rwl\n");
    return 1;
  }

  if (get_event_with_delay(event_id, event_list->head, event_list->tail) != NULL) {
    fprintf(stderr, "Event already exists\n");
    pthread_rwlock_unlock(&event_list->rwl);
    return 1;
  }

  struct Event* event = malloc(sizeof(struct Event));

  if (event == NULL) {
    fprintf(stderr, "Error allocating memory for event\n");
    pthread_rwlock_unlock(&event_list->rwl);
    return 1;
  }

  event->id = event_id;
  event->rows = num_rows;
  event->cols = num_cols;
  event->reservations = 0;
  if (pthread_mutex_init(&event->mutex, NULL) != 0) {
    pthread_rwlock_unlock(&event_list->rwl);
    free(event);
    return 1;
  }
  event->data = calloc(num_rows * num_cols, sizeof(unsigned int));

  if (event->data == NULL) {
    fprintf(stderr, "Error allocating memory for event data\n");
    pthread_rwlock_unlock(&event_list->rwl);
    free(event);
    return 1;
  }

  if (append_to_list(event_list, event) != 0) {
    fprintf(stderr, "Error appending event to list\n");
    pthread_rwlock_unlock(&event_list->rwl);
    free(event->data);
    free(event);
    return 1;
  }
  pthread_rwlock_unlock(&event_list->rwl);
  return 0;
}

int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  // Check if the event_list is initialized
  if (event_list == NULL) {
    fprintf(stderr, "EMS state must be initialized\n");
    return 1;
  }

  // Lock the event_list to prevent concurrent access
  if (pthread_rwlock_rdlock(&event_list->rwl) != 0) {
    fprintf(stderr, "Error locking list rwl\n");
    return 1;
  }

  // Find the event corresponding to the provided event_id
  struct Event* event = get_event_with_delay(event_id, event_list->head, event_list->tail);

  // Unlock the event_list as it's no longer needed for read
  pthread_rwlock_unlock(&event_list->rwl);

  // Check if the event was found
  if (event == NULL) {
    fprintf(stderr, "Event not found\n");
    return 1;
  }

  // Lock the mutex of the event to prevent concurrent modifications
  if (pthread_mutex_lock(&event->mutex) != 0) {
    fprintf(stderr, "Error locking mutex\n");
    return 1;
  }

  // Validate seat coordinates for each seat requested
  for (size_t i = 0; i < num_seats; i++) {
    if (xs[i] <= 0 || xs[i] > event->rows || ys[i] <= 0 || ys[i] > event->cols) {
      fprintf(stderr, "Seat out of bounds\n");
      pthread_mutex_unlock(&event->mutex);
      return 1;
    }
  }

  // Check if the requested seats are already reserved
  for (size_t i = 0; i < event->rows * event->cols; i++) {
    for (size_t j = 0; j < num_seats; j++) {
      if (seat_index(event, xs[j], ys[j]) == i) {
        if (event->data[i] != 0) {
          fprintf(stderr, "Seat already reserved\n");
          pthread_mutex_unlock(&event->mutex);
          return 1;
        }
        break;
      }
    }
  }

  // Increment reservation_id and mark the requested seats as reserved
  unsigned int reservation_id = ++event->reservations;
  for (size_t i = 0; i < num_seats; i++) {
    event->data[seat_index(event, xs[i], ys[i])] = reservation_id;
  }

  // Unlock the event mutex and return success
  pthread_mutex_unlock(&event->mutex);
  return 0;
}

int ems_show(unsigned int event_id, size_t *num_rows_show, size_t *num_cols_show, unsigned int **seats) {
  if (event_list == NULL) {
    fprintf(stderr, "EMS state must be initialized\n");
    return 1;
  }

  if (pthread_rwlock_rdlock(&event_list->rwl) != 0) {
    fprintf(stderr, "Error locking list rwl\n");
    return 1;
  }

  struct Event *event = get_event_with_delay(event_id, event_list->head, event_list->tail);

  pthread_rwlock_unlock(&event_list->rwl);

  if (event == NULL) {
    fprintf(stderr, "Event not found\n");
    return 1;
  }

  if (pthread_mutex_lock(&event->mutex) != 0) {
    fprintf(stderr, "Error locking mutex\n");
    return 1;
  }
  
  *num_rows_show = event->rows;
  *num_cols_show = event->cols;

  *seats = (unsigned int *)calloc((*num_rows_show) * (*num_cols_show), sizeof(unsigned int));
  if (*seats == NULL) {
    fprintf(stderr, "Memory allocation failed!\n");
    return 1;
  }
  
  // Populate seats with event data
  for (size_t i = 0; i < *num_rows_show; i++) {
    for (size_t j = 0; j < *num_cols_show; j++) {
      size_t index = i * (*num_cols_show) + j;
      (*seats)[index] = event->data[index];
    }
  }
  pthread_mutex_unlock(&event->mutex);
  return 0;
}

int ems_list_events(size_t* num_events, unsigned int** ids) {
    *ids = (unsigned int *)calloc(1, sizeof(unsigned int));
    if (*ids == NULL) {
        perror("Error allocating memory for the array");
        exit(EXIT_FAILURE);
    } 

    if (event_list == NULL) {
        fprintf(stderr, "EMS state must be initialized\n");
        return 1;
    }

    if (pthread_rwlock_rdlock(&event_list->rwl) != 0) {
        fprintf(stderr, "Error locking list rwl\n");
        return 1;
    }

    struct ListNode* to = event_list->tail;
    struct ListNode* current = event_list->head;

    while (current != NULL) {
        (*num_events)++;

        // Memory reallocation for ids and event ID assignment
        unsigned int* temp_ids = (unsigned int *)realloc(*ids, (*num_events) * sizeof(unsigned int));
        if (temp_ids == NULL) {
            perror("Error reallocating memory for the array");
            free(*ids);
            exit(EXIT_FAILURE);
        }
        *ids = temp_ids;

        size_t index = (*num_events) - 1;
        (*ids)[index] = (current->event)->id;

        // Break out of the loop when reaching the end
        if (current == to) {
            break;
        }

        current = current->next;
    }
    pthread_rwlock_unlock(&event_list->rwl);
    return 0;
}