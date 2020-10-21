#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

#define BUFFER_SIZE 1024

int socket_connect(char *host, in_port_t port);
 
int main(int argc, char *argv[]);

double findmax(double arr[],int n);
double findmin(double arr[],int n);
double getmean(double arr[],int n) ;
double getmedian(double arr[],int n);
