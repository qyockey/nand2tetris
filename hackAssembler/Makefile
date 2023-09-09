CC = gcc
CFLAGS = -Wall -Wpedantic -Werror -I.
SRCS = Main.c MachineCode.c Parser.c SymbolTable.c
OBJS = $(SRCS:.c=.o)
TARGET = ../../HackAssembler

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
