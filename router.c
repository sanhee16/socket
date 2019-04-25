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

//#define ROU_NUM 5
//#define my_num 1

pthread_t tids[100];
pthread_t rcv_thread[100];
pthread_t snd_thread[100];
pthread_t server;
pthread_t client;


int router_num;
int exist_buf=0;
int buf_count=0;
//int CT[ROU_NUM][ROU_NUM] = {-1, };
int neighbor_sock[ROU_NUM] = {-1, };
int neighbor_sock_srv[ROU_NUM] = {-1, };
int client_num;
int is_fin = 0;
int close_cli;


typedef struct snd_ct{
	int CT[ROU_NUM][ROU_NUM];
	int visit[ROU_NUM];
	int finish;
	int check_finish[ROU_NUM];
}SND_CT;

typedef struct buf{
	SND_CT recv_buf;
	int cli_sockfd;
}BUF;

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

pthread_mutex_t lock;
pthread_cond_t cond;


BUF buffer;

static void * rcvhandle(void *);
static void * sndhandle(void *);
static void * handle(void *);

static void * srv_handle(void *);
static void * cli_handle(void *);


void arr_copy(int(*arr)[ROU_NUM], int(*copy)[ROU_NUM]);
void print_CT();
int connect_rou(char* );
int main(int argc, char *argv[])
{
	makeCT();
	print_CT();
	pthread_create(&server, NULL, srv_handle, NULL);
	pthread_create(&client, NULL, cli_handle, NULL);

	while(1){
	}
}

static void * srv_handle(void * arg){
	int srv_sock, cli_sock;
	int port_num, ret;
	struct sockaddr_in addr;
	int len;
	router_num=0;
	port_num = 1621;

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

	ret = bind (srv_sock, (struct sockaddr *)&addr, sizeof(addr));

	if (ret == -1) {
		perror("BIND error!!");
		close(srv_sock);
		return 0;
	}
	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&cond, NULL);

	printf("bind\n");

	int count_srv=0;
	for(int a=0;a<ROU_NUM;a++){
		if(my_neighbor[a]==1)
			count_srv++;
	}	
	printf("count %d ", count_srv);
	int* cli_sockarr = (int *)malloc(sizeof(int)*count_srv);
	for(int a=0; a<count_srv; a++){

		ret = listen(srv_sock, 0);
		if (ret == -1) {
			perror("LISTEN stanby mode fail");
			close(srv_sock);
			return 0;
		}

		cli_sockarr[a] = accept(srv_sock, (struct sockaddr *)NULL, NULL);

		printf("listen %d",a);
		int ret = -1;
		char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
		/* get peer addr */
		struct sockaddr peer_addr;
		socklen_t peer_addr_len;
		memset(&peer_addr, 0, sizeof(peer_addr));
		peer_addr_len = sizeof(peer_addr);
		ret = getpeername(cli_sockarr[a], &peer_addr, &peer_addr_len);
		ret = getnameinfo(&peer_addr, peer_addr_len,
				hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
				NI_NUMERICHOST | NI_NUMERICSERV);

		if(ret != 0){
			ret = -1;
			pthread_exit(&ret);
		}


		if (cli_sockarr[a] == -1) {
			perror("cli_sock connect ACCEPT fail");
			close(srv_sock);
		}

		if(*(hbuf+14)=='1'){
			neighbor_sock_srv[0]=cli_sockarr[a];
		}
		else if(*(hbuf+14)=='2'){
			neighbor_sock_srv[1]=cli_sockarr[a];
		}
		else if(*(hbuf+14)=='3'){
			neighbor_sock_srv[2]=cli_sockarr[a];
		}

		else if(*(hbuf+14)=='4'){
			neighbor_sock_srv[3]=cli_sockarr[a];
		}

		else if(*(hbuf+14)=='5'){
			neighbor_sock_srv[4]=cli_sockarr[a];
		}

	}
	for(int a=0;a<count_srv;a++){
		printf("make thread \n");
		pthread_create(&rcv_thread[router_num],NULL,rcvhandle,&cli_sockarr[a]);
		router_num++;
	}

	while(1){
		if(is_fin == 1)
			break;
	}
	printf("\n\n-------------server finish----------------------\n\n");
	print_CT();

}

