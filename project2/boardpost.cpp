#include <cstddef>
#include <fstream>
#include <iostream>

#include <sys/stat.h>

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

void doNothing(int signo){}

int
main(int argc, char *argv[]){
    // Block Ctrl-C.
    static struct sigaction act;
    act.sa_handler=doNothing;
    sigemptyset(&(act.sa_mask));
    sigaction(SIGINT, &act, NULL);
    
    // Check args.
    if(argc != 2){
        std::cout << "Usage: ./board <path>" << std::endl;
        return -1;
    }

    // Store path.
    std::string path(argv[1]);

    // Check if a / exists at the end of path.
    // If it does not exist, put it.
    if(path.back() != '/'){
        path.append("/");
    }

    // Check if server exists at path.
    std::ifstream serverInfoFile;
    const std::string serverInfoFileName = path + "server.pid";
    serverInfoFile.open(serverInfoFileName.c_str(), std::ios::in);

    if(!serverInfoFile.good()){
        std::cerr << "Server does not exist at path. Aborting." << std::endl;
        return -2;
    }

    std::cout << "Boardpost found server." << std::endl;

    // Open Post to Server pipe.
    int fd, fdRead;
    const std::string pipePath = path + "post_to_server";
    const std::string pipePathRead = path + "server_to_post";
    const int maxMessageLength = 500;

    // Open pipe for writing to server.
    if((fd = open(pipePath.c_str(), O_WRONLY /*| O_NONBLOCK*/)) < 0){
        perror("Boardpost cannot open pipe");
        return -3;
    }

    // Open pipe for reading from server.
    if((fdRead = open(pipePathRead.c_str(), O_RDWR)) < 0){
        perror("Server to boardpost pipe failed");
        return -3;
    }

    // Give user a prompt to insert commands.
    const int maxCommandLength = 256;
    char cmd[maxCommandLength]; // Full command.
    char name[maxCommandLength]; // Name of command.
    char strParam[maxCommandLength]; // Parameter for string storage.
    char pipeBuffer[maxMessageLength];
    int intParam; // Parameter for int storage.
    int nWrite; // Number of bytes sent to pipe.
    int nRead; // Number of bytes read from pipe.
    bool expectReply; // If true read pipe.

    // Print out options.
    std::cout << "Available comamnds:" << std::endl;
    std::cout << "list" << std::endl;
    std::cout << "send <id> <file>" << std::endl;
    std::cout << "write <id> <message>" << std::endl;
    
    expectReply = false;
        
    // Print prompt and read user's input.
    std::cout << "% ";
    std::cin.getline(cmd, maxCommandLength);

    // Get name of command.
    sscanf(cmd, "%s", name);

    // Parse arguments according to command.
    if(streqn(name, "list", maxCommandLength)){
        if((nWrite = write(fd, name, maxMessageLength)) == -1){
            perror("Cannot write to server pipe");
            return -4;
        }
        expectReply = true;
    }
    else if(streqn(name, "write", maxCommandLength)){
        sscanf(cmd, "%s %d %[^\n]", name, &intParam, strParam);

        sprintf(pipeBuffer, "POST %d %s", intParam, strParam);
        if((nWrite = write(fd, pipeBuffer, maxMessageLength)) == -1){
            perror("Boardpost cannot write to server pipe");
            return -4;
        }
        expectReply = true;
            
    }
    else if(streqn(name, "send", maxCommandLength)){
        sscanf(cmd, "%s %d %s", name, &intParam, strParam);

        struct stat st;
            
        if(stat(strParam, &st) != 0){
            std::cout << "File does not exist." << std::endl;
            return 0;
        }
            
        long fileSize = st.st_size;
        char fileBytes[fileSize];
        int fileDesc;

        if(fileSize > 64000){
            fcntl(fd, F_SETPIPE_SZ, fileSize);
        }
            
        expectReply = true;
        //std::cout << "File size: " << fileSize << std::endl;
        sprintf(pipeBuffer, "UPLOAD %ld %d %s", fileSize, intParam, strParam);
            
        // Load file.
        fileDesc = open(strParam, O_RDONLY);

        safeFileRead(fileDesc, fileBytes, fileSize);

        //std::cout << "Boardpost file: " << fileBytes << std::endl;
            
        if((nWrite = write(fd, pipeBuffer, maxMessageLength)) == -1){
            perror("Boardpost cannot write to server pipe");
            return -4;
        }
            
        if((nWrite = write(fd, fileBytes, fileSize)) == -1){
            perror("Boardpost cannot write to server pipe");
            return -4;
        }
    }
    else{
        std::cout << "Invalid command." << std::endl;
    }

    // Clean up command buffer
    strcpy(name, "");

    if(expectReply){
        // Read pipe from server.
        nRead = 0;
        while(nRead <= 0){
            nRead = read(fdRead, pipeBuffer, maxMessageLength);
            if(nRead > 0){
                std::cout << "Server->Post: " << pipeBuffer << std::endl;
                fflush(stdout);
            }
            else if(nRead == -1 && errno != EAGAIN){
                perror("Server to Client failed");
                break;
            }
        }
    }
    // Close pipes.
    close(fd);
    close(fdRead);
    return 0;
}
