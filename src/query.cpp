#include <stdio.h>

#include <stdio.h>
#include <unistd.h>

//#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>


//pthread_mutex_t qmutex;


class myQuery{
	
	public:
	struct sMesage{
		char *msg;
		int sock;
		bool done;
	};

private:
	int kolejka_size;
	sMesage * kolejka;
	int ikolejka;//wskazuje pozycje na liscie dla nowej wiadomosci

	 int new_counter;//liczy ilosc nowych elementow
	
	void setCounter(int n){
		new_counter=n;
	}

	void Remove(int id)
	{

		//lock();
		if(kolejka[id].done==false)
		{
			kolejka[id].done=true;
			kolejka[id].msg=0;
			setCounter(new_counter-1);
		}
		//unlock();
	}

	int getNewID()
	{
		//lock();
		if(new_counter==0)
		{
			//unlock();
			return -1;
		}

			int ID=-1;
			for(int i=0;i<kolejka_size;i++)
			{
				int id=(ikolejka+i)%kolejka_size;

				if(kolejka[id].done==false)
				{
				ID=id;
				break;
				}
			}

		//unlock();
		return ID;
	}

	//zwalnianie pamieci
        void free_memory(sMesage m)
        {
                if(m.msg!=0)
                {
                        free(m.msg);
                        m.msg=0;
                }
        }

	//void lock()		{pthread_mutex_lock(&qmutex);}
	//void unlock()	{pthread_mutex_unlock(&qmutex);}
	public:
	myQuery(int maxsize)
	{
		//pthread_mutex_init(&qmutex,NULL);
		kolejka_size=maxsize;
		kolejka = new sMesage[kolejka_size];

		for(int i=0;i<kolejka_size;i++)
		{
			kolejka[i].done=true;
			kolejka[i].msg=0;
		}

		ikolejka=-1;
		new_counter=0;
	}

	sMesage pop(){

		int id=getNewID();

		if(id<0) perror("pop getNewID is bad!");
		sMesage m = kolejka[id];
		Remove(id);

		if(m.msg==0) perror("pop fail!");
		return m;
	}

	sMesage* peek(){
		int id=getNewID();

		if(id<0) perror("pop getNewID is bad!");
		return &kolejka[id];
	}
	
	bool push(char *str,int sock=-1)
	{
		sMesage m;
		m.msg = str;
		m.sock=sock;

		return push(m);
	}

	bool push(sMesage msg)
	{
		//lock();

			if(kolejka[ikolejka+1].done==false)
			{
				//unlock();
				return false;
			}

			ikolejka++;
			if(ikolejka==kolejka_size) ikolejka=0;


			msg.done=false;//zaznacz jako niewykonane

			//free_memory(kolejka[ikolejka]);//zwolnij pamiec

			kolejka[ikolejka]=msg;//dodaj do kolejki

			setCounter(new_counter+1);

			//printf("added %s (%d) to %d \n",msg.msg,new_counter,(int)kolejka);

		//unlock();
		return true;
	}

	int getNum()
	{
		return new_counter;
	}

	void getNumAndres()
	{
		printf("%p=%d (time:%d)\n",&new_counter,new_counter,time(NULL));
	}
	//end of calass	
};

