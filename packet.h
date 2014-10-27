#ifndef PACKET_H
#define PACKET_H



struct sensorInfo {
	char hostName[32];		//name of host with sensors
	int numSens;			//number of sensors attached to host
	int sensornum;			//sensor number  /dev/gotemp == sensor 0,  /dev/gotemp2 == sensor 1
	double data;			//data for sensor
	double lowval;			//acceptable low value for sensor
	double highval;			//acceptable high value for sensor
	char timestamp[32];		
	int action;			//action requested; 0 send sensor data (default), 1 request status,
}; 

	


#endif
