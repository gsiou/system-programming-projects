#ifndef _BOARD_H_
#define _BOARD_H_

#include <iostream>
#include "array.h"

class Channel{
public:
    Channel();
    Channel(int id, std::string name);
    ~Channel();
    void addMessage(std::string message);
    void addFile(std::string file);
    Array<std::string> const * getMessages();
    Array<std::string> const * getFiles();
    void resetMessages();
    void resetFiles();
    int messages() const;
    int files() const;
    int id() const;
    std::string name() const;
private:
    int id_;
    std::string name_;
    Array<std::string> *messages_;
    Array<std::string> *files_;
};

class Board{
public:
    ~Board();
    bool createChannel(int id, std::string name);
    bool addMessageTo(std::string message, int id);
    bool addFileTo(std::string file, int id);
    int channels() const;
    Array<std::string> const * getMessagesOf(int id);
    Array<std::string> const * getFilesOf(int id);
    Channel const * getChannel(int index) const;
    void resetMessagesOf(int id);
    void resetFilesOf(int id);
private:
    Array<Channel *> channels_;
};

#endif
