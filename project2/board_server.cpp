#include <fstream>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include "board.h"
#include "utils.h"

void doNothing(int signo){
    //std::cout << "Server signal." << std::endl;
}

int
main(int argc, char *argv[]){
    // Block Ctrl-C.
    static struct sigaction act;
    act.sa_handler=doNothing;
    sigemptyset(&(act.sa_mask));
    sigaction(SIGINT, &act, NULL);
    
    // Check number of args.
    if(argc != 2){
        return 1;
    }
    
    // Store path.
    std::string path(argv[1]);

    // Check if a / exists at the end of path.
    // If it does not exist, put it.
    if(path.back() != '/'){
        path.append("/");
    }
    
    // Check if given dir exists.
    struct stat st;

    if(stat(path.c_str(), &st) == -1){
        mkdir(path.c_str(), 0777);
    }

    // Create data structure to hold messages.
    Board *b = new Board();

    // Create pipes for communication.
    const std::string serverToClient = path + "server_to_client";
    const std::string clientToServer = path + "client_to_server";
    const std::string postToServer = path + "post_to_server";
    const std::string serverToPost = path + "server_to_post";
    const int pipePerms = 0777;

    // Server to client.
    if(mkfifo(serverToClient.c_str(), pipePerms) == -1){
        if(errno != EEXIST){
            perror("Server to client pipe");
            return -2;
        }
    }

    // Client to server.
    if(mkfifo(clientToServer.c_str(), pipePerms) == -1){
        if(errno != EEXIST){
            perror("Client to server pipe");
            return -2;
        }
    }

    // Boardpost to server.
    if(mkfifo(postToServer.c_str(), pipePerms) == -1){
        if(errno != EEXIST){
            perror("Board to server pipe");
            return -2;
        }
    }

    // Server to boardpost.
    if(mkfifo(serverToPost.c_str(), pipePerms) == -1){
        if(errno != EEXIST){
            perror("Server to post pipe");
            return -2;
        }
    }
    
    // Create server.pid file.
    std::ofstream serverInfoFile;
    const std::string serverInfoFileName = path + "server.pid";
    int fdtmp = open(serverInfoFileName.c_str(), O_RDWR|O_CREAT, 0777);
    close(fdtmp);
    serverInfoFile.open(serverInfoFileName.c_str(), std::ios::trunc);
    if(serverInfoFile.good()){
        serverInfoFile << getpid() << std::endl;
        serverInfoFile.close();
    }
    else{
        return -2;
    }

    int fdcts, fdstc, fdpts, fdstp; // Pipe file descriptors.
    
    // Open pipes for reading.
    if((fdcts = open(clientToServer.c_str(), O_RDWR | O_NONBLOCK)) < 0){
        perror("Client to server pipe failed");         
    }

    if((fdpts = open(postToServer.c_str(), O_RDWR | O_NONBLOCK)) < 0){
        perror("Boardpost to server pipe failed");
    }

    // Open pipes for writing.
    if((fdstc = open(serverToClient.c_str(), O_RDWR /*| O_NONBLOCK*/)) < 0){
        perror("Server to client pipe failed");         
    }

    if((fdstp = open(serverToPost.c_str(), O_RDWR /*| O_NONBLOCK*/)) < 0){
        perror("Server to boardpost pipe failed");
    }
    
    // Run forever and read from pipes.
    const int maxMessageLength = 500;
    const int maxCommandLength = 256;
    char buffer1[maxMessageLength];
    char buffer2[maxMessageLength];
    char writeBuffer[maxMessageLength];
    bool run = true;
    int nReadClient; // Bytes read from client.
    int nReadPost; // Bytes read from boardpost.
    int intParam; // Command int parameter storage.
    int intParam2;
    char strParam[maxCommandLength]; // Command string parameter storage.
    char name[maxCommandLength]; // Command name storage.
    while(run){

        nReadPost = read(fdpts, buffer2, maxMessageLength);
        if(nReadPost > 0){
            //std::cout << "Boardpost->Server: " << buffer2 << std::endl;
            fflush(stdout);

            sscanf(buffer2, "%s", name);
            
            // Check command received
            if(streqn(name, "list", maxMessageLength)){

                // Return channel list
                Channel const *temp;
                int index = 0;
                std::string channelList = "";
                while((temp = b->getChannel(index)) != NULL){
                    channelList.append(std::to_string(temp->id()) + " " + temp->name() + " ");
                    index++;
                }
                
                strcpy(writeBuffer, channelList.c_str());
            }
            else if(streqn(name, "POST", maxMessageLength)){

                sscanf(buffer2, "%s %d %[^\n]", name, &intParam, strParam);
                std::string temp(strParam);
                bool idFound = b->addMessageTo(temp, intParam);

                if(idFound){
                    strcpy(writeBuffer, "SUCCESS");
                }
                else{
                    strcpy(writeBuffer, "NOTFOUND");
                }
            }
            else if(streqn(name, "UPLOAD", maxMessageLength)){
                
                sscanf(buffer2, "%s %d %d %s", name, &intParam, &intParam2, strParam);
                
                // Read file.
                int fileSize = intParam;
                char fileBytes[fileSize];
                safeFileRead(fdpts, fileBytes, fileSize);
                
                //std::cout << "Server received: " << fileBytes << std::endl;
                
                std::string filePath = path;
                filePath.append(strParam);              
                // Add _tempfile suffix to avoid collision with pipes.
                filePath.append("_boardfile");

                if(!b->addFileTo(strParam, intParam2)){
                    strcpy(writeBuffer, "NOTFOUND");
                }
                else{
                    int fileDesc = open(filePath.c_str(), O_RDWR|O_CREAT, 0777);
                    if(write(fileDesc, fileBytes, fileSize) == -1){
                        perror("Server could not write to local file");
                        //return -5;
                    }
                    strcpy(writeBuffer, "SUCCESS");
                }
            }

            // Write back to boardpost
            if(write(fdstp, writeBuffer, strlen(writeBuffer) + 1) < 0){
                perror("Server->Boardpost pipe failed");
            }
        }
        else if(nReadPost == -1 && errno != EAGAIN){
            perror("Boardpost to Server pipe failed");
        }
        
        nReadClient = read(fdcts, buffer1, maxMessageLength);
        if(nReadClient > 0){
            //std::cout << "Client->Server: " << buffer1 << std::endl;
            fflush(stdout);
            sscanf(buffer1, "%s", name);
            if(streqn(name, "CREATE", maxMessageLength)){
                bool success;
                std::string temp;
                sscanf(buffer1, "%s %d %s", name, &intParam, strParam);
                temp = strParam;
                success = b->createChannel(intParam, temp);
                
                if(success)
                    strcpy(writeBuffer, "success");
                else
                    strcpy(writeBuffer, "fail");
                
                // Write back to client
                if(write(fdstc, writeBuffer, maxMessageLength) < 0){
                    perror("Server->Client pipe failed");
                }
            }
            else if(streqn(name, "GET", maxMessageLength)){
                sscanf(buffer1, "%s %d", name, &intParam);
                Array<std::string> const *messages = b->getMessagesOf(intParam);
                Array<std::string> const *files = b->getFilesOf(intParam);
                if(messages == nullptr){
                    strcpy(writeBuffer, "NOTFOUND");
                    if(write(fdstc, writeBuffer, maxMessageLength) < 0){
                        perror("Server->Client pipe failed");
                    }
                }
                else{
                    std::string result_number = std::to_string(messages->size());
                    // Let client know number of messages.
                    strcpy(writeBuffer, result_number.c_str());
                    if(write(fdstc, writeBuffer, maxMessageLength) < 0){
                        perror("Server->Client pipe failed");
                    }

                    // Let client know number of files.
                    result_number = std::to_string(files->size());
                    strcpy(writeBuffer, result_number.c_str());
                    if(write(fdstc, writeBuffer, maxMessageLength) < 0){
                        perror("Server->Client pipe failed");
                    }
                    
                    // Send every message one by one.
                    for(int i = 0;i < messages->size(); i++){
                        strcpy(writeBuffer, messages->get(i).c_str());
                        if(write(fdstc, writeBuffer, maxMessageLength) < 0){
                            perror("Server->Client pipe failed");
                        }
                    }

                    // Send every file one by one.
                    // First let client know of each file's size.
                    // Then send actual file.

                    struct stat st;
                    long fileSize;
                    int fileDesc;
                    const char *fileName;
                    std::string fullPath;
                    for(int i = 0;i < files->size(); i++){
                        fileName = files->get(i).c_str();
                        stat(fileName, &st);
                        fileSize = st.st_size;
                        if(fileSize > 64000){
                            fcntl(fdstc, F_SETPIPE_SZ, fileSize);
                        }
                        fileDesc = open(fileName, O_RDONLY);
                        char fileBytes[fileSize];
                        safeFileRead(fileDesc, fileBytes, fileSize);

                        // Let client know about size.
                        sprintf(writeBuffer, "%ld %s", fileSize, fileName);
                        if(write(fdstc, writeBuffer, maxMessageLength) < 0){
                            perror("Server->Client pipe failed");
                        }

                        // Send file.
                        if(write(fdstc, fileBytes, fileSize) == -1){
                            perror("Server->Client pipe failed");
                        }

                        // Delete file from disk.
                        fullPath = path;
                        fullPath.append(fileName);
                        fullPath.append("_boardfile");                  
                        if(unlink(fullPath.c_str()) == -1){
                            perror("Server could not remove file from disk");
                        }
                    }

                    // Reset messages and files of channel.
                    b->resetMessagesOf(intParam);
                    b->resetFilesOf(intParam);
                }
            }
            else if(streqn(name, "DIE", maxMessageLength)){
                // Close pipes.
                close(fdstc);
                close(fdstp);
                close(fdcts);
                close(fdpts);
                
                // Delete pipes.
                unlink(serverToClient.c_str());
                unlink(serverToPost.c_str());
                unlink(clientToServer.c_str());
                unlink(postToServer.c_str());

                // Delete pid file.
                unlink(serverInfoFileName.c_str());

                // Delete all board files.
                Array<std::string> const * files;
                std::string fullPath;
                int id;
                for(int i = 0;i < b->channels(); i++){
                    id = b->getChannel(i)->id();
                    files = b->getFilesOf(id);
                    for(int j = 0; j < files->size(); j++){
                        fullPath = path;
                        fullPath.append(files->get(j));
                        fullPath.append("_boardfile");
                        unlink(fullPath.c_str());
                    }
                }                   
                
                // Delete directory.
                rmdir(path.c_str());

                // Free board memory.
                delete b;

                // Exit.
                return 0;
            }
                
        }
        else if(nReadClient == -1 && errno != EAGAIN){
            perror("Client to Server pipe failed");
        }
        
        usleep(100000); // Dont attempt to read constantly.
    }
    delete b; // Free board memory.
}
