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
#include "making_rt.h"
#include "data_handle_client.h"


pthread_t client_thread;
void main(){

        pthread_create(&client_thread,NULL,real_client_handle,NULL);
        while(1);
        return ;
}

