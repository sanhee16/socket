#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>


#include "fileopen.h"
//#include "making_rt.h"
#define CLI_NUM 2
#define MAX_BUF 100
#define CT_SIZE 4*ROU_NUM*ROU_NUM



pthread_mutex_t lock;
pthread_mutex_t data_lock_read;
pthread_mutex_t data_lock_arr[MAX_BUF];

pthread_mutex_t ct_lock[MAX_BUF];



int edge[ROU_NUM];
int fin_table[ROU_NUM];
int make_table=0;
int rt_done=0;
typedef struct routing{
	int dest[ROU_NUM];
	int next[ROU_NUM];
	int cost[ROU_NUM];
}route_table;

route_table rt;

void print_CT();

int client_connect[ROU_NUM]={0, };
//#include "data_handle_router.h"

pthread_t tids[100];
pthread_t datatids[100];
pthread_t rcv_thread[100];
pthread_t snd_thread[100];
pthread_t server;
pthread_t client;
pthread_t making_rr;
pthread_t cli_srv_connect_thread;
pthread_t data_srv_thread;

typedef struct snd_ct{
	int ct_buffer[ROU_NUM][ROU_NUM];
	int exist_buf;
	int buf_count;
}SND_CT;

SND_CT ct_buf[MAX_BUF];

int router_num;
//int exist_buf[MAX_BUF]=0;
//int buf_count[MAX_BUF]=0;



int thds=0;

int neighbor_sock[ROU_NUM] = {-1, };
int neighbor_sock_srv[ROU_NUM] = {-1, };
int data_neighbor_sock[ROU_NUM]={-1, };
int client_num;
int is_fin = 0;
int close_cli;
int get_buf[ROU_NUM];
int fin_costtable[ROU_NUM]={0,};
int data_get_buf[ROU_NUM];


/*

   typedef struct snd_ct{
   int CT[ROU_NUM][ROU_NUM];
   int visit[ROU_NUM];
   int finish;
   int check_finish[ROU_NUM];
   int check_fin;
   }SND_CT;

   typedef struct buf{
   SND_CT recv_buf;
   int cli_sockfd;
   }BUF;
 */
/*
   typedef struct snd_ct{
   int CT[ROU_NUM][ROU_NUM];
//      int visit[ROU_NUM];
//      int finish;
//      int check_finish[ROU_NUM];
//      int check_fin;
}SND_CT;

typedef struct buf{
SND_CT recv_buf;
int cli_sockfd;
}BUF;

BUF ct_buffer[MAX_BUF];
 */

///////////////////////////////////////////////DATA/////////////
pthread_t data_rcv_thread[100];
pthread_t data_snd_thread[100];
pthread_t data_rcv_thread_srv;
pthread_t data_snd_thread_srv;

int real_srv_sockfd;
int real_cli_sockfd[ROU_NUM-1];
int real_cli_srv_sockfd=-1;


static void * data_cli_handle(void * arg);
static void * srv_listen_handler(void * arg);
static void * data_srv_handle(void * arg);
static void *data_sndhandle(void * arg);
static void *data_rcvhandle(void * arg);
static void * data_srv_connect_handle(void * arg);
static void * data_srv_listen_handler(void * arg);
static void * RT_handler(void *arg);

pthread_t data_client;
pthread_t data_server;
pthread_mutex_t data_lock;

typedef struct msg_data{
	char snd_ip[16];
	char recv_ip[16];
	int snd_port;
	int recv_port;
	char msg[360];
	// this structure size is 400
}MSG_T;

typedef struct buf_data{
	MSG_T data_recv_buf;
	int cli_sockfd;
}DATA_BUF;


MSG_T buffer_arr[MAX_BUF];
int data_exist_buf_arr[MAX_BUF] ={0, };

DATA_BUF data_buffer;
int data_exist_buf=0;
int data_router_num=0;
int connect_rou_data(char*);


char* server_ip = "220.149.244.211";
char* client_ip[CLI_NUM];

//BUF buffer;

