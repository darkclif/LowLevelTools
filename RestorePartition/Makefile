all: restore

restore: restore.c
	gcc -o $@ $< -lncurses

.PHONY: clean
clean:
	rm *.o
