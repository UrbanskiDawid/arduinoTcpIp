#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>



typedef void (*client_proc)(int);


class TcpSercer
{
	int sockfd;
	int PORT;
	client_proc ClientRun;
	void run_client(int sock)
	{
		ClientRun(sock);
	}
	
	bool isRuning;
	bool isBind=false;

public:
	TcpSercer(int port,client_proc run)
	{
		PORT=port;
		ClientRun =run;

		sockfd = socket(AF_INET, SOCK_STREAM,0);
		if(sockfd<0)
		{
			perror("socket() fail");
			return;
		}

		struct sockaddr_in serv_add;

		bzero( (char*) &serv_add, sizeof(serv_add) );
		serv_add.sin_family = AF_INET;
		serv_add.sin_addr.s_addr = INADDR_ANY;
		serv_add.sin_port = htons(PORT);

		if(bind(sockfd, (struct sockaddr *) &serv_add, sizeof(serv_add))<0)
		{
			perror("bind() fail");
			return;
		}
		
		isBind=true;
		
		isRuning=true;
		listen(sockfd,5);
	}

	bool isBinded(){
		return isBind;
	}
	int getPort(){
		return PORT;
	}
	void stop(){
		isRuning=false;
		close(sockfd);
	}

	void run()
	{
		struct sockaddr_in cli_addr;
		socklen_t clilen= sizeof(cli_addr);

		while(isRuning)
		{
			int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,&clilen);
			if(newsockfd<0)
			{
				perror("accept() fail");
				return;
			}
			run_client(newsockfd);
		}
	}


	char* client_read(int sock)
	{
		static char buffer[255];
		bzero(buffer,255);

		int n=read(sock,buffer,255);
		if(n<1)
		{
			//perror("read() fail");
			return 0;
		}
		if(buffer[n-1]==(char)'\n')buffer[n-1]=(char)0;
		
		//buffer[n-2]=(char)0;
		
		return buffer;
	}

	void client_write(int sock,char *responce)
	{
		if(	write(sock,responce,strlen(responce))	<0)
		{
			perror("write() fail");
		}
	}

	void client_close(int sock)
	{
		close(sock);
		sock=-1;
	}
};



/*

void obsluga(int);
TcpSercer server(8082,obsluga);

void obsluga(int sock)
{
	printf("jej");
	server.server_write(sock,(char*)"aha!");
}



int main()
{
	server.run();
	
	return 1;
}
*/
