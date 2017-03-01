#include <cstddef>
#include <fstream>
#include <iostream>

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

void doNothing(int signo){
    //std::cout << "Client signal." << std::endl;
}

int
main(int argc, char *argv[]){
    // Block Ctrl-C.
    static struct sigaction act;
    act.sa_handler=doNothing;
    sigemptyset(&(act.sa_mask));
    sigaction(SIGINT, &act, NULL);
    
    // Check command line args.
    if(argc != 2){
        std::cout << "Usage: ./board <path>" << std::endl;
        return 1;
    }

    // Store path.
    std::string path(argv[1]);

    // Check if a / exists at the end of path.
    // If it does not exist, put it.
    if(path.back() != '/'){
        path.append("/");
    }
    
    std::cout << "Server Path: " << path << std::endl;

    // Check if server exists at path.
    std::ifstream serverInfoFile;
    const std::string serverInfoFileName = path + "server.pid";
    const int maxClientAttempts = 10;
    int clientAttempts = 0;
    int serverPID;
    serverInfoFile.open(serverInfoFileName.c_str(), std::ios::in);

    if(!serverInfoFile.good()){
        std::cout << "Server does not exist at path." << std::endl;

        // Creating server process.
        int pid = fork();

        if(pid == 0){ // Child.
            // Execute server
            if(execl("./board-server", "board-server", path.c_str(), (void *) 0) == -1){
                perror("Failed to spawn server");
                return -1;
            }
        }

        // Wait for server to start
        while(!serverInfoFile.good() && clientAttempts < maxClientAttempts){
            serverInfoFile.open(serverInfoFileName.c_str(), std::ios::in);
            sleep(1);
            clientAttempts++;
        }
    }

    // Get PID of server.
    serverInfoFile >> serverPID;
    std::cout << "PID of server is: " << serverPID << std::endl;
    
    // Check if server process is actually running
    // by sending a signal and checking if it got received.
    if(kill(serverPID, 0) != 0){
        std::cerr << "Server files found, but no server process is running. Aborting." << std::endl;
        return -3;
    }
    
    std::cout << "Client found server." << std::endl;
    serverInfoFile.close(); // Not needed anymore.


    // Open Client to Server pipe.
    int fd, fdRead;
    const std::string pipePath = path + "client_to_server";
    const std::string pipePathRead = path + "server_to_client";
    const int maxMessageLength = 500;

    // Open pipe for writing to server.
    if((fd = open(pipePath.c_str(), O_WRONLY /*| O_NONBLOCK*/)) < 0){
        perror("Client cannot open pipe");
        return -4;
    }

    // Open pipe for reading from server.
    if((fdRead = open(pipePathRead.c_str(), O_RDWR)) < 0){
        perror("Server to client pipe failed");
        return -4;
    }
    
    // Give user a prompt to insert commands.
    const int maxCommandLength = 256;
    char cmd[maxCommandLength]; // Full command.
    char name[maxCommandLength]; // Name of command.
    char strParam[maxCommandLength]; // Parameter for string storage.
    char pipeBuffer[maxMessageLength];
    int intParam; // Parameter for int storage.
    int nWrite; // Number of chars sent to pipe.

    // Print out options.
    std::cout << "Available comamnds:" << std::endl;
    std::cout << "createchannel <id> <name>" << std::endl;
    std::cout << "getmessages <id>" << std::endl;
    std::cout << "exit" << std::endl;
    std::cout << "shutdown" << std::endl;
    
    while(1){
        
        // Print prompt and read user's input.
        std::cin.clear();
        std::cout << "% "; 
        std::cin.getline(cmd, maxCommandLength);
        
        // Get name of command.
        sscanf(cmd, "%s", name);

        // Parse arguments according to command.
        if(streqn(name, "createchannel", maxCommandLength)){
            sscanf(cmd, "%s %d %s", name, &intParam, strParam);
            std::cout << name << " " << intParam  << " " << strParam << std::endl;
            sprintf(pipeBuffer, "CREATE %d %s", intParam, strParam);
            if((nWrite = write(fd, pipeBuffer, maxMessageLength)) == -1){
                perror("Cannot write to server pipe");
                return -4;
            }

            // Get server reply.
            safeRead(fdRead, pipeBuffer, maxMessageLength);
            std::cout << "Server->Client: " << pipeBuffer << std::endl;
        }
        else if(streqn(name, "getmessages", maxCommandLength)){
            sscanf(cmd, "%s %d", name, &intParam);
            std::cout << name << " " << intParam << std::endl;
            sprintf(pipeBuffer, "GET %d", intParam);
            if((nWrite = write(fd, pipeBuffer, maxMessageLength)) == -1){
                perror("Cannot write to server pipe");
                return -4;
            }

            // Get reply from server.
            safeRead(fdRead, pipeBuffer, maxMessageLength);
            
            // We have our reply.
            // Check if request succeded.
            if(streqn(pipeBuffer, "NOTFOUND", strlen("NOTFOUND"))){
                std::cout << "Channel does not exist" << std::endl;           
            }
            else{
                // We are good to go.
                // Message we received was number of messages.
                int nMessages = atoi(pipeBuffer);
                safeRead(fdRead, pipeBuffer, maxMessageLength);         
                int nFiles = atoi(pipeBuffer);
                std::cout << "Messages(" << nMessages << "):" << std::endl;
                for(int i = 0; i < nMessages; i++){
                    safeRead(fdRead, pipeBuffer, maxMessageLength);
                    std::cout << pipeBuffer << std::endl;
                }


                // Deal with files.
                long fileSize;
                char fileName[100];
                int fileDesc;
                std::string fullName;
                
                std::cout << "Files(" << nFiles << "):" << std::endl;          
                for(int i = 0; i < nFiles; i++){
                    safeRead(fdRead, pipeBuffer, maxMessageLength);
                    std::cout << pipeBuffer << std::endl;
                    sscanf(pipeBuffer, "%ld %s", &fileSize, fileName);
                    char fileBytes[fileSize];
                    safeFileRead(fdRead, fileBytes, fileSize);
                    fullName = fileName;
                    fullName.append("_boardfile"); // Prevent collisions.
                    fileDesc = open(fullName.c_str(), O_RDWR|O_CREAT, 0777);
                    if(write(fileDesc, fileBytes, fileSize) == -1){
                        perror("Client could not write to local file");
                        //return -5;
                    }
                }
            }
        }
        else if(streqn(name, "exit", maxCommandLength)){
            std::cout << "Exiting." << std::endl;
            close(fd);
            close(fdRead);
            return 0;
        }
        else if(streqn(name, "shutdown", maxCommandLength)){
            std::cout << "Shutting down server." << std::endl;
            strcpy(pipeBuffer, "DIE");
            if(write(fd, pipeBuffer, maxMessageLength) == -1){
                perror("Could not write to server pipe");
            }
            return 0;
        }
        else{
            std::cout << "Command " << name << " is invalid." << std::endl;
        }

        // Clean up command buffer.
        strcpy(name, "");
    }
}
