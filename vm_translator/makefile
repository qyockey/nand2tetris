CC = gcc
CFLAGS = -Wall -Wpedantic -Werror -I. -g
SRCS = main.c asm_generator.c parser.c
OBJS = $(SRCS:.c=.o)
TARGET = ../../vm_translator

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

