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
#include "packet.h"
#include <string.h>


//Function to convert Celsius to Fahrenheit
float CtoF(float C) {
	return (C*9.0/5.0)+32;
}

//Struct for configuration file
typedef struct {
	char hostname[32];
	int nsensor;
	double lowval, highval;
	double low1, high1, low2, high2;
} config;

// Sets highest temperature value from all the readings
void set_highval(config *x) {
	// Disregardes number of sensors because default value == 0
	x->highval = (x->high1 > x->high2) ? x->high1 : x->high2;
}

// Sets lowest temperature value from all the readings
void set_lowval(config *x) {
	// Disregardes number of sensors because default value == 0
	x->lowval = (x->low1 < x->low2) ? x->low1 : x->low2;
}

// Sets hostname
void set_hostname(config *x) {
	x->hostname[31] = '\0';
	gethostname(x->hostname, 31);
}

// Initializes config struct values with values read from client.config file
void set_config(char *configName, config *ctemp) {
	//Fill config struct with info from config file
	FILE *config_file = fopen(configName, "rb");
	if (config_file == NULL) exit(EXIT_FAILURE);
	
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int first_line = 1;
	int sensor_counter = 0;
	while ((read = getline(&line, &len, config_file)) != -1) {
		strtok(line, "\n");
		
		// First line contains number of sensors
		if (first_line) {
			sscanf(line, "%d", &ctemp->nsensor);
			first_line = 0;
		
		// Following lines contain low and high values
		} else {
			
			// Case: Machine has no sensors
			if (ctemp->nsensor == 0) {
				ctemp->low1 = 0;
				ctemp->high1 = 0;
				ctemp->low2 = 0;
				ctemp->high2 = 0;
				ctemp->lowval = 0;
				ctemp->highval = 0;
			
			// Case: Machine has sensors
			} else {
				char *temp_readings;
				temp_readings = strtok(line, " ");
				
				// First sensor
				if (sensor_counter == 0) {
					ctemp->low1 = strtod(temp_readings,NULL);
					temp_readings = strtok(NULL, " ");
					ctemp->high1 = strtod(temp_readings,NULL);
					ctemp->low2 = 0;
					ctemp->high2 = 0;
					sensor_counter++;
				
				// Second sensor (if exists)
				} else if (sensor_counter == 1 && ctemp->nsensor == 2) {
					ctemp->low2 = strtod(temp_readings,NULL);
					temp_readings = strtok(NULL, " ");
					ctemp->high2 = strtod(temp_readings,NULL);
					sensor_counter++;
				}
			}
		}
	}
	// Cleanup
	fclose(config_file);
	if (line) free(line);
}




int main(int argc,char** argv) {

	char *fileName="/dev/gotemp";
	char *fileName2="/dev/gotemp2";
	char *configName = "/etc/t_client/client.conf";
	struct sensorInfo temp, temp2;
	config ctemp;
	int fd, fd2;

	// Set hostname
	set_hostname(&ctemp);
	
	// Reads config file and initializes values
	set_config(configName, &ctemp);
	
	// Sets lowest and highest value from all readings
	set_highval(&ctemp);
	set_lowval(&ctemp);
	
	// Print, check values
	printf("%d, %.1f, %.1f, %.1f, %.1f\n", ctemp.nsensor, ctemp.low1, ctemp.high1, ctemp.low2, ctemp.high2);
	printf("Low: %.1f\nHigh: %.1f\n", ctemp.lowval, ctemp.highval);
	printf("Hostname: %s\n", ctemp.hostname);
	
	
	//If there are no sensors, exit
	if(ctemp.nsensor == 0){
		exit;
	}
	
	/*

	//If there is at least one, open file for sensor 1
	if((fd=open(fileName,O_RDONLY))==-1) {
		fprintf(stderr,"Could not read %s\n",fileName);
		//    return 1;
	}

	//Read temp reading for sensor 1
	if(read(fd,&temp.data,sizeof(temp))!=8) {
		fprintf(stderr,"Error reading %s\n",fileName);
		//    return 1;
	}

	
	//Fill sensorInfo struct
	temp.hostName = //add hostname		//host name of sensor
	temp.numSens = ctemp.nsensor;		//number of sensors
	temp.sensorNum = 0;			//sensor number
	temp.lowval = ctemp.low1;		//low value of sensor
	temp.highval = ctemp.high1;		//high value of sensor
	temp.timestamp = //add timestamp		
	temp.action = 0;			//action requested
	
	
	//Send first sensor packet
	//Code for sending first packet to server//


	//Check if there is another sensor
	if(ctemp.nsensor == 1){
		close (fd);
		exit;
	}
		
	
	//If there is another sensor, do the same as above for sensor 2
	if((fd2=open(fileName2,O_RDONLY))==-1) {
		fprintf(stderr,"Could not read %s\n",fileName2);
		//    return 1;
	}


	if(read(fd2,&temp2.data,sizeof(temp))!=8) {
   		fprintf(stderr,"Error reading %s\n",fileName2);
		    return 1;
	}

	close(fd2);

	temp2.hostName = //add hostname		//host name of sensor
	temp2.numSens = ctemp.nsensor;		//number of sensors
	temp2.sensorNum = 1;			//sensor number
	temp2.lowval = ctemp.low2;		//low value of sensor
	temp2.highval = ctemp.high2;		//high value of sensor
	temp2.timestamp = //add timestamp		
	temp2.action = 0;			//action requested

	
	//Send second sensor packet
	//Cose for sending second packet to server//

	float conversion=0.0078125;
	printf("%3.2f %3.2f\n",
	(CtoF(((float)temp.data)*conversion)),
	(CtoF(((float)temp2.data)*conversion)));	
	
	*/
	exit;

}
	
