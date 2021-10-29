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

int matriceBuff[255];
int matriceIndex = 0;

int read_ints (const char* file_name)
{
  	FILE* file = fopen (file_name, "r");

	printf("File: %s\n", file_name);
  	int i = 0;
	int n = 0;

  	fscanf (file, "%d", &i);    
  	while (!feof (file))
	{  
		//printf ("%d", i);
		matriceBuff[matriceIndex] = i;
		matriceIndex++;	
		n++;	
		fscanf (file, "%d", &i);      
	}
  	fclose (file);  
	return n;      
}

void func(int sockfd)
{
	
	int R1 = matriceBuff[0];
	int C1 = matriceBuff[1];
	int R2 = matriceBuff[(R1 * C1) + 2];
	int C2 = matriceBuff[(R1 * C1) + 3];
	

	char buff[MAX];

	bzero(buff, sizeof(buff));

	for (int k = 0; k < sizeof(buff); k++)
	{
		buff[k] = matriceBuff[k];
	}
	
	/*
	if (send(sockfd , matriceBuff , (R1 * C1) + (R2 * C2) + 4 , 0) < 0)
	{
		printf("Send failed\n");
	}
	*/

	if (send(sockfd , buff , sizeof(buff) , 0) < 0)
	{
		printf("Send failed\n");
	}

	key_t key;
	int msgid;

	// ftok to generate unique key
	key = ftok("progfile0", 65);

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
	shmctl(shmid, IPC_RMID, NULL);
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
	//func(sockfd);

	char buffer[100];
	int n = 0;

	printf("Enter the first Matrice: ");
    while ((buffer[n++] = getchar()) != '\n');
	buffer[n - 1] = '\0';
	printf("File Name: %s\n", buffer);

	bzero(matriceBuff, sizeof(matriceBuff));
	
	int firstSize = read_ints(buffer);

	matriceIndex = firstSize;

	n = 0;
	bzero(buffer, sizeof(buffer));
	printf("Enter the second Matrice: ");
    while ((buffer[n++] = getchar()) != '\n');
	buffer[n - 1] = '\0';
	printf("File Name: %s\n", buffer);

	int secondSize = read_ints(buffer);
	
	func(sockfd);
	// close the socket
	close(sockfd);
}
