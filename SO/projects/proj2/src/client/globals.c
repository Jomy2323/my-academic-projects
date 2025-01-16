#include "globals.h"
#include <pthread.h>

volatile sig_atomic_t server_disconnected = 0;
pthread_mutex_t server_disconnected_mutex = PTHREAD_MUTEX_INITIALIZER;
