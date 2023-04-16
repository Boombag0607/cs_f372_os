#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/ipc.h>
#include <pthread.h>

#define MAX_CLIENTS 10

struct shared_block {
    char *client_name;
    key_t key;
    int client_res;
    int server_res;
    int action_res;
};

struct client_info {
    int id;
    char client_name[50];
    int shmid;
    int key;
};

struct shared_block *shared_memory;
struct client_info client_list[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *client_handler(void *arg) {
    struct client_info *client = (struct client_info *) arg;
    int shmid = client->shmid;
    int client_id = client->id;
    int key = client->key;
    printf("Worker thread for client %s with ID %d created\n", client->client_name, client_id);

    while (1) {
        if (shared_memory->key == key) {
            printf("Client %s with ID %d requested communication channel\n", client->client_name, client_id);
            shared_memory->server_res = 1; // acknowledge request
            while (shared_memory->key == key) {
                // wait for client to release lock
            }
            printf("Client %s with ID %d disconnected\n", client->client_name, client_id);
            break;
        }
    }

    pthread_exit(NULL);
}

int register_client(char *client_name) {
    pthread_mutex_lock(&mutex);
    if (num_clients >= MAX_CLIENTS) {
        pthread_mutex_unlock(&mutex);
        return -1; // max clients reached
    }
    for (int i = 0; i < num_clients; i++) {
        if (strcmp(client_name, client_list[i].client_name) == 0) {
            pthread_mutex_unlock(&mutex);
            return -2; // client name already registered
        }
    }
    int shmid = shmget(IPC_PRIVATE, sizeof(struct shared_block), 0666 | IPC_CREAT);
    if (shmid == -1) {
        pthread_mutex_unlock(&mutex);
        return -3; // error creating shared memory
    }
    struct shared_block *shared_block = (struct shared_block *) shmat(shmid, NULL, 0);
    if (shared_block == (void *) -1) {
        pthread_mutex_unlock(&mutex);
        return -4; // error attaching to shared memory
    }
    shared_block->client_name = client_name;
    shared_block->key = -1;
    shared_block->client_res = -1;
    shared_block->server_res = -1;
    shared_block->action_res = -1;
    client_list[num_clients].id = num_clients + 1;
    strncpy(client_list[num_clients].client_name, client_name, 50);
    client_list[num_clients].shmid = shmid;
    client_list[num_clients].key = ftok(client_name, 65);
    num_clients++;
    pthread_mutex_unlock(&mutex);
    return client_list[num_clients - 1].id;
}

int main()
{
    int i;
    struct shared_block *shared_memory;
    int connect_shmid;
    int comm_shmid[MAX_CLIENTS];
    pthread_t worker_threads[MAX_CLIENTS];

    // Initialize shared memory
    connect_shmid = shmget((key_t)2345, sizeof(struct shared_block), 0666 | IPC_CREAT);
    shared_memory = (struct shared_block *) shmat(connect_shmid, NULL, 0);
    shared_memory->client_name = "";
    shared_memory->key = -1;
    shared_memory->client_res = 0;
    shared_memory->server_res = 0;
    shared_memory->action_res = 0;

    // Create communication channels for clients
    for (i = 0; i < MAX_CLIENTS; i++) {
        comm_shmid[i] = -1;
    }

    // Wait for clients to connect
    while (1) {
        // Check for client connection
        if (strcmp(shared_memory->client_name, "") != 0) {
            // Check if client name is unique
            int client_index = -1;
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (strcmp(shared_memory->client_name, client_list[i].client_name) == 0) {
                    printf("Client with name '%s' already registered\n", shared_memory->client_name);
                    shared_memory->key = -1;
                    break;
                }
                if (client_list[i].client_name == '\0' && client_index == -1) {
                    client_index = i;
                }
            }
            if (i == MAX_CLIENTS) {
                if (client_index == -1) {
                    printf("Cannot accept more clients, maximum limit reached\n");
                    shared_memory->key = -1;
                } else {
                    strcpy(client_list[client_index].client_name, shared_memory->client_name);
                    printf("Client '%s' registered\n", shared_memory->client_name);

                    // Generate key for client
                    key_t key = sha(shared_memory->client_name);
                    shared_memory->key = key;
                    printf("Key generated for client '%s' is %d\n", shared_memory->client_name, key);

                    // Create communication channel for client
                    comm_shmid[client_index] = comm_channel(key);
                    printf("Communication channel created for client '%s' with shmid %d\n", shared_memory->client_name, comm_shmid[client_index]);

                    // Create worker thread for client
                    pthread_create(&worker_threads[client_index], NULL, NULL, (void *)&comm_shmid[client_index]);
                    printf("Worker thread created for client '%s'\n", shared_memory->client_name);
                }
            }
            // Reset client_name in shared memory
            shared_memory->client_name = "";
        }
        sleep(1);
    }

    // Detach shared memory
    shmdt(shared_memory);

    return 0;
}
