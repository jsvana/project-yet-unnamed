CC=clang

CFLAGS=-g -D_GNU_SOURCE
MYSQL_CFLAGS=$(shell mysql_config --cflags)
MYSQL_LIBS=$(shell mysql_config --libs)

S_PROG=build/bbs-server
S_OBJS=src/server_main.o src/common.o src/bbs_mysql.o
S_CFLAGS=$(MYSQL_CFLAGS)
S_LIBS=$(MYSQL_LIBS)

C_PROG=build/bbs-client
C_OBJS=src/client_main.o src/common.o
C_LIBS=-lncurses

SEED_PROG=build/seed
SEED_OBJS=src/seed.o src/common.o src/bbs_mysql.o
SEED_CFLAGS=$(MYSQL_CFLAGS)
SEED_LIBS=$(MYSQL_LIBS)

all: $(S_PROG) $(C_PROG) $(SEED_PROG)

$(S_PROG): $(S_OBJS)
	$(CC) -o $(S_PROG) $(S_CFLAGS) $(S_OBJS) $(S_LIBS)

$(C_PROG): $(C_OBJS)
	$(CC) -o $(C_PROG) $(C_OBJS) $(C_LIBS)

$(SEED_PROG): $(SEED_OBJS)
	$(CC) -o $(SEED_PROG) $(SEED_CFLAGS) $(SEED_OBJS) $(SEED_LIBS)

%.o: %.c
	$(CC) -c $(LIBS) $(CFLAGS) $< -o $@

run-server: $(S_PROG)
	./$(S_PROG) $(PORT)

run-client: $(C_PROG)
	./$(C_PROG) $(HOST) $(PORT)

seed: $(SEED_PROG)
	./$(SEED_PROG)

clean:
	rm -f $(S_OBJS) $(S_PROG) $(C_OBJS) $(C_PROG) $(SEED_OBJS) $(SEED_PROG)
