/*********************************
 *    Arduino sketch for test
 *********************************

void setup() {
  Serial.begin(115200);
}

void loop() {
  while (Serial.available()) {
    Serial.write((char)Serial.read());
  }
}

*/

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdlib.h>
#include <sys/time.h>

#include <pthread.h>


#define TRUE    1
#define FALSE   0

int initport( int fd, speed_t baudRate )
{
    struct termios options;
    tcgetattr( fd, &options );

    /* Disable canonical mode, and set buffer size to 1 byte */
    options.c_lflag &= (~ICANON);
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN] = 1;

    // Set the baud rates to...
    cfsetispeed( &options, baudRate );
    cfsetospeed( &options, baudRate );

    // Enable the receiver and set local mode...
    options.c_cflag |= ( CLOCAL | CREAD );
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    // Set the new options for the port...
    tcsetattr( fd, TCSANOW, &options );

    // Flush the input & output...
    tcflush( fd, TCIOFLUSH );

    return 1;
}


static struct
{
  const char *string;
  speed_t speed;
  uint32_t value;
} const speeds[] =
{
	{"0",		B0,		0},
	{"50",		B50,	50},
	{"75",		B75,	75},
	{"110",		B110,	110},
	{"134",		B134,	134},
	{"134.5",	B134,	134},
	{"150",		B150,	150},
	{"200",		B200,	200},
	{"300",		B300,	300},
	{"600",		B600,	600},
	{"1200",	B1200,	1200},
	{"1800",	B1800,	1800},
	{"2400",	B2400,	2400},
	{"4800",	B4800,	4800},
	{"9600",	B9600,	9600},
	{"19200",	B19200,	19200},
	{"38400",	B38400,	38400},
	{"exta",	B19200,	19200},
	{"extb",	B38400,	38400},
#ifdef B57600
	{"57600",	B57600, 57600},
#endif
#ifdef B115200
	{"115200",	B115200, 115200},
#endif
#ifdef B230400
	{"230400",	B230400, 230400},
#endif
#ifdef B460800
	{"460800",	B460800, 460800},
#endif
#ifdef B500000
	{"500000",	B500000, 500000},
#endif
#ifdef B576000
	{"576000",	B576000, 576000},
#endif
#ifdef B921600
	{"921600",	B921600, 921600},
#endif
#ifdef B1000000
	{"1000000", B1000000, 1000000},
#endif
#ifdef B1152000
	{"1152000", B1152000, 1152000},
#endif
#ifdef B1500000
	{"1500000", B1500000, 1500000},
#endif
#ifdef B2000000
	{"2000000", B2000000, 2000000},
#endif
#ifdef B2500000
	{"2500000", B2500000, 2500000},
#endif
#ifdef B3000000
	{"3000000", B3000000, 3000000},
#endif
#ifdef B3500000
	{"3500000", B3500000, 3500000},
#endif
#ifdef B4000000
	{"4000000", B4000000, 4000000},
#endif
	{NULL, 0, 0}
};

static speed_t
string_to_baud (const char *arg)
{
	int i;
  	for (i = 0; speeds[i].string != NULL; ++i) {
		if (0 == strcmp(arg, speeds[i].string)) {
			return speeds[i].speed;
		}
	}
	return (speed_t)(-1);
}



/* Subtract the `struct timeval' values X and Y,
storing the result in RESULT.
Return 1 if the difference is negative, otherwise 0.  */

int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
{
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < y->tv_usec) {
     int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
     y->tv_usec -= 1000000 * nsec;
     y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) {
     int nsec = (x->tv_usec - y->tv_usec) / 1000000;
     y->tv_usec += 1000000 * nsec;
     y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
      tv_usec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}



int serport_fd;
char* writeBuff;
char* readBuff; //string to send
unsigned bytesToSend;
unsigned writtenBytes;

void* write_thread_function(void *arg) {
    int lastBytesWritten;

    fprintf(stdout, "write_thread_function spawned\n");

    writtenBytes = 0;
    while(writtenBytes < bytesToSend)
    {
        lastBytesWritten = write(serport_fd, writeBuff + writtenBytes, bytesToSend - writtenBytes);
        writtenBytes += lastBytesWritten;
        if ( lastBytesWritten < 0 )
        {
            fprintf(stdout, "write failed!\n");
            return 0;
        }
        //fprintf(stderr, "   write: %d - %d\n", lastBytesWritten, writtenBytes);
    }
    return NULL;
}

