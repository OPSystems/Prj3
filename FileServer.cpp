#include <iostream>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h> 
#include <pthread.h>
#include <vector>
#define PORT 8080                                   // define local port
#define BLOCK_SIZE 128                              // define block size
int microS = 0;                                     // define microseconds
int blockTot = 0;                                   // define total number of blocks
int blocksUsed = 0;                                 // define total number of blocks being used

// This object will represent a file
class File {
    private:
        std::string name = "";                      // file name
        std::string data = "";                      // file data
        int size = 0;                               // size of the data

    public:
        explicit File(std::string n): name(n) {}    // file requires a name

        void setData(std::string str) {
            data = str;                             // set the data from user input
            size = str.length();                    // set the data length
        }

        const std::string getName() {  
            return name;                            // return the file name
        }

        const std::string getData() {   
            return data;                            // return the data
        }

        const int getSize() {  
            return size;                            // return the data size
        }
};

// This object will represent a filesystem 
class Directory {      
    private:
        std::vector<File> files;                    // list of files
        int filesTot = 0;                           // number of total files
        bool formatted = true;                      // whether the filesystem has been formatted

    public:
        Directory() {                        
            filesTot = 0;                           // initialize files to 0
            formatted = true;                       // initialize format to true
        }

        void format() {
            files.clear();                          // delete all files
            filesTot = 0;                           // set the total files to 0
            formatted = true;                       // set formatted to be true
            blocksUsed = 0;                         // set blocks used back to 0
        }

        bool isUniqueName(std::string str) {                      
            for (int i = 0; i < filesTot; i++) {    
                if (files[i].getName() == str) {    
                    return false;                   // iterate through each file to see if the name already exists
                }
            }
            return true;                            // if the name does not exist return true
        }

        std::string addFile(std::string str) {            
            if (blocksUsed < blockTot) {            // if there is available block/s continue
                if (isUniqueName(str)) {            // if the name does not exist continue
                    File file(str);                 // make a new file object with the given name
                    files.push_back(file);          // add the file to the list of files
                    filesTot++;                     // increase the number of files
                    formatted = false;              // set format to false
                    blocksUsed++;                   // add a block to the blocks used
                    return "0";                     // return code
                }
                else {
                    return "1";                     // return code if the file name does not exist
                }
            }   
            return "2";                             // return code if not enough blocks
        }

        const std::string getFilesNames() {             
            std::string info = "";                  // variable to store file names
            if (filesTot < 1) {    
                info = "no files";                  // if 0 files then there save message
            }
            else {
                for (int i = 0; i < filesTot; i++) {
                    info += files[i].getName() + "\n"; // get the file name for each file and store in info string
                }
            }
            return info;                            // return the string with file names
        }

        const std::string getFilesInfo() {        
            std::string info = "";                  // string to store file info
            if (filesTot > 0) {
                for (int i = 0; i < filesTot; i++) {
                    info += "name: " + files[i].getName() + " size: " + std::to_string(files[i].getSize()) + "\n";
                }
            }
            else {
                info = "no files";                  // info is the string above, with info, or it is no filesw
            }
            return info;                            // reuturn the string
        }

        int getIndex(std::string str) {                     
            int index = -1;                         // index is arbitrarely set intitially
            for (int i = 0; i < filesTot; i++) {
                if (files[i].getName() == str) {
                    index = i;                      // find the index of the file with the given name
                }
            }
            return index;                           // return the index
        }

        std::string addData(std::string n, std::string d, int l) {
            int numOfBlocks = 0;                    // variable to store the amount of blocks needed for the data
            if (l > BLOCK_SIZE) {
                while (BLOCK_SIZE < l) {
                l -= 128;                           // block size is 128 so subtract 128 each iteration through the loop
                numOfBlocks++;                      // amount of blocks that will be used increases every 128 of data
                }
            }
        
            if ((blocksUsed + numOfBlocks) <= blockTot) {
                if (!isUniqueName(n)) {             // if the file name exists and there are enough blocks continue
                    files[getIndex(n)].setData(d);  // load the user desired file with the given data
                    blocksUsed += numOfBlocks;      // increment the blocks used with the amount the data needs
                    return "0";                     // return code
                }
                else {
                    return "1";                     // return code
                }
            }
            return "2";                             // return code
        }

