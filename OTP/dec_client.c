#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>


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
    char buffer1[10];
    char buffer2[10];
    FILE *plaintext = NULL;
    FILE *mykey = NULL;

    // Make sure input file sizes and chars are good
    struct stat buf1;
    stat(argv[1], &buf1);
    off_t size1 = buf1.st_size;

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
        // Read text char into buffer1 until there are no more characters
        size_t nr = fread(buffer1, 1, 1, plaintext);

        // Place text char into buffer2
        buffer2[0] = buffer1[0];
        if (feof(plaintext)) break; //if (nr == 0) break;
        memset(buffer1, '\0', sizeof(buffer1));

        // Read key char into buffer1 until there are no more characters
        fread(buffer1, 1, 1, mykey);

        // Place key char into buffer2
        buffer2[1] = buffer1[0];

        // write to socket
        write(socketFD, buffer2, sizeof(buffer2));

        memset(buffer1, '\0', sizeof(buffer1));
        memset(buffer2, '\0', sizeof(buffer2));

        // Get return encrypted char from server
        charsRead = read(socketFD, buffer1, sizeof(buffer1));
        printf("%s", buffer1);

        if (nr<0 || charsRead<0){
            fprintf(stderr, "client error: reading or writing to socket");
        }
    }

    // Clear out the buffer1 again for reuse
    memset(buffer1, '\0', sizeof(buffer1));

    // Close the socket
    close(socketFD);
    return 0;
}