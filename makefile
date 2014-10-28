all: therm thermd

therm: therm.o
	gcc therm.o -o therm

therm.o: therm.c
	gcc -c therm.c

thermd: thermd.o
	gcc thermd.o -o thermd -lpthread

thermd.o: thermd.c
	gcc -c thermd.c -lpthread

clean:
	rm -f *.o therm thermd *~
