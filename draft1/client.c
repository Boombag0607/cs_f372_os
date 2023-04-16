#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>

struct shared_block
{
	char *client_name;
	key_t key;
	int client_res;
	int server_res;
	int action_res;
};

int comm_channel(key_t key)
{
	int shmid;
	shmid = shmget(key, 1024, 0666 | IPC_CREAT);
	printf("Key of communication channel is %d", shmid);
	return shmid;
}

int main()
{
	int i;
	void *shared_memory = NULL;
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

	shared_memory = shmat(shmid, NULL, 0);
	if (shared_memory == (void *)-1)
	{
		perror("shmat");
		exit(1);
	}
	printf("Process attached at %p\n", shared_memory);
	if (((char *)shared_memory)[0] != '\0')
	{
		printf("Key read from shared memory: %s\n", (char *)shared_memory);
		int comm_channel_id = comm_channel((key_t)atoi((char *)shared_memory));
		void *comm_channel = shmat(shmid, NULL, 0);
	}
	else
	{
		printf("Enter some data to write to shared memory\n");
		read(0, buff, 100);
		strcpy(shared_memory, buff);
		printf("You wrote : %s\n", (char *)shared_memory);

		shmdt(shared_memory);
	}

	return 0;
}
