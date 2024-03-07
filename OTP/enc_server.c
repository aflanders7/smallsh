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

static char const allowed_characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

// handler for child processes
// Source: Linux Programming Interface chapter 60.3
static void
handler(int sig)
{
    int savedErrno;
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

        // do the encryption here
        char str = buffer[0];
        char key = buffer[1];
        int index = 0;
        if (str == ' ') {
            index = 26;
        }
        else {
            index = str - 'A';
        }
        int enc_val = (index + key) % 27;

        // send encrypted data
        if (str == '\n') {
            buffer2[0] = str;
        }
        else {
            buffer2[0] = allowed_characters[enc_val];
        }
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
    int connectionSocket, listenSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    struct sigaction sa;
    int charRead = 0;

    // Check usage & args
    if (argc < 2) {
        fprintf(stderr,"USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Initialize sigaction for forking
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        fprintf(stderr, "Error from sigaction");
        exit(EXIT_FAILURE);
    }

    // Create the socket that will listen for connections
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
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

    for (;;) {
        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket,
                                  (struct sockaddr *)&clientAddress,
                                  &sizeOfClientInfo);
        // handshake
        char buffer[] = "e";
        write(connectionSocket, buffer, sizeof(buffer));
        for (;;) {
            memset(buffer, '\0', sizeof(buffer));
            charRead = read(connectionSocket, buffer, sizeof(buffer));
            if (charRead > 0) {
                char handshake = buffer[0];
                if (handshake != 'e') {
                    fprintf(stderr, "Client rejected");
                }
                break;
            }
        }

        // Use child processes to send and receive the data
        // forking source: The Linux programming Interface, chapter 60.3
        switch (fork()) {
            case -1:
                perror("fork() failed\n");
                close(connectionSocket);
                break;
            case 0: // Child
                close(listenSocket);
                handleRequest(connectionSocket);
                _exit(EXIT_SUCCESS);
            default: // Parent
                close(connectionSocket);
                break;
        }
    }
    // Close the listening socket
    close(listenSocket);
    return 0;
}