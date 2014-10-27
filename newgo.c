/* Go! Temp reader
* written by Jeff Sadowski <jeff.sadowski@gmail.com>
* with information gathered from David L. Vernier
* and Greg KH This Program is Under the terms of the 
* GPL http://www.gnu.org/copyleft/gpl.html
* Any questions feel free to email me :-)
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <errno.h>


/* This is close to the structure I found in Greg's Code */

struct packet {
	unsigned char measurements;
	unsigned char counter;
	int16_t measurement0;
	int16_t measurement1;
	int16_t measurement2; 
	};

/* Function to convert Celsius to Fahrenheit*/
float CtoF(float C){return (C*9.0/5.0)+32;}

int main(int argc,char** argv)
{
char *fileName="/dev/gotemp";
char *fileName2="/dev/gotemp2";
struct stat buf, buf2;
struct packet temp, temp2;

/* I got this number from the GoIO_SDK and it matched 
   what David L. Vernier got from his Engineer */

float conversion=0.0078125;
int fd, fd2;

if(stat( fileName, &buf ))
{
   if(mknod(fileName,S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP |S_IWGRP|S_IROTH|S_IWOTH,makedev(180,176)))
   {
      fprintf(stderr,"Cannot creat device %s  need to be root",fileName);
/*      return 1; */
   }
}

if(stat( fileName2, &buf2 ))
{
   if(mknod(fileName2,S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP |S_IWGRP|S_IROTH|S_IWOTH,makedev(180,177)))
   {
      fprintf(stderr,"Cannot creat device %s  need to be root",fileName2);
/*      return 1; */
   }
}

/* If cannot open, check permissions on dev, and see if it is plugged in */

if((fd=open(fileName,O_RDONLY))==-1)
{
   fprintf(stderr,"Could not read %s\n",fileName);
/*    return 1; */
}
if((fd2=open(fileName2,O_RDONLY))==-1)
{
   fprintf(stderr,"Could not read %s\n",fileName2);
/*    return 1; */
}

/* if cannot read, check is it plugged in */

if(read(fd,&temp,sizeof(temp))!=8)
{
   fprintf(stderr,"Error reading %s\n",fileName);
/*    return 1; */
}

if(read(fd2,&temp2,sizeof(temp))!=8)
{
   fprintf(stderr,"Error reading %s\n",fileName2);
/*    return 1; */
}

close(fd);
close(fd2);

printf("%3.2f %3.2f\n",
	(CtoF(((float)temp.measurement0)*conversion)),
	(CtoF(((float)temp2.measurement0)*conversion)));

return 0;
}
