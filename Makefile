MEINEFLAGS = -std=c11 -Wpedantic -g -D_XOPEN_SOURCE=700 -Wall -Werror

.PHONY: all clean
all: download

clean:
	rm -f download
download: downloader.o
	gcc $(MEINEFLAGS) -o download downloader.o -lm
downloader.o: downloader.c
	gcc $(MEINEFLAGS) -c downloader.c -lm
