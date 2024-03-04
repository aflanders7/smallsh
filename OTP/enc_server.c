#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

static void /* SIGCHLD handler to reap dead child processes */
grimReaper(int sig)
{
    int savedErrno; /* Save 'errno' in case changed here */
    savedErrno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        continue;
    errno = savedErrno;
}

static void
handleRequest(int connectionSocket) {
    char buffer[10];
    char buffer2[10];

    memset(buffer, '\0', sizeof(buffer));
    memset(buffer2, '\0', sizeof(buffer2));

    for (;;) {
        // Read the client's message from the socket until no chars left
        size_t charsRead = read(connectionSocket, buffer, sizeof(buffer));
        if (charsRead == 0) break;
        // read key
        // read(connectionSocket, buffer2, sizeof(buffer2));

        // do the encryption here
        char str = buffer[0];
        char key = buffer[1];
        char enc_val = (str + key) % 26 + 'A';

        buffer2[0] = enc_val;

        // send encrypted data
        size_t nw = write(connectionSocket, buffer2, sizeof(buffer2));

        memset(buffer, '\0', sizeof(buffer));
        memset(buffer2, '\0', sizeof(buffer2));

        if (nw<0){
            fprintf(stderr, "client error: writing to socket");
        }
    }

}

// Error function used for reporting issues
void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
                        int portNumber){

    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
    int connectionSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    struct sigaction sa;

    // Check usage & args
    if (argc < 2) {
        fprintf(stderr,"USAGE: %s port\n", argv[0]);
        exit(1);
    }

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = grimReaper;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        fprintf(stderr, "Error from sigaction");
        exit(EXIT_FAILURE);
    }

    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket,
             (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0){
        error("ERROR on binding");
    }

    // Start listening for connections. Allow up to 5 connections to queue up
    listen(listenSocket, 5);

    // Accept a connection, blocking if one is not available until one connects
    for (;;) {
        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket,
                                  (struct sockaddr *)&clientAddress,
                                  &sizeOfClientInfo);
        if (connectionSocket < 0){
            error("ERROR on accept");
        }

        // Send and receive the data
        switch (fork()) {
            case -1:
                perror("fork() failed\n");
                close(connectionSocket); /* Give up on this client */
                break; /* May be temporary; try next client */
            case 0: /* Child */
                close(listenSocket); /* Unneeded copy of listening socket */
                handleRequest(connectionSocket);
                _exit(EXIT_SUCCESS);
            default: /* Parent */
                close(connectionSocket); /* Unneeded copy of connected socket */
                break; /* Loop to accept next connection */
        }
    }
    // Close the listening socket
    close(listenSocket);
    return 0;
}