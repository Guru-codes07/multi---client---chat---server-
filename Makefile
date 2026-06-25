```makefile
CC = gcc
CFLAGS = -Wall -Wextra -pthread

SERVER = server
CLIENT = client

all: $(SERVER) $(CLIENT)

$(SERVER): src/server.c
	$(CC) $(CFLAGS) src/server.c -o $(SERVER)

$(CLIENT): src/client.c
	$(CC) $(CFLAGS) src/client.c -o $(CLIENT)

clean:
	rm -f $(SERVER) $(CLIENT)

run-server:
	./$(SERVER)

run-client:
	./$(CLIENT)

.PHONY: all clean run-server run-client
```

