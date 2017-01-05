CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -pedantic -O3
# CFLAGS=-std=c99 -Wall -Wextra -pedantic -g
GTKFLAGS=`pkg-config --cflags --libs gtk+-3.0`
GTKLIBS=`pkg-config --cflags --libs gtk+-3.0`

SOURCE = main.c gacal.c
EXEC = gacal

$(EXEC): $(SOURCE)
	$(CC) $(SOURCE) -o $(EXEC) $(EXECUTABLE) $(CFLAGS) $(GTKFLAGS) -I ./

.PHONY: clean
clean:
	rm -f gacal
