DEPS = main.c \
	options/hidden.c \
	options/none.c \
	options/list.c \
	command.c

stuff: $(DEPS)
	gcc -g -Wall $(DEPS) -o stuff

clean:
	rm -f stuff
