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
#include <pthread.h>

char* GROUPNO = "g05";
char* PORTNO = "9766";

// Struct for temperature reading
typedef struct {
	char hostname[32];	// Name of host with the sensors
	int nsensor;				// Number of sensors
	int sensor_num;			// Sensor number
	double low, high;		// Acceptable low/high value for this sensor
	double data;				// Sensor reading
	char timestamp[32];	// Timestamp
	int action;					// Action requested
	// action == 0: Send sensor data
	// action == 1: Request status
	// Sensor data fields in your struct should be set to zero for a “request status” packet.
} sensorInfo;


// Fills sensorInfo struct by parsing buffer with , delimiter
void fill_struct(sensorInfo *x, char buffer[]) {
	char *structfill;
	structfill = strtok(buffer, ",");
	strcpy(x->hostname, structfill);
	structfill = strtok (NULL, ",");
	x->nsensor = atoi(structfill);
	structfill = strtok (NULL, ",");
	x->sensor_num = atoi(structfill);
	structfill = strtok(NULL, ",");
	x->low = strtod(structfill, NULL);
	structfill = strtok(NULL, ",");
	x->high = strtod(structfill, NULL);
	structfill = strtok(NULL, ",");
	x->data = strtod(structfill, NULL);
	structfill = strtok(NULL, ",");
	strcpy(x->timestamp, structfill);
	structfill = strtok(NULL, ",");
	x->action = atoi(structfill);
}

// Prints values of sensorInfo struct
void print_struct(const sensorInfo *sensor) {
	printf("\nFROM SERVER: Hostname: %s\nNumber of Sensors: %d\nSensor Number: %d\nLow: %.1f\nHigh: %.1f\nReading: %.1f\nTimestamp: %s\nAction: %d\n", sensor->hostname, sensor->nsensor, sensor->sensor_num, sensor->low, sensor->high, sensor->data, sensor->timestamp, sensor->action);
}

// Function to receive data from client and fill struct
void * rec_struct (void * param){
	
	int r = *((int *) param);
	sensorInfo sensor, sensor2;	
	int rbyte;
	char buffer[256];
	char *tok, *tok2;
	uint16_t datasize;
	char *logfile;

	// Read data size from client
	if((rbyte = read(r, &datasize, sizeof(uint16_t))) < 0){
		fprintf(stderr, "Error in datasize read\n");
		return 0;
	}

	datasize = ntohs(datasize);
	bzero(buffer, datasize);	// Clean buffer
	
	//read data from client
	if((rbyte = read(r, buffer, datasize)) < 0){
		fprintf(stderr, "Error in data read\n");
		return 0;
	}
	buffer[datasize] = 0;		// Close string
	

	//Parse through data and fill first struct
	fill_struct(&sensor, buffer);
	print_struct(&sensor);

	//If action == 0, append data to logfile
	/*if(sensor.action == 0){
		char *datetok = strtok (sensor.timestamp, " ");
		sprintf(logfile, "/var/log/therm/temp_logs/g05_"
		
	*/

	
	// Prepare to receive from client if more than 1 sensor
	if(sensor.nsensor == 2){
		
		//read data size from client
		if((rbyte = read(r, &datasize, sizeof(uint16_t))) < 0){
			fprintf(stderr, "Error in datasize read\n");
			return 0;
		}

		datasize = ntohs(datasize);
		bzero(buffer, datasize);	// Clean buffer
	
		//read data from client
		if((rbyte = read(r, buffer, datasize)) < 0){
			fprintf(stderr, "Error in data read\n");
			return 0;
		}
		buffer[datasize] = 0;	// Close string

		//Parse through data and fill second struct
		fill_struct(&sensor2, buffer);
		print_struct(&sensor2);
	}

	close(r);
}
		
	


int main (int argc, char *argv[]){

	int r, s;
	char *portno = PORTNO;
	struct addrinfo hint, *sinfo, *t;
	struct sockaddr_storage servaddr;

	memset (&hint, 0, sizeof(struct addrinfo));
	hint.ai_family = AF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_flags = AI_PASSIVE;

	if((r = getaddrinfo(NULL, portno, &hint, &sinfo)) != 0){
		perror("Error in getaddrinfo\n");
		return 1;
	}
		
	for (t = sinfo; t != NULL; t = t->ai_next){
		if((s = socket(t->ai_family, t->ai_socktype, t->ai_protocol)) == -1){
			perror("server:socket error\n");
			continue;
		}

		int opt = 1;
		if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int)) == -1){
			perror("Setsockopt error\n");
			exit(1);
		}
	
		if(bind(s, t->ai_addr, t->ai_addrlen) == -1){
			close (s);
			perror ("server: bind error\n");
			continue;
		}

		break;
	}

	
	//Check if bind was successful
	if (t == NULL){
		fprintf(stderr, "Error in bind\n");
		return 1;
	}

	freeaddrinfo(sinfo);

	//Listen for clients
	while (listen(s, 5) > -1) {

		socklen_t sinlen = sizeof(struct sockaddr_storage);

		//Create new socket
		if((r = accept(s, (struct sockaddr *)&servaddr, &sinlen))< 0){		
			fprintf(stderr, "Error accepting\n");
			return 1;
		}

		
		//Create new thread & run rec_struct function
		pthread_t id;
		if(pthread_create(&id, NULL, rec_struct, &r) < 0){
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}
		
	}

	

	
	return 0;
}









