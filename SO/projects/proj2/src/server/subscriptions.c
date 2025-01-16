#define _XOPEN_SOURCE 700

#include "subscriptions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

pthread_rwlock_t subscription_lock = PTHREAD_RWLOCK_INITIALIZER;
static SubKeyNode* hashtable[HASH_SIZE];

void init_subscription_manager() {
    pthread_rwlock_init(&subscription_lock, NULL);
    for (int i = 0; i < HASH_SIZE; i++) {
        hashtable[i] = NULL;
    }
}

static unsigned int hash_function(const char* key) {
    unsigned int hash = 0;
    while (*key) {
        hash = (hash * 31) + (unsigned char)*key++;
    }
    return hash % HASH_SIZE;
}

int add_subscription(const char* key, int notif_fd) {
    pthread_rwlock_wrlock(&subscription_lock);

    unsigned int index = hash_function(key);
    SubKeyNode* entry = hashtable[index];

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            ClientNode* client = entry->clients;
            while (client != NULL) {
                if (client->notif_fd == notif_fd) { // Match by file descriptor
                    pthread_rwlock_unlock(&subscription_lock);
                    return 0; // Already subscribed
                }
                client = client->next;
            }

            // Add the client
            ClientNode* new_client = malloc(sizeof(ClientNode));
            if (!new_client) {
                pthread_rwlock_unlock(&subscription_lock);
                return 1;
            }
            new_client->notif_fd = notif_fd; // Store file descriptor
            new_client->next = entry->clients;
            entry->clients = new_client;

            pthread_rwlock_unlock(&subscription_lock);
            return 0;
        }
        entry = entry->next;
    }

    // Key not found, create new entry
    SubKeyNode* new_entry = malloc(sizeof(SubKeyNode));
    if (!new_entry) {
        pthread_rwlock_unlock(&subscription_lock);
        return 1;
    }

    strncpy(new_entry->key, key, MAX_STRING_SIZE);
    new_entry->key[MAX_STRING_SIZE - 1] = '\0'; // Ensure null termination
    new_entry->clients = malloc(sizeof(ClientNode));
    if (!new_entry->clients) {
        free(new_entry);
        pthread_rwlock_unlock(&subscription_lock);
        return 1;
    }
    new_entry->clients->notif_fd = notif_fd; // Store file descriptor
    new_entry->clients->next = NULL;
    new_entry->next = hashtable[index];
    hashtable[index] = new_entry;

    pthread_rwlock_unlock(&subscription_lock);
    return 0;
}

int remove_subscription(const char* key, int notif_fd) {
    pthread_rwlock_wrlock(&subscription_lock);

    unsigned int index = hash_function(key);
    SubKeyNode* entry = hashtable[index];
    SubKeyNode* prev_entry = NULL;

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            ClientNode* client = entry->clients;
            ClientNode* prev_client = NULL;

            while (client != NULL) {
                if (client->notif_fd == notif_fd) { // Match by file descriptor
                    if (prev_client) {
                        prev_client->next = client->next;
                    } else {
                        entry->clients = client->next;
                    }
                    free(client);

                    if (!entry->clients) { // Remove key if no clients remain
                        if (prev_entry) {
                            prev_entry->next = entry->next;
                        } else {
                            hashtable[index] = entry->next;
                        }
                        free(entry);
                    }

                    pthread_rwlock_unlock(&subscription_lock);
                    return 0;
                }
                prev_client = client;
                client = client->next;
            }
            pthread_rwlock_unlock(&subscription_lock);
            return 1; // Client not found
        }
        prev_entry = entry;
        entry = entry->next;
    }

    pthread_rwlock_unlock(&subscription_lock);
    return 1; // Key not found
}

void notify_subscribers(const char* key, const char* value, int is_deleted) {
    pthread_rwlock_rdlock(&subscription_lock);

    unsigned int index = hash_function(key);
    SubKeyNode* entry = hashtable[index];

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            ClientNode* client = entry->clients;

            while (client != NULL) {
                char notification[MAX_STRING_SIZE * 2 + 2] = {0};
                strncpy(notification, key, MAX_STRING_SIZE);
                if (is_deleted) {
                    strncpy(notification + MAX_STRING_SIZE, "DELETED", MAX_STRING_SIZE);
                } else {
                    strncpy(notification + MAX_STRING_SIZE, value, MAX_STRING_SIZE);
                }

                if (write(client->notif_fd, notification, sizeof(notification)) < 0) {
                    if (errno == EPIPE) {
                        fprintf(stderr, "Notification FIFO closed for fd %d\n", client->notif_fd);
                    } else {
                        fprintf(stderr, "Failed to notify client on fd %d\n", client->notif_fd);
                    }
                }
                client = client->next;
            }
        }
        entry = entry->next;
    }

    pthread_rwlock_unlock(&subscription_lock);
}

int remove_subscriptions_for_fd(int notif_fd) {
    pthread_rwlock_wrlock(&subscription_lock);

    for (int i = 0; i < HASH_SIZE; i++) {
        SubKeyNode* entry = hashtable[i];
        SubKeyNode* prev_entry = NULL;

        while (entry != NULL) {
            ClientNode* client = entry->clients;
            ClientNode* prev_client = NULL;

            while (client != NULL) {
                if (client->notif_fd == notif_fd) { // Match by file descriptor
                    if (prev_client) {
                        prev_client->next = client->next;
                    } else {
                        entry->clients = client->next;
                    }
                    free(client);
                    break;
                }
                prev_client = client;
                client = client->next;
            }

            if (entry->clients == NULL) {
                SubKeyNode* temp = entry;
                if (prev_entry) {
                    prev_entry->next = entry->next;
                } else {
                    hashtable[i] = entry->next;
                }
                entry = entry->next;
                free(temp);
                continue;
            }

            prev_entry = entry;
            entry = entry->next;
        }
    }
    printf("LIVREIME DO FD 17\n");
    pthread_rwlock_unlock(&subscription_lock);
    return 0;
}