/*
   void print_snd(SND_CT pp){
   printf("CT \n");
   for(int a=0;a<ROU_NUM;a++){
   for(int b=0;b<ROU_NUM;b++){
   printf("%d ",pp.CT[a][b]);
   }
   printf("\n");
   }
   printf("\nvisit \n");
   for(int a=0;a<ROU_NUM;a++){
   printf("%d ",pp.visit[a]);
   }
   printf("\nvisit finish\n");
   for(int a=0;a<ROU_NUM;a++){
   printf("%d ",pp.check_finish[a]);
   }

   printf("\nfinish\n %d\n",pp.finish);

   }
 */
//pthread_mutex_t lock;
pthread_cond_t cond;


//BUF buffer;

static void * rcvhandle(void *);
static void * sndhandle(void *);

static void * srv_handle(void *);
static void * cli_handle(void *);


void arr_copy(int(*arr)[ROU_NUM], int(*copy)[ROU_NUM]);
//void print_CT();
int connect_rou(char* );
int main(int argc, char *argv[])
{
	makeCT();
	pthread_mutex_init(&lock, NULL);
	pthread_mutex_init(&data_lock_read,NULL);
	pthread_mutex_init(&data_lock, NULL);
	for(int a=0;a<MAX_BUF;a++){
		pthread_mutex_init(&data_lock_arr[a], NULL);
		pthread_mutex_init(&ct_lock[a], NULL);
	}
	pthread_create(&making_rr,NULL,RT_handler,NULL);
	pthread_create(&server, NULL, srv_handle, NULL);
	while(1){
	}
}
static void * srv_handle(void * arg)
{
	int srv_sock, cli_sock;
	int port_num, ret1;
	struct sockaddr_in addr;
	int len;
	router_num=0;
	port_num = 1621;

	srv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (srv_sock == -1)
	{
		perror("Server socket CREATE fail!!");
		return 0;

	}

	// addr binding
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htons (INADDR_ANY);
	addr.sin_port = htons (port_num);

	int nSockOpt=1;
	setsockopt(srv_sock,SOL_SOCKET,SO_REUSEADDR,&nSockOpt,sizeof(nSockOpt));

	ret1 = bind (srv_sock, (struct sockaddr *)&addr, sizeof(addr));

	if (ret1 == -1)
	{
		perror("BIND error!!");
		close(srv_sock);
		return 0;
	}
	pthread_create(&client, NULL, cli_handle, NULL);

	printf("route bind\n");

	int cli_sockarr;
	while(1){
		ret1= listen(srv_sock, 0);
		perror("listen");

		if (ret1 == -1)
		{
			perror("LISTEN stanby mode fail");
			close(srv_sock);
			return 0;
		}

		cli_sockarr = accept(srv_sock, (struct sockaddr *)NULL, NULL);
		if (cli_sockarr== -1){
			close(srv_sock);
		}

		pthread_create(&tids[thds], NULL, srv_listen_handler, &cli_sockarr);
		thds++;
	}
}

static void * srv_listen_handler(void * arg){
	int cli_sock = *(int *)arg;

	int ret = -1;
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	/* get peer addr */
	struct sockaddr peer_addr;
	socklen_t peer_addr_len;
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr_len = sizeof(peer_addr);
	ret = getpeername(cli_sock, &peer_addr, &peer_addr_len);
	ret = getnameinfo(&peer_addr, peer_addr_len,
			hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
			NI_NUMERICHOST | NI_NUMERICSERV);


	if(ret != 0)
	{
		ret = -1;
		pthread_exit(&ret);
	}

	int con=-1;
	if(*(hbuf+14)=='1'){
		neighbor_sock_srv[0]=cli_sock;
		con=0;
	}
	else if(*(hbuf+14)=='2'){
		neighbor_sock_srv[1]=cli_sock;
		con=1;
	}
	else if(*(hbuf+14)=='3'){
		neighbor_sock_srv[2]=cli_sock;
		con=2;
	}
	else if(*(hbuf+14)=='4'){
		neighbor_sock_srv[3]=cli_sock;
		con=3;
	}
	else if(*(hbuf+14)=='5'){
		neighbor_sock_srv[4]=cli_sock;
		con=4;
	}

	printf("route : ip %s \n clisock %d \n",hbuf,cli_sock);
	pthread_create(&rcv_thread[router_num],NULL,rcvhandle,&cli_sock);
	client_connect[con]=1;
	router_num++;

}

