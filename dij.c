#define ROU_NUM 5
#define my_num 2
#define INFINITE 9999

int my_neighbor[ROU_NUM] = {-1, };
int CT[ROU_NUM][ROU_NUM] = {-1, };

void makeCT(){
	char* path = "input.txt";
	FILE* fp = NULL;
	int val = 0;
	char* ptr=NULL;
	char* tok[ROU_NUM];
	int tok_int[ROU_NUM];

	for(int a=0;a<ROU_NUM;a++){
		for(int b=0;b<ROU_NUM;b++)
			CT[a][b]=INFINITE;
	}

	fp = fopen(path, "rb");
	if(fp==NULL){
		printf("invalid input file: %s\n",path);
		return ;
	}

	int a=0;
	while(1){
		char val[100];
		char* res = fgets(val, sizeof(val), fp);
		if(res == NULL)
			break;

		tok[0] = strtok_r(val, " ", &ptr);
		CT[a][0]=atoi(tok[0]);
		for(int i=1;i<ROU_NUM;i++){
			tok[i] = strtok_r(NULL, " ", &ptr);
			tok_int[i] = atoi(tok[i]);
			CT[a][i]=tok_int[i];
		}
		a++;
	}
}
sanhee16@assam:~/dijsktra$ cat input.txt
0 4 2 17 9999
4 0 1 9999 1
2 1 0 9999 9
17 9999 9999 0 6
9999 1 9 6 0
sanhee16@assam:~/dijsktra$ cat dij.c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "file.h"

int edge[ROU_NUM];

typedef struct routing{
	int dest[ROU_NUM];
	int next[ROU_NUM];
	int cost[ROU_NUM];
}route_table;

route_table rt;

void print_CT();

void main(){
	makeCT();
	print_CT();

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
		printf("%d ",d[a]);
	}
	int source = my_num;

	d[source]=0;
	//set_s[source]=1;
	//set_c[source]=0;
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
			printf("u is %d , v is %d \n",u+1,v+1);
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


	printf("\n\n------result------\n\n");
	for(int a=0;a<ROU_NUM;a++){
		printf("%d -> %d : %d",source+1, a+1, d[a]);

		printf(" next %d \n",edge[a]+1);
	}
	printf("\n\n------routing table------\n\n");
	printf(" dest next cost \n");
	for(int a=0;a<ROU_NUM;a++){
		if(a==my_num)
			continue;
		printf(" %3d  %3d  %3d",rt.dest[a]+1,rt.next[a]+1,rt.cost[a]);
		printf("\n");
	}


	return ;
}




void print_CT(){
	for(int a=0;a<ROU_NUM;a++){
		for(int b=0;b<ROU_NUM;b++){
			printf("%6d ",CT[a][b]);
		}

		printf("\n");
	}
}

