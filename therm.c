// Kim Ngo & Dinh Do
// Networks: Project 2
// TCP Client
// therm.c

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


//Function to convert Celsius to Fahrenheit
float CtoF(float C){return (C*9.0/5.0)+32;}

//Struct for configuration file
struct config{
	int nsensor = 0;
	double lowval = 0, highval = 0;
	double low1 = 0, high1 = 0, low2 = 0, high2 = 0;
};

int main(int argc,char** argv) {
	char *fileName="/dev/gotemp";
	char *fileName2="/dev/gotemp2";
	char *configName = "/etc/t_client/client.conf";
	struct sensorInfo temp, temp2;
	struct config ctemp;
	int fd, fd2;

	//Fill config struct with info from config file
	//code for above


	//If there are no sensors, exit
	if(ctemp.nsensor == 0){
		exit;
	}

	//If there is at least one, open file for sensor 1
	if((fd=open(fileName,O_RDONLY))==-1) {
		fprintf(stderr,"Could not read %s\n",fileName);
		/*    return 1; */
	}

	//Read temp reading for sensor 1
	if(read(fd,&temp.data,sizeof(temp))!=8) {
		fprintf(stderr,"Error reading %s\n",fileName);
		/*    return 1; */
	}

	
	//Fill sensorInfo struct
	temp.hostName = //add hostname		//host name of sensor
	temp.numSens = ctemp.nsensor;		//number of sensors
	temp.sensorNum = 0;			//sensor number
	temp.lowval = ctemp.low1;		//low value of sensor
	temp.highval = ctemp.high1;		//high value of sensor
	temp.timestamp = //add timestamp		
	temp.action = 0;			//action requested
	
	
	/*Send first sensor packet*/
	//Code for sending first packet to server//


	//Check if there is another sensor
	if(ctemp.nsensor == 1){
		close (fd);
		exit;
	}
		
	
	//If there is another sensor, do the same as above for sensor 2
	if((fd2=open(fileName2,O_RDONLY))==-1) {
		fprintf(stderr,"Could not read %s\n",fileName2);
		/*    return 1; */
	}


	if(read(fd2,&temp2.data,sizeof(temp))!=8) {
   		fprintf(stderr,"Error reading %s\n",fileName2);
		/*    return 1; */
	}

	close(fd2);

	temp2.hostName = //add hostname		//host name of sensor
	temp2.numSens = ctemp.nsensor;		//number of sensors
	temp2.sensorNum = 1;			//sensor number
	temp2.lowval = ctemp.low2;		//low value of sensor
	temp2.highval = ctemp.high2;		//high value of sensor
	temp2.timestamp = //add timestamp		
	temp2.action = 0;			//action requested

	
	/*Send second sensor packet*/
	//Cose for sending second packet to server//

	float conversion=0.0078125;
	printf("%3.2f %3.2f\n",
	(CtoF(((float)temp.data)*conversion)),
	(CtoF(((float)temp2.data)*conversion)));	
	
	exit;

}
	
