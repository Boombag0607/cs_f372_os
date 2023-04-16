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

// key_t generate_key(char *client_name)
// {
//     key_t key = ftok(client_name, 65);
//     return key;
// }

// int generate_key(char* str) {
//     int hash = 5381; // initial hash value
//     int len = strlen(str);
//     for (int i = 0; i < len; i++) {
//         hash = ((hash << 5) + hash) + str[i]; // multiply by 33 and add ASCII value
//     }
//     hash = hash * len; // incorporate string length
//     return hash;
// }

char *int_to_str(int num)
{
    char *str = malloc(sizeof(char) * 12); // assuming max integer value of 10 digits
    sprintf(str, "%d", num);
    return str;
}

int comm_channel(key_t key)
{
    int shmid;
    shmid = shmget(key, 1024, 0666 | IPC_CREAT);
    return shmid;
}

int generate_id(char *str)
{
    int i, key = 0;
    for (i = 0; i < strlen(str); i++)
    {
        key = key * 1000 + str[i]; // concatenate ASCII values
    }
    return key;
}

int main()
{
    // int i;
    void *shared_memory;
    char buff[100];
    int connect_id;
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
    printf("Key of shared memory is %d\n", connect_id);
    shared_memory = shmat(connect_id, NULL, 0); // process attached to shared memory segment
    printf("Name of the client is %s\n", (char *)shared_memory);
    char *client_name = (char *)shared_memory;
    int id = generate_id(client_name);
    int client_key = ftok(".", id);
    char *client_key_str = int_to_str(client_key);
    strcpy(shared_memory, client_key_str);
    // key_t key = generate_key((char *)shared_memory);
    // key_t key = ftok("Ananya", 'a');
    printf("Client key generated is %d\n", client_key);
    printf("Data written to shared memory is : %s\n", (char *)shared_memory);

    int comm_channel_id = comm_channel(client_key);
    void *comm_channel = shmat(comm_channel_id, NULL, 0);
    printf("Key of comm channel is %d", comm_channel_id);
}
