#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int main()
{

int fd=0;  //file descriptor pointer
char buf[10];  //initialize a character buffer of size 10 where data from copy to user is stored
int t;
char axis;  //for selecting axis
printf("Enter the axis: x or y or z");
scanf("%c",&axis);  //taking axis as input from the user

switch(axis)   //choosing the file we want according to axis
{
case 'x':   
        fd=open("/dev/adxl_x",O_RDONLY);
	break;
case 'y':
	fd=open("/dev/adxl_y",O_RDONLY);
	break;
case 'z':
	fd=open("/dev/adxl_z",O_RDONLY);
	break;
default: 
	printf("Invalid axis");
}

read(fd, buf,sizeof(buf));  //reading the buffer
for (t=0;t<10;t++)
{
	printf("%u",buf[t]);  // printing all the elements in the buffer
}
close(fd);
return 0;
}
