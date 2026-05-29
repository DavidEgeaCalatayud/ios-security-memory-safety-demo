CC ?= gcc
CFLAGS ?= -std=c99 -O0 -Wall -Wextra -Iinclude
ASAN_FLAGS ?= -std=c99 -O0 -Wall -Wextra -fsanitize=address -Iinclude

SRC := src/main.c src/uaf_demo.c src/bootchain_model.c src/activation_lock_model.c
LIB_SRC := src/uaf_demo.c src/bootchain_model.c src/activation_lock_model.c
TARGET := demo
ASAN_TARGET := demo_asan
TEST_TARGET := smoke_test

.PHONY: all run clean asan test

all: $(TARGET)

$(TARGET): $(SRC) include/demo.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

asan:
	$(CC) $(ASAN_FLAGS) $(SRC) -o $(ASAN_TARGET)
	@echo "Running AddressSanitizer educational check..."
	ASAN_OPTIONS=halt_on_error=0:abort_on_error=0 ./$(ASAN_TARGET) --asan-trigger || true

test:
	$(CC) $(CFLAGS) tests/smoke_test.c $(LIB_SRC) -o $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(ASAN_TARGET) $(TEST_TARGET)
