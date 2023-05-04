SOURCE = $(wildcard ./src/comm/tcp/*.c ./src/protocol/json/*.c ./src/utils/*.c ./example/main.c)
OBJECT = $(patsubst %.c, %.o, $(SOURCE))

INCLUEDS = -I ./include

dep_files := $(patsubst %.o, %.o.d, $(OBJECT))
dep_files := $(wildcard $(dep_files))

TARGET  = example
CC      = gcc
CFLAGS  = -Wall -g

$(TARGET): $(OBJECT)
	@mkdir -p output/
	$(CC) $^ $(CFLAGES) -lpthread -o output/$(TARGET)

%.o: %.c
	$(CC) $(INCLUEDS) $(CFLAGES) -c -o $@ $< -MD -MF $@.d

ifneq ($(dep_files),)
  include $(dep_files)
endif

.PHONY:clean
clean:
	@rm -rf $(OBJECT) output/