static void * cli_handle(void *arg){
	int make_cli[ROU_NUM] = {0, };
	while(1){
		for(int a=0;a<ROU_NUM;a++){
			if(my_neighbor[a]==1 && make_cli[a]==0){
				char* send_ip;
				if(a==0)
					send_ip="220.149.244.211";
				else if(a==1)
					send_ip="220.149.244.212";
				else if(a==2)
					send_ip="220.149.244.213";
				else if(a==3)
					send_ip="220.149.244.214";
				else if(a==4)
					send_ip="220.149.244.215";

				int make_fd = connect_rou(send_ip);
				neighbor_sock[a]=make_fd;
				if(make_fd==-1){
					continue;
				}

				if(make_cli[a]==0){
					pthread_create(&snd_thread[router_num],NULL,sndhandle,&make_fd);
					router_num++;
					make_cli[a]=1;
					printf("make cli fd %d ip %s \n\n",make_fd, send_ip);
				}
			}
		}
	}
}



int connect_rou(char* send_ip){
	int port = 1621;
	int fd_sock;
	int ret;
	int len;
	struct sockaddr_in addr;

	fd_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_sock == -1) {
		perror("socket");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);
	inet_pton(AF_INET, send_ip, &addr.sin_addr);
	ret = connect(fd_sock, (struct sockaddr *)&addr, sizeof(addr));
	if(ret == -1){
		//perror("connect");
		close(fd_sock);
		return -1;
	}
	return fd_sock;

}


int ptr_ct_read=0;

static void * rcvhandle(void *arg){
	int cli_sockfd = *(int *)arg;
	int ct_read=0;
	while(1){
		int len;
		int get_ct[ROU_NUM][ROU_NUM];
		len = recv(cli_sockfd, &get_ct, sizeof(get_ct), 0);
		if(len<0)
			continue;
		while(1){
			pthread_mutex_lock(&lock);

			ct_read=ptr_ct_read;
			ptr_ct_read++;
			if(ptr_ct_read==MAX_BUF){
				ptr_ct_read=0;
			}
			if(ct_buf[ct_read].exist_buf==1){
				pthread_mutex_unlock(&lock);
				continue;
			}
			pthread_mutex_lock(&ct_lock[ct_read]);

			if(ct_buf[ct_read].exist_buf==1){
				pthread_mutex_unlock(&ct_lock[ct_read]);
				continue;
			}
			memcpy(&(ct_buf[ct_read].ct_buffer), &get_ct , sizeof(get_ct));
			ct_buf[ct_read].exist_buf = 1;
			ct_buf[ct_read].buf_count=client_num;
			fflush(NULL);

			pthread_mutex_unlock(&lock);
			pthread_mutex_unlock(&ct_lock[ct_read]);
			break;
		}
	}
	while(1);
}

int ptr_ct_snd=0;

static void * sndhandle(void *arg){
	int cli_sockfd = *(int *)arg;
	//
	size_t getline_len;
	int ret;
	//SND_CT first;
	int ct_snd=0;
	int takeit[MAX_BUF]={0, };

	int first[ROU_NUM][ROU_NUM];
	memset(&first,0,CT_SIZE);
	arr_copy(first,CT);
	send(cli_sockfd, (char*)&first, sizeof(first), 0);

	while(1){
		pthread_mutex_lock(&lock);
		ct_snd=ptr_ct_snd;
		ptr_ct_snd++;
		if(ptr_ct_snd==MAX_BUF){
			ptr_ct_snd=0;
		}
		if(ct_buf[ct_snd].exist_buf==0){
			pthread_mutex_unlock(&lock);
			continue;
		}

		pthread_mutex_lock(&ct_lock[ct_snd]);

		//pthread_mutex_unlock(&lock);
		if(ct_buf[ct_snd].exist_buf==0){
			pthread_mutex_unlock(&ct_lock[ct_snd]);
			pthread_mutex_unlock(&lock);
			continue;
		}

		if(ct_buf[ct_snd].exist_buf==1 && takeit[ct_snd]==0){
			for(int a=0;a<ROU_NUM;a++){
				for(int b=0;b<ROU_NUM;b++){
					if(ct_buf[ct_snd].ct_buffer[a][b]!=INFINITE && CT[a][b]== INFINITE){
						printf("[%d][%d] %d -> %d \n",a,b,CT[a][b],ct_buf[ct_snd].ct_buffer[a][b]);
						CT[a][b]=ct_buf[ct_snd].ct_buffer[a][b];
					}
				}
			}
			arr_copy(ct_buf[ct_snd].ct_buffer, CT);

			takeit[ct_snd]=1;

			send(cli_sockfd,(char*)&ct_buf[ct_snd].ct_buffer, sizeof(ct_buf[ct_snd].ct_buffer),0);
			//send(cli_sockfd,(char*)&ct_buf[ct_snd],sizeof(SND_CT),0);

			ct_buf[ct_snd].buf_count--;
			if(ct_buf[ct_snd].buf_count==0){
				memset(&ct_buf[ct_snd], 0, sizeof(SND_CT));
				takeit[ct_snd]=0;
				ct_buf[ct_snd].exist_buf=0;
			}

			fflush(NULL);
			pthread_mutex_unlock(&lock);
			pthread_mutex_unlock(&ct_lock[ct_snd]);
			continue;
		}
		pthread_mutex_unlock(&ct_lock[ct_snd]);
		pthread_mutex_unlock(&lock);
	}
}

