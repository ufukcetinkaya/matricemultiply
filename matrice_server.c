#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>


#define MAX 10
#define PORT 8080

// structure for message queue
struct mesg_buffer {
	long mesg_type;
	char mesg_text[100];
} message;

const int MAX_MES_SIZE = 255;
const int MAX_THREAD_SIZE = 60;
const int MAX_CON_SIZE = 50;
char client_message[MAX_MES_SIZE];
//char buffer[MAX_MES_SIZE];
//pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int shmMem = 0;

void * socketThread(void *arg)
{
    int newSocket = *((int *)arg);

    recv(newSocket , client_message , MAX_MES_SIZE , 0);
    /*
    // Send message to the client socket 
    pthread_mutex_lock(&lock);
    char *message = malloc(sizeof(client_message) + 20);
    strcpy(message, "Hello Client : ");
    strcat(message, client_message);
    strcat(message, "\n");
    strcpy(buffer, message);
    free(message);
    pthread_mutex_unlock(&lock);
    //sleep(1);
    */

    //[10, -10, 
    // 15, -5,
    // 12, 30]

    //[1, -2, 3,
    // -4, 5, 6]

    int R1 = (int)client_message[0];        //3
    int C1 = (int)client_message[1];        //2
    int R2 = (int)client_message[(R1*C1) + 2];  //2
    int C2 = (int)client_message[(R1*C1) + 3];  //3

    if (C1 == R2)
    {
        int firstMatrice[R1][C1];
        int secondMatrice[R2][C2];
        int j = 2;

        for (int i = 0; i < R1; i++)
        {
            for (int k = 0; k < C1; k++)
            {
                firstMatrice[i][k] = client_message[j];
                j++;
            }
        }

        j = (R1 * C1) + 4;

        for (int i = 0; i < R2; i++)
        {
            for (int k = 0; k < C2; k++)
            {
                secondMatrice[i][k] = client_message[j];
                j++;
            }
        }

        int multiplyMatrices[R1][C2];

        for (int i = 0; i < R1; i++) 
        {
            for (int j = 0; j < C2; j++) 
            {
                multiplyMatrices[i][j] = 0;
    
                for (int k = 0; k < R2; k++) 
                {
                    multiplyMatrices[i][j] += firstMatrice[i][k] * secondMatrice[k][j];
                }
            }
        }

        char keyName[9];
        sprintf(keyName, "shmfile%d", shmMem);

        ++shmMem;

        key_t key = ftok(keyName, 65);

        // shmget returns an identifier in shmid
        int shmid = shmget(key, (R1 * C2), 0666|IPC_CREAT);

        // shmat to attach to shared memory
        int *result = shmat(shmid, (void*) 0, 0);

        printf("Write Data : ");

        int index = 0;

        for (int i = 0; i < R1; i++)
        {
            for (int j = 0; j < C2; j++)
            {
                result[index] = multiplyMatrices[i][j];
                index++;
            }
        }

        printf("Data written in memory: %d\t%d\t%d\t%d\t",result[0], result[1], result[2], result[3]);
        
        //detach from shared memory
        shmdt(result);

        /*
        if (send(newSocket , buffer , sizeof(buffer) , 0) < 0)
        {
            printf("Send failed\n");
        }
        */

        key_t msgKey;
        int msgid;

        // ftok to generate unique key
        msgKey = ftok("progfile", 65);

        // msgget creates a message queue
        // and returns identifier
        msgid = msgget(msgKey, 0666 | IPC_CREAT);
        message.mesg_type = 1;

        //fgets(message.mesg_text,MAX,keyName);
        strcpy(message.mesg_text, keyName);
        // msgsnd to send message
        msgsnd(msgid, &message, sizeof(message), 0);

        // display the message
        printf("Data send is : %s \n", message.mesg_text);
    }
    else
    {
        printf("Matrices are not suitable for Multiply");
    }
    printf("Exit socket Thread \n");
    close(newSocket);
    pthread_exit(NULL);
}

int main()
{
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;

    //Create the socket. 
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);

    // Configure settings of the server address struct
    // Address family = Internet 
    serverAddr.sin_family = AF_INET;

    //Set port number, using htons function to use proper byte order 
    serverAddr.sin_port = htons(PORT);

    //Set IP address to localhost 
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");


    //Set all bits of the padding field to 0 
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    //Bind the address struct to the socket 
    bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    //Listen on the socket, with 40 max connection requests queued 
    if (listen(serverSocket, MAX_CON_SIZE) == 0)
    { 
        printf("Listening\n");
    }
    else
    {
        printf("Error\n");
    }
    pthread_t tid[MAX_THREAD_SIZE];
    int i = 0;

    while(1)
    {
        //Accept call creates a new socket for the incoming connection
        addr_size = sizeof serverStorage;
        newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);

        //for each client request creates a thread and assign the client request to it to process
        //so the main thread can entertain next request
        if (pthread_create(&tid[i++], NULL, socketThread, &newSocket) != 0)
        {
            printf("Failed to create thread\n");
        }

        if (i >= MAX_CON_SIZE)
        {
            i = 0;
            while (i < MAX_CON_SIZE)
            {
                pthread_join(tid[i++], NULL);
            }
            i = 0;
        }
    }
    return 0;
}