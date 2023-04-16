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

int main()
{
    int connect_shmid;
    int comm_shmid;
    void *connect_memory = (void *)0;
    void *comm_memory = (void *)0;
    struct shared_block *connect_shared_block;
    struct shared_block *comm_shared_block;
    char client_name[256];

    printf("Enter client name: ");
    scanf("%s", client_name);

    connect_shmid = shmget((key_t)1234, sizeof(struct shared_block), 0666 | IPC_CREAT);
    if (connect_shmid == -1)
    {
        perror("shmget failed : ");
        exit(EXIT_FAILURE);
    }

    connect_memory = shmat(connect_shmid, (void *)0, 0);
    if (connect_memory == (void *)-1)
    {
        perror("shmat failed : ");
        exit(EXIT_FAILURE);
    }

    connect_shared_block = (struct shared_block *)connect_memory;

    strcpy(connect_shared_block->client_name, client_name);

    while (connect_shared_block->server_res != 1)
    {
        sleep(1);
    }

    printf("Connection successful\n");

    comm_shmid = connect_shared_block->client_res;

    comm_memory = shmat(comm_shmid, (void *)0, 0);
    if (comm_memory == (void *)-1)
    {
        perror("shmat failed : ");
        exit(EXIT_FAILURE);
    }

    comm_shared_block = (struct shared_block *)comm_memory;

    comm_shared_block->action_res = 1;

    while (1)
    {
        if (comm_shared_block->server_res == 1)
        {
            printf("Response received: %d\n", comm_shared_block->server_res);
            break;
        }
    }

    if (shmdt(connect_memory) == -1)
    {
        perror("shmdt failed : ");
        exit(EXIT_FAILURE);
    }

    if (shmdt(comm_memory) == -1)
    {
        perror("shmdt failed : ");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
