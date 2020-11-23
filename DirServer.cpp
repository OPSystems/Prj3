#include <iostream>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h> 
#include <pthread.h>
#define PORT 8080                                                       // define local port

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

    pid_t pid = fork();                                                 // fork process

    if (pid < 0) {
        std::cout << "Process creation failed.\n";                      // issue error if fork process fails
        exit(1);                                                        // exit if error issued
    }
    else if (pid == 0) {                                                // child process
        char* argv[cmdLen];                                             // initialize variable to store commands and issue to exec call
        char* cmd = cmdAr[0];                                           // variable cmd will hold the first index of cmdAr (main command)
        
        for (int i = 0; i < cmdLen; i++) {
            argv[i] = cmdAr[i];                                         // and assign each command to the argv array
        }
        argv[cmdLen] = NULL;                                            // final index will be set to NULL
        dup2(newSock, STDOUT_FILENO);                                   // redirect output through the socket
        close(newSock);                                                 // close socket
        execvp(cmd, argv);                                              // issue command
        std::cout << "This is not a command\n";                         // issue error if command is not executable
        exit(1);                                                        // exit if command is bad
    }
    else {
        wait(&pid);                                                     // wait for child process
        fflush(stdout);                                                 // flush the output
        close(newSock);                                                 // close socket
    }
    pthread_exit(NULL);                                                 // exit the thread
}

int main( )
{
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