/*
   static void * sndhandle(void *arg){
   int cli_sockfd = *(int *)arg;

   size_t getline_len;
   int ret;
   int done=0;
   SND_CT first;

   arr_copy(first.CT,CT);
   for(int a=0;a<ROU_NUM;a++){
   first.visit[a]=0;
   first.check_finish[a]=0;
   }
   first.visit[my_num]=1;
   first.finish=0;
   send(cli_sockfd, (char*)&first, sizeof(SND_CT), 0);
//perror("send");
//RT_handler(&done);
pthread_create(&making_rr[my_num],NULL,RT_handler,&done);
while(1){
pthread_mutex_lock(&lock);

if(done==1){
make_table=1;
}
if(exist_buf==1){
SND_CT snd_ct;
memcpy(&snd_ct,&(buffer.recv_buf),sizeof(SND_CT));
if(buffer.recv_buf.check_fin==1){
make_table=1;
}

int snd_sockfd = buffer.cli_sockfd;


for(int a=0;a<ROU_NUM;a++){
for(int b=0;b<ROU_NUM;b++){
if(buffer.recv_buf.CT[a][b]==INFINITE && CT[a][b]==INFINITE){
CT[a][b]=CT[a][b];
}
else if(buffer.recv_buf.CT[a][b]!=INFINITE && CT[a][b]==INFINITE){
CT[a][b]=buffer.recv_buf.CT[a][b];
}
else if(buffer.recv_buf.CT[a][b]== INFINITE && CT[a][b]!=INFINITE){
buffer.recv_buf.CT[a][b]=CT[a][b];
CT[a][b]=CT[a][b];
}
else if(buffer.recv_buf.CT[a][b]!=INFINITE && CT[a][b]!=INFINITE){
CT[a][b]=buffer.recv_buf.CT[a][b];
}
}
}
//print_CT();
for(int a=0;a<ROU_NUM;a++){
if(my_neighbor[a]==1 && (cli_sockfd == neighbor_sock[a])){ // my neighbor and thread's connected node
arr_copy(snd_ct.CT,CT);
snd_ct.visit[my_num]=1;
for(int x=0;x<ROU_NUM;x++){
if(snd_ct.visit[x]==1){

}
else{
snd_ct.finish=0;
break;
}
snd_ct.finish=1;
}
if(buffer.recv_buf.finish==1){
for(int x=0;x<ROU_NUM;x++){
if(snd_ct.check_finish[x]==1){
}
else if(snd_ct.check_finish[x]!=1){
	snd_ct.check_fin=0;
	done=0;
	break;
}
snd_ct.check_fin=1;
done=1;
}
}

int len = sizeof(snd_ct);
send(neighbor_sock[a],(char*)&snd_ct, sizeof(SND_CT), 0);
get_buf[my_num]=1;

buf_count--;
for(int i=0;i<client_num;i++){
	if(get_buf[i]==0){

		break;
	}
}
if(buf_count==0){
	exist_buf=0;
	memset(&buffer,0,sizeof(buffer));
}
}
}
}
fflush(NULL);
pthread_mutex_unlock(&lock);
}
while(1);
}
*/