int main( int argc, char **argv )
{
    char *serport;
    char *serspeed;
    speed_t serspeed_t;
    int recdBytes, totlBytes;

    char* readBuffer;

    struct timeval timeStart, timeEnd, timeDelta;

    pthread_t myWriteThread;

    // Get the PORT name
    serport = argv[1];
    fprintf(stdout, "Opening port %s;\n", serport);

    // Get the baudrate
    serspeed = argv[2];
    serspeed_t = string_to_baud(serspeed);
    fprintf(stdout, "Got speed %s (%d/0x%x);\n", serspeed, serspeed_t, serspeed_t);

    //Get file or command;
    FILE* inputFile = fopen(argv[3], "rb");
    if (inputFile >= 0) {
        struct stat st;
        stat(argv[3], &st);
        bytesToSend = st.st_size;
        writeBuff = (char *)malloc(bytesToSend);
        readBuffer = (char *)malloc(bytesToSend);
        fread(writeBuff, 1, bytesToSend, inputFile);
        fclose(inputFile);
    } else {
    	perror(argv[3]);
    	return 1;
    }

    // Open and Initialise port
    serport_fd = open( serport, O_RDWR | O_NOCTTY );
    if ( serport_fd < 0 ) { perror(serport); return 1; }
    initport( serport_fd, serspeed_t );

    FILE* outputFile = fopen("out.bin", "wb");


    recdBytes = 0;

    gettimeofday( &timeStart, NULL );

    // start the thread for writing..
    if ( pthread_create( &myWriteThread, NULL, write_thread_function, NULL) ) {
        printf("error creating thread.");
        abort();
    }

    // run read loop
    while ( recdBytes < bytesToSend )
    {
        ssize_t readChars = read(serport_fd, readBuffer+recdBytes, bytesToSend-recdBytes);

        if (readChars > 0) {
        	fwrite(readBuffer+recdBytes, 1, readChars, outputFile);
        	fflush(outputFile);
            recdBytes += readChars;
        } else {
            if ( errno == EAGAIN ) {
                fprintf(stdout, "SERIAL EAGAIN ERROR\n");
                return 0;
            } else {
                fprintf(stdout, "SERIAL read error: %d = %s\n", errno , strerror(errno));
                return 0;
            }
        }
        //fprintf(stderr, "   read: %d\n", recdBytes);
    }

    if ( pthread_join ( myWriteThread, NULL ) ) {
        printf("error joining thread.");
        abort();
    }

    gettimeofday( &timeEnd, NULL );

    // Close the open port
    close( serport_fd );

    fprintf(stdout, "\n+++DONE+++\n");

    totlBytes = writtenBytes + recdBytes;
    timeval_subtract(&timeDelta, &timeEnd, &timeStart);
    float deltasec = timeDelta.tv_sec+timeDelta.tv_usec*1e-6;
    float expectBps = atoi(serspeed)/10.0f;
    float measWriteBps = writtenBytes/deltasec;
    float measReadBps = recdBytes/deltasec;

    fprintf(stdout, "Wrote: %d bytes; Read: %d bytes; Total: %d bytes. \n", writtenBytes, recdBytes, totlBytes);
    fprintf(stdout, "Start: %ld s %ld us; End: %ld s %ld us; Delta: %ld s %ld us. \n", timeStart.tv_sec, timeStart.tv_usec, timeEnd.tv_sec, timeEnd.tv_usec, timeDelta.tv_sec, timeDelta.tv_usec);
    fprintf(stdout, "%s baud for 8N1 is %d Bps (bytes/sec).\n", serspeed, (int)expectBps);
    fprintf(stdout, "Measured: write %.02f Bps (%.02f%%), read %.02f Bps (%.02f%%), total %.02f Bps.\n", measWriteBps, (measWriteBps/expectBps)*100, measReadBps, (measReadBps/expectBps)*100, totlBytes/deltasec);

    return 0;
}
