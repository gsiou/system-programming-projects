#include "board.h"

Board::~Board(){
    for(int i = 0; i < channels_.size(); i++){
        delete channels_.get(i);
    }
}

bool
Board::createChannel(int id, std::string name){
    // First check if id exists
    bool exists = false;
    for(int i = 0; i < channels_.size(); i++){
        if(channels_.get(i)->id() == id){
            exists = true;
            break;
        }
    }

    if(exists){
        return false; // Cant create channel with existing id.
    }
    else{
        // Create channel.
        Channel *temp = new Channel(id, name);
        channels_.add(temp);
        return true; // Success
    }
}

bool
Board::addMessageTo(std::string message, int id){
    for(int i = 0; i < channels_.size(); i++){
        if(channels_.get(i)->id() == id){
            channels_.get(i)->addMessage(message);
            return true;
        }
    }
    return false;
}

bool
Board::addFileTo(std::string file, int id){
    for(int i = 0; i < channels_.size(); i++){
        if(channels_.get(i)->id() == id){
            channels_.get(i)->addFile(file);
            return true;
        }
    }
    return false;
}

int
Board::channels() const{
    return channels_.size();
}

Array<std::string> const *
Board::getMessagesOf(int id){
    for(int i = 0; i < channels_.size(); i++){
        if(channels_.get(i)->id() == id){
            return channels_.get(i)->getMessages(); 
        }
    }
    return nullptr;
}

Array<std::string> const *
Board::getFilesOf(int id){
    for(int i = 0; i < channels_.size(); i++){
        if(channels_.get(i)->id() == id){
            return channels_.get(i)->getFiles(); 
        }
    }
    return nullptr;
}

Channel const *
Board::getChannel(int index) const{
    if(index < channels_.size()){
        return channels_.get(index);
    }
    else{
        return NULL;
    }
}

void
Board::resetMessagesOf(int id){
    for(int i = 0; i < channels_.size(); i++){
        if(channels_.get(i)->id() == id){
            channels_.get(i)->resetMessages();
            return ;
        }
    }
}

void
Board::resetFilesOf(int id){
    for(int i = 0; i < channels_.size(); i++){
        if(channels_.get(i)->id() == id){
            channels_.get(i)->resetFiles();
            return ;
        }
    }
}

Channel::Channel(){
    id_ = 0;
    name_ = "";
}

Channel::Channel(int id, std::string name){
    id_ = id;
    name_ = name;
    messages_ = new Array<std::string>();
    files_ = new Array<std::string>();
}

Channel::~Channel(){
    delete messages_;
    delete files_;
}

void
Channel::addMessage(std::string message){
    messages_->add(message);
}

void
Channel::addFile(std::string file){
    files_->add(file);
}

Array<std::string> const *
Channel::getMessages(){
    return messages_;
}

Array<std::string> const *
Channel::getFiles(){
    return files_;
}

void
Channel::resetMessages(){
    delete messages_;
    messages_ = new Array<std::string>();
}

void
Channel::resetFiles(){
    delete files_;
    files_ = new Array<std::string>();
}

int
Channel::messages() const{
    return messages_->size();
}

int
Channel::files() const{
    return files_->size();
}

int
Channel::id() const{
    return id_;
}

std::string
Channel::name() const{
    return name_;
}
