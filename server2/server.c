/*
*server.c - multiprocess server which can handle multiple requests
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "5000"
#define BACKLOG 10

#define MAXDATASIZE 100


void sigchld_handler(int s)
{
	while(waitpid(-1,NULL,WNOHANG) > 0) ;
}

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
 
	int i ;
	for(i = 0 ; msg[i] != '\0'; i++)
	{
		if (msg[i] == '\n')
			msg[i] = '\0' ;
	}
	
	
}


void handle_client_connection(int new_fd ,char *buf)
{	
	int bytes_rcv, bytes_snd; 
	int wait_for_message = 1;

	puts("a");
	
	while(wait_for_message)
	{
		        bytes_rcv = recv(new_fd,buf,100,0);
                        puts("b");
                        if (bytes_rcv == -1)
                        {
                                printf("No bytes received\n");
				puts("c");
                                continue ;
                        }

                        clean_message(buf) ;
			puts("d");
			printf("%s\n", buf);

                        if ( (strcmp(buf,"Hello") == 0)  || (strcmp(buf,"Hi") == 0 ))
                        {
                                bytes_snd = send(new_fd,"OK ABC",100,0);
				puts("e");
                                if (bytes_snd == -1)
                                {
                                        printf("No bytes send\n");
					puts("f");
                                        //continue;
                                }
                        }

                        else if ( strcmp(buf,"Bye") == 0)
                        {
                                 bytes_snd = send(new_fd,"Goodbye ABC",100,0);
				 puts("g");

                                if (bytes_snd == -1)
                                {
                                        printf("No bytes send\n");
					puts("h");
                                        //continue;
                                }
                                wait_for_message = 0 ;
        //                        close(new_fd) ;
                        }
                        else
                        {
                                bytes_snd = send(new_fd,"Goodbye ABC\n",100,0);
				puts("i");
                                if (bytes_snd == -1)
                                {
                                        printf("No bytes send\n");
					puts("j");
                                        //continue;
                                }
                                wait_for_message = 0 ;
      //                          close(new_fd) ;
			}
	}

	close(new_fd);
	printf("Closing the connection\n");
	exit(1) ;
}

int main(int argc, char ** argv)
{
	int sockfd, new_fd;
	struct addrinfo hints, *servinfo, *p ;
	struct sockaddr_storage their_addr;
	socklen_t sin_size ;
	struct sigaction sa;
	int yes =1;
	
	char buf[MAXDATASIZE] ;
	int bytes_rcv = 0 ;
	int bytes_snd = 0 ;

	char s[INET6_ADDRSTRLEN];
	int rv;

	int wait_for_message = -1 ;


	memset(&hints,0 ,sizeof hints);
	//setting up the information about the type of the address to query
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;


	if((rv = getaddrinfo(NULL,PORT,&hints,&servinfo)) != 0)
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
	
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	while(1)
	{
		printf("Waiting for the connection:\n");
		sin_size = sizeof their_addr ;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1)
		{
			printf("accept error\n");
			continue ;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);
		
		pid_t pID = fork();

		if (pID == 0)
		{
			printf("I am handling %s connection\n",s) ;
			handle_client_connection(new_fd,buf);
		}
	}
		
	return 0 ;
}

