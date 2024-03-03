#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <sys/stat.h>
#include <dirent.h>

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) {
    perror(msg);
    exit(0);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address,
                        int portNumber,
                        char* hostname){

    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address));

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
    memcpy((char*) &address->sin_addr.s_addr,
           hostInfo->h_addr_list[0],
           hostInfo->h_length);
}

int main(int argc, char *argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char buffer[10];
    char buf[10];
    FILE *plaintext = NULL;
    // int size1;
    FILE *mykey = NULL;
    // int size2;
    int stop = 0;

    // Make sure input file sizes and chars are good
    struct stat buf1;
    stat(argv[1], &buf1);
    off_t size1 = buf1.st_size; // need to figure out how to deal with buffer sizes

    struct stat buf2;
    stat(argv[2], &buf2);
    off_t size2 = buf2.st_size;

    if (size1 > size2) {
        fprintf(stderr, "CLIENT: ERROR, key is shorter than text\n"); // TODO also error if invalid char
        exit(1);
    }


    // Check usage & args
    if (argc < 3) {
        fprintf(stderr,"USAGE: %s hostname port\n", argv[0]);
        exit(0);
    }

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0){
        error("CLIENT: ERROR opening socket");
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]), argv[2]);

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        error("CLIENT: ERROR connecting");
    }
    // Get input from files
    plaintext = fopen(argv[1], "r");
    mykey = fopen(argv[2], "r");

    for (;;) { // based on base64 code
        // put data in the buffer and write to socket
        size_t nr = fread(buffer, 1, 1, plaintext);
        buf[0] = buffer[0];
        if (feof(plaintext)) break;
        //if (nr == 0) break;
        //size_t nw = write(socketFD, buffer, nr);
        memset(buffer, '\0', sizeof(buffer));
        fread(buffer, 1, 1, mykey);
        buf[1] = buffer[0];



        //fread(buffer, 1, 1, mykey);
        //write(socketFD, buffer, nr);

        write(socketFD, buf, sizeof(buf));

        memset(buffer, '\0', sizeof(buffer));
        memset(buf, '\0', sizeof(buffer));

        // Get return message from server
        charsRead = read(socketFD, buffer, sizeof(buffer));
        printf("%s", buffer);

        if (nr<0 || charsRead<0){
            fprintf(stderr, "client error: reading or writing to socket");
        }
    }


    // Clear out the buffer again for reuse
    memset(buffer, '\0', sizeof(buffer));

    // Close the socket
    close(socketFD);
    return 0;
}