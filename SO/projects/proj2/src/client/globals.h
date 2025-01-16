#ifndef GLOBALS_H
#define GLOBALS_H

#include <signal.h>
#include <pthread.h>

extern volatile sig_atomic_t server_disconnected;
extern pthread_mutex_t server_disconnected_mutex;

#endif // GLOBALS_H
