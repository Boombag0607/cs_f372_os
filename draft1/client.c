#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>

struct shared_block
{
	char *client_name;
	key_t comm_key;
	int client_req;
	int server_res;
	int action_res;
};

int create_comm_channel_id(key_t key)
{
	int shmid;
	shmid = shmget(key, 1024, 0666 | IPC_CREAT);
	printf("Key of communication channel is %d\n", shmid);
	return shmid;
}

int main()
{
	struct shared_block *comm_channel;
	int choice;
	void *connect_channel = NULL;
	char buff[100];
	int shmid;
	key_t key = ftok(".", 'b'); // generate key based on current directory and 'a'
	if (key == -1)
	{
		perror("ftok");
		exit(1);
	}
	shmid = shmget(key, 1024, 0666 | IPC_CREAT);
	if (shmid == -1)
	{
		perror("shmget");
		exit(1);
	}
	printf("Key of shared memory is %d\n", shmid);

	connect_channel = shmat(shmid, NULL, 0);
	if (connect_channel == (void *)-1)
	{
		perror("shmat");
		exit(1);
	}
	printf("Process attached at %p\n", connect_channel);


	if (((char *)connect_channel)[0] != '\0')
	{
		printf("Key read from connect channel: %s\n", (char *)connect_channel);
		int comm_channel_id = create_comm_channel_id((key_t)atoi((char *)connect_channel));

		printf("Commmunication channel id = %d\n", comm_channel_id);
		strcpy((char *)connect_channel, "");
		comm_channel = (struct shared_block *)shmat(comm_channel_id, NULL, 0);
		// comm_memory->comm_key = (key_t)atoi((char *)connect_channel);
		comm_channel = malloc(sizeof(struct shared_block));
		printf("-------------Send request to server----------------\n");
		printf("Enter client key and functionality request to server: \n");

		scanf("%d %d", &comm_channel->comm_key, &comm_channel->client_req);
		// (struct shared_block *)connect_channel = &comm_channel;
	}
	else
	{
		printf("Enter client name to register:\n");
		memset(buff, 0, sizeof(buff));
		read(0, buff, 100);
		buff[strlen(buff) - 1] = '\0';
		strcpy(connect_channel, buff);
		printf("You wrote : %s\n", (char *)connect_channel);
		printf("------------------Waiting for server to register client----------------\n");

		shmdt(connect_channel);
	}

	return 0;
}
