#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <stdlib.h>


#include <pthread.h>
#include <pthread.h>

#include "src/cArduino.cpp"
#include "src/server.cpp"
#include "src/query.cpp"
 
cArduino *ARDUINO = new cArduino();
string powitanie = "";//co powiedzial modol na dziendobry


//kolejka
//##########################################
static myQuery kolejka(2550);
using namespace std;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

void smartExecuteStart();
void Add(char *msg,int sock=-1){
	
	pthread_mutex_lock(&mutex1);
	
		kolejka.push(msg,sock);
		
	pthread_mutex_unlock(&mutex1);
	
	smartExecuteStart();
}
//##########################################


//server
//##########################################
void server_onNewClient(int);
static TcpSercer * server;
int TcpSercer_defaultport = 3333;

//obsluga clienta
void* task_server_client(void *int_sock){

	int sock = ((int)int_sock);

	server->client_write(sock,(char*)powitanie.c_str());
	server->client_write(sock,(char*)"\n>>");

	while(true)
	{
		//odczytaj zapytanie
		char *request=server->client_read(sock);
		
		if(request==0){ //disconect
			break;
		}

		//dodaj do kolejki
		Add(request,sock);
	}
	
	server->client_close(sock);
}

void server_onNewClient(int sock){

	int a=sock;
	pthread_t t;
	pthread_create(&t,NULL,task_server_client,(void*)a);
}

//zdanie obsugi servera
void* task_server(void *arg){

	printf("server runing at port %d\n",server->getPort());
	server->run();
}
//##########################################



//############################
#define KDEFAULT 	"\x1B[37m"
#define KINPUT 		"\x1B[32m"
#define KOUTPUT		"\x1B[33m"
//############################

// wypisz odczytany napis
 void myprint(char *str){
	
	printf("%s%s",KOUTPUT,str);
	
	printf("%s",KDEFAULT);//dalej domyslny kolor
}

//zdanie odczytania polecen z klawiarury
void* task_stdin(void *arg){

	while(1)
	{
		char send_buffer[255];
	
		//wczytaj z klawiatury polecnie & wyslij
		printf("%s>> ",KINPUT);
		scanf("%s",send_buffer);
		printf("%s",KDEFAULT);//dalej domyslny kolor

		Add(send_buffer);

		//sleep(2);
	}
}


//zadamoe wykonania polecen (wywolywane przez 'smartExecuteStart' )
void* task_execute(void *){

		pthread_mutex_lock(&mutex1);

			while(kolejka.getNum()>0)
			{
				myQuery::sMesage m = kolejka.pop();

				//wyslij polecenie i odczytaj odpowiedz
					ARDUINO->write(m.msg);//zapisz do arduino

					string odpowiedz="";
					ARDUINO->read(odpowiedz,(char)3,500000);
					
					if(odpowiedz.length()<1)
					odpowiedz="FAIL\n";
				//---

				//odpowiedz (komu trzeba)
					if(m.sock==-1){//STDOUT
						myprint((char*)odpowiedz.c_str());
					}

					if(m.sock!=-1){//TPC
					
						server->client_write(m.sock,(char*)odpowiedz.c_str());
					}
				//-----
			}

		pthread_mutex_unlock(&mutex1);

}

//sprytne Wykonywanie polecenia
pthread_mutex_t mutexSmart = PTHREAD_MUTEX_INITIALIZER;
bool smart=false;
void smartExecuteStart(){
	
	pthread_mutex_lock(&mutex1);
	
	if(smart==true) 
	{
		pthread_mutex_unlock(&mutex1);
		return;
	}
	smart=true;
	pthread_mutex_unlock(&mutex1);
	
		usleep(1000);//TODO: czy potrzebne?
	
	pthread_t t1;
	pthread_create(&t1,NULL,task_execute,NULL);
	pthread_join(t1,0);
	
	pthread_mutex_lock(&mutex1);
	smart=false;
	pthread_mutex_unlock(&mutex1);
}
//--

//łapanie przerwan programu
void sighandler(int sig){

	cout<< "Signal " << sig << " caught...exiting" << endl;

		server->stop();
		
		ARDUINO->close();
		
	sleep(1);
	exit(0);
}



int main (int argc, char **argv) {

	//##
	//sprawdz czy podloaczono arduino
	if(ARDUINO->findArduino()==0)
	{
		perror("nie podloczono Arduino");
		return 1;
	}
	//############################

	//##
	//sprawdz czy server uruchomiony
	if(!server->isBinded())
	{
		perror("server problem");
		return 1;
	}
	//##########################

	signal(SIGABRT, &sighandler);
	signal(SIGTERM, &sighandler);
	signal(SIGINT, &sighandler);

	//otwieranie Arduino
	if(!ARDUINO->open( (ArduinoBaundRate)B115200bps)){
		perror("nie moge podloczonyc Arduino");
		return 1;
	}
	if(!ARDUINO->isOpen())	{
		perror("nie moge otworzyc Arduino");
		return 1;
	}
	//--

	//powitanie
	ARDUINO->read(powitanie,(char)3,3*1000000);
	if(powitanie.length()<10){
		perror("arduino problem (nie ma powitania)");
		return 1;
	}else{
	
		powitanie = 
			"server date version: "+string(__DATE__)+"\n"
			+powitanie;
	
		myprint((char*)powitanie.c_str());//pokaz powiadomienie
	}
	//--

	//tworze watek tcpip
	pthread_t t1;
	
	if(argc==1)	
		server = new TcpSercer(atoi(argv[1]),server_onNewClient);
	else
		server = new TcpSercer(TcpSercer_defaultport,server_onNewClient);

	pthread_create(&t1,NULL,task_server,NULL);
	//--

	//tworze watek klawiatury
	pthread_t t2;
	//pthread_create(&t2,NULL,task_stdin,NULL);


	//poczekaj na zakonczenie abu watkow
	pthread_join(t1,0);
	//pthread_join(t2,0);
	//---

	//koniec pracy
	ARDUINO->close();
	//--

	return 0;
}
