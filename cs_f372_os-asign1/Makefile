CC = gcc
CFLAGS = -Wall -Wextra
RM = rm -f
LDFLAGS = -pthread

ifeq ($(OS),Windows_NT)
    RM = del /Q
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDFLAGS = -lpthread
endif
ifeq ($(UNAME_S),Darwin)
endif

all: calculator

calculator: multi_thread_calc.c header.h
	$(CC) $(CFLAGS) -o calculator multi_thread_calc.c $(LDFLAGS)

clean:
	$(RM) calculator