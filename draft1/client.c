#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <pthread.h>

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

struct connect_channel_block
{
	pthread_rwlock_t rwlock;
	int comm_key;
	char client_name[100];
	int req_status;
};

// void *connect_channel = NULL;
struct connect_channel_block *connect_channel;

char buff[100];
int shmid;
int func_choice;
int comm_channel_id;

int create_comm_channel_id(key_t key)
{
	int shmid;
	shmid = shmget(key, sizeof(struct shared_block), 0666 | IPC_CREAT);
	printf("Key of communication channel is %d\n", shmid);
	return shmid;
}

void register_client()
{
	// pthread_rwlock_wrlock(&(connect_channel->rwlock));
	printf("Enter client name to register:\n");
	scanf("%s", connect_channel->client_name);
	printf("You wrote : %s\n", connect_channel->client_name);
	printf("------------------------------------------------------------------\n");
	printf("------------Waiting for server to register client-----------------\n");
	sleep(2);
	// pthread_rwlock_unlock(&(connect_channel->rwlock));
}

void send_request()
{
	if (comm_channel->comm_key == 0)
	{
		printf("Communication channel not yet made!\n");
		return;
		// comm_channel_id = create_comm_channel_id((key_t)connect_channel->comm_key);
		// printf("Commmunication channel id = %d\n", comm_channel_id);
		// connect_channel->comm_key = 0;
	}

	int req_no = 0;
	printf("--------------------Send request to server------------------------\n");
	printf("------------------------------------------------------------------\n");

	int result = 0, ret;
	printf("Which function do you want to exectute?\n");
	printf(" 1. Arithmetic \n 2. Even or Odd \n 3. Prime or Composite \n 4. Negative or Not \n 5. Unregister\n");
	printf("------------------------------------------------------------------\n");
	printf("Your choice :  ");
	scanf("%d", &func_choice);

	printf("------------------------------------------------------------------\n");

	req_no = req_no * 10 + func_choice;
	switch (func_choice)
	{
	case 1:
		printf("Enter the operation to perform: \n 1. Addition \n 2. Subtraction \n 3. Multiplication \n 4. Division \n");
		int operation;
		printf("------------------------------------------------------------------\n");
		printf("You choice :  ");
		scanf("%d", &operation);
		printf("------------------------------------------------------------------\n");

		req_no = req_no * 10 + operation;
		comm_channel->client_req = req_no;
		int a, b;
		printf("Enter the two numbers: \n");
		printf("------------------------------------------------------------------\n");
		scanf("%d %d", &a, &b);
		comm_channel->input_data[0] = a;
		comm_channel->input_data[1] = b;
		break;
	case 2:
		printf("Enter the number to check: \n");
		scanf("%d", &a);
		printf("------------------------------------------------------------------\n");
		comm_channel->input_data[0] = a;
		comm_channel->client_req = req_no;
		break;
	case 3:
		printf("Enter the number to check: \n");
		scanf("%d", &a);
		printf("------------------------------------------------------------------\n");
		comm_channel->input_data[0] = a;
		comm_channel->client_req = req_no;
		break;
	case 4:
		printf("Enter the number to check: \n");
		scanf("%d", &a);
		printf("------------------------------------------------------------------\n");
		comm_channel->input_data[0] = a;
		comm_channel->client_req = req_no;
		break;
	case 5:
		comm_channel->client_req = req_no;
		break;
	default:
		printf("Invalid choice!");
		printf("------------------------------------------------------------------\n");
		break;
	}
	connect_channel->req_status = 1;
}

void get_result()
{
	switch (func_choice)
	{
	case 1:
		printf("Result received from server: %d\n", comm_channel->action_res);
		break;
	case 2:
		if (comm_channel->action_res == 0)
		{
			printf("Result received from server: Even\n");
		}
		else if (comm_channel->action_res == 1)
		{
			printf("Result received from server: Odd\n");
		}
		break;
	case 3:
		if (comm_channel->action_res == 0)
		{
			printf("Result received from server: Prime\n");
		}
		else if (comm_channel->action_res == 1)
		{
			printf("Result received from server: Composite\n");
		}
		break;
	case 4:
		if (comm_channel->action_res == 0)
		{
			printf("Result received from server: Negative\n");
		}
		else if (comm_channel->action_res == 1)
		{
			printf("Result received from server: Not Negative\n");
		}
		break;
	case 5:
		printf("Unregistered from server!\n");
		break;
	default:
		printf("Invalid choice!");
		break;
	}

	printf("------------------------------------------------------------------\n");
}

void connect_to_server()
{
	// key_t key = ftok(".", 'b'); // generate key based on current directory and 'a'
	// if (key == -1)
	// {
	// 	perror("ftok");
	// 	exit(1);
	// }
	shmid = shmget(1235, 256, 0666 | IPC_CREAT);
	if (shmid == -1)
	{
		perror("shmget");
		exit(1);
	}
	printf("Key of shared memory (connect channel) is : %d\n", shmid);
	printf("------------------------------------------------------------------\n");

	connect_channel = (struct connect_channel_block *)shmat(shmid, NULL, 0);
	if (connect_channel == (struct connect_channel_block *)-1)
	{
		perror("shmat");
		exit(1);
	}
	printf("Process attached at : %p\n", connect_channel);
	printf("------------------------------------------------------------------\n");
}

void connect_comm_channel()
{
	printf("Key read from connect channel: %d\n", connect_channel->comm_key);
	comm_channel_id = create_comm_channel_id((key_t)connect_channel->comm_key);
	printf("Commmunication channel id = %d\n", comm_channel_id);
	comm_channel = (struct shared_block *)shmat(comm_channel_id, NULL, 0);
	comm_channel->comm_key = connect_channel->comm_key;
	connect_channel->comm_key = 0;
}

int main()
{
	connect_to_server();
	int choice;
	pthread_rwlock_init(&(connect_channel->rwlock), NULL);
	// pthread_rwlock_unlock(&(connect_channel->rwlock));
	while (1)
	{
		printf("Enter 1: To register client\nEnter 2: To send request to server for a registered client\nEnter anything else: To quit the client interface\n"); // menu
		printf("------------------------------------------------------------------\n");
		printf("Your choice : ");
		scanf("%d", &choice);
		printf("------------------------------------------------------------------\n");
		switch (choice)
		{
		case 1:
			register_client();
			while (connect_channel->comm_key == 0)
			{
			}
			connect_comm_channel();
			break;
		case 2:
			send_request();
			sleep(1);
			get_result();
			break;
		default:
			printf("Quitting\n");
			printf("------------------------------------------------------------------\n");
			return 0;
		}
	}

	return 0;
}
