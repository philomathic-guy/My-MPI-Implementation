/*
Single Author info:
rtnaik Rohit Naik
Group info:
nphabia Niklesh Phabiani
rtnaik	Rohit Naik
anjain2 Akshay Narendra Jain
*/

// client will connect, server will accept.
// client does not need to have the same port empty as the server - it uses a
// random available port when you connect (to the known port of the server).

// server headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
// only one extra in client.
#include <netdb.h>
#include <time.h>

// Takes the number of bytes of the datatype that is sent.
#define MPI_Datatype int
#define MPI_Comm int
#define MPI_COMM_WORLD 0
//The datatype of MPI
#define MPI_Datatype int
#define MPI_Status int
#define MPI_Status int
#define F_OFFSET 20

//Using size of to mock double and char sizes
#define MPI_DOUBLE (sizeof(double))
#define MPI_CHAR (sizeof(char))

char hostname[50];
int numproc;
int rank;

int sockfd;
struct sockaddr_in serv_addr;//, cli_addr;

// Returns 0 on successful execution and -1 if any error. It also prints the error in that case.
int MPI_Init(int *argc, char ***argv) {
    unsigned int myPort;
    char filename[50];
    struct hostent *myIP;

    strncpy(hostname, (*argv)[1], 50);
    rank = atoi((*argv)[2]);
    numproc = atoi((*argv)[3]);

    // Set up sockets and bind to port for this node.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
       perror("ERROR opening socket");
       return -1;
    }

    // Initialize serv_addr.
    struct sockaddr_in serv_addr, my_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    //Using library calls to get host name
    myIP = gethostbyname(hostname);
    if (myIP == NULL) {
      perror("ERROR, no such host\n");
      return -1;
    }
    bcopy((char *)myIP->h_addr,
        (char *)&serv_addr.sin_addr.s_addr,
        myIP->h_length);
    serv_addr.sin_port = htons(0);

    //Binding to an available port
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      return -1;
    }

    bzero(&my_addr, sizeof(my_addr));
  	int len = sizeof(my_addr);
    getsockname(sockfd, (struct sockaddr *) &my_addr, &len);
    inet_ntop(AF_INET, &my_addr.sin_addr, myIP, sizeof(myIP));
    myPort = ntohs(my_addr.sin_port);

    // Add host name and portno to a file
    FILE *fp;
    sprintf(filename, "%d.txt", rank);
    fp = fopen(filename, "w");
    if (fp == NULL) {
      perror("ERROR opening file");
      return -1;
    }
    fprintf(fp, "%d %s", myPort, hostname);
    fclose(fp);

    listen(sockfd,10);
    return 0;
}

// Returns 0 on successful execution and -1 if any error. It also prints the error in that case.
int MPI_Send(const void *buffer, int count, int datatype, int dest, int tag, int comm) {
  int n;
  char filename[50];
  char dest_hostname[50];
  int dest_portno;
  int send_socket;
  struct hostent *server;

  send_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (send_socket < 0) {
     perror("ERROR opening socket");
     return -1;
  }

  // Get hostname and portno from the file.
  FILE *fp;
  sprintf(filename, "%d.txt", dest);
  fp = fopen(filename, "r");
  int x = 1;
  while (fp == NULL) {
    sleep(3);
    if(x){
      //printf("Waiting\n");
      //fflush(stdout);
      x=0;
    }
    fp = fopen(filename, "r");
  }
  rewind(fp);
  fscanf(fp, "%d %s", &dest_portno, dest_hostname);
  fclose(fp);

  server = gethostbyname(dest_hostname);
  if (server == NULL) {
    perror("ERROR, no such host\n");
    return -1;
  }

  struct sockaddr_in serv_addr;
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
      (char *)&serv_addr.sin_addr.s_addr,
      server->h_length);
  serv_addr.sin_port = htons(dest_portno);

  //Connecting to node needed to send to
  while (connect(send_socket,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
	//Will wait to connect!
  }

  //Sending data
  n = send(send_socket,buffer,count*datatype, 0);
  if (n < 0) {
      perror("ERROR writing to socket");
      return -1;
  }
  return 0;
}

//Custom Gathering averages and standard deviation for all pairs.
int MPI_Gather(double *buffer1, double *buffer2, int count, int datatype, int source, int tag,
              int comm, int *status) {
  // Tag 0 is for averages.
  if(tag == 0) {
		MPI_Recv(buffer1+(source/2)*count, count, datatype, source, tag, comm, status);
	}
  // Tag 1 is for std deviation.
  else {
		MPI_Recv(buffer2+(source/2)*count, count, datatype, source, tag, comm, status);
	}
}

// Returns 0 on successful execution and -1 if any error. It also prints the error in that case.
int MPI_Recv(void *buffer, int count, int datatype, int source, int tag,
             int comm, int *status) {
  int newsockfd;
  socklen_t clilen;
  struct sockaddr_in cli_addr;
  int n;

  clilen = sizeof(cli_addr);
  newsockfd = accept(sockfd,
             (struct sockaddr *) &cli_addr,
             (socklen_t *)&clilen);
  // fflush(stdout);
  if (newsockfd < 0) {
      perror("ERROR on accept");
      return -1;
  }

  //Reading data
  n = read(newsockfd,buffer,count*datatype);
  close(newsockfd);
  return 0;
}

//Each node waits for its previous node to write a file, then only writes its own file.
//In this way, nodes will wait to come to the same point
int MPI_Barrier() {
  FILE *fp;
  char *f = (char *)malloc(sizeof(char)*20);
  if(rank != 0){
  	sprintf(f, "%dbarrier.txt", rank-1);
  	fp = fopen(f, "r");
	//Waiting for rank-1 to come to this point and create a file.
 	while (fp == NULL) {
		sleep(10);
    		fp = fopen(f, "r");
	}
  	fclose(fp);
  }
  //Creating its own file
  sprintf(f, "%dbarrier.txt", rank);
  fp = fopen(f, "w");
  while(fp == NULL){
  	sleep(3);
  	fp = fopen(f, "w");
  }
  fclose(fp);
}

//Removing files created and closing socket 'sockfd'.
int MPI_Finalize(void) {
  char f[50];
  sprintf(f, "%d.txt",rank);
  remove(f);
  if(rank==numproc-1){
  	for(int i=0;i<numproc;i++){
  		sprintf(f, "%dbarrier.txt", i);
  		remove(f);
  	}
  }
  close(sockfd);
}