static void * cli_handle(void *arg){

	int con_done[5] = {0, };
	int all_done=0;
	while(1){
		if(is_fin == 1)
			break;
		if(all_done==0){
			for(int a=0;a<5;a++){
				if(my_neighbor[a]==1 && con_done[a]==0){
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
						//printf("cannot make connection");
					}
					con_done[a]=1;

					//pthread_create(&tids[router_num], NULL, handle, &make_fd);
					pthread_create(&snd_thread[router_num],NULL,sndhandle,&make_fd);
					//pthread_create(&rcv_thread[router_num],NULL,rcvhandle,&make_fd);
					printf("make cli \n\n");
					client_num++;
					router_num++;
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
		}
	}


	printf("\n\n-------------client finish----------------------\n\n");
	print_CT();
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

static void * handle(void * arg)
{
	int cli_sockfd = *(int *)arg;
	int ret = -1;
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

	/* get peer addr */
	struct sockaddr peer_addr;
	socklen_t peer_addr_len;
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr_len = sizeof(peer_addr);
	ret = getpeername(cli_sockfd, &peer_addr, &peer_addr_len);
	ret = getnameinfo(&peer_addr, peer_addr_len,
			hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
			NI_NUMERICHOST | NI_NUMERICSERV);

	if(ret != 0){
		ret = -1;
		pthread_exit(&ret);
	}


	int rcv_arg,snd_arg;
	if(*(hbuf+14) == '1'){
		rcv_arg=0;
		snd_arg=0;	
	}
	if(*(hbuf+14) == '2'){
		snd_arg=1;
		rcv_arg=1;
	}
	if(*(hbuf+14) == '3'){
		snd_arg=2;
		rcv_arg=2;
	}
	if(*(hbuf+14) == '4'){
		snd_arg=3;
		rcv_arg=3;
	}
	if(*(hbuf+14) == '5'){
		rcv_arg=4;
		snd_arg=4;
	}



	//pthread_create(&rcv_thread[cli_sockfd],NULL,rcvhandle,&rcv_arg);
	//pthread_create(&snd_thread[cli_sockfd],NULL,sndhandle,&cli_sockfd);


	//pthread_create(&snd_thread[cli_sockfd],NULL,sndhandle,&snd_arg);
	//printf("make rcv and snd \n\n");
	while(1);
}


static void * rcvhandle(void *arg){
	int cli_sockfd = *(int *)arg;
	//printf("rcv %d \n",cli_sockfd);
	
	while(1){
		SND_CT get_ct;
		memset(&(get_ct.CT),-1,sizeof(get_ct.CT));
		memset(&(get_ct.visit),0,sizeof(get_ct.visit));
		get_ct.finish=0;
		int len;
		SND_CT *get;
		int rcv_sock;
		for(int x=0;x<ROU_NUM;x++){
			if(neighbor_sock[x]==cli_sockfd){
				rcv_sock=x;
				break;
			}
		}

		//printf("loop rcv %d \n\n",cli_sockfd);
		len = recv(cli_sockfd, &get_ct, sizeof(SND_CT), 0);
		//perror("recv");

		//printf("------rcv--------- \n");
		//print_snd(get_ct);
		if (len < 0)
			break;

		pthread_mutex_lock(&lock);
		if(is_fin == 1){
			pthread_mutex_unlock(&lock);
			continue;
		}
		if(get_ct.finish==1){
			//printf("finish the table \n");
			get_ct.check_finish[my_num]=1;
		}
		get_ct.visit[my_num]=1;
		memcpy(&(buffer.recv_buf),&get_ct,sizeof(SND_CT));
		buffer.cli_sockfd = rcv_sock;
		exist_buf = 1;
		buf_count=client_num;
		fflush(NULL);
		pthread_mutex_unlock(&lock);
	}
	while(1);
}


static void * sndhandle(void *arg){
	int cli_sockfd = *(int *)arg;

	size_t getline_len;
	int ret;
	SND_CT first;
	//print_CT();

	int ch_fin=0;
	arr_copy(first.CT,CT);
	for(int a=0;a<ROU_NUM;a++){
		first.visit[a]=0;
		first.check_finish[a]=0;
	}
	first.finish=0;
	send(cli_sockfd, (char*)&first, sizeof(SND_CT), 0);
	//perror("send");

	while(1){
		pthread_mutex_lock(&lock);
		if(exist_buf==1){
			SND_CT snd_ct;
			memcpy(&snd_ct,&(buffer.recv_buf),sizeof(SND_CT));
			//printf("---------snd- %d ---------\n",cli_sockfd);
			
			ch_fin = 0;
			if(buffer.recv_buf.finish==1){
				ch_fin=1;
				for(int ch=0;ch<ROU_NUM;ch++){
					if(buffer.recv_buf.check_finish[ch]==0){
						ch_fin=0;
						break;
					}
				}
				if(ch_fin==1){
					is_fin=1;
					//print_CT();
					//close(buffer.cli_sockfd);
					pthread_mutex_unlock(&lock);
					//printf("close \n");
					exist_buf=0;
					//close_cli++;
					break;
				}
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
						CT[a][b]=CT[a][b];
					}
					else if(buffer.recv_buf.CT[a][b]!=INFINITE && CT[a][b]!=INFINITE){
						CT[a][b]=buffer.recv_buf.CT[a][b];
					}
				}

			}
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
					int len = sizeof(snd_ct);
					send(neighbor_sock[a],(char*)&snd_ct, sizeof(SND_CT), 0);
					buf_count--;
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
	        //`printf("\n\n---------done-----------\n\n");
		   	//print_CT();
			while(1);
}

void arr_copy(int(*arr)[ROU_NUM], int(*copy)[ROU_NUM]){
	for(int a=0;a<ROU_NUM;a++){
		for(int b=0;b<ROU_NUM;b++){
			arr[a][b] = copy[a][b];
		}
	}
}


void print_CT(){
	for(int a=0;a<ROU_NUM;a++){
		for(int b=0;b<ROU_NUM;b++){
			printf("%6d ",CT[a][b]);
		}
		printf("\n");
	}

}

