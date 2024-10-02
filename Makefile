DEPS = main.c \
	options.c

stuff: $(DEPS)
	gcc -g -Wall $(DEPS) -o stuff

clean:
	rm -f stuff
