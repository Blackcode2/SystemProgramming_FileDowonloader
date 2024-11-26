CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lncurses
TARGET = mpget
SRCS = mpget.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run-files1: $(TARGET)
	./$(TARGET) files1.txt

run-files2: $(TARGET)
	./$(TARGET) files2.txt