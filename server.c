/*
Author: Brittany Legget
Date: 3/9/2022
Client Server Chat
*/

/*
* Server code
1. The server creates a socket and binds to ‘localhost’ and port xxxx
2. The server then listens for a connection
3. When connected, the server calls recv to receive data
4. The server prints the data, then prompts for a reply
5. If the reply is /q, the server quits
6. Otherwise, the server sends the reply
7. Back to step 3
8. Sockets are closed

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*
* Error messageing and setting up a struct for address and port number
*/

void error(const char* msg) {
    perror(msg);
    exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
    int portNumber) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
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
        //Send length of data then data
        if (strcmp(type, "send") == 0) {
            if (len == 0) {
                send(conn, &size, sizeof(size), 0);
                len = 1;
            }
            chars = send(conn, buffer + received, remaining, 0);
        }
        //Receive data length and then data
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
        //Update number of characters received abd keft to receive
        received += chars;
        remaining -= chars;
    }
}

/*
* Main function sets up connection, listens on port until data is received, and then sends
* a response. The receive/send loop continues until the stop signal '/q' is received from
* the client.
*/
int main(int argc, char* argv[]) {
    int connectionSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    char sendBuffer[10000];
    char receiveBuffer[10000];
    int stop = 0;
    int port = atoi(argv[1]);
    int sendLength;

    // Check usage & args
    if (argc < 2) {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, port);

    // Associate the socket to the port
    if (bind(listenSocket,
        (struct sockaddr*)&serverAddress,
        sizeof(serverAddress)) < 0) {
        error("ERROR on binding");
    }

    // Start listening for connetions. Allow up to 5 connections to queue up
    listen(listenSocket, 5);

    printf("\nServer listening on: localhost port %d\n", port);

    // Get input message from user
    printf("SERVER: Waiting for message...\n\n");

    // Accept a connection, blocking if one is not available until one connects
    while (1) {
        connectionSocket = accept(listenSocket,
            (struct sockaddr*)&clientAddress,
            &sizeOfClientInfo);
        if (connectionSocket < 0) {
            error("ERROR on accept");
        }

        // Clear out the buffer arrays
        memset(sendBuffer, '\0', sizeof(sendBuffer));
        memset(receiveBuffer, '\0', sizeof(receiveBuffer));

        /********************************************************
        RECEIVE DATA
        *********************************************************/
        SendReceive(connectionSocket, receiveBuffer, 1, "recv");

        //If stop signal, close connection socket and listening socket
        if (strcmp(receiveBuffer, "/q") == 0) {
            close(connectionSocket);
            close(listenSocket);
            exit(0);
        }
    
        //Display text from client removing trailing newline
        printf("> %s\n", receiveBuffer);
        fgets(sendBuffer, sizeof(sendBuffer) - 1, stdin);
        sendBuffer[strcspn(sendBuffer, "\n")] = '\0';

        // Save length of content being sent
        sendLength = strlen(sendBuffer);


        /********************************************************
            SEND DATA TO Client
        *********************************************************/
        SendReceive(connectionSocket, sendBuffer, sendLength, "send");

        close(connectionSocket);      
    }

    close(listenSocket);
    return 0;
}
