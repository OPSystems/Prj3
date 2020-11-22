#include <iostream>
#include <arpa/inet.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <string.h> 
#define PORT 8080                                                   // define local port

int main() 
{   
    char clientStr[BUFSIZ];                                         // string from the user, BUFSIZ is 1024 bytes
    int cSock;                                                      // socket fd
    char rbuff[BUFSIZ] = {0};                                       // read buffer (initially zero'd out)
    struct sockaddr_in caddr;                                       // socket address struct
    int caddrlen = sizeof(caddr);                                   // get address size

    // while the word "exit" is not typed, continue the client
    while(strcmp(clientStr, "exit") != 0) {
        
        bzero((char*)rbuff, BUFSIZ);                                // zero out the read buffer each loop
        std::cout << "\nEnter a string (Or type 'exit' to quit):";  // output message to user
        std::cin.getline(clientStr, BUFSIZ);                        // get the string from the user
        
        if (strcmp(clientStr, "exit") == 0) {
            break;                                                  // if the word "exit" is entered, exit the loop
        }
        
        cSock = socket(AF_INET, SOCK_STREAM, 0);                    // create the socket
        if (cSock < 0) { 
            std::cout << "Socket Failed \n";                        // if socket fails issue error
            exit(1);                                                // exit in the case of an error
        } 

        memset(&caddr, '\0', caddrlen);                             // clear out the struct
        caddr.sin_family = AF_INET;                                 // for IPv4 (from socket)
        caddr.sin_port = htons(PORT);                               // set port to listen to local PORT
   
        // Connect to the server 
        if (connect(cSock, (struct sockaddr *)&caddr, caddrlen) < 0) { 
            std::cout << "Connection failed \n";                    // if can't connect to server, issue error
            exit(1);                                                // exit in the case of an error                                 
        } 

        std::cout << "\nString Entered: " << clientStr << "\n";     // print the original string
        send(cSock, clientStr, strlen(clientStr), 0);               // send string through socket
        read(cSock, rbuff, 1024);                                   // read the returned (reversed) string
        std::cout << "String Reversed: " << rbuff <<"\n";           // print the reversed string
    }
    
    close(cSock);                                                   // close the socket
    return 0;                                                       // exit the program
}