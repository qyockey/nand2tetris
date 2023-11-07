CC = gcc
CFLAGS = -Wall -Wpedantic -Werror -I. -g
SRCS = main.c assembly_generator.c parser.c
OBJS = $(SRCS:.c=.o)
TARGET = ../../vm_emulator

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

