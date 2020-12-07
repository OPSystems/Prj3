#include <iostream>
#include <arpa/inet.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <string.h> 
#define PORT 5837                                                   // define local port

int main(int argc, char* argv[])
{   

    if (argc != 3) {
        printf("Client requires 2 paramters (1) Number of calls to server (2) Seed of random number generator.\n");
        exit(EXIT_FAILURE);
    }

    int N = atoi(argv[1]);
    int rngNo = atoi(argv[2]);

    srand(rngNo);


    int cSock = 0;                                                  // socket fd
    char rbuff[BUFSIZ] = {0};                                       // read buffer (initially zero'd out)
    struct sockaddr_in caddr;                                       // socket address struct
    int caddrlen = sizeof(caddr);                                   // get address size

    cSock = socket(AF_INET, SOCK_STREAM, 0);

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

    send(cSock, "I", strlen("I"), 0); 

    long cylinders;
    long sectors;
    read(cSock, &cylinders, sizeof(long));
    read(cSock, &sectors, sizeof(long));        

    // while the word "exit" is not typed, continue the client
    for(int i = 0; i < N; i++) {
        
        bzero((char*)rbuff, BUFSIZ);                                // zero out read buffer       

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

        int commandNo = rand() % 2;
        int cylinder = rand() % cylinders + 1;
        int sector = rand() % sectors + 1; 
        std::string outputString;

        if (commandNo == 0) {
            outputString = "R " + std::to_string(cylinder) + " " + std::to_string(sector);
        }
        else {
            char data[128];
            for (int i = 0; i < 128; i++) {
                data[i] = 'a' + (rand() % 26);
            }
            outputString = "W " + std::to_string(cylinder) + " " + std::to_string(sector) + " " + std::to_string(128) + " " + std::string(data);
        }

        
        send(cSock, outputString.c_str(), strlen(outputString.c_str()), 0);                   // send command through socket
        
        if (outputString[0] == 'W') {
            int code;
            read(cSock, &code, sizeof(int));
            std::cout << "W\n";
        }
        else {
            read(cSock, rbuff, 1024);                                   // read the results
            std::cout << "R\n";                                 // print the results
        }

        
    }
    close(cSock);                                                   // close the socket
    return 0;                                                       // exit the program 
}