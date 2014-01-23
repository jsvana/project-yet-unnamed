CC=clang

CFLAGS=-g -D_GNU_SOURCE

S_PROG=build/bbs-server
S_OBJS=src/server_main.o src/common.o
S_CFLAGS=$(shell mysql_config --cflags)
S_LIBS=$(shell mysql_config --libs)

C_PROG=build/bbs-client
C_OBJS=src/client_main.o src/common.o
C_LIBS=-lncurses

all: $(S_PROG) $(C_PROG)

$(S_PROG): $(S_OBJS)
	$(CC) -o $(S_PROG) $(S_CFLAGS) $(S_OBJS) $(S_LIBS)

$(C_PROG): $(C_OBJS)
	$(CC) -o $(C_PROG) $(C_OBJS) $(C_LIBS)

%.o: %.c
	$(CC) -c $(LIBS) $(CFLAGS) $< -o $@

run-server: $(S_PROG)
	./$(S_PROG) $(PORT)

run-client: $(C_PROG)
	./$(C_PROG) $(HOST) $(PORT)

clean:
	rm -f $(S_OBJS) $(S_PROG) $(C_OBJS) $(C_PROG)
