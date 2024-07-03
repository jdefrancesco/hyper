# Define the compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -framework OpenGL -framework GLUT

# Define the target executable
TARGET = hyper

# Define the source files
SRCS = hyper.c

# Define the object files
OBJS = $(SRCS:.c=.o)

# Default rule to build the target
all: $(TARGET)

# Rule to link the object files into the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets to avoid conflicts with files named 'all' or 'clean'
.PHONY: all clean
