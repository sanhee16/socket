pthread_t real_srv_rcvthread;
pthread_t real_srv_sndthread;

static void *real_srv_rcvhandle(void * arg);
static void *real_srv_sndhandle(void * arg);

int cli_list[ROU_NUM];

pthread_mutex_t srv_lock;
pthread_cond_t srv_cond;


static void * real_server_handle(void * arg){
	int srv_sock, cli_sock;
	int port_num, ret;
	struct sockaddr_in addr;
	int len;

	for(int a=0;a<ROU_NUM;a++){
		cli_list[a]=0;
	}
	cli_list[1]=1; //connected client 212
	cli_list[2]=2; //connected client 213
	
	data_router_num=0;
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

	if (ret == -1){
		perror("BIND error!!");
		close(srv_sock);
		return 0;
	}
	//pthread_create(&data_client, NULL, data_cli_handle, NULL);

	pthread_mutex_init(&srv_lock, NULL);
	pthread_cond_init(&srv_cond, NULL);
	ret = listen(srv_sock, 0);
	if (ret == -1) {
		perror("LISTEN stanby mode fail");
		close(srv_sock);
		return 0;
	}
	int cli_acc = accept(srv_sock, (struct sockaddr *)NULL, NULL);
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

	if(cli_acc == -1) {
		perror("cli_sock connect ACCEPT fail");
		close(srv_sock);
	}
	pthread_create(&real_srv_rcvthread,NULL,real_srv_rcvhandle,&cli_acc);
	pthread_create(&real_srv_sndthread,NULL,real_srv_sndhandle,&cli_acc);
	while(1){
	}
}


static void * real_srv_rcvhandle(void *arg){
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
		pthread_mutex_lock(&srv_lock);

		memcpy(&(data_buffer.recv_buf),&get_msg,sizeof(MSG_T));
		data_buffer.cli_sockfd = rcv_sock;

		srv_data_exist_buf=1;
		fflush(NULL);

		pthread_mutex_unlock(&srv_lock);
	}
	while(1);
}


static void * real_srv_sndhandle(void *arg){
	int cli_sockfd = *(int *)arg;

	size_t getline_len;
	int ret;
	int done=0;
	while(1){
		pthread_mutex_lock(&srv_lock);

		if(srv_data_exist_buf==1){
			MSG_T snd_msg;
			memcpy(&snd_msg,&(data_buffer.recv_buf),sizeof(MSG_T));
			if(data_buffer.recv_buf.check_fin==1){
				//print_CT();
				pthread_mutex_unlock(&data_lock);
				continue;
			}

			//int snd_sockfd = data_buffer.cli_sockfd;
			//snd all client
			snd_msg.snd_ip="220.149.244.211";
			snd_msg.snd_port=4712;
			snd_msg.recv_port=4712;
			int set[ROU_NUM];
			if(int a=0;a<ROU_NUM;a++){
				switch(a){
					case 0:
						set[a]="220.149.244.211";
						break;
					case 1:
						set[a]="220.149.244.212";
						break;
					case 2:
						set[a]="220.149.244.213";
						break;
					case 3:
						set[a]="220.149.244.214";
						break;
					case 4:
						set[a]="220.149.244.215";
						break;
					default:
						break;
				}
			}
			for(int a=0;a<ROU_NUM;a++){
				if(cli_list[a]==1){
					snd_msg.recv_ip = set[a];
					send(cli_sockfd,(char*)&snd_msg, sizeof(MSG_T), 0);
				}
			}
			data_exist_buf=0;
			memset(&data_buffer,0,sizeof(BUF));
			fflush(NULL);
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
			memset(&data_buffer,0,sizeof(BUF));
			fflush(NULL);
		}
		fflush(NULL);
		pthread_mutex_unlock(&srv_lock);
	}
	while(1);
}
















