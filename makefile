CC = clang
CFLAGS = -Wall -Werror -Wextra -pedantic --std=c11 -g -fsanitize=address

bfi: bfi.c
	$(CC) $(CFLAGS) -o $@ bfi.c
