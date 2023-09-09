CC = gcc
CFLAGS = -Wall -Wpedantic -Werror -I.
SRCS = Main.c AssemblyGenerator.c Parser.c
OBJS = $(SRCS:.c=.o)
TARGET = ../../VmEmulator

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
