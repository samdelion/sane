MKDIR_P = mkdir -p
OUT_DIR = ./build
BIN_DIR = ./bin

.PHONY: dir all clean

all: dir sane

dir: ${OUT_DIR} ${BIN_DIR}

${OUT_DIR}:
	${MKDIR_P} ${OUT_DIR}

${BIN_DIR}:
	${MKDIR_P} ${BIN_DIR}

sane: dir token.o command.o sane.o main.c
	gcc ${OUT_DIR}/token.o ${OUT_DIR}/command.o ${OUT_DIR}/sane.o main.c -o ${BIN_DIR}/sane -std=c99

sane.o: dir sane.c sane.h
	gcc -c sane.c -std=c99 -o ${OUT_DIR}/sane.o

command.o: dir command.c command.h
	gcc -c command.c -std=c99 -o ${OUT_DIR}/command.o

token.o: dir token.c token.h
	gcc -c token.c -std=c99 -o ${OUT_DIR}/token.o

clean:
	rm ${OUT_DIR}/*.o
	rm ${BIN_DIR}/sane
