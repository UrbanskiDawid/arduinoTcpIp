#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "rs232.c"

#define COMPORT         24       // this is '/dev/ttyUSB0' (for arduino) or choose wherever you added '/dev/ttyAMA0' (for raspberry pi UART pins) to the list
#define BAUDRATE      115200   // or whatever baudrate you want

#define RECEIVE_CHARS   255     // or whatever size of buffer you want to receive
#define SEND_CHARS      255     // or whatever size of buffer you want to send


//############################
#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

#define KDEFAULT 	KWHT
#define KINPUT 		KGRN
#define KOUTPUT		KYEL
//############################

/*
 * okresla dlugosc napisu
 */
int strlen(unsigned char str[])
{
	for(int i=0;i<=255;i++)
	{
		if(str[i]==(char)0)
		return i;
	}
	
	return  0;
}

/*
 * wysyanie
 */
void arduino_send(unsigned char send_buffer[])
{
	RS232_SendBuf(COMPORT, send_buffer, strlen(send_buffer));
}


/*
 * Odczytywanie:
 */
unsigned char receive_buffer[RECEIVE_CHARS];   
int receive_buffer_len = 0;
int read()
{
	receive_buffer_len=
    (int)RS232_PollComport(COMPORT, receive_buffer, RECEIVE_CHARS-1);
    
    receive_buffer[receive_buffer_len]=(char)0;
    
    return receive_buffer_len;
}

/*
 * odczytywanie do skutku (lub timeout)
 */
int read2(int times=5){
	
	for(int i=0;i<times;i++)
	{
		int len=read();
		if(len>0) return len;
		sleep(1);
	}
	return 0;
}

int read3(int times=5){

	for(int i=0;i<times*1000;i++)
	{
		int len=read();
		if(len<0) usleep(500000);//1000000microsec = 1sec
		
	}

	return 0;
}

/*
 * wypisz odczytany napis
 */
void myprint(){
	
	if(receive_buffer_len<1)		printf("%s<empty>",KOUTPUT);
    else		                    printf("%s%s",KOUTPUT,receive_buffer);
    
    printf("%s",KDEFAULT);//dalej domyslny kolor
}

/*
 * wczytaj z klawiatury
 */
 char send_buffer[255];
void myinput()
{
	printf("%s>> ",KINPUT);
	scanf("%s",send_buffer);
	
	printf("%s",KDEFAULT);//dalej domyslny kolor
	
	return;
}

/*
 * otwarcie portu
 */
bool open()
{
	if(RS232_OpenComport(COMPORT, BAUDRATE)!=0) return false;
	return true;
}

/*
 *  zamykanie portu
 */
void close()
{
   RS232_CloseComport(COMPORT);
}