        std::string removeFile(std::string n) {
            int fIndex = getIndex(n);               // store the index of the desired file
            int length = 0;                         // hold the length of the selected file data
            int removeBlocks = 1;                   // amount of blocks that will be removed due to file removal
            if (!isUniqueName(n)) {                 // if the file exists...
                length = files[fIndex].getSize();   // set length fo data
                if (length > BLOCK_SIZE) {          // if the length is greater than 128 (uses more than 1 block)
                    while (BLOCK_SIZE < length) {   // figure out how many blocks it uses
                    length -= 128;                  // keep decrementing length by 128 (block size)
                    removeBlocks++;                 // this will increment every 128 (or every block used)
                    }
                }
                blocksUsed -= removeBlocks;         // decrement the amount of blocks used by the amount of blocks the file uses
                files.erase(files.begin() + fIndex);// erase the file at the index desired
                filesTot--;                         // decrement the amount of files
                return "0";                         // return code
            }
            else {
                return "1";                         // return code
            }
        }

        const std::string getFileData(std::string n) {
            std::string d = "";                     // this will be the data string
            int index = getIndex(n);                // get the index of the file
            if (!isUniqueName(n)) {                 // if the file exists get the length and the data
                d = "0" + std::to_string(files[index].getSize()) + " " + files[index].getData();
            }
            else {
                d = "1";                            // return code if the file does not exist
            }
            return d;                               // return the string
        }
};

Directory dir;                                      // Directory object to be used

