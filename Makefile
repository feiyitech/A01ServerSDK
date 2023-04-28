SOURCE = $(wildcard ./src/comm/tcp/*.c ./src/protocol/json/*.c ./src/utils/*.c ./example/main.c)
OBJECT = $(patsubst %.c, %.o, $(SOURCE))

INCLUEDS = -I ./include

TARGET  = example
CC      = gcc
CFLAGS  = -Wall -g

$(TARGET): $(OBJECT)
	@mkdir -p output/
	$(CC) $^ $(CFLAGES) -lpthread -o output/$(TARGET)

%.o: %.c
	$(CC) $(INCLUEDS) $(CFLAGES) -c $< -o $@

.PHONY:clean
clean:
	@rm -rf $(OBJECT) output/