void arr_copy(int(*arr)[ROU_NUM], int(*copy)[ROU_NUM]){
	for(int a=0;a<ROU_NUM;a++){
		for(int b=0;b<ROU_NUM;b++){
			arr[a][b] = copy[a][b];
		}
	}
}


static void * data_srv_connect_handle(void * arg){
	int port = 4712;
	int fd_sock;
	int ret;
	int len;
	struct sockaddr_in addr;

	char send_ip[16];

	fd_sock = socket(AF_INET, SOCK_STREAM, 0);

	real_cli_srv_sockfd=-1;
	if(my_num==0){
		strcpy(send_ip,"220.149.244.211");
		real_cli_srv_sockfd=fd_sock;
		//real_srv_sockfd=fd_sock;
	}
	else if(my_num==1){
		strcpy(send_ip,"220.149.244.212");
		real_cli_srv_sockfd=fd_sock;
		//real_cli_sockfd[0]=fd_sock;
	}
	else if(my_num==2){
		strcpy(send_ip,"220.149.244.213");
		real_cli_srv_sockfd=fd_sock;

		//real_cli_sockfd[1]=fd_sock;
	}

	if (fd_sock == -1) {
		perror("socket");
	}

	printf("semnd ip %s \n",send_ip);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);
	inet_pton(AF_INET, send_ip, &addr.sin_addr);

	//connect
	ret = connect(fd_sock, (struct sockaddr *)&addr, sizeof(addr));
	if(ret == -1){
		//perror("connect");
		close(fd_sock);
	}
	printf("make router - server/client connect");

	pthread_create(&data_rcv_thread_srv,NULL,data_rcvhandle,&fd_sock);
	pthread_create(&data_snd_thread_srv,NULL,data_sndhandle,&fd_sock);

	while(1);
	//return fd_sock;
}


static void * data_srv_handle(void * arg){
	/*
	   while(1){
	   if(rt_done==1)
	   break;
	   }
	 */
	printf("ready to connect \n");
	int srv_sock, cli_sock;
	int port_num, ret1;
	struct sockaddr_in addr;
	int len;

	data_router_num=0;
	port_num = 1721;

	//int cli_sockarr = (int *)malloc(sizeof(int)*ROU_NUM);
	// socket creation
	srv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (srv_sock == -1) {
		perror("Server socket CREATE fail!!");
		return 0;
	}

	// addr binding
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htons (INADDR_ANY);
	addr.sin_port = htons (port_num);

	int nSockOpt=1;
	setsockopt(srv_sock,SOL_SOCKET,SO_REUSEADDR,&nSockOpt,sizeof(nSockOpt));
	ret1 = bind (srv_sock, (struct sockaddr *)&addr, sizeof(addr));
	printf("bind data socket\n");
	if (ret1 == -1) {
		perror("BIND error!!");
		close(srv_sock);
		return 0;
	}
	pthread_create(&data_client, NULL, data_cli_handle, NULL);
	printf("data bind\n");
	int count_srv=0;
	for(int a=0;a<ROU_NUM;a++){
		if(my_neighbor[a]==1)
			count_srv++;
	}
	printf("count %d ", count_srv);
	//int* cli_sockarr = (int *)malloc(sizeof(int)*count_srv);

	int cli_sockarr;

	for(;;){
		ret1 = listen(srv_sock, 0);
		if (ret1 == -1) {
			perror("LISTEN stanby mode fail");
			close(srv_sock);
			return 0;
		}

		cli_sockarr = accept(srv_sock, (struct sockaddr *)NULL, NULL);

		if (cli_sockarr== -1){
			close(srv_sock);
		}
		pthread_create(&datatids[thds], NULL, data_srv_listen_handler, &cli_sockarr);
		thds++;
	}
}
static void * data_srv_listen_handler(void * arg){

	int cli_sock = *(int *)arg;

	int ret = -1;
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	/* get peer addr */
	struct sockaddr peer_addr;
	socklen_t peer_addr_len;
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr_len = sizeof(peer_addr);
	ret = getpeername(cli_sock, &peer_addr, &peer_addr_len);
	ret = getnameinfo(&peer_addr, peer_addr_len,
			hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
			NI_NUMERICHOST | NI_NUMERICSERV);

	if(ret != 0){
		ret = -1;
		pthread_exit(&ret);
	}

	//      pthread_create(&data_rcv_thread[data_router_num],NULL,data_rcvhandle,&cli_sockarr[a]);
	//      data_router_num++;
	//      a++;

	printf("make data thread ip %s clisock %d \n",hbuf,cli_sock);
	pthread_create(&data_rcv_thread[data_router_num],NULL,data_rcvhandle,&cli_sock);
	data_router_num++;

	while(1){
	}
}


