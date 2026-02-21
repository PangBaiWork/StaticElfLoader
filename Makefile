CC      ?= gcc
CFLAGS  = -Wall -Wextra -g -pie -fPIE
TARGET  = loader
OBJS    = main.o loader.o launch.o
CMD     ?= ./test/go/test_static hello
all: $(TARGET) run

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c loader.h
	$(CC) $(CFLAGS) -c $< -o $@

loader.o: loader.c loader.h
	$(CC) $(CFLAGS) -c $< -o $@

launch.o: launch.S
	$(CC) -c $< -o $@

run: $(TARGET)
	./$(TARGET) $(CMD)
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
