OBJS_SERVER = board_server.o utils.o board.o
SOURCE_SERVER = board_server.cpp utils.cpp board.cpp
OUT_SERVER = board-server

OBJS_POST = boardpost.o utils.o
SOURCE_POST = boardpost.cpp utils.cpp
OUT_POST = boardpost

OBJS_CLIENT = board_client.o utils.o
SOURCE_CLIENT = board_client.cpp utils.cpp
OUT_CLIENT = board

FLAGS = -g -c -Wall -std=c++0x

all: server post client

server: $(OBJS_SERVER)
	g++ $(OBJS_SERVER) -o $(OUT_SERVER)

post: $(OBJS_POST)
	g++ $(OBJS_POST) -o $(OUT_POST)

client: $(OBJS_CLIENT)
	g++ $(OBJS_CLIENT) -o $(OUT_CLIENT)

board_server.o: board_server.cpp
	g++ $(FLAGS) board_server.cpp

boardpost.o: boardpost.cpp
	g++ $(FLAGS) boardpost.cpp

board_client.o: board_client.cpp
	g++ $(FLAGS) board_client.cpp

utils.o: utils.cpp
	g++ $(FLAGS) utils.cpp

board.o: board.cpp
	g++ $(FLAGS) board.cpp

clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT) $(OBJS_POST)
	rm -f $(OUT_SERVER) $(OUT_CLIENT) $(OUT_POST)
