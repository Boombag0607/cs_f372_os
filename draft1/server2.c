#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/ipc.h>

struct shared_block
{
    char *client_name;
    key_t key;
    int client_res;
    int server_res;
    int action_res;
};

struct connect_channel_struct {
    char *client_name;
    int key;
};

char *int_to_str(int num)
{
    char *str = malloc(sizeof(char) * 12); // assuming max integer value of 10 digits
    sprintf(str, "%d", num);
    return str;
}

int comm_channel(int key)
{
    int shmid;
    shmid = shmget(key, 1024, 0666 | IPC_CREAT);
    return shmid;
}

int generate_id(char *str)
{
    int i, key = str[0];
    for (i = 1; i < strlen(str); i++)
    {
        key = key * 1000 + str[i]; // concatenate ASCII values
    }
    return key;
}

int main()
{
    struct connect_channel_struct *connect_channel;
    int connect_id;
    char buff[100];
    
    key_t key = ftok(".", 'a'); // generate key based on current directory and 'a'
    
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
    
    printf("Key of shared memory is %d\n", connect_id);

    connect_channel = (struct connect_channel_struct*)shmat(connect_id, NULL, 0); // process attached to shared memory segment
    
    char *client_name_req = connect_channel->client_name;
    printf("Name of the client is %s\n", client_name_req);
    
    int id = generate_id(client_name_req);
    int client_key = ftok(".", id);
    char *client_key_str = int_to_str(client_key);
    
    connect_channel->key = client_key;
    
    printf("Client key generated is %d\n", client_key);
    printf("Data written to shared memory is : %s\n", client_key_str);

    int comm_channel_id = comm_channel(client_key);
    void *comm_channel = shmat(comm_channel_id, NULL, 0);
    printf("Key of comm channel is %d", comm_channel_id);
}
