#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/ipc.h>

#define MAX_CLIENTS 100
int client_count = 0;
int client_ids[MAX_CLIENTS];
char *client_name[MAX_CLIENTS];
void *connect_channel;
char buff[100];
long long int connect_id;
// shared block will contain the client request as well as the server response
struct shared_block
{
    char *client_name; // client
    key_t comm_key;    // client
    int client_req;    // client
    int server_res;    // server
    int action_res;    // server
};

char *int_to_str(int num)
{
    char *str = malloc(sizeof(char) * 20); // assuming max integer value of 10 digits
    sprintf(str, "%d", num);
    return str;
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

int create_comm_channel_id(key_t key)
{
    int shmid;
    shmid = shmget(key, 1024, 0666 | IPC_CREAT);
    return shmid;
}

void register_client()
{
    connect_channel = shmat(connect_id, NULL, 0); // process attached to shared memory segment

    if (((char *)connect_channel)[0] == '\0')
    {
        printf("No client to register\n", NULL);
        return;
    }
    printf("Name of the client is %s\n", (char *)connect_channel);
    char *client_name = (char *)connect_channel;
    int client_id = generate_id(client_name);
    int client_flg = validate_and_store(client_name);
    if (client_flg == 1)
    {
        printf("Client already registered\n", NULL);
        strcpy((char *)connect_channel, "");
        return;
    }
    int client_key = ftok(".", client_id);
    printf("----------(%d)---------\n", ftok(".", 1));
    char *client_key_str = int_to_str(client_key);
    strcpy(connect_channel, client_key_str);
    // key_t key = generate_key((char *)shared_memory);
    // key_t key = ftok("Ananya", 'a');

    // while (1) {
    printf("Client key generated is %d\n", client_key);
    printf("Data written to connect channel is : %s\n", (char *)connect_channel);

    int comm_channel_id = create_comm_channel_id(client_key);
    struct shared_block *comm_channel = (struct shared_block *)shmat(comm_channel_id, NULL, 0);
    printf("Key of comm channel is %d\n", comm_channel_id);
}

void create_connect_channel()
{

    key_t key = ftok(".", 'b'); // generate key based on current directory and 'a'
    if (key == -1)
    {
        perror("ftok");
        exit(1);
    }

    connect_id = shmget(key, 1024, 0666 | IPC_CREAT);
    if (connect_id == -1)
    {
        perror("shmget");
        exit(1);
    }
    printf("Key of shared memory is %lld\n", connect_id);
}

int main()
{
    // int i;
    create_connect_channel();
    while (1)
    {
        int input = 0;
        printf("Enter 1: To register the client\nEnter 2: To exit\n---------------\n", NULL);
        scanf("%d", &input);
        if (input == 1)
            register_client();
        else if (input == 2)
            return 0;
    }
}
