CC = gcc

# Compiler lags //unecessary compiler flags // -Wall: warnings -g: debug info
CFLAGS = -Wall -g 

# Source files
SRCS = Schedulers.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = assignment_5

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
	


