#define ROU_NUM 4
#define my_num 0

int my_neighbor[ROU_NUM] = {-1, };
int CT[ROU_NUM][ROU_NUM] = {-1, };

void makeCT(){
	char* path = "input.txt";
	FILE* fp = NULL;
	int val = 0;
	char* ptr=NULL;
	char* tok[5];

	fp = fopen(path, "rb");
	if(fp==NULL){
		printf("invalid input file: %s\n",path);
		return ;
	}

	while(1){
		char val[100];
		char* res = fgets(val, sizeof(val), fp);
		if(res == NULL)
			break;

		tok[0] = strtok_r(val, " ", &ptr);
		printf("tok[0] %s \n",tok[0]);
		for(int i=1;i<5;i++){
			tok[i] = strtok_r(NULL, " ", &ptr);
			printf("tok[%d] %s \n",i,tok[i]);
		}
		
		if(*(tok[0]+14)=='1'){
			CT[my_num][0]=atoi(tok[4]);
			CT[0][my_num]=atoi(tok[4]);
			printf("C[%d][0] %d \n",my_num,CT[my_num][0]);
			my_neighbor[0]=1;
		}
		else if(*(tok[0]+14)=='2'){
			CT[my_num][1]=atoi(tok[4]);
			CT[1][my_num]=atoi(tok[4]);
			printf("C[%d][1] %d \n",my_num,CT[my_num][1]);
			my_neighbor[1]=1;
		}
		else if(*(tok[0]+14)=='3'){
			CT[my_num][2]=atoi(tok[4]);
			CT[2][my_num]=atoi(tok[4]);
			printf("C[%d][1] %d \n",my_num,CT[my_num][2]);
			my_neighbor[2]=1;
		}
		else if(*(tok[0]+14)=='4'){
			CT[my_num][3]=atoi(tok[4]);
			CT[3][my_num]=atoi(tok[4]);
			printf("C[%d][1] %d \n",my_num,CT[my_num][3]);
			my_neighbor[3]=1;
		}
		else if(*(tok[0]+14)=='5'){
			CT[my_num][4]=atoi(tok[4]);
			CT[4][my_num]=atoi(tok[4]);
			printf("C[%d][1] %d \n",my_num,CT[my_num][4]);
			my_neighbor[4]=1;
		}
	}
	for(int a=0;a<ROU_NUM;a++){
		for(int b=0;b<ROU_NUM;b++){
			if(a==b){
				//my_neighbor[a]=0;
				CT[a][a]=0;
			}
			printf("%d ",CT[a][b]);
		}
		printf("\n");
	}
}

