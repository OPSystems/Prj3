#include <iostream>
#include <arpa/inet.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <string.h> 
#define PORT 8080                                                   // define local port

int main() 
{   
    char command[BUFSIZ];                                           // command from the user, BUFSIZ is 1024 bytes
    int cSock = 0;                                                  // socket fd
    char rbuff[BUFSIZ] = {0};                                       // read buffer (initially zero'd out)
    struct sockaddr_in caddr;                                       // socket address struct
    int caddrlen = sizeof(caddr);                                   // get address size

    // Intructions to user
    std::cout << "FILE SIMULATION\n\n";
    std::cout << "Commands:\n";
    std::cout << "[C]reate - Create a File 'C [filename]'\n";
    std::cout << "[D]elete - Delete a File 'D [filename]'\n";
    std::cout << "[L]ist - List all Files Names(0) or Names and Sizes(1) 'L [0 or 1]'\n";
    std::cout << "[R]ead - Read File Data 'R [filename]'\n";
    std::cout << "[W]rite - Write Data to File 'W [filename] [length of data] [data]\n";

    // while the word "exit" is not typed, continue the client
    while(strcmp(command, "exit") != 0) {
        
        bzero((char*)rbuff, BUFSIZ);                                // zero out read buffer
        std::cout << "\nEnter a command (Or type 'exit' to quit): ";// print user instruction
        std::cin.getline(command, BUFSIZ);                          // get the command from the user
        
        if (strcmp(command, "exit") == 0) {
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
   
        if (connect(cSock, (struct sockaddr *)&caddr, caddrlen) < 0) { 
            std::cout << "Connection failed \n";                    // if can't connect to server, issue error
            exit(1);                                                // exit in the case of an error
        } 
        
        std::cout << "\nCommand Entered: " << command << "\n";      // print the command
        send(cSock, command, strlen(command), 0);                   // send command through socket
        read(cSock, rbuff, 1024);                                   // read the results
        std::cout << rbuff << "\n";                                 // print the results
    }
    close(cSock);                                                   // close the socket
    return 0;                                                       // exit the program 
}