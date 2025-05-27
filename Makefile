DEPS = main.c \
	options/hidden.c \
	options/none.c \
	options/init.c \
	options/link.c \
	options/list.c \
	options/unlink.c \
	command.c

stuff: $(DEPS)
	gcc -g -Wall $(DEPS) -o stuff

clean:
	rm -f stuff
