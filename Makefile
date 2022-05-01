#------------Makefile--------

CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
EXECUTABLE = proj2


$(EXECUTABLE): proj2.o
	$(CC) $(CFLAGS) -pthread -lrt $^ -o $@


proj2.o: proj2.c proj2.h
	$(CC) $(CFLAGS) -pthread -lrt -c $< -o $@

clean:
	rm -f *.o $(EXECUTABLE)