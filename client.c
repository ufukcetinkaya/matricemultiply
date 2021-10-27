#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

// structure for message queue
struct mesg_buffer {
	long mesg_type;
	char mesg_text[100];
} message;

#define MAX 255
#define PORT 8080
#define SA struct sockaddr

void func(int sockfd)
{
	char buff[MAX];
	int R1 = 2;
	int C1 = 3;
	int R2 = 3;
	int C2 = 2;

	bzero(buff, sizeof(buff));
	//first matrice
	buff[0] = R1;
	buff[1] = C1;
	buff[2] = 1;
	buff[3] = 2;
	buff[4] = 3;
	buff[5] = 4;
	buff[6] = 5;
	buff[7] = 6;
	//second matrice
	buff[8] = R2;
	buff[9] = C2;
	buff[10] = 7;
	buff[11] = 8;
	buff[12] = 9;
	buff[13] = 10;
	buff[14] = 11;
	buff[15] = 12;

	if (send(sockfd , buff , sizeof(buff) , 0) < 0)
	{
		printf("Send failed\n");
	}

	key_t key;
	int msgid;

	// ftok to generate unique key
	key = ftok("progfile", 65);

	// msgget creates a message queue
	// and returns identifier
	msgid = msgget(key, 0666 | IPC_CREAT);

	// msgrcv to receive message
	msgrcv(msgid, &message, sizeof(message), 1, 0);

	// display the message
	printf("Data Received is : %s \n",
					message.mesg_text);

	// to destroy the message queue
	msgctl(msgid, IPC_RMID, NULL);

	// ftok to generate unique key
	key_t memKey = ftok(message.mesg_text, 65);

	// shmget returns an identifier in shmid
	int shmid = shmget(memKey, R1 * C2, 0666|IPC_CREAT);

	// shmat to attach to shared memory
	int *str = (int*) shmat(shmid, (void*) 0, 0);

	printf("Multiply Result: \t");
	
	for (int i = 0; i < R1 * C2; i++)
	{
		printf("%d\t", str[i]);
	}
	printf("\n");
	
	//detach from shared memory
	shmdt(str);
	
	// destroy the shared memory
	shmctl(shmid,IPC_RMID,NULL);
}

int main()
{
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	// socket create and varification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) 
    {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
    {
		printf("Socket successfully created..\n");
    }
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) 
    {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
    {
		printf("connected to the server..\n");
    }

	// function for chat
	func(sockfd);

	// close the socket
	close(sockfd);
}
