#3rd System Programming Project

Implemented a transaction system (client & server). Communication between client and server is happening through sockets. The server deticates a thread(posix) for each client. Since the server is multi-threaded, mutexes are being used when absolutely neccessary to make sure transactions are atomic. See more at Project3.pdf
