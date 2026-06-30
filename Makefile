CC = gcc
CFLAGS = -Wall -Wextra -pthread

SERVER = server
CLIENT = client

all: $(SERVER) $(CLIENT)

$(SERVER): src/server.c src/chat.c src/chat.h
	$(CC) $(CFLAGS) src/server.c src/chat.c -o $(SERVER)

$(CLIENT): src/client.c
	$(CC) $(CFLAGS) src/client.c -o $(CLIENT)

clean:
	rm -f $(SERVER) $(CLIENT)

run-server:
	./$(SERVER)

run-client:
	./$(CLIENT)

.PHONY: all clean run-server run-client

