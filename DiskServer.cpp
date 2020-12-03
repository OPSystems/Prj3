#include <iostream>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h> 
#include <pthread.h>
#include <vector>
#define PORT 8080                       // define local port
#define BLOCK_SIZE 128                  // define block size
int microS = 0;                         // define microseconds
int cylinders = 0;                      // define cylinder amount
int sectors = 0;                        // define sector amount
int blockTot = 0;                       // define total number of blocks

// Create a block class to hold data
class Block {      
    private:           
        char data[BLOCK_SIZE];          // to hold the data
        int size;                       // size of the data

    public:
        Block() {                       // constructor
        memset(data, 0, sizeof(data));  // clear data
        size = 0;                       // initialize size to 0
        }

        void setData(std::string str) { // setter (takes a string)
            strcpy(data, str.c_str());  // copy string to data
            size = str.size();          // set size
        }

        const std::string getData() {   // getter
            return data;                // return the data
        }
};
std::vector<Block> blocks;              // create a vector of blocks

/* This is the thread function that receives a command from the client
upon successfull connection (through a socket), executes command and 
sends results back through the socket */
void *connection(void *newS) 
{
    int newSock = static_cast<int>(reinterpret_cast<intptr_t>(newS));   // cast the sock-fd back to an int  
    int blockIndex = 0;                                                 // used to get specific block index 
    int c = 0;                                                          // user defined cylinder
    int s = 0;                                                          // user defined sector
    int l = 0;                                                          // user defined input length
    char rbuf[BUFSIZ];                                                  // allocate 1024 bytes to read buffer
    char wbuf[BUFSIZ];                                                  // allocate 1024 bytes to write buffer
    char *cmdAr[BUFSIZ];                                                // command array allocated 1024 bytes
    bzero((char*)rbuf, BUFSIZ);                                         // zero out read buffer
    bzero((char*)wbuf, BUFSIZ);                                         // zero out write buffer
    bzero((char*)cmdAr, BUFSIZ);                                        // zero out command buffer
    std::string sendStr = "";                                           // string that will be converted to char array and sent
    std::string userData = "";                                          // string that will receive userData
            
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

    char* argv[cmdLen];                                                 // initialize variable to store commands and issue to exec call
    char* cmd = cmdAr[0];                                               // variable cmd will hold the first index of cmdAr (main command)
    for (int i = 0; i < cmdLen; i++) {
        argv[i] = cmdAr[i];                                             // assign each command to the argv array
    }
    argv[cmdLen] = NULL;                                                // final index will be set to NULL

    if (strcmp(cmd, "I") == 0) {                                        // I is the information command
        if (cmdLen != 1) {
            sendStr = "Not a command";                                  // more than 1 argument is illegal
        }
        else {                                                          // otherwise save info to string
            sendStr = std::to_string(cylinders) + " cylinders " + std::to_string(sectors) + " sectors";
        }
        strcpy(wbuf, sendStr.c_str());                                  // copy string to write buffer
        send(newSock, wbuf, strlen(wbuf), 0);                           // send the buffer to the client
        close(newSock);                                                 // close socket
    }
    else if (strcmp(cmd, "R") == 0) {                                   // R is the read command
        if (cmdLen != 3) {
            sendStr = "Not a command";                                  // more or less than 3 arguments is illegal
        }
        else {
            usleep(microS);                                             // simulate read time
            c = std::stoi(std::string(cmdAr[1]));                       // get cylinder from user
            s = std::stoi(std::string(cmdAr[2]));                       // sector from user
            blockIndex = ((c-1) * sectors) + s;                         // get block number

            if (c < cylinders && c >= 0 && s < sectors && s >= 0) {
                sendStr = "1 " + blocks[blockIndex].getData();          // if legal, create string with Block data
            }
            else {
                sendStr = "0";                                          // if illegal create illegal (0) string 
            } 
        }
        strcpy(wbuf, sendStr.c_str());                                  // copy string to write buffer
        send(newSock, wbuf, strlen(wbuf), 0);                           // send the buffer to the client
        close(newSock);                                                 // close socket
    }
    else if (strcmp(cmd, "W") == 0) {
        if (cmdLen != 3) {
            sendStr = "Not a command";                                  // more or less than 4 arguments is illegal
        }
        else {
            usleep(microS);                                             // simulate write time
            c = std::stoi(std::string(cmdAr[1]));                       // get cylinder from user
            s = std::stoi(std::string(cmdAr[2]));                       // sector from user
            l = std::stoi(std::string(cmdAr[3]));                       // get data length from user
            userData = std::string(cmdAr[4]);                           // get user data
            blockIndex = ((c-1) * sectors) + s;                         // get block number

            if (c < cylinders && c >= 0 && s < sectors && s >= 0 && l <= BLOCK_SIZE && l > 0 && l == userData.size()) {
                sendStr = "1";                                          // if legal, create legal (1) string
                blocks[blockIndex].setData(userData);                   // load data into user selected block   
            }
            else {
                sendStr = "0";                                          // if illegal, create illegal (0) string
            }
        }
        strcpy(wbuf, sendStr.c_str());                                  // copy string to write buffer
        send(newSock, wbuf, strlen(wbuf), 0);                           // send the buffer to the client
        close(newSock);                                                 // close socket
    }
    else {
        sendStr = "Not a command";                                      // create error string
        strcpy(wbuf, sendStr.c_str());                                  // copy string to write buffer
        send(newSock, wbuf, strlen(wbuf), 0);                           // send the buffer to the client
        close(newSock);                                                 // close socket
    }

    pthread_exit(NULL);                                                 // exit the thread
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("Server requires 3 parameters: (1) no. of cylinders (2) no. of sectors (3) track-to-track time in microseconds");
        exit(1);
    }
    
    microS = atoi(argv[1]);                     // get microseconds from user
    cylinders = atoi(argv[2]);                  // get cylinders from user
    sectors = atoi(argv[3]);                    // get sectors from user
    blockTot = cylinders*sectors;               // get block amount
    Block block;                                // create a block object
    for (int i = 0; i < blockTot; i++) {
        blocks.push_back(block);                // create "blockTot" of block objects
    }   
    int serverFd;                               // fd for server/socket
    int newSock;                                // fd for new socket to pass to thread
    struct sockaddr_in addr;                    // socket address struct
    int addrlen = sizeof(addr);                 // get address size
    char buf[BUFSIZ];                           // create buffer with 1024 bytes allocated
    pthread_t thread_id;                        // fd for threads

    serverFd = socket(AF_INET, SOCK_STREAM, 0); // create socket 
    if (serverFd < 0) { 
        std::cout << "Socket failed";           // if socket creation fails, issue error
        exit(1);                                // exit if error
    }
    
    memset(&addr, '\0', addrlen);               // clear out the struct
    addr.sin_family = AF_INET;                  // for IPv4 (from socket)
    addr.sin_addr.s_addr = INADDR_ANY;          // connect to any active local address/port
    addr.sin_port = htons(PORT);                // set port to listen to local PORT
        
    if (bind(serverFd, (struct sockaddr *)&addr, addrlen) < 0) {
        std::cout << "Binding failed";          // if a bind is unsuccessfull issue error
        exit(1);                                // exit in the case of an error
    }   
    
    if (listen(serverFd, 5) < 0) {
        std::cout << "Listening failed";        // if listening for connection fails (5 pending), issue error
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