CC ?= gcc
CFLAGS ?= -std=c99 -O0 -Wall -Wextra -Iinclude
ASAN_FLAGS ?= -std=c99 -O0 -Wall -Wextra -fsanitize=address -Iinclude

SRC := src/main.c src/uaf_demo.c src/bootchain_model.c src/activation_lock_model.c
LIB_SRC := src/uaf_demo.c src/bootchain_model.c src/activation_lock_model.c
TARGET := demo
ASAN_TARGET := demo_asan
TEST_TARGET := unit_tests
ASAN_LOG := asan_output.log

.PHONY: all run clean asan test

all: $(TARGET)

$(TARGET): $(SRC) include/demo.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

asan:
	$(CC) $(ASAN_FLAGS) $(SRC) -o $(ASAN_TARGET)
	@echo "Running AddressSanitizer educational check..."
	@set +e; \
	ASAN_OPTIONS=halt_on_error=0:abort_on_error=0 ./$(ASAN_TARGET) --asan-trigger > $(ASAN_LOG) 2>&1; \
	status=$$?; \
	if grep -q "ERROR: AddressSanitizer: heap-use-after-free" $(ASAN_LOG); then \
		echo "ASan correctly detected heap-use-after-free."; \
		rm -f $(ASAN_LOG); \
		exit 0; \
	else \
		echo "ASan did not detect the expected heap-use-after-free."; \
		cat $(ASAN_LOG); \
		rm -f $(ASAN_LOG); \
		exit 1; \
	fi

test:
	$(CC) $(CFLAGS) tests/unit_tests.c $(LIB_SRC) -o $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(ASAN_TARGET) $(TEST_TARGET) $(ASAN_LOG)
