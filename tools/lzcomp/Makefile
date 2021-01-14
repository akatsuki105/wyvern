CC := gcc
CFLAGS := -O3 -std=c99 -Wall -Wextra -Wno-missing-field-initializers

lzcomp: CFLAGS = -O3 -flto -std=c11 -Wall -Wextra -pedantic -Wno-strict-overflow -Wno-sign-compare
lzcomp: $(wildcard *.c) $(wildcard *.h)
	$(CC) $(CFLAGS) -o $@ *.c

%: %.c
	$(CC) $(CFLAGS) -o $@ $<
