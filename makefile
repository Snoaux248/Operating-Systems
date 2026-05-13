CC = gcc

# Compiler lags //unecessary compiler flags // -Wall: warnings -g: debug info
CFLAGS = -Wall -g -lcurl -lpthread -O3

# Source files
SRCS = api.c threads.c main.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = scheduler_os

# Default target
all: $(TARGET)

# Linking the executable // $@ sourcefile $^ all files/prerequesites
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compiling .c to .o //$< sourcefile $@ generated file name
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up // build clean
clean:
	rm -f $(OBJS) $(TARGET)
	


