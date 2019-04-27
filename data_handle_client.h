pthread_t real_cli_rcvthread;
pthread_t real_cli_sndthread;

static void *real_cli_rcvhandle(void * arg);
static void *real_cli_sndhandle(void * arg);

char* server_ip="220.149.244.211";

pthread_mutex_t cli_data_lock;

typedef struct msg_data{
    char snd_ip[15];
    char recv_ip[15];
    int snd_port;
    int recv_port;
    char msg[362];
    // this structure size is 400
}MSG_T;

typedef struct buf{
    MSG_T recv_buf;
    int cli_sockfd;
}BUF;



static void * real_client_handle(void * arg){
	int srv_sock, cli_sock;
	int port_num, ret;
	struct sockaddr_in addr;
	int len;

	int data_router_num=0;
	port_num = 4712;

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
	//pthread_create(&data_client, NULL, data_cli_handle, NULL);

	//pthread_mutex_init(&data_lock, NULL);
	//pthread_cond_init(&data_cond, NULL);
	ret = listen(srv_sock, 0);
	if (ret == -1) {
		perror("LISTEN stanby mode fail");
		close(srv_sock);
		return 0;
	}
	int cli_acc = accept(srv_sock, (struct sockaddr *)NULL, NULL);
	ret = -1;
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	/* get peer addr */
	struct sockaddr peer_addr;

	socklen_t peer_addr_len;
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr_len = sizeof(peer_addr);
	ret = getpeername(cli_acc, &peer_addr, &peer_addr_len);
	ret = getnameinfo(&peer_addr, peer_addr_len,
			hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
			NI_NUMERICHOST | NI_NUMERICSERV);

	if(ret != 0){
		ret = -1;
		pthread_exit(&ret);
	}


	if (cli_acc == -1) {
		perror("cli_sock connect ACCEPT fail");
		close(srv_sock);
	}
	pthread_create(&real_cli_rcvthread,NULL,real_cli_rcvhandle,&cli_acc);
	pthread_create(&real_cli_sndthread,NULL,real_cli_sndhandle,&cli_acc);
	while(1){
	}
}


static void * real_cli_rcvhandle(void *arg){
	int cli_sockfd = *(int *)arg;
	//printf("rcv %d \n",cli_sockfd);
	int done=0;
	
	while(1){
		pthread_mutex_lock(&cli_data_lock);
		MSG_T get_msg;
		memset(&(get_msg),0,sizeof(get_msg));
		int len;
		int rcv_sock;
		/*
		for(int x=0;x<ROU_NUM;x++){
			if(neighbor_sock[x]==cli_sockfd){
				rcv_sock=x;
				break;
			}
		}
		*/
		len = recv(cli_sockfd, &get_msg, sizeof(MSG_T), 0);
		if(len<0)
			continue;

		printf("%s",get_msg.msg);
		fflush(NULL);
		pthread_mutex_unlock(&cli_data_lock);
	//	pthread_mutex_lock(&data_lock);
	//	pthread_mutex_unlock(&data_lock);
	}
	while(1);
}


static void * real_cli_sndhandle(void *arg){
	int cli_sockfd = *(int *)arg;

	size_t getline_len;
	int ret;
	int done=0;
	while(1){
		pthread_mutex_lock(&cli_data_lock);

			MSG_T snd_msg;
			memset(&snd_msg,0,sizeof(MSG_T));
			//char* read_buffer = (char *)malloc(362);
			//ret = read(1, read_buffer, 362);
			ret = read(1, snd_msg.msg, 362);
			fflush(NULL);
			
			if (ret == -1) {
				perror("getline");
				break;
			}
			int len = strlen(snd_msg.msg);
			//int len = strlen(read_buffer);
			if (len == 0) {
				pthread_mutex_unlock(&cli_data_lock);
				//free(read_buffer);
				continue;
			}
			strcpy(snd_msg.snd_ip,"220.149.244.212");
			strcpy(snd_msg.recv_ip,"220.149.244.211");
			//snd_msg.snd_ip="220.149.244.212";
			//snd_msg.recv_ip="220.149.244.211";
			snd_msg.snd_port=4712;
			snd_msg.recv_port=4712;

			send(cli_sockfd,(char*)&snd_msg, sizeof(MSG_T), 0);
		fflush(NULL);
		pthread_mutex_unlock(&cli_data_lock);

	}
	while(1);
}
















