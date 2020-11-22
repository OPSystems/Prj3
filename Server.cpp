#include <iostream>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h> 
#include <pthread.h>
#define PORT 8080                                                       // define local port

// This function just reverses the inputted string
void reverseString(char* read, char* write, int size) {
    for(int i = 0; i < size; i++) {
        write[i] = read[size - 1 - i];                                  // last index value to first index 
    }
}

/* This is the thread function that receives a string from the client
upon successfull connection (through a socket), calls the reverse 
function, and then returns the reversed string through a socket */
void *connection(void *newS) 
{
    int newSock = static_cast<int>(reinterpret_cast<intptr_t>(newS));   // cast the sock-fd back to an int
    int n = 0;                                                          // set variable to read length
    char rbuf[BUFSIZ];                                                  // allocate 1024 bytes to read buffer
    char wbuf[BUFSIZ];                                                  // allocate 1024 bytes to write buffer
    
    bzero((char*)rbuf, BUFSIZ);                                         // clear out the read buffer
    bzero((char*)wbuf, BUFSIZ);                                         // clear out the write buffer
            
    n = read(newSock,rbuf, BUFSIZ);                                     // read socket input into read buffer
    reverseString(rbuf, wbuf, n);                                       // call the reverse function to revers the string

    std::cout << "String Read: " <<  rbuf << "\n";                      // print the original string
    std::cout << "String Reversed: " << wbuf << "\n";                   // print the reversed string

    send(newSock,wbuf ,strlen(wbuf), 0);                                // send the reversed string to the client
    close(newSock);                                                     // close the socket
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
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   // connect to any active local address/port
    addr.sin_port = htons(PORT);                // set port to listen to local PORT
    
    
    if (bind(serverFd, (struct sockaddr *)&addr, addrlen) < 0) {
        std::cout << "Binding failed\n";        // if a bind is unsuccessfull issue error
        exit(1);                                // exit in the case of an error
    }

    if (listen(serverFd, 5) < 0) {  
        std::cout << "Listening failed\n";      // if listening for connection fails (5 pending), issue error
        exit(1);                                // if listen fails, exit
    }

    // keep creating threads for new connections, exit if connection fails
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