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
char *client_names[MAX_CLIENTS];
int comm_keys[MAX_CLIENTS];
int comm_channel_id;
int register_id;
int client_exists_id;

char buff[100];
long long int connect_id;
int server_res_cnt = 0;
// shared block is the communication channel structure
struct shared_block
{
    char *client_name; // client
    key_t comm_key;    // client
    int client_req;    // client
    int server_res;    // server
    int action_res;    // server
    int input_data[2]; // server
    int request_cnt;   // server
};

struct connect_channel_block
{
    pthread_rwlock_t rwlock;
    int client_key;
    char client_name[100];
    int req_status;
};

struct shared_block *comm_channel;
struct connect_channel_block *connect_channel;

int action_res = 0;

char *int_to_str(int num)
{
    char *str = malloc(sizeof(char) * 20); // assuming max integer value of 10 digits
    sprintf(str, "%d", num);
    return str;
}

struct worker_thread_args
{
    int action_req;
    int input[2];
    int action_res;
};

void *worker_thread_fn(void *args)
{
    printf("Thread Created with id : %d\n", pthread_self());
    printf("------------------------------------------------------------------\n");

    struct worker_thread_args *t_args = (struct worker_thread_args *)args;
    printf("Thread Started\n");
    printf("------------------------------------------------------------------\n");

    printf("Action Initialized\n");
    printf("------------------------------------------------------------------\n");

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
        else if (num >= 0)
        {
            printf("1 - Not Negative\n");
            ret = 1;
        }
        else
        {
            printf("Invalid Input\n");
            ret = -1;
        }
        comm_channel->action_res = ret;
        break;
    case 5:
        shmctl(comm_channel_id, IPC_RMID, NULL);
        printf("Client at index : %d is unregistered\n", register_id);
        client_ids[register_id] = -1;
        comm_keys[register_id] = -1;
        break;
    default:
        printf("Invalid choice\n");
        break;
    }

    printf("------------------------------------------------------------------\n");
    printf("Action terminated\n");
    printf("------------------------------------------------------------------\n");

    printf("Thread Terminated\n");
    printf("------------------------------------------------------------------\n");
    pthread_exit(NULL);
}

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
    int count = 0;

    for (int i = 0; i < client_count; i++)
    {
        if (client_ids[i] == client_id)
        {
            client_exists_id = i;
            return 1;
        }
    }

    client_ids[client_count] = client_id;
    client_count++;

    return 0;
}

void listen_to_comm_channel()
{
    for (int i = 0; i < client_count; i++)
    {
        if (comm_keys[i] == -1)
        {
            continue;
        }
        else
        comm_channel = shmat(comm_keys[i], NULL, 0);

        if (comm_channel == (void *)-1)
        {
            printf("shmat failed\n");
            exit(EXIT_FAILURE);
        }
        comm_channel_id = comm_keys[i];
        register_id = i;
        if (comm_channel->client_req == 0)
        {
            continue;
        }

        printf("Request to be processed : %d\n", comm_channel->client_req);
        printf("------------------------------------------------------------------\n");

        struct worker_thread_args thread_args_ip = {0};
        pthread_t worker_thread;
        pthread_create(&worker_thread, NULL, worker_thread_fn, &thread_args_ip);
        pthread_join(worker_thread, NULL);

        connect_channel->req_status = 0;
        comm_channel->client_req = 0;
        server_res_cnt++;
        printf("Number of requests serviced by the server is %d\n", server_res_cnt);
        comm_channel->request_cnt++;
        printf("Number of requests serviced for the client is %d\n", comm_channel->request_cnt);
        printf("Action response : %d\n", comm_channel->action_res);
        printf("------------------------------------------------------------------\n");
        if (comm_channel->action_res != -1)
        {
            comm_channel->server_res = 0;
            printf("Server Response : %d\n", comm_channel->server_res);
            printf("------------------------------------------------------------------\n");
        }
        else
        {
            comm_channel->server_res = 1;
            printf("Server Response : Error Code = %d\n", comm_channel->server_res);
            printf("------------------------------------------------------------------\n");
        }
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
    if ((connect_channel->client_name)[0] == '\0')
    {
        return;
    }
    printf("Client Info : \n");
    printf("Name of the client is : %s\n", connect_channel->client_name);
    char *client_name = connect_channel->client_name;
    client_names[client_count] = client_name;
    int client_id = generate_id(client_name);
    int client_flg = validate_and_store(client_name);
    if (client_flg == 1)
    {
        printf("Client already registered\n", NULL);
        printf("------------------------------------------------------------------\n");
        strcpy(connect_channel->client_name, "");
        connect_channel->client_key = ftok(".", client_ids[client_exists_id]);
        return;
    }
    int client_key = ftok(".", client_id);
    connect_channel->client_key = client_key;
    connect_channel->client_name[0] = '\0';
    printf("Client key generated is : %d\n", client_key);
    printf("------------------------------------------------------------------\n");
    printf("Data written to connect channel is : %d\n", connect_channel->client_key);
    printf("------------------------------------------------------------------\n");
    int comm_channel_id = create_comm_channel_id(client_key);
    comm_keys[client_count - 1] = comm_channel_id;
    server_res_cnt++;
    printf("Comm channel created for client.\nKey of commmunication channel is : %d\n", comm_channel_id);
    printf("Number of requests serviced by the server is %d\n", server_res_cnt);
    printf("------------------------------------------------------------------\n");
    for (int j = 0; j < client_count; j++)
    {
        printf("Client ID of client at index %d is : %d\n", j, client_ids[j]);
        printf("------------------------------------------------------------------\n");
    }
}

void create_connect_channel()
{
    connect_id = shmget(1235, 256, 0666 | IPC_CREAT);
    if (connect_id == -1)
    {
        perror("shmget");
        exit(1);
    }
    printf("Connect channel created.\nKey of shared memory is (connect channel) : %lld\n", connect_id);
    printf("------------------------------------------------------------------\n");
    connect_channel = (struct connect_channel_block *)shmat(connect_id, NULL, 0); // process attached to shared memory segment
}

int main()
{
    create_connect_channel();

    pid_t pid = getpid();
    printf("Process ID : %d\n", pid);
    printf("------------------------------------------------------------------\n");

    while (1)
    {
        register_client();

        if (connect_channel->req_status == 1)
        {
            printf("Client Count (failed requests also counted) : %d\n", client_count);
            printf("------------------------------------------------------------------\n");
            for (int i = 0; i < client_count; i++)
            {
                printf("Client ID %d : %d\n", i, client_ids[i]);
            }
            listen_to_comm_channel();
        }
    }
}
