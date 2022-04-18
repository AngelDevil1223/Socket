#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


// display the error message and terminate the program
void error(const char *msg)
{
	perror(msg);
	exit(1);
}


int main(int argc, char *argv[])
{
	
	// check for, Is all the arguments are available?
	if(argc < 3)
	{
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(1);
	}


	int sockfd, newsockfd, portno, n;

	const int BUF_SIZE = 4990;

	char buffer[BUF_SIZE];

	struct sockaddr_in serv_addr, cli_addr;
	struct hostent *server;
	
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd < 0) error("Error Opening Socket.");
 
	server = gethostbyname(argv[1]);

	if(server == NULL) fprintf(stderr, "Error, no such host");
 

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr) ) < 0 )
		error("Connection Failed");


	// Create two processes: clientEncoder, clientDecoder
	pid_t clientEncoder, clientDecoder;

	(clientEncoder = fork()) && (clientDecoder = fork()); // Creates two child processes

	if (clientEncoder == 0) {
		/* Child clientEncoder code goes here */
		 
		FILE *fptr;

		// use appropriate location of file
		fptr = fopen("intext.txt","r");

		if(fptr == NULL) error("Input file not opening");
		int i = 0;
		char ch;
		
		// clear the buffer
		bzero(buffer, BUF_SIZE); 

		// read data from file and write it in buffer
		ch = fgetc(fptr); 
		// Checking if character is not EOF.
		// If it is EOF stop eading.
		while (ch != EOF) {
			buffer[i++] = ch; 
			ch = fgetc(fptr); 
		} 

		// Closing the file
		fclose(fptr);
 
		// send the data to the server using socket
		n = write(sockfd, buffer, strlen(buffer));

		if(n < 0) error("ERROR writingg to socket");
 
	    sleep(2);
  
		// send the size of buffer data to the server using socket
		n = write(sockfd, &i, sizeof(i));

		if(n < 0) error("ERROR writingg to socket for buffer elements size");

		printf("Client: The data is sent to the server.\n");

	} else if (clientDecoder == 0) {
		/* Child clientDecoder code goes here */
 
		// clear the buffer
		bzero(buffer, BUF_SIZE);	

		// read the resulting data from the server 
		n = read(sockfd, buffer, BUF_SIZE);

		if(n < 0) error("ERROR reading from socket");
		
		FILE *fptr;
		// use appropriate location of file
		fptr = fopen("result.txt","w");

		if(fptr == NULL) error("Output file not opening");

		// write the resulting data to the result.txt file
		fprintf(fptr,"%s",buffer);
		// close the file
		fclose(fptr);

		printf("Client: The resulting data is received from server and saved in the result.txt file.\n");
		 
	} 

	 




	// close the socket connection
	close(sockfd);

	return 0;

}


