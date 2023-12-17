CC = gcc
CFLAGS = -Wall -Wpedantic -I. -g -O0
SRCS = main.c asm_writer.c error_check.c parser.c linked_list.c hash_table.c
OBJS = $(SRCS:.c=.o)
TARGET = ../../vm_translator

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

