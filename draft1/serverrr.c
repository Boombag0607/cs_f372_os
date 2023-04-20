#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/ipc.h>
#include <pthread.h>

#define MAX_CLIENTS 100
int client_count = 0;
int client_ids[MAX_CLIENTS];
// char *client_name[MAX_CLIENTS];
int comm_keys[MAX_CLIENTS];
void *connect_channel;
int comm_channel_id;
pthread_mutex_t lock;
int register_id;

char buff[100];
long long int connect_id;
// shared block will contain the client request as well as the server response
struct shared_block
{
    char *client_name; // client
    key_t comm_key;    // client
    int client_req;    // client
    int server_res;    // server
    int action_res;
    int input_data[2]; // server
};
struct shared_block *comm_channel;

int action_res = 0;

char *int_to_str(int num)
{
    char *str = malloc(sizeof(char) * 20); // assuming max integer value of 10 digits
    sprintf(str, "%d", num);
    return str;
}

// void worker_thread_fn(void *arg)

void addition()
{
    int a, b;
    scanf("%d %d", &a, &b);
    printf("%d\n", a + b);
    return;
}

struct worker_thread_args
{
    int action_req;
    int input[2];
    int action_res;
};

void *worker_thread_fn(void *args)
{
    struct worker_thread_args *t_args = (struct worker_thread_args *)args;
    pthread_mutex_lock(&lock);
    printf("Thread Started, Mutex Lock Activated\n");
    printf("------------------------------------------------------------------\n");

    printf("Action Initialized\n");
    printf("------------------------------------------------------------------\n");
    // struct worker_args_t *t_args = (struct worker_args_t *)arg;
    // int connect_shmid = t_args->connect_shmid;
    // void *shared_memory;
    // char *filepath = malloc(sizeof(char) * 100);
    // shared_memory = shmat(connect_shmid, NULL, 0);

    // read file path from shared memory
    // strcpy(filepath, (char *)shared_memory);

    // printf("Worker thread started for file: %s\n", filepath);

    // TODO: implement worker thread logic here
    // printf("Choose the function to perform on the file:\n");
    // printf("1. Arithmetic \n 2. Even or Odd \n 3. Prime or Composite \n 4. Negative or Not \n");
    // int choice, result = 0, ret;
    // scanf("%d", &choice);
    int num = comm_channel->input_data[0];
    int ret;
    int flag = 0;
    switch (comm_channel->client_req)
    {

    case 11:
        comm_channel->action_res = comm_channel->input_data[0] + comm_channel->input_data[1];
        break;
    case 12:
        comm_channel->action_res = comm_channel->input_data[0] - comm_channel->input_data[1];
        break;
    case 13:
        comm_channel->action_res = comm_channel->input_data[0] * comm_channel->input_data[1];
        break;
    case 14:
        comm_channel->action_res = comm_channel->input_data[0] / comm_channel->input_data[1];
        break;

    case 2:
        if (num % 2 == 0)
        {
            printf("0 - Even\n");
            ret = 0;
        }
        else
        {
            printf("1 - Odd\n");
            ret = 1;
        }
        comm_channel->action_res = ret;
        break;

    case 3:

        for (int i = 2; i <= num / 2; ++i)
        {
            // condition for non-prime
            if (num % i == 0)
            {
                flag = 1;
                break;
            }
        }

        if (num == 1)
        {
            printf("1 - Not Prime\n");
            ret = 1;
        }
        else
        {
            if (flag == 0)
            {
                printf("0 - Prime\n");
                ret = 0;
            }
            else
            {
                printf("1 - Not Prime\n");
                ret = 1;
            }
        }
        comm_channel->action_res = ret;
        break;

    case 4:
        if (num < 0)
        {
            printf("0 - Negative\n");
            ret = 0;
        }
        else
        {
            printf("1 - Not Negative\n");
            ret = 1;
        }
        comm_channel->action_res = ret;
        break;
    case 5:
        shmctl(comm_channel_id, IPC_RMID, NULL);
        printf("Client unregistered.\n");
        client_ids[register_id] = -1;
        printf("%d\n",register_id);
        break;
    default:
        printf("Invalid choice\n");
        break;
    }

    printf("------------------------------------------------------------------\n");
    printf("Action terminated\n");
    printf("------------------------------------------------------------------\n");

    pthread_mutex_unlock(&lock);
    printf("Thread Terminated, Mutex Lock Deactivated\n");
    printf("------------------------------------------------------------------\n");
    pthread_exit(NULL);

    // shmdt(shared_memory);
    // free(filepath);
}

// int validate_and_store_client(int client_id)
// {
//     char *client_key_str = int_to_str(ftok(".", client_id));
//     char *client_name = (char *)connect_channel;
//     int count = 0;

