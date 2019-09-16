FLAGS = -std=c11 -Wpedantic -g -D_XOPEN_SOURCE=700 -Wall -Werror
CC = gcc

.PHONY: all clean
all: download

clean:
	rm -f download downloader.o
download: downloader.o
	$(CC) $(FLAGS) -o download downloader.o -lm
downloader.o: downloader.c
	$(CC) $(FLAGS) -c downloader.c -lm
