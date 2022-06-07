/*
Author: Brittany Legget
Date: 3/9/2022
Client Server Chat
*/

/*
* Client code
1. The client creates a socket and connects to ‘localhost’ and port xxxx
2. When connected, the client prompts for a message to send
3. If the message is /q, the client quits
4. Otherwise, the client sends the message
5. The client calls recv to receive data
6. The client prints the data
7. Back to step 2
8. Sockets are closed
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netdb.h>   

/*
* Error messaging and setting up a struct for address and port number
*/

void error(const char* msg) {
    perror(msg);
    exit(0);
}


void setupAddressStruct(struct sockaddr_in* address, int portNumber) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname("localhost");
    if (hostInfo == NULL) {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*)&address->sin_addr.s_addr,
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

/*
* Function will handle the sending and receiving of data to and from the server
* It accepts the connection, the string to be sent, the length of the string, and the type
* if the type is send, then the data is sent to the server. If the type is rcv, then the 
* data is being received from the server.
* For each type, the first send or receive is sending or receiving the length of the data to
* be expected by the client or server. Then the data of specified length is sent or received.
*/
void SendReceive(int conn, char* buffer, int size, char* type)
{
    int remaining = size;
    int received = 0;
    int chars;
    int len = 0;
    int exp;

    while (remaining > 0)
    {
        //Send Data length then data
        if (strcmp(type, "send") == 0) {

            if (len == 0) {
                send(conn, &size, sizeof(size), 0);
                len = 1;
            }
            chars = send(conn, buffer + received, remaining, 0);
            if (chars < 0) {
                error("CLIENT: ERROR writing to socket");
            }
        }

        //Receive data length then data
        if (strcmp(type, "recv") == 0) {
            if (len == 0) {
                recv(conn, &exp, sizeof(exp), 0);
                remaining = exp;
                len = 1;
            }
            chars = recv(conn, buffer + received, remaining, 0);
        }
        if (chars == -1)
        {
            exit(1);
        }
        //Update number of characters received and left to receive
        received += chars;
        remaining -= chars;
    }
}

/*
* Main function sets up connection to server, and then sends
* data. The sending/receiving of data loop continues until the stop 
* signal '/q' is sent to the server.
*/
int main(int argc, char* argv[]) {

    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char sendBuffer[10000];
    char receiveBuffer[10000];
    int stop = 0;
    int port = atoi(argv[1]);
    int first = 0;

    // Check usage & args
    if (argc < 1) {
        fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
        exit(0);
    }

    while (stop == 0) {
        int sendLength = 0;

        // Create a socket, setup server struct, and connect
        socketFD = socket(AF_INET, SOCK_STREAM, 0);
        if (socketFD < 0) {
            error("CLIENT: ERROR opening socket");
        }

        setupAddressStruct(&serverAddress, port);

        if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
            error("CLIENT: ERROR connecting");
        }

         /********************************************************
            PREPARE DATA TO BE SENT TO SERVER
         *********************************************************/

        //Only display this the first time (keeps console cleaner on chat)
        if (first == 0) {
            printf("\nCLIENT: Enter a message to send: \n\n");
            first = 1;
        }

        // Clear out the buffer array
        memset(sendBuffer, '\0', sizeof(sendBuffer));
        memset(receiveBuffer, '\0', sizeof(receiveBuffer));

        // Get input from the user and remove newline
        fgets(sendBuffer, sizeof(sendBuffer) - 1, stdin);
        sendBuffer[strcspn(sendBuffer, "\n")] = '\0';
        sendLength = strlen(sendBuffer);

        /********************************************************
            SEND DATA TO SERVER
        *********************************************************/
        //If no stop signla, send message
        if (strcmp(sendBuffer, "/q") == 0) {
            stop = 1;
        }

        SendReceive(socketFD, sendBuffer, sendLength, "send");

        if (stop == 1) {
            exit(0);
        }

        // receive data and check for stop signal     
        SendReceive(socketFD, receiveBuffer, sendLength, "recv");

        if (strcmp(receiveBuffer, "/q") == 0) {
            stop = 1;
        }

        printf("> %s\n", receiveBuffer);

        close(socketFD); 
    }

    close(socketFD);
    return 0;
}