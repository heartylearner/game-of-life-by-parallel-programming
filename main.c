//犧牲每輪速度?補0?
#include <unistd.h>
//#include <sys/time.h>
//#include <wait.h>//valid?
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/mman.h>
//#include <sys/time.h>
int bias;
int *Arr[2],IDS[1001],*RoundID,M,N,Rounds,Round,Range,T,P,Thread_nums,Process_nums,left,offs,fd,fd2;
char *buf2,*rmap,*wmap,*buf;
sem_t *sem,sem2;
int Min(int a,int b){
    return a<b?a:b;
}
/*int inbound(int i,int j){
    if((0<=i&&i<M)&&(0<=j&&j<N))return 1;
    return 0;
}*/
int live(int *arr,int I,int J){
    //int count=arr[(I-1)*(N+2)+J-1]+arr[(I-1)*(N+2)+J]+arr[I-1][J+1]+arr[I][J-1]+arr[I][J]+arr[I][J+1]+arr[I+1][J-1]+arr[I+1][J]+arr[I+1][J+1];
    int count=arr[(I-1)*(N+2)+J-1]+arr[(I-1)*(N+2)+J]+arr[(I-1)*(N+2)+J+1]+arr[(I)*(N+2)+J-1]+arr[(I)*(N+2)+J]+arr[(I)*(N+2)+J+1]+arr[(I+1)*(N+2)+J-1]+arr[(I+1)*(N+2)+J]+arr[(I+1)*(N+2)+J+1];
    //printf("I=%d J=%d live=%d count=%d\n",I,J,arr[I][J],count);
    if(arr[(I)*(N+2)+J]==0){
        return (count==3);
    }
    else{
        return ((count==3)||(count==4));
    }
}
char plive(char *arr,int I,int J,int curr){
    char count=(int)arr[(I-1)*(N+2)+J-1+(1-curr)*bias]+(int)arr[(I-1)*(N+2)+J+(1-curr)*bias]+(int)arr[(I-1)*(N+2)+J+1+(1-curr)*bias]
    +(int)arr[I*(N+2)+J-1+(1-curr)*bias]+(int)arr[I*(N+2)+J+(1-curr)*bias]+
    (int)arr[I*(N+2)+J+1+(1-curr)*bias]+(int)arr[(I+1)*(N+2)+J-1+(1-curr)*bias]
    +(int)arr[(I+1)*(N+2)+J+(1-curr)*bias]+(int)arr[(I+1)*(N+2)+J+1+(1-curr)*bias];
    if(arr[(1-curr)*bias+I*(N+2)+J]==0){
	    return (count==3);
    }
    else{
	
        return ((count==3)||(count==4));
    }
}
void* thread_process(void* ID){
    int id=*((int*)(ID)),curr=Round%2;
    int from;
    int to,I,J,range;
    //char *wmap=mmap(NULL,M*(N+1)-1,PROT_WRITE,MAP_SHARED,fd2,0);
    range=(N+2)/T+1;from=(id)*range;to=Min(from+range-1,N+1);
    for(int i=from;i<=to;++i)Arr[0][i]=Arr[0][(M+1)*(N+2)+i]=Arr[1][i]=Arr[1][(M+1)*(N+2)+i]=0;
    range=(M)/T+1;from=(id)*range+1;to=Min(from+range-1,M);
    for(int i=from;i<=to;++i)Arr[0][i*(N+2)]=Arr[0][i*(N+2)+N+1]=Arr[1][i*(N+2)]=Arr[1][i*(N+2)+N+1]=0;
    if(id<left){
        from=id*Range+id;to=from+Range;
    }
    else{
        from=id*Range+left;to=from+Range-1;
    }
    for(int i=from;i<=to;++i){
        I=i/N+1;J=i%N+1;//buf[(N+1)*(I-1)+J-1]=
		Arr[0][I*(N+2)+J]=rmap[offs+(N+1)*(I-1)+J-1]=='O';
    }
    int c=0;
    sem_post(&sem2);
    //printf("thread id=%d from %d to %d\n",id,from,to);
    while(1){
        sem_wait(sem+id);
	if(Round>Rounds)break;
        curr=Round%2;
        for(int i=from;i<=to;++i){
            I=i/N+1;J=i%N+1;
	    ++c;
	    //if(c==1000){printf("id=%d c=%d cell=%d\n",id,c,i-from);c=0;for(int j=0;j<1000000;++j);}

	    Arr[curr][I*(N+2)+J]=live(Arr[1-curr],I,J);
            //buf[(N+1)*(I-1)+J-1]=
		Arr[curr][I*(N+2)+J]?'O':'.';
        }
	
	c=0;
        //if(Round==Rounds)printf("round=%d ID=%d from=%d to=%d curr=%d\n",Round,id,from,to,curr);
        sem_post(&sem2);
    }
    //printf("in thread %d\n",id);fflush(stdout);
    for(int i=from;i<=to;++i){
        I=i/N+1;J=i%N+1;buf[(N+1)*(I-1)+J-1]=Arr[Rounds%2][I*(N+2)+J]?'O':'.';
	//printf("wmap[%d]=%c\n",(N+1)*(I-1)+J-1,wmap[(N+1)*(I-1)+J-1]);
    }
    //buf2[0]='0';
    //pthread_exit(NULL);
    range=(M-1)/T+1;from=(id)*range+1;to=Min(from+range-1,M-1);
    for(int i=from;i<=to;++i)buf[i*(N+1)-1]='\n';
    //printf("round=%d ID=%d from=%d to=%d\n",Round,id,from,to);
    //msync(wmap,M*(N+1)-1i0);//pthread_exit(NULL);
    //munmap(wmap,M*(N+1)-1);
    pthread_exit(NULL);
}
int main(int argc,char **argv){
    //bias=(M+2)*(N+2)*2+10;
    //struct timeval t;gettimeofday(&t,NULL);
    //time_t usec=t.tv_usec,sec=t.tv_sec;  
    fd=open(argv[3],O_RDONLY);fd2=open(argv[4],O_RDWR|O_TRUNC|O_CREAT,0777);//開啟方式?
    FILE *rfp=fdopen(fd,"r"),*wfp=fdopen(fd2,"w");
    int mode=(argv[1][1]=='t'),from,to,curr,I,J,val,ch;
    fscanf(rfp,"%d %d %d",&M,&N,&Rounds);
    buf2=(char*)malloc(sizeof(char)*(M*(N+1)-1));
    char *tmp=(char*)malloc(sizeof(char)*M*(N+1));
    //write(fd2,tmp,M*(N+1));return 0;
    bias=(M+2)*(N+2)*2+10;
    offs=ftell(rfp)+1;
    //lseek(fd,offs,SEEK_SET);
    //truncate(argv[4],M*(N+1)-1);
    rmap=mmap(NULL,offs+(N+1)*M+10,PROT_READ,MAP_SHARED,fd,0);
    //wmap=mmap(NULL,M*(N+1)-1,PROT_WRITE,MAP_SHARED,fd2,0);
    if(rmap==MAP_FAILED||wmap==MAP_FAILED){
	    printf("map fail\n");
	    return 0;
    }
    buf=(char*)malloc(sizeof(char)*(M*(N+1)-1));
    Arr[0]=(int*)malloc(sizeof(int)*(N+3)*(M+3));Arr[1]=(int*)malloc(sizeof(int)*(N+3)*(M+3));
    if(mode==0){
        lseek(fd,offs,SEEK_SET);
        read(fd,buf,M*(N+1)-1);
        Range=(M*N)/2;
        int shmid=shmget(IPC_PRIVATE,bias+(M+2)*(N+2)*2+10, IPC_CREAT|0600 );
        char *Arr2=(char*)shmat(shmid,NULL,0);
	    for(int i=0;i<=N+1;++i){
        	Arr2[i]=Arr2[(M+1)*(N+2)+i]=Arr2[bias+i]=Arr2[bias+(M+1)*(N+2)+i]=0;
    	}
	    for(int i=1;i<=M;++i){
        	for(int j=1;j<=N;++j){
            	Arr2[i*(N+2)+j]=(buf[(i-1)*(N+1)+j-1]=='O');
        	}
        	Arr2[i*(N+2)]=Arr2[i*(N+2)+N+1]=Arr2[bias+i*(N+2)]=Arr2[bias+i*(N+2)+N+1]=0;
    	}
        for(Round=1;Round<=Rounds;++Round){
            if(fork()==0){
                char *arr=(char*)shmat(shmid,NULL,0);
		        curr=Round%2;
		        for(int i=0;i<Range;++i){
                    I=i/N+1;J=i%N+1;
                    arr[curr*bias+I*(N+2)+J]=plive(arr,I,J,curr);                
                }
                _exit(0);
            }
            if(fork()==0){
                char *arr=(char*)shmat(shmid,NULL,0);
                curr=Round%2;
                for(int i=Range;i<M*N;++i){
                    I=i/N+1;J=i%N+1;
                    arr[curr*bias+I*(N+2)+J]=plive(arr,I,J,curr);
                }
                _exit(0);
            }
            wait(NULL);wait(NULL);
        }
        curr=Rounds%2;
        for(int i=1;i<=M;++i){
            for(int j=1;j<=N;++j){
                buf[(i-1)*(N+1)+j-1]=(Arr2[i*(N+2)+j+bias*curr]==1)?'O':'.';
            }  
        }
	    write(fd2,buf,M*(N+1)-1);return 0;
    }
    else{
        T=atoi(argv[2]);
        for(int i=0;i<=T;++i)IDS[i]=i;
        Range=(M*N)/T;
        Thread_nums=T;
        left=M*N-T*Range;
        pthread_t *tid=(pthread_t*)malloc(sizeof(pthread_t)*(Thread_nums+10));
        sem=(sem_t*)malloc(sizeof(sem_t)*T);
        for(int i=0;i<T;++i)sem_init(sem+i,0,0);
        sem_init(&sem2,0,0);
        Round=1;
        for(int j=0;j<Thread_nums;++j)pthread_create(tid+j,NULL,thread_process,(void*)(IDS+j));
        for(int i=1;i<=T;++i)sem_wait(&sem2);
    	//gettimeofday(&t,NULL);printf("read +fill 0 time=%d\n",(t.tv_sec-sec)*1000000+t.tv_usec-usec);sec=t.tv_sec;usec=t.tv_usec;
        sem_destroy(&sem2);
        sem_init(&sem2,0,0);
        for(int i=0;i<T;++i)sem_post(sem+i);
        for(int i=1;i<=T;++i)sem_wait(&sem2);
        //if(1==Rounds){gettimeofday(&t,NULL);printf("compute time=%d\n",(t.tv_sec-sec)*1000000+t.tv_usec-usec);sec=t.tv_sec;usec=t.tv_usec;}
        sem_destroy(&sem2);
        for(Round=2;Round<=Rounds;++Round){
            sem_init(&sem2,0,0);
            for(int i=0;i<T;++i)sem_post(sem+i);
            for(int i=1;i<=T;++i)sem_wait(&sem2);
            sem_destroy(&sem2);
	        //if(Round==Rounds){gettimeofday(&t,NULL);printf("compute time=%d\n",(t.tv_sec-sec)*1000000+t.tv_usec-usec);sec=t.tv_sec;usec=t.tv_usec;}
        }
	    for(int j=0;j<Thread_nums;++j)sem_post(sem+j);
	    for(int j=0;j<T;++j)pthread_join(tid[j],NULL);
	    write(fd2,buf,M*(N+1)-1);
        return 0;
    }
    
    //gettimeofday(&t,NULL);printf("write time=%d\n",(t.tv_sec-sec)*1000000+t.tv_usec-usec);sec=t.tv_sec;usec=t.tv_usec;
    fclose(wfp);fclose(rfp);
    return 0;
}

