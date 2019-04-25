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

