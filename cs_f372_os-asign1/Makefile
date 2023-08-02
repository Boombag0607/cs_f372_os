CC = gcc
CFLAGS = -Wall -Wextra

all: calculator

calculator: multi_thread_calc.c header.h
	$(CC) $(CFLAGS) -o calculator multi_thread_calc.c -Iinclude -pthread

clean:
	rm -f calculator