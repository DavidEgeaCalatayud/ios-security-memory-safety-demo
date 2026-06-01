CC ?= gcc
CFLAGS ?= -std=c99 -O0 -Wall -Wextra -Iinclude
STRICT_FLAGS ?= -std=c99 -O0 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror -Wno-error=use-after-free -Iinclude
ASAN_FLAGS ?= -std=c99 -O0 -Wall -Wextra -fsanitize=address -Iinclude
UBSAN_FLAGS ?= -std=c99 -O0 -Wall -Wextra -fsanitize=undefined -Iinclude

SRC := src/main.c src/uaf_demo.c src/bootchain_model.c src/activation_lock_model.c
LIB_SRC := src/uaf_demo.c src/bootchain_model.c src/activation_lock_model.c
TARGET := demo
STRICT_TARGET := demo_strict
ASAN_TARGET := demo_asan
UBSAN_TARGET := demo_ubsan
TEST_TARGET := unit_tests
ASAN_LOG := asan_output.log

.PHONY: all run clean asan ubsan strict test

all: $(TARGET)

$(TARGET): $(SRC) include/demo.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

strict:
	$(CC) $(STRICT_FLAGS) $(SRC) -o $(STRICT_TARGET)

asan:
	$(CC) $(ASAN_FLAGS) $(SRC) -o $(ASAN_TARGET)
	@echo "Running AddressSanitizer educational check..."
	@set +e; \
	ASAN_OPTIONS=halt_on_error=0:abort_on_error=0 ./$(ASAN_TARGET) --asan-trigger > $(ASAN_LOG) 2>&1; \
	if grep -Eq "AddressSanitizer:.*heap-use-after-free" $(ASAN_LOG); then \
		echo "ASan correctly detected heap-use-after-free."; \
		rm -f $(ASAN_LOG); \
		exit 0; \
	else \
		echo "ASan did not detect the expected heap-use-after-free."; \
		cat $(ASAN_LOG); \
		rm -f $(ASAN_LOG); \
		exit 1; \
	fi

ubsan:
	$(CC) $(UBSAN_FLAGS) $(SRC) -o $(UBSAN_TARGET)
	./$(UBSAN_TARGET)

test:
	$(CC) $(CFLAGS) tests/unit_tests.c $(LIB_SRC) -o $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(STRICT_TARGET) $(ASAN_TARGET) $(UBSAN_TARGET) $(TEST_TARGET) $(ASAN_LOG)