static void * data_cli_handle(void *arg){
	int con_done[5] = {0, };
	int all_done=0;
	//	printf("\n\n\n\nmake data client \n\n\n\n\n");
	//	printf("neighbot %d %d %d %d ",my_neighbor[0],my_neighbor[1],my_neighbor[2],my_neighbor[3]);
	while(1){
		//if(all_done==0){
		for(int a=0;a<ROU_NUM;a++){
			if(my_neighbor[a]==1 && con_done[a]==0){
				printf("neighbot %d %d %d %d",my_neighbor[0],my_neighbor[1],my_neighbor[2],my_neighbor[3]);

				char* send_ip;
				if(a==0)
					send_ip="220.149.244.211";
				else if(a==1)
					send_ip="220.149.244.212";
				else if(a==2)
					send_ip="220.149.244.213";
				else if(a==3)
					send_ip="220.149.244.214";
				else if(a==4)
					send_ip="220.149.244.215";

				int make_fd = connect_rou_data(send_ip);
				data_neighbor_sock[a]=make_fd;

				//printf("connect sock %d \n\n",data_neighbor_sock[a]);
				if(make_fd==-1){
					con_done[a]=0;
					continue;
				}
				con_done[a]=1;

				pthread_create(&data_snd_thread[data_router_num],NULL,data_sndhandle,&make_fd);
				printf("make data cli \n\n");
				//data_client_num++;
				data_router_num++;
			}
			else{
				con_done[a]=1;
			}
		}
		for(int a=0;a<5;a++){
			if(con_done[a]==0){
				break;
			}
			if(a==4)
				all_done=1;
		}
		//}
	}
	//while(1);
}

int connect_rou_data(char* send_ip){
	int port = 1721;
	int fd_sock;
	int ret;
	int len;
	struct sockaddr_in addr;

	printf("send ip is %s \n",send_ip);
	fd_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_sock == -1) {
		perror("socket");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);
	inet_pton(AF_INET, send_ip, &addr.sin_addr);
	ret = connect(fd_sock, (struct sockaddr *)&addr, sizeof(addr));
	if(ret == -1){
		//      perror("connect");
		close(fd_sock);
		return -1;
	}
	return fd_sock;

}

int ptr_read=0;
static void * data_rcvhandle(void *arg){
	int cli_sockfd = *(int *)arg;
	//printf("rcv %d \n",cli_sockfd);
	printf("hello");

	int ch_read=0;
	//int ptr_read=0;
	int done=0;
	while(1){
		fflush(NULL);
		MSG_T get_msg;
		memset(&get_msg,0,sizeof(MSG_T));

		int len;
		len = recv(cli_sockfd, (char *)&get_msg, sizeof(MSG_T), 0);
		if(len<0){
			continue;
		}
		while(1){
			fflush(NULL);
			pthread_mutex_lock(&data_lock_read);
			//pthread_mutex_lock(&data_lock_arr[ptr_read]);
			ch_read = ptr_read;
			ptr_read++;
			if(ptr_read==MAX_BUF){
				ptr_read=0;
			}
			pthread_mutex_lock(&data_lock_arr[ch_read]);
			pthread_mutex_unlock(&data_lock_read);

			if(data_exist_buf_arr[ch_read]==1){
				pthread_mutex_unlock(&data_lock_arr[ch_read]);
				continue;
			}

			printf("\ndata rcv(data) : %s ",get_msg.msg);
			printf("data rcv(snd ip) : %s \n",get_msg.snd_ip);
			printf("data rcv(recv ip) : %s \n",get_msg.recv_ip);

			strncpy(data_buffer.data_recv_buf.msg, get_msg.msg,360);
			strncpy(data_buffer.data_recv_buf.snd_ip, get_msg.snd_ip,16);
			strncpy(data_buffer.data_recv_buf.recv_ip, get_msg.recv_ip,16);

			data_buffer.data_recv_buf.snd_port = get_msg.snd_port;
			data_buffer.data_recv_buf.recv_port = get_msg.recv_port;

			data_buffer.cli_sockfd = cli_sockfd;
			data_exist_buf_arr[ch_read]=1;
			memcpy(&buffer_arr[ch_read],&data_buffer.data_recv_buf,sizeof(MSG_T));
			fflush(NULL);
			pthread_mutex_unlock(&data_lock_arr[ch_read]);
			printf("read buffer in %d \n",ch_read);
			printf("break \n");
			break;
		}
	}
	while(1);
}

