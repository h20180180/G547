#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#define FILE_NAME "/dev/pir"
#define MSG_LEN 2
#define BUF_SIZE (MSG_LEN + 1)

long r=0;
long s=0;

void delay(int number_of_seconds) 
{ 
    // Converting time nto milli_seconds 
    int milli_seconds = 1000 * number_of_seconds; 
  
    // Storing start time 
    clock_t start_time = clock(); 
  
    // looping till required time is not acheived 
    while (clock() < start_time + milli_seconds) 
        ; 
}

int main()
{	
    int i=0;
	int fd;
	char val[2];
	int t=0;
	int dis;
	
	fd = open(FILE_NAME, O_RDWR | O_SYNC);
	if (fd < 0) {
		perror("Failed to open device");
		exit(errno);
	}
	printf("Specify sensors distance :");
	scanf("%d",&dis);
	while (i<200) {
	delay(100);
	read(fd, val, 1);
   { 
			struct timeval tv;
			time_t curtime;
			gettimeofday(&tv, NULL); 
			curtime=tv.tv_sec;    /* getting the current epoch time in seconds*/
			
			if (val[0]==2 && t==0)
			{ 	s = curtime;
				t=1;
				printf("PIR2, %lu",s);
				delay(4000);
				t=0;
			}
			else if(val[0]==1)
			{ 	r = curtime;
				t=1;
				printf("PIR1, %lu",r);
				delay(4000);
				t=0;
			}
			
			if((r!=0) && (s!=0))
			{
				float vel = dis/(s-r);
				printf("Speed is %f cm/s", vel);
				r=0;
				s=0;
			}
	printf(" Detected Sensor state: %u\n",val[0]);
    }
	i=i+1;
		}

	
	
	close(fd);

	return EXIT_SUCCESS;
}
