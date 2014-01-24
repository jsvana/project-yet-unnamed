CC=clang

CFLAGS=-g
LIBS=-lpthread

MQUEUE_TEST_PROG=__tests__/mqueue-test
MQUEUE_TEST_OBJS=__tests__/mqueue_test.o mqueue.o

all: $(MQUEUE_TEST_PROG)

$(MQUEUE_TEST_PROG): $(MQUEUE_TEST_OBJS)
	$(CC) -o $(MQUEUE_TEST_PROG) $(MQUEUE_TEST_OBJS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(LIBS)

test: $(MQUEUE_TEST_PROG)
	./$(MQUEUE_TEST_PROG)

clean:
	rm -f $(MQUEUE_TEST_PROG) $(MQUEUE_TEST_OBJS)
