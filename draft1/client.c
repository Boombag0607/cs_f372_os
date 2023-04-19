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
	int input_data[2];
};

struct shared_block *comm_channel;
int choice;
void *connect_channel = NULL;
char buff[100];
int shmid;

int create_comm_channel_id(key_t key)
{
	int shmid;
	shmid = shmget(key, 1024, 0666 | IPC_CREAT);
	printf("Key of communication channel is %d\n", shmid);
	return shmid;
}

void register_client()
{
	if (((char *)connect_channel)[0] != '\0')
	{
		printf("Key read from connect channel: %s\n", (char *)connect_channel);
		int comm_channel_id = create_comm_channel_id((key_t)atoi((char *)connect_channel));

		printf("Commmunication channel id = %d\n", comm_channel_id);
		strcpy((char *)connect_channel, "");
		comm_channel = shmat(comm_channel_id, NULL, 0);
		int req_no = 0;
		printf("-------------Send request to server----------------\n");
		printf("Which function do you want to exectute?\n");
		printf("1. Arithmetic \n 2. Even or Odd \n 3. Prime or Composite \n 4. Negative or Not \n");
		int choice, result = 0, ret;
		scanf("%d", &choice);
		req_no = req_no * 10 + choice;
		switch (choice)
		{
		case 1:
			printf("Enter the operation to perform: \n 1. Addition \n 2. Subtraction \n 3. Multiplication \n 4. Division \n");
			int operation;
			scanf("%d", &operation);
			req_no = req_no * 10 + operation;
			comm_channel->client_req = req_no;
			int a, b;
			printf("Enter the two numbers: \n");
			scanf("%d %d", &a, &b);
			comm_channel->input_data[0] = a;
			comm_channel->input_data[1] = b;
			break;
		case 2:
			printf("Enter the number to check: \n");
			scanf("%d", &a);
			comm_channel->input_data[0] = a;
			comm_channel->client_req = req_no;
			break;
		case 3:
			printf("Enter the number to check: \n");
			scanf("%d", &a);
			comm_channel->input_data[0] = a;
			comm_channel->client_req = req_no;
			break;
		case 4:
			printf("Enter the number to check: \n");
			scanf("%d", &a);
			comm_channel->input_data[0] = a;
			comm_channel->client_req = req_no;
			break;
		default:
			printf("Invalid choice!");
			break;
		}
		// scanf("%d %d", &comm_channel->comm_key, &comm_channel->client_req);
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

		// shmdt(connect_channel);
	}
}

void get_result()
{
	printf("Result received from server: %d\n", comm_channel->action_res);
}

void connect_to_server()
{
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
}

int main()
{
	connect_to_server();

	while (1)
	{
		printf("Enter 1 to register client\nEnter 2 to see function response\n"); // menu
		scanf("%d", &choice);
		switch (choice)
		{
		case 1:
			register_client();
			break;
		case 2:
			get_result();
			break;
		default:
			printf("Invalid choice\n");
		}
	}

	return 0;
}
