all: thermd

thermd: thermd.o
	gcc thermd.o -o thermd -lpthread

thermd.o: thermd.c
	gcc -c thermd.c -lpthread

clean:
	rm -f *.o thermd *~
