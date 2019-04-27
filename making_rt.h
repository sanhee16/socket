int edge[ROU_NUM];
int fin_table[ROU_NUM];

typedef struct routing{
	int dest[ROU_NUM];
	int next[ROU_NUM];
	int cost[ROU_NUM];
}route_table;

route_table rt;

void print_CT();

static void * RT_handler(void *arg){
	//makeCT();
	print_CT();
	int done = *(int *)arg;
	while(1){
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

		for(int count=0; count<ROU_NUM; count++){
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
				if(set_c[b]==0)
					continue;
				v=b;
			//	printf("u is %d , v is %d \n",u,v);
				if(d[v] > CT[u][v]+d[u]){
					d[v]=CT[u][v]+d[u];
					edge[v]=u;
					//rt.next[v]=edge[v];
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
	//	printf("\n\n------result------\n\n");
		for(int a=0;a<ROU_NUM;a++){
			//printf("%d -> %d : %d",source, a, d[a]);
			//printf(" next %d \n",edge[a]);
		}
	//	printf("\n\n------routing table------\n\n");
	//	printf(" dest next cost \n");
		for(int a=0;a<ROU_NUM;a++){
			if(a==my_num)
				continue;
			//printf(" %3d  %3d  %3d",rt.dest[a],rt.next[a],rt.cost[a]);
			//printf("\n");
		}

		if(fin_table[my_num]==1){
			//printf("fin table?? ");
			for(int a=0;a<ROU_NUM;a++){
			//	printf("%d -> %d : %d",source, a, d[a]);
			//	printf(" next %d \n",edge[a]);
			}

		}

	}
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