/* This is the thread function that receives a command from the client
upon successfull connection (through a socket), executes command and 
sends results back through the socket */
void *connection(void *newS) 
{
    int newSock = static_cast<int>(reinterpret_cast<intptr_t>(newS));   // cast the sock-fd back to an int  
    int l = 0;                                                          // user defined input length
    char rbuf[BUFSIZ];                                                  // allocate 1024 bytes to read buffer
    char wbuf[BUFSIZ];                                                  // allocate 1024 bytes to write buffer
    char *cmdAr[BUFSIZ];                                                // command array allocated 1024 bytes
    bzero((char*)rbuf, BUFSIZ);                                         // zero out read buffer
    bzero((char*)wbuf, BUFSIZ);                                         // zero out write buffer
    bzero((char*)cmdAr, BUFSIZ);                                        // zero out command buffer
    std::string sendStr = "";                                           // string that will be converted to char array and sent
    std::string userData = "";                                          // string that will receive userData
    std::string fileName = "";                                          // string that represents the file name from the user
            
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

    if (strcmp(cmd, "F") == 0) {                                        // if the command is F, then format
        if (cmdLen != 1) {
            sendStr = "Not a command";                                  // more than 1 argument is illegal
        }
        else {
            dir.format();                                               // format the filesystem
            sendStr = "Filesystem formatted";                           // send message that it has been formatted
        }
        strcpy(wbuf, sendStr.c_str());                                  // copy string to write buffer
        send(newSock, wbuf, strlen(wbuf), 0);                           // send the buffer to the client
        close(newSock);                                                 // close socket
    }
    else if (strcmp(cmd, "C") == 0) {                                   // C command to create file
        if (cmdLen != 2) {                                              // more or less than 2 arguments is illegal
            sendStr = "Not a command";                                  // store error to send
        }
        else {
            fileName = std::string(cmdAr[1]);                           // this will store the file name argument
            if (fileName.length() > BLOCK_SIZE) {
                sendStr = "Choose a shorter file name";                 // error if the size of the file name is more than 128
            }
            else {
                sendStr = dir.addFile(fileName);                        // set the file name
            }
        }
        strcpy(wbuf, sendStr.c_str());                                  // copy string to write buffer
        send(newSock, wbuf, strlen(wbuf), 0);                           // send the buffer to the client
        close(newSock);                                                 // close socket
    }
    else if (strcmp(cmd, "D") == 0) {                                   // D command to delete a file
        if (cmdLen != 2) {                                              // argument length less or more than 2 is illegal
            sendStr = "Not a command";                                  // store error
        }
        else {
            fileName = std::string(cmdAr[1]);                           // store the name given from the user in variable    
            sendStr = dir.removeFile(fileName);                         // remove the file, given the filename 
        }                                                            
        strcpy(wbuf, sendStr.c_str());                                  // copy string to write buffer
        send(newSock, wbuf, strlen(wbuf), 0);                           // send the buffer to the client
        close(newSock);                                                 // close socket
    }
    else if (strcmp(cmd, "L") == 0) {                                   // L command to list file info
        if (cmdLen != 2) {                                              // more or less than 2 arguments illegal
            sendStr = "Not a command";                                  // store error
        }   
        else {
            if (std::stoi(std::string(cmdAr[1])) == 0) {
                sendStr = dir.getFilesNames();                          // if a 0 is the 2nd argument than just get the filenames
            }   
            else if (std::stoi(std::string(cmdAr[1])) == 1){
                sendStr = dir.getFilesInfo();                           // if a 1 is 2nd argument get file names and sizes
            }
            else {
                sendStr = "Not a command";                              // 2nd argument that is not 1 or 0 is illegal
            }
        }                                        
        strcpy(wbuf, sendStr.c_str());                                  // copy string to write buffer
        send(newSock, wbuf, strlen(wbuf), 0);                           // send the buffer to the client
        close(newSock);                                                 // close socket
    }
    else if (strcmp(cmd, "R") == 0) {                                   // R is read command
        if (cmdLen != 2) {                                              // more or less than 2 arguments is illegal
            sendStr = "Not a command";                                  // save error
        }
        else {
            fileName = std::string(cmdAr[1]);                           // store filename from user 
            usleep(microS);                                             // simulate read time             
            sendStr = dir.getFileData(fileName);                        // string to store data 
        }                          
        strcpy(wbuf, sendStr.c_str());                                  // copy string to write buffer
        send(newSock, wbuf, strlen(wbuf), 0);                           // send the buffer to the client
        close(newSock);                                                 // close socket
    }
    else if (strcmp(cmd, "W") == 0) {                                   // W is the write command
        if (cmdLen != 4) {                                              // more or less than 4 arguments is illegal
            sendStr = "Not a command";                                  // store error
        }
        else {
            fileName = std::string(cmdAr[1]);                           // store filename from user
            l = std::stoi(std::string(cmdAr[2]));                       // store user-given length
            userData = std::string(cmdAr[3]);                           // store the data given by user   
            if (userData.length() != l) {                               // user given length must equal actual length or error
                sendStr = "Your input is not " + std::to_string(l) + " in size";
            }
            else {
                usleep(microS);                                         // simulate write time
                sendStr = dir.addData(fileName, userData, l);           // if user given length is equal to actual length, store data
            }
        }                                  
        strcpy(wbuf, sendStr.c_str());                                  // copy string to write buffer
        send(newSock, wbuf, strlen(wbuf), 0);                           // send the buffer to the client
        close(newSock);                                                 // close socket
    }
    else if (strcmp(cmd, "B") == 0) {                                   // B command to view block status
        if (cmdLen != 1) {                                              // only 1 argument allowed
            sendStr = "Not a command";                                  // send error otherwise
        }
        else {                                                          // string showing how many blocks used
            sendStr = std::to_string(blocksUsed) + " out of " + std::to_string(blockTot) +" blocks used.";
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
    int cylinders = atoi(argv[2]);              // get cylinders from user
    int sectors = atoi(argv[3]);                // get sectors from user
    blockTot = cylinders*sectors;               // get block amount
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