# Makefile for libcaesar cryptographic library
# Targets: all, install, test

CC = g++
CFLAGS = -Wall -Wextra -pedantic -fPIC -std=c++17
LDFLAGS = -shared

# Library name and paths
LIB_NAME = libcaesar.so
LIB_SOURCE = libcaesar.cpp
LIB_OBJECT = libcaesar.o

# Install directory
INSTALL_DIR = /usr/local/lib

# Test program
TEST_PROG = test_caesar
TEST_SOURCE = test_caesar.cpp
TEST_OBJECT = test_caesar.o

# Test data
TEST_INPUT = input.txt
TEST_OUTPUT = output.txt
TEST_DECRYPTED = decrypted.txt

# Ensure prepared test data exists (can be deleted accidentally)
$(TEST_INPUT):
	@printf "Hello, XOR library test!\nThis file will be encrypted and decrypted.\nIf you decrypt with the same key, you get the same text back.\n" > $(TEST_INPUT)

# Default target
all: $(LIB_NAME)

# Compile library object file
$(LIB_OBJECT): $(LIB_SOURCE)
	$(CC) $(CFLAGS) -c $< -o $@

# Link library
$(LIB_NAME): $(LIB_OBJECT)
	$(CC) $(LDFLAGS) $< -o $@
	@echo "Successfully built $(LIB_NAME)"

# Compile test program
$(TEST_OBJECT): $(TEST_SOURCE)
	$(CC) $(CFLAGS) -c $< -o $@

# Link test program
$(TEST_PROG): $(TEST_OBJECT)
	$(CC) -o $@ $< -ldl
	@echo "Successfully built $(TEST_PROG)"

# Install library to system directory
install: $(LIB_NAME)
	@if [ ! -w $(INSTALL_DIR) ]; then \
		echo "Installing to $(INSTALL_DIR) requires sudo"; \
		sudo cp $(LIB_NAME) $(INSTALL_DIR)/; \
		(sudo /sbin/ldconfig 2>/dev/null || sudo ldconfig); \
	else \
		cp $(LIB_NAME) $(INSTALL_DIR)/; \
		(/sbin/ldconfig 2>/dev/null || ldconfig); \
	fi
	@echo "Library installed to $(INSTALL_DIR)"

# Test target: compile and run the test program
test: $(LIB_NAME) $(TEST_PROG) $(TEST_INPUT)
	@echo "=== Running encryption test ==="
	./$(TEST_PROG) ./$(LIB_NAME) 'K' $(TEST_INPUT) $(TEST_OUTPUT)
	@echo "Encryption complete. Output: $(TEST_OUTPUT)"
	@echo ""
	@echo "=== Running decryption test ==="
	./$(TEST_PROG) ./$(LIB_NAME) 'K' $(TEST_OUTPUT) $(TEST_DECRYPTED)
	@echo "Decryption complete. Output: $(TEST_DECRYPTED)"
	@echo ""
	@echo "=== Verifying data integrity ==="
	@if cmp -s $(TEST_INPUT) $(TEST_DECRYPTED); then \
		echo "SUCCESS: Original and decrypted data are identical!"; \
		echo "Double XOR property verified: A XOR K XOR K = A"; \
	else \
		echo "ERROR: Data mismatch!"; \
		diff $(TEST_INPUT) $(TEST_DECRYPTED); \
		exit 1; \
	fi

# Clean up generated files
clean:
	rm -f $(LIB_OBJECT) $(LIB_NAME)
	rm -f $(TEST_OBJECT) $(TEST_PROG)
	rm -f $(TEST_OUTPUT) $(TEST_DECRYPTED)
	@echo "Cleanup complete"

# Display help
help:
	@echo "Available targets:"
	@echo "  make all    - Build the libcaesar.so library (default)"
	@echo "  make install - Install library to /usr/local/lib"
	@echo "  make test   - Build and run test program"
	@echo "  make clean  - Remove generated files"
	@echo "  make help   - Display this help message"

.PHONY: all install test clean help
