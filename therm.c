/*

	Kim Ngo & Dinh Do
	Networks: Project 2
	TCP Client
	therm.c

	therm.c is a client application that reads a configuration file, reads sensors that are connected to machines, and transfer the readings to a multi-threaded server.

	Referenced code from Jeff Sadowski <jeff.sadowski@gmail.com>
	with information gathered from David L. Vernier
	and Greg KH This Program is Under the terms of the 
	GPL http://www.gnu.org/copyleft/gpl.html

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
#include <time.h>	// Timestamp

char* GROUPNO = "g05";
int PORTNO = 9766;

// Timestamp function ~24 char
char * timestamp() {
	time_t ltime;				// Calendar time
	ltime = time(NULL);	// Current cal time
	char * ts = asctime(localtime(&ltime));
	strtok(ts, "\n");		// Remove carriage return
	return ts;
}

// Function to convert Celsius to Fahrenheit
double CtoF(double C) {
	return (C*9.0/5.0)+32;
}

// Struct for temperature reading
typedef struct {
	char hostname[32];	// Name of host with the sensors
	int nsensor;				// Number of sensors
	int sensor_num;			// Sensor number
	double low, high;	// Acceptable low/high value for this sensor
	double data;				// Sensor reading
	char timestamp[32];	// Timestamp
	int action;					// Action requested
	// action == 0: Send sensor data
	// action == 1: Request status
	// Sensor data fields in your struct should be set to zero for a “request status” packet.
} sensorInfo;

// Struct for reading temperature from sensors
typedef struct {
	unsigned char measurements;
	unsigned char counter;
	int16_t measurement0;
	int16_t measurement1;
	int16_t measurement2;
} tempReading;


// Sets hostname
void set_hostname(sensorInfo *x) {
	x->hostname[31] = '\0';
	gethostname(x->hostname, 31);
}

// Prints values of sensorInfo struct
void print_struct(const sensorInfo *sensor) {
	printf("\nFROM SERVER: Hostname: %s\nNumber of Sensors: %d\nSensor Number: %d\nLow: %.1f\nHigh: %.1f\nReading: %.1f\nTimestamp: %s\nAction: %d\n", sensor->hostname, sensor->nsensor, sensor->sensor_num, sensor->low, sensor->high, sensor->data, sensor->timestamp, sensor->action);
}


// Sends struct information to server
// Returns 0 if error occured
// Returns 1 if successful
int send_struct(const int sockfd, const sensorInfo *x) {
	int n;
	uint16_t len, len2;			// Length of message to be sent to server
	char buf[256];					// Concatenated struct into message str

	sprintf(buf, "%s,%d,%d,%.2f,%.2f,%.2f,%s,%d", x->hostname, x->nsensor, x->sensor_num, x->low, x->high, x->data, x->timestamp, x->action);
	
	len = strlen(buf);
	len2 = htons(len);

	if ((n = write(sockfd, &len2, sizeof(uint16_t))) < 0) {
		fprintf(stderr, "Error in message length writing to socket\n");
		return 0;
	}
 
	
	if ((n = write(sockfd, buf, len)) < 0) {
		fprintf(stderr, "Error writing message to socket\n");
		return 0;
	}
	
	//printf("%s\nMessage Length: %d\n", buf, len);
	return 1;
}


int main(int argc, char *argv[]) {
	
	// Requires hostname argument in command line
	if (argc != 2) {
		printf("usage: %s hostname\n", argv[0]);
		exit(0);
	}
	
	char *configName = "/etc/t_client/client.conf";
	char *errorFile = "/var/log/therm/error/g05_error_log";
	char *temp_file1="/dev/gotemp";
	char *temp_file2="/dev/gotemp2";
	sensorInfo sensor, sensor2;
	tempReading temp, temp2;
	struct stat buf, buf2;
	int fd, fd2;
	int nsensor = 0; // Number of sensors
	double CONVERSION = 0.0078125;
	
	//Open error log
	FILE *errorlog = fopen(errorFile, "a");
	if(errorlog == NULL) exit(EXIT_FAILURE);
	
	// Reads from client.config file and initializes appropriate number of packets
	FILE *config_file = fopen(configName, "rb");
	if (config_file == NULL) exit(EXIT_FAILURE);
	
	char *line = NULL;
	size_t len = 0;
	int first_line = 1;
	int sensor_counter = 0;
	while ((getline(&line, &len, config_file)) != -1) {
		strtok(line, "\n");
		
		// First line contains number of sensors
		if (first_line) {
			sscanf(line, "%d", &nsensor);
			first_line = 0;
		
		// Following lines contain low and high values
		} else {
			
			// Case: Machine has no sensors
			if (nsensor == 0) {
				break;
			
			// Case: Machine has sensors
			} else {
				char *range_readings;
				range_readings = strtok(line, " ");
				
				// Initialize first sensor values
				if (sensor_counter == 0) {
					set_hostname(&sensor);	// Set hostname
					sensor.nsensor = nsensor;
					sensor.sensor_num = 0;	// /dev/gotemp
					sensor.low = strtod(range_readings,NULL);
					range_readings = strtok(NULL, " ");
					sensor.high = strtod(range_readings,NULL);
					sensor_counter++;
				
				// Second sensor (if exists)
				} else if (sensor_counter == 1 && nsensor == 2) {
					set_hostname(&sensor2);	// Set hostname
					sensor2.nsensor = nsensor;
					sensor2.sensor_num = 1;	// /dev/gotemp2
					sensor2.low = strtod(range_readings,NULL);
					range_readings = strtok(NULL, " ");
					sensor2.high = strtod(range_readings,NULL);
					sensor_counter++;
				}
			}
		}
	}
	// Cleanup
	fclose(config_file);
	if (line) free(line);

	
	// If there are no sensors
	if (nsensor == 0){
		// Do something?
	
	// If there are sensors
	} else {
		if (stat(temp_file1, &buf)) {
	
			if (mknod(temp_file1, S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP |S_IWGRP|S_IROTH|S_IWOTH, makedev(180,176))) {
				fprintf(stderr,"Cannot create device %s need to be root", temp_file1); 
				fprintf(errorlog, "Cannot create device %s need to be root\n", temp_file1); 
				return 1;
			}
		}
	
		
		// If there is at least one, open file for sensor 1
		if((fd = open(temp_file1, O_RDONLY)) == -1) {
			fprintf(stderr,"Could not read %s\n", temp_file1);
			fprintf(errorlog, "Could not read %s\n", temp_file1);
			return 1;
		}
		
		// Read temp reading for sensor 1
		if(read(fd, &temp, sizeof(temp)) != 8) {
			fprintf(stderr,"Error reading %s\n", temp_file1);
			fprintf(errorlog, "Error reading %s\n", temp_file1);
			return 1;
		}
	
		// Fill sensor1 info
		sensor.data = CtoF(temp.measurement0 * CONVERSION);
		strcpy(sensor.timestamp, timestamp());			// add timestamp	
		sensor.action = 0;								// action requested
	
		// Print, check values
		print_struct(&sensor);
		close (fd);
		
		
		// Check if there is another sensor
		if(nsensor == 2){
			if (stat(temp_file2, &buf2)) {
				if (mknod(temp_file1, S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP |S_IWGRP|S_IROTH|S_IWOTH,makedev(180,176))) {
					fprintf(stderr,"Cannot create device %s need to be root", temp_file2);
					fprintf(errorlog, "Cannot create device %s need to be root\n", temp_file2);
					return 1;
				}
			}
	
			// If there is another sensor, do the same as above for sensor 2
			if((fd2 = open(temp_file2, O_RDONLY)) == -1) {
				fprintf(stderr,"Could not read %s\n", temp_file2);
				fprintf(errorlog, "Could not read %s\n", temp_file2);
				return 1;
			}
	
			// Read temp reading for sensor 2
			if(read(fd2, &temp2, sizeof(temp2)) != 8) {
				fprintf(stderr,"Error reading %s\n", temp_file2);
				fprintf(errorlog, "Error reading %s\n", temp_file2);
				return 1;
			}
	
			// Fill sensorInfo struct for sensor2
			sensor2.data = CtoF(temp2.measurement0 * CONVERSION);
			strcpy(sensor2.timestamp, timestamp());		// add timestamp		
			sensor2.action = 0;												// action requested
			
			// Print, check values
			print_struct(&sensor2);
			close(fd2);
		}
	}




	// TCP Client portion
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];
	
	portno = PORTNO;		// port number
	server = gethostbyname(argv[1]);		// insert hostname here
	
	// Openning socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "Error openning socket\n");
		fprintf(errorlog, "Error opening socket\n");
	}
	
	// Server information
	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
	
	// Connect to port
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "Error connecting\n");
		fprintf(errorlog, "Error connecting\n");
		exit(0);
	}

	// Sends sensorInfo struct to server
	if (nsensor > 0) send_struct(sockfd, &sensor);
	
	// Sends sensorInfo struct 2 to server
	if (nsensor == 2) send_struct(sockfd, &sensor2);

	fclose(errorlog);

	exit;
}
	