int ptr_cons=0;

static void * data_sndhandle(void *arg){
	int cli_sockfd = *(int *)arg;
	int ch=0;
	size_t getline_len;
	int ret;
	int done=0;
	for(int a=0;a<ROU_NUM;a++){
		printf("RT \n");
		printf("%d %d %d ",rt.dest[a],rt.next[a],rt.cost[a]);
	}
	//print_CT();
	while(1){
		fflush(NULL);

		pthread_mutex_lock(&data_lock);
		//              printf("snd in the mutex !\n ");
		ch=ptr_cons;
		ptr_cons++;
		if(ptr_cons==MAX_BUF){
			ptr_cons=0;
		}
		pthread_mutex_lock(&data_lock_arr[ch]);
		pthread_mutex_unlock(&data_lock);

		//              printf("snd in the mutex ch!\n ");

		if(data_exist_buf_arr[ch]==0){
			pthread_mutex_unlock(&data_lock_arr[ch]);
			continue;
		}
		if(data_exist_buf_arr[ch]==1){
			//printf("buffer! \n");
			MSG_T snd_msg;
			memset(&snd_msg,0,sizeof(MSG_T));
			memcpy(&snd_msg,&buffer_arr[ch],sizeof(MSG_T));

			int compare=-1;
			if(*(snd_msg.recv_ip+14)=='1'){
				compare=0;
			}
			else if(*(snd_msg.recv_ip+14)=='2'){
				compare=1;
			}
			else if(*(snd_msg.recv_ip+14)=='3'){
				compare=2;
			}
			else if(*(snd_msg.recv_ip+14)=='4'){
				compare=3;
			}
			else if(*(snd_msg.recv_ip+14)=='5'){
				compare=4;
			}

			printf("recv ip is %s \n",snd_msg.snd_ip);
			printf("compare %d my num %d \n\n",compare,my_num);
			if(compare==my_num){
				if(real_cli_srv_sockfd==cli_sockfd){
					//pthread_mutex_lock(&data_lock);
					//if this thread is connected to server, then send msg
					send(cli_sockfd,(char*)&snd_msg, sizeof(MSG_T), 0);
					perror("send");
					printf("send to server !\n");
					data_exist_buf_arr[ch]=0;
					memset(&buffer_arr[ch],0,sizeof(MSG_T));
					memset(&data_buffer,0,sizeof(DATA_BUF));
					fflush(NULL);
					pthread_mutex_unlock(&data_lock_arr[ch]);
					continue;
				}
				else{
					pthread_mutex_unlock(&data_lock_arr[ch]);
					continue;
				}
			}
			int snd_sockfd=-1;
			int dest_num=-1;
			if(*(snd_msg.recv_ip + 14)=='1'){
				dest_num=0;
			}
			else if(*(snd_msg.recv_ip + 14)=='2'){
				dest_num=1;
			}
			else if(*(snd_msg.recv_ip + 14)=='3'){
				dest_num=2;
			}
			else if(*(snd_msg.recv_ip + 14)=='4'){
				dest_num=3;
			}
			else if(*(snd_msg.recv_ip + 14)=='5'){
				dest_num=4;
			}

			for(int a=0;a<ROU_NUM;a++){
				if(a==my_num){
					//do not check mine
					continue;
				}
				if(rt.dest[a]==dest_num){
					fflush(NULL);

					snd_sockfd=rt.next[a];
					break;
				}
				else{
					fflush(NULL);
				}
			}

			fflush(NULL);

			if(data_neighbor_sock[snd_sockfd]==cli_sockfd){
				fflush(NULL);
				send(cli_sockfd,(char*)&snd_msg, sizeof(MSG_T), 0);
				perror("send");

				printf("send ip is %s \n",snd_msg.snd_ip);
				printf("send to router! \n");
				data_exist_buf_arr[ch]=0;
				memset(&buffer_arr[ch],0,sizeof(MSG_T));
				memset(&data_buffer,0,sizeof(DATA_BUF));

				pthread_mutex_unlock(&data_lock_arr[ch]);
				fflush(NULL);
				fflush(stdin);

			}
			pthread_mutex_unlock(&data_lock_arr[ch]);
			fflush(NULL);
			fflush(stdin);
			//pthread_mutex_unlock(&data_lock);

		} //buffer exist

		//end while loop
		//`printf("\n\n---------done-----------\n\n");
		//print_CT();
		//while(1);
	}
	while(1);
}







