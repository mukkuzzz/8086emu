TARGET = myprogram
CC = gcc
CFLAGS = -Wall -Werror
SRC = main.c
OBJ = $(SRC:.c=.o)
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ)
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean

