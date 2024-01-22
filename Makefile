CC=gcc

.PHONY: all windows linux clean

# Default rule
all:
	@echo "Specify 'windows' or 'linux' to compile for the respective platform."

# Windows compilation rule
windows:
	@echo "Compiling for Windows..."
	$(CC) -o ipupdown.exe ipupdown_win.c common.c -lws2_32 -liphlpapi

# Linux compilation rule
linux:
	@echo "Compiling for Linux..."
	$(CC) -o ipupdown ipupdown_linux.c common.c

clean:
	-del /Q ipupdown.exe 2> NUL
	-del /Q *.o > 2> NUL
	-rm -f ipupdown $(wildcard *.o)
