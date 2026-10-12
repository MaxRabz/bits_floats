CC ?= cc
PYTHON ?= python3

CPPFLAGS := -Iinclude
CFLAGS := -std=c11 -Wall -Wextra -Werror -pedantic -O0 -g
LDLIBS := -lm

BUILD_DIR := build
STUDENT_TEST := $(BUILD_DIR)/test_assignment
REFERENCE_TEST := $(BUILD_DIR)/test_reference

.PHONY: all test test-1 test-2 test-3 test-4 test-5 test-6 test-7 test-8 test-9
.PHONY: test-asan valgrind verify-reference clean

all: $(STUDENT_TEST)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(STUDENT_TEST): tests/test_assignment.c src/assignment.c include/assignment.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_assignment.c src/assignment.c -o $@ $(LDLIBS)

$(REFERENCE_TEST): tests/test_assignment.c instructor/reference_solution.c include/assignment.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_assignment.c instructor/reference_solution.c -o $@ $(LDLIBS)

test: $(STUDENT_TEST)
	./$(STUDENT_TEST) all
	$(PYTHON) tests/check_restrictions.py src/assignment.c

test-1: $(STUDENT_TEST)
	./$(STUDENT_TEST) 1

test-2: $(STUDENT_TEST)
	./$(STUDENT_TEST) 2

test-3: $(STUDENT_TEST)
	./$(STUDENT_TEST) 3

test-4: $(STUDENT_TEST)
	./$(STUDENT_TEST) 4

test-5: $(STUDENT_TEST)
	./$(STUDENT_TEST) 5

test-6: $(STUDENT_TEST)
	./$(STUDENT_TEST) 6
	$(PYTHON) tests/check_restrictions.py src/assignment.c

test-7: $(STUDENT_TEST)
	./$(STUDENT_TEST) 7

test-8: $(STUDENT_TEST)
	./$(STUDENT_TEST) 8

test-9: $(STUDENT_TEST)
	./$(STUDENT_TEST) 9

test-asan:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fsanitize=address,undefined \
		tests/test_assignment.c src/assignment.c \
		-o $(BUILD_DIR)/test_assignment_asan $(LDLIBS)
	./$(BUILD_DIR)/test_assignment_asan all

valgrind: $(STUDENT_TEST)
	valgrind --leak-check=full --show-leak-kinds=all \
		--errors-for-leak-kinds=all --error-exitcode=1 \
		./$(STUDENT_TEST) all

verify-reference: $(REFERENCE_TEST)
	./$(REFERENCE_TEST) all
	$(PYTHON) tests/check_restrictions.py instructor/reference_solution.c

clean:
	rm -rf $(BUILD_DIR)
