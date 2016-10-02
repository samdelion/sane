c_exc2: token.o command.o sane.o c_exc2.c
	gcc token.o command.o sane.o c_exc2.c -o c_exc2 -std=c99

sane.o: sane.c sane.h
	gcc -c sane.c -std=c99 -o sane.o

command.o: command.c command.h
	gcc -c command.c -std=c99 -o command.o

token.o: token.c token.h
	gcc -c token.c -std=c99 -o token.o

clean:
	rm *.o
	rm c_exc2
