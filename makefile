CC=gcc
CFLAGS=-std=c99 -g -Wall 
GTKFLAGS=`pkg-config --cflags --libs gtk+-3.0`
GTKLIBS=`pkg-config --cflags --libs gtk+-3.0`

SOURCE = gacal.c
EXEC = gacal

$(EXEC): $(SOURCE)
	$(CC) $(SOURCE) -o $(EXEC) $(EXECUTABLE) $(CFLAGS) $(GTKFLAGS)

.PHONY: clean
clean:
	rm -f gacal
