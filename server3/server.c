/*
*server.c - multithreaded server which can handle multiple requests
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>

#define PORT "5000"
#define BACKLOG 10

#define MAXDATASIZE 100
#define MAXTHREADS 20000


/*void sigchld_handler(int s)
{
	while(waitpid(-1,NULL,WNOHANG) > 0) ;
}*/

void *get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET)
	{
		return &((struct sockaddr_in*)sa)->sin_addr;
	}

	return &((struct sockaddr_in6 *)sa)->sin6_addr ;
}

void clean_message(char * msg)
{

	// need to add more to the list of \n
 
	int i ;
//	printf("\n%d is len of msg\n",(int) strlen(msg));
	for(i = 0 ; msg[i] != '\0'; i++)
	{
		if (msg[i] == '\n')
			msg[i] = '\0' ;
	}
	
	
}


void *  handle_client_connection(void * fd)
{	
	int new_fd = *(int*) fd ;
	int bytes_rcv, bytes_snd; 
	int wait_for_message = 1;
		
	char buf[MAXDATASIZE] ;

	
	while(wait_for_message)
	{
			//char *buf = (char *) malloc(30) ;
			memset(buf,'\0',sizeof buf);
		        bytes_rcv = recv(new_fd,buf,100,0);
			//buf[strlen(buf) - 1] = '\0' ;
			//buf[]
			printf("Client %d has sent message:%s",new_fd,buf) ;
                        if (bytes_rcv == -1)
                        {
                                printf("No bytes received\n");
                                continue ;
                        }

                        clean_message(buf) ;
			//printf("%s\n", buf);

                        if ( (strcmp(buf,"Hello") == 0)  || (strcmp(buf,"Hi") == 0 ))
                        {
                                bytes_snd = send(new_fd,"OK Uddhav",100,0);
				printf("Replied to the client\n");
                                if (bytes_snd == -1)
                                {
                                        printf("No bytes send\n");
                                        //continue;
                                }
                        }

                        else if ( strcmp(buf,"Bye") == 0)
                        {
                                bytes_snd = send(new_fd,"Goodbye Uddhav",100,0);
				printf("Replied to the client\n");

                                if (bytes_snd == -1)
                                {
                                        printf("No bytes send\n");
                                        //continue;
                                }
                                wait_for_message = 0 ;
                        }
                        else
                        {
                                bytes_snd = send(new_fd,"OK Uddhav",100,0);
				printf("Replied to the client\n");
                                if (bytes_snd == -1)
                                {
                                        printf("No bytes send\n");
                                }
			}
	}

	close(new_fd);
	printf("Closing the connection\n");
	//exit(1) ;
}

int main(int argc, char ** argv)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p ;
	struct sockaddr_storage their_addr;
	socklen_t sin_size ;
	struct sigaction sa;
	int yes =1;
	


	if(argc != 2)
	{
		printf("<usage>: ./server 5000\n");
		exit(0) ;
	}
	//char buf[MAXDATASIZE] ;
	//int bytes_rcv = 0 ;
	//int bytes_snd = 0 ;

	char s[INET6_ADDRSTRLEN];
	int rv;


	pthread_t serverThreads [MAXTHREADS] ;
	int iret[MAXTHREADS]  = {-1} ;
	//int wait_for_message = -1 ;


	memset(&hints,0 ,sizeof hints);
	//setting up the information about the type of the address to query
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;


	if((rv = getaddrinfo(NULL,argv[1],&hints,&servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}


	sockfd = socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol);
	if (sockfd == -1)
	{
		printf("could not create socket\n");	
		return 1;
	}
	
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR, &yes,sizeof (int)) == -1)
	{
		printf("setsockopt\n");
		exit(1);
	}
	if (bind(sockfd,servinfo->ai_addr, servinfo->ai_addrlen) == -1)
	{
		close(sockfd);
		printf("socket:bind\n");
		exit(1);
	}
	
	freeaddrinfo(servinfo); //done with the job of this structure
	
	if(listen(sockfd,BACKLOG) == -1)
	{
		printf("listen\n");	
		exit(1);
	}
	
/*	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}*/

	int i = 0 ; // counter for number of threads

	while(1)
	{
		printf("Waiting for the connection:\n");
		sin_size = sizeof their_addr ;
		int  new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1)
		{
			printf("accept error\n");
			continue ;
		}
	
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s and socket %d\n", s,new_fd);
		

		iret[i] = pthread_create(&serverThreads[i],NULL,handle_client_connection,(void *) &new_fd);
		if(iret[i])
		{
			printf("Error while creating the threads\n");
			continue ;
		}
		else
		{
			//printf("%d is the return value\n",iret[i]);
			if (i == 19999)
			{	
				printf("Thread limit over\n");
			 	break ;
			}
			
			i ++ ;

		}	
		
		//printf("I am handling %s connection\n",s) ;
		//handle_client_connection(new_fd,buf);
		
	}
		
	return 0 ;
}

