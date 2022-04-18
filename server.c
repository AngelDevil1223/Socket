#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/wait.h>

// Buffer size
const int BUF_SIZE = 4990;

// Length for data the buffer has
int bufferLength; 

int p[2];

// display the error message and terminate the program
void error(const char *msg)
{
	perror(msg);
	exit(1);
}


// replace all the lowercase 'a' letters to uppdercase 'A' letter
void *replaceCharA(void *varg)
{ 
	char *buffer = (char *) varg;
	for(int i=0; i < bufferLength; i++){
		if(*(buffer + i) == 'a')
			*(buffer + i) = 'A';
	} 
   pthread_exit((void*) buffer);
}


// replace all the lowercase 'e' letters to uppdercase 'E' letter
void *replaceCharE(void *varg)
{ 
	char *buffer = (char *) varg;
	for(int i=0; i < bufferLength; i++){
		if(*(buffer + i) == 'e')
			*(buffer + i) = 'E';
	}
   pthread_exit((void*) buffer);
}


// replace all the lowercase 'i' letters to uppdercase 'I' letter
void *replaceCharI(void *varg)
{ 
	char *buffer = (char *) varg;
	for(int i=0; i < BUF_SIZE; i++){
		if(*(buffer + i) == 'i')
			*(buffer + i) = 'I';
	}
   pthread_exit((void*) buffer);
}



// replace all the lowercase 'o' letters to uppdercase 'O' letter
void *replaceCharO(void *varg)
{ 
	char *buffer = (char *) varg;
	for(int i=0; i < bufferLength; i++){
		if(*(buffer + i) == 'o')
			*(buffer + i) = 'O';
	}
   pthread_exit((void*) buffer);
}



// replace all the lowercase 'u' letters to uppdercase 'U' letter
void *replaceCharU(void *varg)
{ 
	char *buffer = (char *) varg;
	for(int i=0; i < BUF_SIZE; i++){
		if(*(buffer + i) == 'u')
			*(buffer + i) = 'U';
	}
   pthread_exit((void*) buffer);
}



// count the number of digits found in the data
// write the count at the last in the data
void *digitThread(void *varg)
{ 
	char *buffer = (char *) varg;
	int sum = 0;
	int i = 0;
	for(i=0; i < bufferLength; i++){
		if(*(buffer + i) >= '0' && *(buffer + i) <= '9')
			sum++; 
	}
	
	char buf[15];

	bzero(buf, 15);	
	sprintf(buf,"%d",sum);
 

	for(i=bufferLength; i < bufferLength + 15; i++){
		*(buffer + i) = buf[i - bufferLength]; 
	}
	
 
   pthread_exit((void*) buffer);
}

// send the data to serverEncoder process
void *writerThread(void *varg)
{ 
	char *buffer = (char *) varg;	 
				
   pthread_exit((void*) buffer);
}



int main(int argc, char *argv[])
{

	// check for, Is all the arguments are available?
	if(argc < 2)
	{
		fprintf(stderr, "Port No not provided. Program terminated\n");
		exit(1);
	}


	int sockfd, newsockfd, portno, n, status; 
	
	char buffer[BUF_SIZE];

	struct sockaddr_in serv_addr, cli_addr;

	socklen_t clilen;

	fprintf(stdout, "Run client by providing host and port\n");	

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd < 0) error("Error Opening Socket.");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	
	portno = atoi(argv[1]);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("Binding Failed.");

	// Listen to at most 5 clients requests
	listen(sockfd, 5);

	clilen = sizeof(cli_addr);

	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

	if(newsockfd < 0) error("Error on Accept");

	if (pipe(p) < 0) error("Error on pipe");
        

	// Create two processes: ServerDecoder, ServerEncoder
	pid_t ServerDecoder, ServerEncoder;

	(ServerDecoder = fork()) && (ServerEncoder = fork()); // Creates two child processes

	if (ServerDecoder == 0) {
		/* Child ServerDecoder code goes here */
		 
		// clear the buffer
		bzero(buffer, BUF_SIZE);	

		// read the data from the socket
		n = read(newsockfd, buffer, BUF_SIZE);

		if(n < 0) error("Error on Reading from Socket.");
 
	    sleep(2);

		// read length of data sent from client from socket 
		n = read(newsockfd, &bufferLength, sizeof(bufferLength));

		if(n < 0) error("Error on Reading from Socket bufferLength.");
  
		// create seven threads for the data lowercase vowels to uppercase conversion
	    pthread_t charA, charE, charI, charO, charU, digit, writer;

		// perform thread replaceCharA operation
		pthread_create(&charA, NULL, replaceCharA, (void *)&buffer);

		// wait for thread to finish
		pthread_join(charA, NULL);

		// perform thread replaceCharE operation
		pthread_create(&charE, NULL, replaceCharE, (void *)&buffer);

		// wait for thread to finish
		pthread_join(charE, NULL);
 
		// perform thread replaceCharI operation
		pthread_create(&charI, NULL, replaceCharI, (void *)&buffer);

		// wait for thread to finish
		pthread_join(charI, NULL);

		// perform thread replaceCharO operation
		pthread_create(&charO, NULL, replaceCharO, (void *)&buffer);

		// wait for thread to finish		
		pthread_join(charO, NULL);

		// perform thread digitThread operation
		pthread_create(&digit, NULL, digitThread, (void *)&buffer);

		// wait for thread to finish
		pthread_join(digit, NULL);
		
		// perform thread writerThread operation
		pthread_create(&writer, NULL, writerThread, (void *)&buffer);
		
		// wait for thread to finish
		pthread_join(writer, NULL);
 
		// share the data to the serverEncoder process using pipe
		write(p[1], buffer, BUF_SIZE);

		printf("Server: All vowel lowercase letters are converted to uppercase.\n");

	} else if (ServerEncoder == 0) {
		
		/* Child ServerEncoder code goes here */
		
		// wait for serverDecoder to finish
	    sleep(4);
 
		// clear the buffer
		bzero(buffer, BUF_SIZE);

		// read the data from the pipe 
		read(p[0], buffer, BUF_SIZE);
 
		// send the data to client, write the resulting data to the socket,
		n = write(newsockfd, buffer, BUF_SIZE);

		if(n < 0) error("ERROR writing to socket.");

		printf("Server: The resulting data is sent to the client.\n"); 
	} 


 	// close the socket connections
	close(newsockfd);
	close(sockfd);

	return 0;

}




























