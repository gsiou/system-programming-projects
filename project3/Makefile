OBJS_SERVER = bank_server.o utils.o hash.o bankrecord.o
SOURCE_SERVER = bank_server.c utils.c hash.c bankrecord.c
OUT_SERVER = bankserver

OBJS_CLIENT = bank_client.o utils.o
SOURCE_CLIENT = bank_client.c utils.c
OUT_CLIENT = bankclient

LIBS = -lpthread
FLAGS = -g -c -Wall -std=c99

all: server client

server: $(OBJS_SERVER)
	gcc $(OBJS_SERVER) -o $(OUT_SERVER) $(LIBS)

client: $(OBJS_CLIENT)
	gcc $(OBJS_CLIENT) -o $(OUT_CLIENT)

bank_server.o: bank_server.c
	gcc $(FLAGS) bank_server.c

bank_client.o: bank_client.c
	gcc $(FLAGS) bank_client.c

utils.o: utils.c
	gcc $(FLAGS) utils.c

hash.o: hash.c
	gcc $(FLAGS) hash.c

bankrecord.o: bankrecord.c
	gcc $(FLAGS) bankrecord.c

clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT)
	rm -f $(OUT_SERVER) $(OUT_CLIENT)

loc:
	wc -l *.h *.c
