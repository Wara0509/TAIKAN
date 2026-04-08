CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -Iinclude
TARGET = taikan
SRCS = src/main.c src/logic.c src/ui_cli.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
