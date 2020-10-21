#include <../include/geturl.h>

int socket_connect(char *host, in_port_t port){
	struct hostent *hp;
	struct sockaddr_in addr;
	int on = 1, sock;     

	if((hp = gethostbyname(host)) == NULL){
		herror("gethostbyname");
		exit(1);
	}
	bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));
    
	if(sock == -1){
		perror("setsockopt");
		exit(1);
	}
	
	if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
		perror("connect");
		exit(1);

	}
	return sock;
}
 

int main(int argc, char *argv[]){
	int fd;
	char buffer[BUFFER_SIZE];
    
    char *url;
    int profile=1;
    char port[5]="80";
    int waittime=150;
    
    char *usage="This tool uses socket to make requests to a given url.\n \
-u --url       destination url \n \
-p --profile   number of requests (default: 1)\n \
-n --port      port number (default: 80)\n \
-t --port      max wait time(ms) (default: 150)\n \
-h --help      help information\n";
    if(argc < 2){
        fprintf(stderr,"%s",usage);
        exit(1);
    }
    printf("\n-------------------- Options --------------------\n");
    int o;
    const char    * short_opt = "hu:p:n:t:";
    struct option   long_opt[] =
    {
       {"help",          no_argument,       0, 'h'},
       {"url",           required_argument, 0, 'u'},
       {"port",          required_argument, 0, 'n'},
       {"profile",       required_argument, 0, 'p'},
       {"maxtime",       required_argument, 0, 't'},
       {0,               0,                 0, 0  }
    };
    while ((o = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
        switch (o) {
            case 'h':
                printf("%s",usage);
                exit(1);
            case 'u':
                fprintf(stderr,"url: %s\n",optarg);
                url=optarg;
                break;
            case 'n':
                fprintf(stderr,"port number: %s\n",optarg);
                strcpy(port,optarg);
                break;
            case 't':
                fprintf(stderr,"max wait time: %sms\n",optarg);
                waittime=atoi(optarg);
                break;
            case 'p':
                profile=atoi(optarg);
                fprintf(stderr,"profile: %d\n",profile);
                if( (profile != (int)profile)||(profile<1)){
                    fprintf(stderr,"please input a positive integer using --profile\n");
                    exit(1);
                }
                break;
        }
    }
    if(!url){
        fprintf(stderr,"please input a complete url using --url\n");
        exit(1);
    }
    char *parsed;
    char *host;
    char path[200]="/";
    char *str1 = url;
    char *saveptr1;
    for (int j = 1; j<3; j++) {
        parsed = strtok_r(str1, "/", &saveptr1);
        //printf("%d parsed:%s,remain:%s\n",j,parsed,saveptr1);
        if (parsed == NULL)
            fprintf(stderr,"please input a complete url using --url\n");
        str1 = NULL;
    }

    host=parsed;
    if(saveptr1!=NULL)
        strcat(path,saveptr1);
    printf("host:%s\npath:%s\n",host,path);
	fd = socket_connect(host, atoi(port));
        
    char message[200];
    sprintf(message, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", path, host);
    printf("\n-------------------- Request --------------------\n%s", message);
    
    //set max wait time for each read
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = waittime*1000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    
    int failed=0;
    int errors[profile];
    clock_t start, end;
    double time_used[profile];
    double sizes[profile];
    for (int j = 0; j<profile; j++) {
        start = clock();
        double size=0.0;
        write(fd, message, strlen(message));
        bzero(buffer, BUFFER_SIZE);
        
        //check connection status
        int error = 0;
        socklen_t len = sizeof (error);
        int retval = getsockopt (fd, SOL_SOCKET, SO_ERROR, &error, &len);
        if (retval != 0) {
            //there was a problem getting the error code
            fprintf(stderr, "error getting socket error code: %s\n", strerror(retval));
            failed+=1;
        }

        if (error != 0) {
            //socket has a non zero error status
            fprintf(stderr, "socket error: %s\n", strerror(error));
            failed+=1;
        }
        errors[j]=error;

        //read message from server
        printf("---------- Time:%d  Message form server ----------\n\n", j+1);
        int c=BUFFER_SIZE-1;
        //while(c == (BUFFER_SIZE-1)){
        while(c>0){
            c=read(fd, buffer, BUFFER_SIZE - 1);
            if(c>0)
                size+=(double)c;
            printf("%s", buffer);
            bzero(buffer, BUFFER_SIZE);
        }
        printf("\n\n----------------- End of Message ----------------\n");
        end = clock();
        time_used[j]=((double) (end - start)) / CLOCKS_PER_SEC;
        sizes[j]=size;
    }
    shutdown(fd, SHUT_RDWR);
    close(fd);
    printf("-------------------- Analysis -------------------\n");
    printf("number of requests: %d\n",profile);
    printf("fastest time: %.3fms\n",findmax(time_used,profile)*1000);
    printf("slowest time: %.3fms\n",findmin(time_used,profile)*1000);
    printf("mean time: %.3fms\n",getmean(time_used,profile)*1000);
    printf("median time: %.3fms\n",getmedian(time_used,profile)*1000);
    printf("percentage of requests that succeeded: %d%%\n",(int)(profile-failed)/profile*100);
    printf("error codes returned that weren't a success: ");
    for (int j = 0; j<profile; j++) {
        if (errors[j]>0)
            printf("%d ",errors[j]);
    }
    printf("\nsize in bytes of the smallest response: %.0f\n",findmin(sizes,profile));
    printf("size in bytes of the largest response: %.0f\n",findmax(sizes,profile));
    return 0;
}

double findmax(double arr[],int n)
{
    double max = arr[0];
  
    for (int i = 1; i < n; i++)
        if (arr[i] > max)
            max = arr[i];
  
    return max;
}

double findmin(double arr[],int n)
{
    double min = arr[0];

    for (int i = 1; i < n; i++)
        if (arr[i] < min)
            min = arr[i];

    return min;
}

double getmean(double arr[],int n) {
    double sum=0, i;
    for(int i=0; i<n; i++)
        sum+=arr[i];
    return((double)sum/n);
}

double getmedian(double arr[],int n) {
    double temp;
    int i, j;
    // sort the array x in ascending order
    for(i=0; i<n-1; i++) {
        for(j=i+1; j<n; j++) {
            if(arr[j] < arr[i]) {
                // swap elements
                temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }

    if(n%2==0) {
        // if even number of elements, return mean of  two middle elements
        return((arr[n/2] + arr[n/2 - 1]) / 2.0);
    } else {
        // return the element in the middle
        return arr[n/2];
    }
}
