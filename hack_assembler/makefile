CC = gcc
CFLAGS = -Wall -Wpedantic -Werror -I.
SRCS = main.c machine_code.c parser.c symbol_table.c
OBJS = $(SRCS:.c=.o)
TARGET = ../../hack_assembler

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

