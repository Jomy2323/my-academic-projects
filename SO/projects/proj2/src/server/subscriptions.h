#ifndef SUBSCRIPTIONS_H
#define SUBSCRIPTIONS_H

#include <stddef.h>
#include <pthread.h>

#define HASH_SIZE 128
#define MAX_STRING_SIZE 40
#define MAX_FDS 1024

typedef struct ClientInfo{
    int notif_fd;
    int resp_fd;
    int req_fd;
    char client_id[MAX_STRING_SIZE];
    int thread_id;
    pthread_mutex_t lock;
} ClientInfo;

typedef struct ClientNode {
    int notif_fd;              
    struct ClientNode* next;     
} ClientNode;

typedef struct SubKeyNode {
    char key[MAX_STRING_SIZE];                   
    ClientNode* clients;         
    struct SubKeyNode* next;        
} SubKeyNode;

extern pthread_rwlock_t subscription_lock;

void init_subscription_manager();

int add_subscription(const char* key, int notif_fd);
int remove_subscription(const char* key, int notif_fd);
void notify_subscribers(const char* key, const char* value, int is_deleted);
int remove_subscriptions_for_fd(int notif_fd);



#endif // SUBSCRIPTIONS_H
