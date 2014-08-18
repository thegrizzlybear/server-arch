/*
*server.c - basic server responding to only one client at a time
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
#define BACKLOG -1 // did not make any difference for the scenario I was looking for

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

	int count_client_connections = 0; 

	// requirement is to take server port from commandline
	if (argc != 2 )
	{
		printf("<usage>: ./server 5000\n") ;
		exit(0 );
	}

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
	
	/*if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR, &yes,sizeof (int)) == -1)
	{
		printf("setsockopt\n");
		exit(1);
	}*/
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
	


	
		printf("Waiting for the connection:\n");
		sin_size = sizeof their_addr ;
	
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		errno = 48 ; // did not work either to throw Address Already in Use exception
		if (new_fd == -1)
		{
			printf("accept error:%d\n",errno);
			close(sockfd);
			exit(0);
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s and socket number %d\n", s,new_fd);

		wait_for_message = 1 ;
		while(wait_for_message)
		{
			bytes_rcv = recv(new_fd,buf,100,0);
			if (bytes_rcv == -1)
			{
				printf("No bytes received\n");
				continue ;
			}
			
			clean_message(buf) ;
			
			printf("Client socket %d sent %s message\n",new_fd,buf);

			if ( (strcmp(buf,"Hello") == 0)  || (strcmp(buf,"Hi") == 0 ))
			{
				bytes_snd = send(new_fd,"OK Uddhav",100,0);
				printf("Replied to client %d\n",new_fd) ;
				if (bytes_snd == -1)
				{
					printf("No bytes send\n");
				}
			}

			else if ( strcmp(buf,"Bye") == 0)
               		{
               	        	bytes_snd = send(new_fd,"Goodbye Uddhav",100,0);
				printf("Replied to client %d\n",new_fd) ;
				printf("Client said Bye; finishing \n") ;


	                        if (bytes_snd == -1)
        	                {
                	                printf("No bytes send\n");
                        	}
				wait_for_message = 0 ;
				close(new_fd) ;
        	        }
			else
			{
				bytes_snd = send(new_fd,"OK Uddhav",100,0);
				printf("Replied to client %d\n",new_fd) ;
				if (bytes_snd == -1)
                                {
                                        printf("No bytes send\n");
                                }

			}

		}

		
	
	close(sockfd) ;
		
	return 0 ;
}

