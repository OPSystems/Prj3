#include <iostream>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h> 
#include <pthread.h>
#include <vector>
#define PORT 8080                       // define local port

class File {
    private:
        std::string name = "";
        std::string data = "";
        int size = 0;

    public:
        explicit File(std::string n): name(n) {}

        void setData(std::string str) {
            data = str;
            size = str.length();
        }

        const std::string getName() {   // getter
            return name;                // return the data
        }

        const std::string getData() {   // getter
            return data;                // return the data
        }

        // void setSize(int s) {
        //     size = s;
        // }

        const int getSize() {   // getter
            return size;                // return the data
        }
};

class Directory {      
    private:
        std::vector<File> files;
        int filesTot = 0;
        bool formatted = true;

    public:
        Directory() {                               // constructor
            filesTot = 0;
            formatted = true;
        }

        void format() {
            files.clear();
            // for (int i = 0; i < filesTot; i++) {
            //     files.erase(files.begin() + i);
            // }
            filesTot = 0;
            formatted = true;
        }

        bool isUniqueName(std::string str) {                      // getter
            for (int i = 0; i < filesTot; i++) {
                if (files[i].getName() == str) {
                    return false;
                }
            }
            return true;
        }

        std::string addFile(std::string str) {             // setter (takes a string)
            if (isUniqueName(str)) {
                File file(str);
                files.push_back(file);
                filesTot++;
                formatted = false;
                return "0";
            }
            else {
                return "1";
            }
        }

        const std::string getFilesNames() {              // getter
            std::string info = "";
            if (formatted) {
                info = "no files";
            }
            else {
                for (int i = 0; i < filesTot; i++) {
                    info += files[i].getName() + "\n";
                }
            }
            return info;
        }

        const std::string getFilesInfo() {              // getter
            std::string info = "";
            if (filesTot > 0) {
                for (int i = 0; i < filesTot; i++) {
                    info += "name: " + files[i].getName() + " size: " + std::to_string(files[i].getSize()) + "\n";
                }
            }
            else {
                info = "no files";
            }
            return info;
        }

        int getIndex(std::string str) {                      // getter
            int index = -1;
            for (int i = 0; i < filesTot; i++) {
                if (files[i].getName() == str) {
                    index = i;
                }
            }
            return index;
        }

        std::string addData(std::string n, std::string d) {
            if (!isUniqueName(n)) {
                files[getIndex(n)].setData(d);
                return "0";
            }
            else {
                return "1";
            }
        }

        std::string removeFile(std::string n) {
            if (!isUniqueName(n)) {
                files.erase(files.begin() + getIndex(n));
                filesTot--;
                return "0";
            }
            else {
                return "1";
            }
        }

        const std::string getFileData(std::string n) {
            std::string d = "";
            int index = getIndex(n);
            if (!isUniqueName(n)) {
                d = "0" + std::to_string(files[index].getSize()) + " " + files[index].getData();
            }
            else {
                d = "1";
            }
            return d;
        }
};

Directory dir;

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
    std::string userName = "";
            
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

    if (strcmp(cmd, "F") == 0) {
        dir.format();
        sendStr = "Filesystem formatted";
        strcpy(wbuf, sendStr.c_str());                                  // copy string to write buffer
        send(newSock, wbuf, strlen(wbuf), 0);                           // send the buffer to the client
        close(newSock);                                                 // close socket
    }
    else if (strcmp(cmd, "C") == 0) {
        userName = std::string(cmdAr[1]);
        sendStr = dir.addFile(userName);
        strcpy(wbuf, sendStr.c_str());                                  // copy string to write buffer
        send(newSock, wbuf, strlen(wbuf), 0);                           // send the buffer to the client
        close(newSock);                                                 // close socket
    }
    else if (strcmp(cmd, "D") == 0) {                                               // simulate write time
        userName = std::string(cmdAr[1]);                    
        sendStr = dir.removeFile(userName);
        strcpy(wbuf, sendStr.c_str());                                  // copy string to write buffer
        send(newSock, wbuf, strlen(wbuf), 0);                           // send the buffer to the client
        close(newSock);                                                 // close socket
    }
    else if (strcmp(cmd, "L") == 0) {                                               // simulate write time
        if (std::stoi(std::string(cmdAr[1])) == 0) {
            sendStr = dir.getFilesNames();
        }
        else if (std::stoi(std::string(cmdAr[1])) == 1){
            sendStr = dir.getFilesInfo();
        }
        else {
            sendStr = "Not a valid entry";
        }
        strcpy(wbuf, sendStr.c_str());                                  // copy string to write buffer
        send(newSock, wbuf, strlen(wbuf), 0);                           // send the buffer to the client
        close(newSock);                                                 // close socket
    }
    else if (strcmp(cmd, "R") == 0) {                                               // simulate write time
        userName = std::string(cmdAr[1]);                    
        sendStr = dir.getFileData(userName);
        strcpy(wbuf, sendStr.c_str());                                  // copy string to write buffer
        send(newSock, wbuf, strlen(wbuf), 0);                           // send the buffer to the client
        close(newSock);                                                 // close socket
    }
    else if (strcmp(cmd, "W") == 0) {                                               // simulate write time
        userName = std::string(cmdAr[1]);         
        l = std::stoi(std::string(cmdAr[2]));
        userData = std::string(cmdAr[3]);                               // get user data
        if (userData.length() > l) {
            sendStr = "Your input is larger than " + std::to_string(l) + " in size";
        }
        else {
            sendStr = dir.addData(userName, userData);
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

int main()
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