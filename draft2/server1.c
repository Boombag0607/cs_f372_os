#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/ipc.h>
#include <pthread.h>


#define MAX_CLIENTS 10

struct shared_block
{
    char *client_name;
    key_t key;
    int client_res;
    int server_res;
    int action_res;
};

struct worker_args_t {
    int connect_shmid;
    int action_res;
};

int num_clients = 0;
struct shared_block clients[MAX_CLIENTS];

void *worker_thread_fn(void *arg)
{
    struct worker_args_t *t_args = (struct worker_args_t *)arg;
    int connect_shmid = t_args->connect_shmid;
    void *shared_memory;
    char *filepath = malloc(sizeof(char) * 100);
    shared_memory = shmat(connect_shmid, NULL, 0);

    // read file path from shared memory
    strcpy(filepath, (char *)shared_memory);

    printf("Worker thread started for file: %s\n", filepath);

    // TODO: implement worker thread logic here
    printf("Choose the function to perform on the file:\n");
    printf("1. Arithmetic \n 2. Even or Odd \n 3. Prime or Composite \n 4. Negative or Not \n 5. Exit \n");
    int choice, result;
    scanf("%d", &choice);


    switch (choice)
    {
        case 1:
            printf("Enter the operation to perform: \n 1. Addition \n 2. Subtraction \n 3. Multiplication \n 4. Division \n");
            int operation;
            scanf("%d", &operation);
            int a, b;
            printf("Enter the two numbers: \n");
            scanf("%d %d", &a, &b);
            int result;
            switch (operation)
            {
                case 1:
                    result = a + b;
                    break;
                case 2:
                    result = a - b;
                    break;
                case 3:
                    result = a * b;
                    break;
                case 4:
                    result = a / b;
                    break;
                default:
                    printf("Invalid operation\n");
                    break;
            }
            printf("Result: %d\n", result);
            break;
        case 2:
            printf("Enter the number: \n");
            int num;
            scanf("%d", &num);
            if (num % 2 == 0)
                printf("0 - Even\n");
            else
                printf("1 - Odd\n");
            break;

        case 3:
            printf("Enter the number: \n");
            int num1;
            scanf("%d", &num1);
            int flag = 0;
            for (int i = 2; i <= num1 / 2; ++i) {
                if (num1 % i == 0) {
                    flag = 1;
                    break;
                }
            }
            if (num1 == 1) {
                printf("1 is neither prime nor composite.\n");
            }
            else {
                if (flag == 0)
                    printf("%d is a prime number. - res = 1\n", num1);
                else
                    printf("%d is not a prime number. - res = 0\n", num1);
            }
            break;

        case 4:
            printf("Enter the number: \n");
            int num2;
            scanf("%d", &num2);
            if (num2 < 0)
                printf("Negative - res = 1\n");
            else
                printf("Not Negative - res = 0\n");
            break;

        case 5:
            printf("Exiting...- res = 2\n");
            break;
    }


    shmdt(shared_memory);
    free(filepath);
    pthread_exit(NULL);
}

int comm_channel(key_t key)
{
    int shmid;
    shmid = shmget(key, 1024, 0666 | IPC_CREAT);
    return shmid;
}

int register_client(char *client_name)
{
    // check if client already registered
    for (int i = 0; i < num_clients; i++)
    {
        if (strcmp(clients[i].client_name, client_name) == 0)
        {
            printf("Client already registered with name: %s\n", client_name);
            return clients[i].key;
        }
    }

    // generate key using client_name
    key_t key = *((key_t *)client_name);

    // check if key already used by another client
    for (int i = 0; i < num_clients; i++)
    {
        if (clients[i].key == key)
        {
            printf("Key already used by another client, please try a different name\n");
            return -1;
        }
    }

    // register new client
    clients[num_clients].client_name = malloc(sizeof(char) * 100);
    strcpy(clients[num_clients].client_name, client_name);
    clients[num_clients].key = key;
    num_clients++;

    printf("New client registered with name: %s, key: %d\n", client_name, key);
    return key;
}

int is_client_name_used(char *client_name)
{
    for (int i = 0; i < num_clients; i++)
    {
        if (strcmp(clients[i].client_name, client_name) == 0)
            return 1;
    }
    return 0;
}

int main()
{
    int i;
    int shmid;
    void *shared_memory = (void *)0;
    struct shared_block *shared_block_ptr;
    char buff[100];

    // Create shared memory segment
    shmid = shmget((key_t)1234, sizeof(struct shared_block), 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        perror("shmget failed : ");
        exit(EXIT_FAILURE);
    }

    // Attach to shared memory
    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1)
    {
        perror("shmat failed : ");
        exit(EXIT_FAILURE);
    }

    printf("Server process attached at %p\n", shared_memory);

    // Initialize shared block
    shared_block_ptr = (struct shared_block *)shared_memory;
    shared_block_ptr->client_name = NULL;
    shared_block_ptr->client_res = -1;
    shared_block_ptr->server_res = -1;
    shared_block_ptr->action_res = -1;

    while (1)
    {
        // Wait for client registration
        while (shared_block_ptr->client_name == NULL)
            sleep(1);

        // Verify client name is unique
        if (is_client_name_used(shared_block_ptr->client_name))
        {
            shared_block_ptr->client_res = -1;
            shared_block_ptr->client_name = NULL;
            continue;
        }

        // Generate key using client_name
        key_t key = *((key_t *)shared_block_ptr->client_name);

        // Create communication channel
        int channel_id = comm_channel(key);

        // Create worker thread
        pthread_t worker_thread;
        struct worker_args_t args;
        args.connect_shmid = channel_id;
        if (pthread_create(&worker_thread, NULL, worker_thread_fn, (void *)&args))
        {
            perror("pthread_create failed : ");
            exit(EXIT_FAILURE);
        }

        // Send key to client
        shared_block_ptr->key = key;
        shared_block_ptr->client_res = 0;
        shared_block_ptr->client_name = NULL;
    }

    // Detach from shared memory
    if (shmdt(shared_memory) == -1)
    {
        perror("shmdt failed : ");
        exit(EXIT_FAILURE);
    }

    // Delete shared memory segment
    if (shmctl(shmid, IPC_RMID, 0) == -1)
    {
        perror("shmctl(IPC_RMID) failed : ");
        exit(EXIT_FAILURE);
    }

    return 0;
}
