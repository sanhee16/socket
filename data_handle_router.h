
int neighbor_sock[ROU_NUM] = {-1, };
int neighbor_sock_srv[ROU_NUM] = {-1, };

pthread_t data_rcv_thread[100];
pthread_t data_snd_thread[100];
pthread_t data_rcv_thread_srv;
pthread_t data_snd_thread_srv;

int real_srv_sockfd;
int real_cli_sockfd[ROU_NUM-1];

static void * data_cli_handle(void * arg);
static void * data_srv_handle(void * arg);
static void *data_sndhandle(void * arg);
static void *data_rcvhandle(void * arg);

pthread_t data_client;
pthread_t data_server;
pthread_mutex_t data_lock;

typedef struct msg_data{
	char snd_ip[15];
	char recv_ip[15];
	int snd_port;
	int recv_port;
	char msg[362];
	// this structure size is 400
}MSG_T;

typedef struct buf_data{
	MSG_T recv_buf;
	int cli_sockfd;
}DATA_BUF;

DATA_BUF data_buffer;
int data_exist_buf=0;
int data_router_num=0;
int connect_rou_data(char*);
static void * data_srv_connect_handle(void * arg){
	int port = 4712;
	int fd_sock;
	int ret;
	int len;
	struct sockaddr_in addr;

	char send_ip[15];

	if(my_num==0){
		//send_ip = "220.149.244.211";
		strcpy(send_ip,"220.149.244.211");
		real_srv_sockfd=0;
		//strcpy(real_srv_sockfd,"220.149.244.211");
		//real_srv_sockfd = send_ip;
	}
	else if(my_num==1){
		strcpy(send_ip,"220.149.244.212");
		real_srv_sockfd=1;
		//strcpy(real_srv_sockfd,"220.149.244.212");
		//send_ip = "220.149.244.212";
		//real_cli_sockfd[0] = send_ip;
		//strcpy(real_srv_sockfd,send_ip);
	}
	else if(my_num==2){
		strcpy(send_ip,"220.149.244.213");
		real_srv_sockfd=2;
		//strcpy(real_srv_sockfd,"220.149.244.213");
		//strcpy(real_srv_sockfd,send_ip);
		//send_ip = "220.149.244.213";
		//real_cli_sockfd[1] = send_ip;
	}
	
	fd_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_sock == -1) {
		perror("socket");
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);
	inet_pton(AF_INET, send_ip, &addr.sin_addr);
	ret = connect(fd_sock, (struct sockaddr *)&addr, sizeof(addr));
	if(ret == -1){
		//perror("connect");
		close(fd_sock);
	}
	pthread_create(&data_rcv_thread_srv,NULL,data_srv_handle,&fd_sock);
	pthread_create(&data_snd_thread_srv,NULL,data_cli_handle,&fd_sock);

	while(1);
	//return fd_sock;
}


static void * data_srv_handle(void * arg){
	int srv_sock, cli_sock;
	int port_num, ret;
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

	ret = bind (srv_sock, (struct sockaddr *)&addr, sizeof(addr));
	printf("bind data socket\n");
	if (ret == -1) {
		perror("BIND error!!");
		close(srv_sock);
		return 0;
	}
	pthread_create(&data_client, NULL, data_cli_handle, NULL);

	pthread_mutex_init(&data_lock, NULL);

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
	}
	for(int a=0;a<count_srv;a++){
		printf("make thread \n");
		pthread_create(&data_rcv_thread[data_router_num],NULL,data_rcvhandle,&cli_sockarr[a]);
		data_router_num++;

	}

	while(1){
	}
}


static void * data_cli_handle(void *arg){
	int con_done[5] = {0, };
	int all_done=0;
	while(1){
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

					int make_fd = connect_rou_data(send_ip);
					//neighbor_sock[a]=make_fd;
					if(make_fd==-1){
						continue;
					}
					con_done[a]=1;

					pthread_create(&data_snd_thread[data_router_num],NULL,data_sndhandle,&make_fd);
					printf("make data cli \n\n");
					//data_client_num++;
					//data_router_num++;
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
}


int connect_rou_data(char* send_ip){
	int port = 1721;
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


static void * data_rcvhandle(void *arg){
	int cli_sockfd = *(int *)arg;
	//printf("rcv %d \n",cli_sockfd);
	int done=0;
	while(1){
		MSG_T get_msg;
		memset(&(get_msg),0,sizeof(get_msg));

		int len;
		int rcv_sock;
		for(int x=0;x<ROU_NUM;x++){
			if(neighbor_sock[x]==cli_sockfd){
				rcv_sock=x;
				break;
			}
		}

		len = recv(cli_sockfd, &get_msg, sizeof(MSG_T), 0);
		if(len<0)
			continue;
		pthread_mutex_lock(&data_lock);
		printf("data rcv : %s ",get_msg.msg);
		memcpy(&(data_buffer.recv_buf),&get_msg,sizeof(MSG_T));
		data_buffer.cli_sockfd = rcv_sock;

		data_exist_buf = 1;
		fflush(NULL);

		pthread_mutex_unlock(&data_lock);
	}
	while(1);
}




static void * data_sndhandle(void *arg){
	int cli_sockfd = *(int *)arg;

	size_t getline_len;
	int ret;
	int done=0;
	while(1){
		pthread_mutex_lock(&data_lock);

		if(data_exist_buf==1){
			MSG_T snd_msg;
			memcpy(&snd_msg,&(data_buffer.recv_buf),sizeof(MSG_T));
			if(real_srv_sockfd==cli_sockfd){
				send(cli_sockfd,(char*)&snd_msg, sizeof(MSG_T), 0);
				data_exist_buf=0;
				memset(&data_buffer,0,sizeof(DATA_BUF));
				fflush(NULL);
				pthread_mutex_unlock(&data_lock);
				continue;
			}
			for(int a=0;a<ROU_NUM-1;a++){
				if(real_cli_sockfd[a]==cli_sockfd){
					send(cli_sockfd,(char*)&snd_msg, sizeof(MSG_T), 0);
					data_exist_buf=0;
					memset(&data_buffer,0,sizeof(DATA_BUF));
					fflush(NULL);
					pthread_mutex_unlock(&data_lock);
					continue;
				}
			}

			int snd_sockfd=-1;
			//check routing table (rt) -> set snd_sockfd
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
				if(rt.dest[a]==dest_num){
					snd_sockfd=rt.next[a];
					break;
				}
			}
			send(neighbor_sock[snd_sockfd],(char*)&snd_msg, sizeof(MSG_T), 0);
			data_exist_buf=0;
			memset(&data_buffer,0,sizeof(DATA_BUF));
			fflush(NULL);
		}
		fflush(NULL);
		pthread_mutex_unlock(&data_lock);

	}
	//`printf("\n\n---------done-----------\n\n");
	//print_CT();
	while(1);
}