//     for (int i = 0; i < client_count; i++)
//     {
//         for (int j = 0; j < strlen(client_key_str); j++)
//         {
//             char ch = client_id % 10 + '0';
//             printf("%c\n", ch);
//             client_id = client_id / 10;
//             int cmp = (client_key_str[j] == ch);
//             // printf("%d\n", cmp);
//             if (client_ids[i] == client_id && (client_key_str[j] == ch))
//                 count++;
//         }
//         if (count == strlen(client_key_str))
//             return 1;
//         // if (client_ids[i] == client_id && cmp == 0)
//         // {
//         //     return 1;
//     }
//     printf("string of client key : %s", client_key_str);

//     client_ids[client_count] = client_id;
//     client_count++;
//     return 0;
// }

long long int generate_id(char *str)
{
    int i = 0;
    long long int key = 0;
    for (i = 0; i < strlen(str); i++)
    {
        key = key * 1000 + str[i]; // concatenate ASCII values
    }
    return key;
}

int validate_and_store(char *client_name)
{
    int client_id = generate_id(client_name);
    // char *client_key_str = int_to_str(ftok(".", client_id));
    int count = 0;

    for (int i = 0; i < client_count; i++)
    {
        if (client_ids[i] == client_id)
            return 1;

        // if (client_ids[i] == client_id && cmp == 0)
        // {
        //     return 1;
    }
    // printf("string of client key : %s", client_key_str);

    client_ids[client_count] = client_id;
    client_count++;
    return 0;
}

void listen_to_comm_channel()
{
    for (int i = 0; i < client_count; i++)
    {
        comm_channel = shmat(comm_keys[i], NULL, 0);
        comm_channel_id = comm_keys[i];
        register_id = i;
        printf("Request to be processed : %d\n", comm_channel->client_req);
        printf("------------------------------------------------------------------\n");
        struct worker_thread_args thread_args_ip = {0};
        pthread_t worker_thread;
        // comm_channel->client_req
        pthread_create(&worker_thread, NULL, worker_thread_fn, &thread_args_ip);
        pthread_join(worker_thread, NULL);
        printf("Action response : %d\n", comm_channel->action_res);
        printf("------------------------------------------------------------------\n");
    }
}

int create_comm_channel_id(key_t key)
{
    int shmid;
    shmid = shmget(key, sizeof(struct shared_block), 0666 | IPC_CREAT);
    return shmid;
}

void register_client()
{
    connect_channel = shmat(connect_id, NULL, 0); // process attached to shared memory segment

    if (((char *)connect_channel)[0] == '\0')
    {
        printf("No client to register\n", NULL);
        printf("------------------------------------------------------------------\n");
        return;
    }
    printf("Client Info : \n");
    printf("Name of the client is : %s\n", (char *)connect_channel);
    char *client_name = (char *)connect_channel;
    int client_id = generate_id(client_name);
    int client_flg = validate_and_store(client_name);
    if (client_flg == 1)
    {
        printf("Client already registered\n", NULL);
        printf("------------------------------------------------------------------\n");
        strcpy((char *)connect_channel, "");
        return;
    }
    int client_key = ftok(".", client_id);
    // printf("----------(%d)---------\n", ftok(".", 1));
    char *client_key_str = int_to_str(client_key);
    strcpy(connect_channel, client_key_str);
    // key_t key = generate_key((char *)shared_memory);
    // key_t key = ftok("Ananya", 'a');

    printf("Client key generated is : %d\n", client_key);
    printf("------------------------------------------------------------------\n");
    printf("Data written to connect channel is : %s\n", (char *)connect_channel);
    printf("------------------------------------------------------------------\n");

    int comm_channel_id = create_comm_channel_id(client_key);
    comm_keys[client_count - 1] = comm_channel_id;
    // struct shared_block *comm_channel = (struct shared_block *)shmat(comm_channel_id, NULL, 0);

    printf("Key of commmunication channel is : %d\n", comm_channel_id);
    printf("------------------------------------------------------------------\n");
}

void create_connect_channel()
{
    key_t key = ftok(".", 'b'); // generate key based on current directory and 'a'
    if (key == -1)
    {
        perror("ftok");
        exit(1);
    }

    connect_id = shmget(key, 256, 0666 | IPC_CREAT);
    if (connect_id == -1)
    {
        perror("shmget");
        exit(1);
    }
    printf("Key of shared memory is (connect channel) : %lld\n", connect_id);
    printf("------------------------------------------------------------------\n");
}

int main()
{
    // int i;
    pthread_mutex_init(&lock, NULL);
    create_connect_channel();
    while (1)
    {
        int input = 0;
        printf("Enter 1: To register the client\nEnter 2: To listen to communication channel\nEnter anything else: To quit the server interface\n", NULL);
        printf("------------------------------------------------------------------\n");
        printf("Your choice : ");

        scanf("%d", &input);
        printf("------------------------------------------------------------------\n");
        if (input == 1)
            register_client();
        else if (input == 2)
            // return 0;
            listen_to_comm_channel();
        else
            return 0;
    }
}
