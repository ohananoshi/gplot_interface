CC      ?= cc
CFLAGS  ?= -std=c99 -Wall -Wextra -Wpedantic -Werror -g
SAN     ?= -fsanitize=address,undefined -fno-omit-frame-pointer
BUILD   := build

.PHONY: all test examples clean

all: test examples

$(BUILD):
	mkdir -p $(BUILD)

# script-level tests; gnuplot is not needed
$(BUILD)/test_gplot: tests/test_gplot.c tests/second_tu.c gplot_interface.h | $(BUILD)
	$(CC) $(CFLAGS) $(SAN) tests/test_gplot.c tests/second_tu.c -o $@ -lm

test: $(BUILD)/test_gplot
	./$(BUILD)/test_gplot

# examples need gnuplot; images are written to build/
$(BUILD)/%: examples/%.c gplot_interface.h | $(BUILD)
	$(CC) $(CFLAGS) $< -o $@ -lm

examples: $(BUILD)/lines $(BUILD)/scatter
	cd $(BUILD) && ./lines && ./scatter

clean:
	rm -rf $(BUILD)
