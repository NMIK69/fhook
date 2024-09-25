CC=gcc
CFLAGS= -Wall -pedantic -std=c99 -Wextra -Wmissing-prototypes
OPTIMIZE=-O0

TARGET=example

all: $(TARGET)

$(TARGET) : ../fhook.c example.c
	$(CC) $(CFLAGS) $(OPTIMIZE) $^ -o $@

.PHONY : clean
clean :
	rm -f $(TARGET)
