#ifndef cArduino_CPP
#define cArduino_CPP 1

/*
Arduino Serial Port Comunication

Arduino information:
	http://playground.arduino.cc/Interfacing/LinuxTTY
	
	warning setting up port speed does not work :/ you nead to do in mannualy:
	
			stty -F /dev/ttyUSB0 cs8 115200 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts
	
Serial Conection Based On:
	Gary Frerking   gary@frerking.org
	http://www.tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <termios.h>
#include <unistd.h>

#include <string>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
	#include <sys/time.h>  
using namespace std;

//srs: http://arduino.cc/en/Serial/begin
enum ArduinoBaundRate{
  B300bps  = B300,
  B600bps  = B600,
  B1200bps = B1200,
  B2400bps = B2400,
  B4800bps = B4800,
  B9600bps = B9600,
  //B14400bps= B14400, //WARNING: don't exist
  B19200bps= B19200,
  //B28800bps= B28800,	//WARNING: don't exist
  B38400bps= B38400,
  B57600bps= B57600,
  B115200bps=B115200
};

class cArduino{

private:
	/*serial port FileDescriptor*/
	int fd=0;

	struct termios oldtio;

	/*Arduino FileName*/
	char *MODEMDEVICE =0;
public:

	cArduino(){
	}

	cArduino(ArduinoBaundRate baundRate){

                findArduino();

                if(MODEMDEVICE!=0)
                open(baundRate,MODEMDEVICE);
	}

	cArduino(ArduinoBaundRate baundRate,char *deviceFileName){

		open(baundRate,deviceFileName);
	}

	~cArduino(){

                /* restore the old port settings */
		if(fd!=0)
                tcsetattr(fd,TCSANOW,&oldtio);
	}

	/*get Arduino Device FileName*/
	char* getDeviceName(){
		if(MODEMDEVICE==0)	findArduino();
		return MODEMDEVICE;
	}

	/*
	src1: http://students.mimuw.edu.pl/SO/LabLinux/STRUKTURY/WEJSCIE-WYJSCIE/termios.html
	src2: http://publib.boulder.ibm.com/infocenter/zvm/v6r1/index.jsp?topic=/com.ibm.zvm.v610.edclv/rtcsis.htm
	//9600bps, 8 data bits, no parity, one stop bit.
	*/
	void print_port(char *devName){
		int ttyDevice =0;
		if(fd>0)
		ttyDevice = fd;
		else
		ttyDevice = ::open(devName, O_RDWR | O_NOCTTY);

		if ( ! isatty(ttyDevice) )
				return;
		

		struct termios term;

		if (tcgetattr(ttyDevice, &term) != 0){
			perror("tcgetatt() error");
			return;
		}
		speed_t speed = cfgetispeed(&term);
		if(fd<=0)
		::close(ttyDevice);
	
		printf("%s: ",devName);

		//SPEED
		//--------------------------------
		static char   SPEED[20];
		switch (speed) {
			case B0:       strcpy(SPEED, "0");      break;
			case B50:      strcpy(SPEED, "50");      break;
			case B75:      strcpy(SPEED, "75");      break;
			case B110:     strcpy(SPEED, "110");      break;
			case B134:     strcpy(SPEED, "134");      break;
			case B150:     strcpy(SPEED, "150");      break;
			case B200:     strcpy(SPEED, "200");      break;
			case B300:     strcpy(SPEED, "300");      break;
			case B600:     strcpy(SPEED, "600");      break;
			case B1200:    strcpy(SPEED, "1200");      break;
			case B1800:    strcpy(SPEED, "1800");      break;
			case B2400:    strcpy(SPEED, "2400");      break;
			case B4800:    strcpy(SPEED, "4800");       break;
			case B9600:    strcpy(SPEED, "9600");       break;
			case B57600:   strcpy(SPEED, "7600");       break;
			case B19200:   strcpy(SPEED, "19200");      break;
			case B38400:   strcpy(SPEED, "38400");      break;
			case B115200:  strcpy(SPEED, "115200");     break;
			case B230400:  strcpy(SPEED, "230400");     break;
			case B460800:  strcpy(SPEED, "460800");     break;
			default:       sprintf(SPEED, "unknown (%d)", (int) speed);
		  }

			if (term.c_cflag & CBAUD)
				  printf("%s bps, ",SPEED);
			else
				  printf("BAUND notset, ");
		//--------------------------------

		//DataBits
		//----------------------------
		switch(term.c_cflag&CSIZE)
		{
			case CS5:printf("5bits, ");break;
			case CS6:printf("6bits, ");break;
			case CS7:printf("7bits, ");break;
			case CS8:printf("8bits, ");break;
		}
		//---------------------------

		//Parity
		//--------------------------------
		if (term.c_cflag & PARODD)
			  printf("Odd parity, ");
		else
			  printf("Even parity, ");
		//--------------------------------


		//Stop Bits
		//--------------------------------
		if (term.c_cflag & CSTOPB )
			printf("two stop bit");
		else
			printf("one stop bit");
		//------------------------------

		printf("\n");
		return;
	}

	/*Find Arduino device*/
	char *findArduino(){

		char  dir [] = "/dev/serial/by-id/";

		DIR *d=opendir(dir);

		if(d == NULL) //Couldn't open directory
			return 0;

		struct dirent *de=NULL;
		while(de = readdir(d))
		{
			if(strstr(de->d_name,"arduino")!=0)
			{
				char s[PATH_MAX+1];
				sprintf(s,"%s%s",dir,de->d_name);

				char buf[1024];
				int len;
				if ((len = readlink(s, buf, sizeof(buf)-1)) != -1)
				   buf[len] = '\0';

				MODEMDEVICE=new char[PATH_MAX+1];
				realpath(s, MODEMDEVICE);

				closedir(d);
				return  MODEMDEVICE;
			}
		}

		closedir(d);
		return 0;
	}

	/*is Arduino serial port Open?*/
	bool isOpen(){
		if(fd==0) return false;
		return true;
	}

	bool open(ArduinoBaundRate baundRate)
	{
		findArduino();
		
		return open(baundRate,0);
	}
	
	/*open serial port*/
	bool open(ArduinoBaundRate baundRate,char *DeviceFileName)
	{
		int c;
		
		
		if(DeviceFileName==0) {
			DeviceFileName = findArduino();			
		}
		MODEMDEVICE = DeviceFileName;
											
		/*
		Open modem device for reading and writing and not as controlling tty
		because we don't want to get killed if linenoise sends CTRL-C.
		*/
		if(MODEMDEVICE==0)
		return false;

		fd = ::open(MODEMDEVICE, O_RDWR | O_NOCTTY );
		if (fd <0) {
			perror(MODEMDEVICE);
			return false;
		}

		tcgetattr(fd,&oldtio); /* save current serial port settings */
		
		struct termios newtio;
		bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */

		/*
		BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
		CRTSCTS : output hardware flow control (only used if the cable has
		    all necessary lines. See sect. 7 of Serial-HOWTO)
		CS8     : 8n1 (8bit,no parity,1 stopbit)
		CLOCAL  : local connection, no modem contol
		CREAD   : enable receiving characters
		*/
		newtio.c_cflag =  	
							baundRate | 
							
							CRTSCTS | //Ustawiona oznacza, że jest dostępna sprzętowa kontrola przepływu danych.
							
							CS8 | //(CSIZE) określenia rozmiaru znaku 8bit
							
							CLOCAL | //control local?
							
							CREAD	// można czytać
							;

		/*
		IGNPAR  : ignore bytes with parity errors
		ICRNL   : map CR to NL (otherwise a CR input on the other computer
		    will not terminate input)
		otherwise make device raw (no other input processing)
		*/
		newtio.c_iflag = IGNPAR | ICRNL;

		/*
		Raw output.
		*/
		newtio.c_oflag = 0;

		/*
		ICANON  : enable canonical input
		disable all echo functionality, and don't send signals to calling program
		*/
		newtio.c_lflag = ICANON;

		/*
		initialize all control characters
		default values can be found in /usr/include/termios.h, and are given
		in the comments, but we don't need them here
		*/
		newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
		newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
		newtio.c_cc[VERASE]   = 0;     /* del */
		newtio.c_cc[VKILL]    = 0;     /* @ */
		newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
		newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
		newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
		newtio.c_cc[VSWTC]    = 0;     /* '\0' */
		newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */
		newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
		newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
		newtio.c_cc[VEOL]     = 0;     /* '\0' */
		newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
		newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
		newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
		newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
		newtio.c_cc[VEOL2]    = 0;     /* '\0' */

		/*
		now clean the modem line and activate the settings for the port
		*/
		tcflush(fd, TCIFLUSH);
		tcsetattr(fd,TCSANOW,&newtio);

		/*
		terminal settings done, now handle input
		*/
		return true;
	}

	/*zamykanie*/
	void close(){
		::close(fd);
	}
	
	/*Flush port*/
	void flush()	{
		tcflush(fd, TCIFLUSH);
	}

	/*read from Arduino*/
	string read()	{
		/* read blocks program execution until a line terminating character is 
		input, even if more than 255 chars are input. If the number
		of characters read is smaller than the number of chars available,
		subsequent reads will return the remaining chars. res will be set
		to the actual number of characters actually read */
		char buf[255];

		int res = ::read(fd,buf,255);
		buf[res]=0;             /* set end of string, so we can printf */

		string ret(buf);
		return ret;
	}

	/*read form arduino (witch timeout)
	 *ret - responce
	 *timeOut_MicroSec - (mikro sekundy 10-6)
	 *print_error - print errors to stderr?
	*/
	bool read(
				string &ret,
				unsigned long int timeOut_MicroSec=(1*1000000)//1sec
				,bool print_error=false
			)	{

		char buff[100];
		int len = 100;

		fd_set set;
		FD_ZERO(&set); /* clear the set */
		FD_SET(fd, &set); /* add our file descriptor to the set */

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = timeOut_MicroSec;

		int rv = select(fd + 1, &set, NULL, NULL, &timeout);

		if(rv == -1){
			if(print_error)
			perror("arduino select"); /* an error accured */
		}
		else
		{
			if(rv == 0)
			{
				if( print_error )
				fputs("arduino timeout!\n", stderr); /* a timeout occured */
			}else{
				ret=read();
				return true; /* there was data to read */
			}
		}

		return false;
	}

	double getTimeSec(){

		struct timeval tim; 
		gettimeofday(&tim, NULL);  
		 
		double t2 = tim.tv_sec+(tim.tv_usec/1000000.0); 
		return t2;
	}

	/*
	odczytuj az do napotkania znaku / lub przekroczenia czasu
	 *ret - responce
	 *ultin - do jakiego znaku czytac
	 *timeOut_MicroSec - (mikro sekundy 10-6)
	*/
	bool read(
				string &ret,
				char until,
				unsigned long int timeOut_MicroSec)	{
	
		ret="";
		
		double t1 = getTimeSec();
		while(true)
		{
			double dt = getTimeSec() - t1;
			
			string s="";
			if(!read(s,timeOut_MicroSec,false))
			break;
		
			ret+=s;
				
			if(s.find(until)!=std::string::npos){
				//printf("executed in %.6lfsec\n",dt);
				return true;
				}
		}		
	
		ret="";
		return false;
	}

	/*write to Arduinio*/
	void write(string text)	{
		::write(	fd,(char*)text.c_str(),(size_t)text.length() );
	}

};

#endif
