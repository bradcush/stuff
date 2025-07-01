DEPS = main.c \
	options/hidden.c \
	options/none.c \
	options/link.c \
	options/list.c \
	options/unlink.c \
	command.c

stuff: $(DEPS)
	gcc -g -Wall -Wextra $(DEPS) -o usr/local/bin/stuff

clean:
	rm -f usr/local/bin/stuff

test:
	cd ./tests/project && ../run.sh