static void * RT_handler(void *arg){
	//makeCT();
	//print_CT();
	//int done = *(int *)arg;

	while(1){
		if(rt_done==1){
			/*
			   pthread_create(&data_srv_thread,NULL,data_srv_handle,NULL);
			   if(my_num==0 || my_num==1 || my_num==2){
			   pthread_create(&cli_srv_connect_thread, NULL, data_srv_connect_handle, NULL);
			   }
			 */
			/*
			printf("\n\n------routing table------\n\n");
			printf(" dest next cost \n");
			for(int a=0;a<ROU_NUM;a++){
				printf(" %3d  %3d  %3d",rt.dest[a],rt.next[a],rt.cost[a]);
				printf("\n");
			}

			printf("-----------------cost table --------------------\n");
			print_CT();
			*/
			continue;
		}
		pthread_mutex_lock(&lock);
		int d[ROU_NUM];
		int set_s[ROU_NUM];
		int set_c[ROU_NUM];

		for(int a=0;a<ROU_NUM;a++){
			d[a]=INFINITE;
			edge[a]=0;
			set_c[a]=1;
			set_s[a]=0;
			rt.dest[a]=a;
			rt.next[a]=-1;
			rt.cost[a]=-1;
		}
		for(int a=0;a<ROU_NUM;a++){
			//printf("%d ",d[a]);
		}
		int source = my_num;

		d[source]=0;
		int u=-1;
		int v=-1;
		int small=-1;
		int small_dist=INFINITE+1;

		for(int count=0; count<ROU_NUM-1; count++){
			small=-1;
			small_dist=INFINITE+1;
			u=-1; v=-1;
			for(int b=0;b<ROU_NUM;b++){
				if((set_c[b]==1) && (small_dist > d[b])){
					small_dist = d[b];
					small = b;
				}
			}
			set_c[small]=0;
			set_s[small]=1;
			u=small;
			for(int b=0;b<ROU_NUM;b++){
				v=-1;
				if(set_c[b]==0){
					continue;
				}
				v=b;
				if(d[v] > CT[u][v]+d[u]){
					d[v]=CT[u][v]+d[u];
					edge[v]=u;
					int search_n=v;
					while(1){
						if(edge[search_n]==source){
							rt.next[v]=search_n;
							break;
						}
						search_n=edge[search_n];
					}
					rt.cost[v]=d[v];
				}
			}
		}
		printf("\n\n------routing table------\n\n");
		printf(" dest next cost \n");
		for(int a=0;a<ROU_NUM;a++){
			printf(" %3d  %3d  %3d",rt.dest[a],rt.next[a],rt.cost[a]);
			printf("\n");
		}

		for(int a=0;a<ROU_NUM;a++){
			if(CT[a][a]==0){
				rt_done=1;
			}
			else{
				rt_done=0;
				break;
			}
		}

		if(rt_done==1){
			printf("-----------------cost table --------------------\n");
			print_CT();

		}
		pthread_mutex_unlock(&lock);
	}
	while(1);
	return 0;
}



void print_CT(){
	for(int a=0;a<ROU_NUM;a++){
		for(int b=0;b<ROU_NUM;b++){
			printf("%6d ",CT[a][b]);
		}
		printf("\n");
	}
}


