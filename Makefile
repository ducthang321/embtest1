CC = gcc
CFLAGS = -Wall -g -O2
LDFLAGS = -lm -pthread
TARGET = rootfinder
SOURCES = main.c postfix.c findroot.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = postfix.h findroot.h

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
