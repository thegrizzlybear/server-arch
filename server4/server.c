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

#define BACKLOG 10

#define MAXDATASIZE 100
#define MAXCLIENTS 20000

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
//	printf("\n%d is len of the message\n",(int)strlen(msg)) ;
	for(i = 0 ; i < strlen(msg); i++)
	{
		if (msg[i] == '\n')
			msg[i] = '\0' ;
	}
	
}



void handle_client_connection(int new_fd,int * conn, int client_number)
{	
	int bytes_rcv, bytes_snd; 
	//int wait_for_message = 1;
	char buf[MAXDATASIZE] ;
	
	//while(wait_for_message)
	{
			memset(buf, '\0',sizeof buf) ;
		        bytes_rcv = recv(new_fd,buf,100,0);
		        if (bytes_rcv == -1)
                        {
                                printf("No bytes received\n");
                                //continue ;
                        }
			
			printf("Client socket %d sent message %s",new_fd, buf) ;

                        clean_message(buf) ;
			

                        if ( (strcmp(buf,"Hello") == 0)  || (strcmp(buf,"Hi") == 0 ))
                        {
                                bytes_snd = send(new_fd,"OK Uddhav",100,0);
				printf("Replied to client %d\n",new_fd);
                                if (bytes_snd == -1)
                                {
                                        printf("No bytes send\n");
                                }
                        }

                        else if ( strcmp(buf,"Bye") == 0)
                        {
                                bytes_snd = send(new_fd,"Goodbye Uddhav",100,0);
				printf("Replied to client %d\n",new_fd);

                                if (bytes_snd == -1)
                                {
                                        printf("No bytes send\n");
                                }
                                //wait_for_message = 0 ;
				close(new_fd);
				conn[client_number] = 0 ;
				printf("Client %d said Bye; finishing\n",new_fd);
                        }
                        else
                        {
                                bytes_snd = send(new_fd,"OK Uddhav",100,0);
				printf("Replied to client %d\n",new_fd);
                                if (bytes_snd == -1)
                                {
                                        printf("No bytes send\n");
                                }
			}
			
	}

	//close(new_fd);
	//printf("Client said Bye; finishing\n");
	//exit(0) ;
}



int main(int argc, char ** argv)
{
	int sockfd, new_fd;
	struct addrinfo hints, *servinfo, *p ;
	struct sockaddr_storage their_addr;
	socklen_t sin_size ;
	struct sigaction sa;
	int yes =1;
	
	//char buf[MAXDATASIZE] ;
	int bytes_rcv = 0 ;
	int bytes_snd = 0 ;

	char s[INET6_ADDRSTRLEN];
	int rv;

	int wait_for_message = -1 ;

		
	//initializing variables for use in select function
	fd_set fdset;
	int client_connections[MAXCLIENTS] = {0};
	int max_fd ;

	if(argc != 2)
	{
		printf("<usage>: ./server 5000\n");
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
	

//	initializeFD(fdset,sockfd,client_connections);
	sin_size = sizeof their_addr ;
//	max_fd = sockfd ;
	
	int i = 0 ; //counter variable
	int sd ; // temporary holdder	

	int clients_to_read = 0 ;

	printf("Starting select loop, fdmax %d\n",sockfd) ;
	while(1)
	{

		/* adding logic for using select function*/				
		FD_ZERO(&fdset) ;
	        FD_SET(sockfd,&fdset) ;
		max_fd = sockfd ;
        	for(i = 0; i <MAXCLIENTS ;i ++)
        	{
               		sd = client_connections[i];


			if(sd >0)
			{
		               	FD_SET(sd,&fdset);
			}
		
		
			if (max_fd < sd)
			{
				max_fd = sd;
			}
			
        	}

		clients_to_read = select (max_fd +1 ,&fdset,NULL,NULL,NULL) ; // wait till request arrives

		//if (clients_to_read > 0)
				
			if (FD_ISSET(sockfd, &fdset) != 0)	
			{
				new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
				if (new_fd == -1)
				{
					printf("accept error\n");
					continue ;
				}

				inet_ntop(their_addr.ss_family,
					get_in_addr((struct sockaddr *)&their_addr),
					s, sizeof s);
				printf("server: got connection from %s and socket %d\n", s,new_fd);
				

				// adding new connection to clients array
				for (i = 0 ; i<MAXCLIENTS; i++)	
				{
					if (client_connections[i] == 0)
					{
						client_connections[i] = new_fd ;
						break ;
					}
						
				}
			
				
			}		
			
			
			for(i = 0 ; i< MAXCLIENTS ; i++)
			{
				if (FD_ISSET(client_connections[i], &fdset) !=0)
				{
					printf("In select loop: socket %d is ready to read\n", client_connections[i]) ;
					handle_client_connection(client_connections[i],client_connections,i) ;
				}
			}
			
		
		
	}
		
	return 0 ;
}

