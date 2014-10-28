/*	

	Kim Ngo & Dinh Do
	Networks: Project 2
	TCP Server
	thermd.c
	
	thermd.c is a multithreaded server application that processes requests from clients and loops around to handle future requests. It reads data from the client, parses it and writes it to a logfile.

*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>







