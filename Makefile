DEPS = main.c \
	options/hidden.c \
	options/none.c \
	options/link.c \
	options/list.c \
	options/unlink.c \
	command.c

BUILD_DIR = usr/local/bin

stuff: $(DEPS)
	mkdir -p ${BUILD_DIR} && \
	gcc -g -Wall -Wextra $(DEPS) -o ${BUILD_DIR}/stuff

clean:
	rm -f ${BUILD_DIR}/stuff

test:
	cd ./tests/project && ../run.sh
