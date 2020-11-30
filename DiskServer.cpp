#include <iostream>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h> 
#include <pthread.h>
#include <cstring>
#include <strings.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <vector>
#include <string>
#define PORT 8080                                                       // define local port


std::vector<std::vector<std::vector<char>>> disk;
int microSeconds;


/* This is the thread function that receives a command from the client
upon successfull connection (through a socket), executes command and 
sends results back through the socket */
void *connection(void *newS) 
{
    int newSock = static_cast<int>(reinterpret_cast<intptr_t>(newS));   // cast the sock-fd back to an int     
    char rbuf[BUFSIZ];                                                  // allocate 1024 bytes to read buffer
    char *cmdAr[BUFSIZ];                                                // command array allocated 1024 bytes
    bzero((char*)rbuf, BUFSIZ);                                         // zero out read buffer
    bzero((char*)cmdAr, BUFSIZ);                                        // zero out command buffer
            
    if ((read(newSock, rbuf, BUFSIZ)) > 0) {
        std::cout << "Command received is: " << rbuf << std::endl;      // print command if received
    }
    else {
        std::cout << "Couldn't receive command /n";                     // issue error if could not receive command
    }

    int cmdLen = 0;                                                     // counter, will count how long the command is
    char* token = strtok(rbuf, " ");                                    // tokenize, split the command by words, space is the delimiter
        
    // while iterating through the command, increment the cmdLen counter, 
    // to determine size of the command, and add each word to the cmdAr array
    while (token) {
        cmdAr[cmdLen] = token;
        cmdLen++;
        token = strtok(NULL, " ");
    }

    char * cmd = cmdAr[0];
    
    long cylinderSize = disk.size();
    long sectorSize = disk[0].size();
    
    if (strcmp(cmd, "I") == 0) {
        send(newSock, &cylinderSize ,sizeof(cylinderSize), 0);
        send(newSock, &sectorSize, sizeof(sectorSize), 0);   
        close(newSock); 
    }
    else if (strcmp(cmd, "R") == 0) {
        usleep(microSeconds);
        long reqCylinder = std::stoi(std::string(cmdAr[1]));
        long reqSector = std::stoi(std::string(cmdAr[2]));
        std::string outputData = "";
        if (reqCylinder > 0 && reqSector > 0 && reqCylinder <= cylinderSize && reqSector <= sectorSize) {
            outputData = outputData + std::to_string(1) + std::string(disk[reqCylinder - 1][reqSector - 1].begin(), disk[reqCylinder - 1][reqSector - 1].end());
        }
        else {
            outputData = std::to_string(0);
        }
        send(newSock, outputData.c_str(), 1024, 0);   
        close(newSock);
    }
    else if (strcmp(cmd, "W") == 0) {
        usleep(microSeconds);
        long reqCylinder = std::stoi(std::string(cmdAr[1]));
        long reqSector = std::stoi(std::string(cmdAr[2]));
        long dataLength  = std::stoi(std::string(cmdAr[3]));
        std::string data = std::string(cmdAr[4]);
        
        int code;

        if (dataLength <= 128 && dataLength > 0 && reqCylinder > 0 && reqSector > 0 && reqCylinder <= cylinderSize && reqSector <= sectorSize &&  (dataLength == data.size())) {
            for (int i = 0; i < 128; i++) {
                disk[reqCylinder - 1][reqSector - 1][i] = '\0';
            }
            for (int i = 0; i < dataLength; i++) {
                disk[reqCylinder - 1][reqSector - 1][i] = data[i];
            }
            code = 1;
        }
        else {
            printf("%ld", sizeof(data)/sizeof(data[0]));
            code = 0;
        }
        
        send(newSock, &code , sizeof(int), 0);   
        close(newSock);
    }
    else {
        send(newSock, "Invalid command.", 1024, 0);
        close(newSock);
    }

    pthread_exit(NULL);                                                 // exit the thread
}

int main(int argc, char* argv[])
{

    if (argc != 4) {
        printf("Server requires 3 parameters: (1) no. of cylinders (2) no. of sectors (3) track-to-track time in microseconds");
        exit(EXIT_FAILURE);
    }

    int cylinders = atoi(argv[1]);
    int sectors = atoi(argv[2]);
    microSeconds = atoi(argv[3]);

    //populate disk with empty strings
    for (int i = 0; i < cylinders; i++) {
        std::vector<std::vector<char>> sectorArr;
        for (int j = 0; j < sectors; j++) {
            std::vector<char> charArr;
            for(int k = 0; k < 128; k++) {
                charArr.push_back('\0');
            }
            sectorArr.push_back(charArr);
        }
        disk.push_back(sectorArr);
    }


    int serverFd;                               // fd for server/socket
    int newSock;                                // fd for new socket to pass to thread
    struct sockaddr_in addr;                    // socket address struct
    int addrlen = sizeof(addr);                 // get address size
    char buf[BUFSIZ];                           // create buffer with 1024 bytes allocated
    pthread_t thread_id;                        // fd for threads

    serverFd = socket(AF_INET, SOCK_STREAM, 0); // create socket 
    if (serverFd < 0) { 
        std::cout << "Socket failed\n";         // if socket creation fails, issue error
        exit(1);                                // exit if error
    }
    
    memset(&addr, '\0', addrlen);               // clear out the struct
    addr.sin_family = AF_INET;                  // for IPv4 (from socket)
    addr.sin_addr.s_addr = INADDR_ANY;          // connect to any active local address/port
    addr.sin_port = htons(PORT);                // set port to listen to local PORT
        
    if (bind(serverFd, (struct sockaddr *)&addr, addrlen) < 0) {
        std::cout << "Binding failed\n";        // if a bind is unsuccessfull issue error
        exit(1);                                // exit in the case of an error
    }   
    if (listen(serverFd, 5) < 0) {
        std::cout << "Listening failed\n";      // if listening for connection fails (5 pending), issue error
        exit(1);                                // if listen fails, exit
    }

    while ((newSock = accept(serverFd, (struct sockaddr *)&addr, (socklen_t*)&addrlen)) > 0) {
        if (newSock < 0) { 
            std::cout << "Accept failed";       // if connection not accepted, issue error
            exit(1);                            // if can't connect, exit
        }
        pthread_create(&thread_id, NULL, connection, (void*)(size_t)newSock);   // create a thread to connection function with sockFD
		pthread_join(thread_id, NULL);                                          // join the thread after exit
    }
    close(serverFd);                            // close the socket
    return 0;                                   // exit program
}