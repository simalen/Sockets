/* File: client.c
 * Trying out socket communication between processes using the Internet protocol family.
 * Usage: client [host name], that is, if a server is running on 'lab1-6.idt.mdh.se'
 * then type 'client lab1-6.idt.mdh.se' and follow the on-screen instructions.
 */

 /* Labb 2.
  * Following instructions in the pdf provided we have altered the code for server.c and client.c.
  * Altered code is commented with a '(Labb 2)'.
  *
  * Authors: Simon Alen, Mohamad Almalat.
  */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <netdb.h>

#define PORT 5555
#define hostNameLength 50
#define messageLength  256
#define MAXMSG 512

/* initSocketAddress
 * Initialises a sockaddr_in struct given a host name and a port.
 */
void initSocketAddress(struct sockaddr_in *name, char *hostName, unsigned short int port) {
  struct hostent *hostInfo; /* Contains info about the host */
  /* Socket address format set to AF_INET for Internet use. */
  name->sin_family = AF_INET;     
  /* Set port number. The function htons converts from host byte order to network byte order.*/
  name->sin_port = htons(port);   
  /* Get info about host. */
  hostInfo = gethostbyname(hostName); 
  if(hostInfo == NULL) {
    fprintf(stderr, "initSocketAddress - Unknown host %s\n",hostName);
    exit(EXIT_FAILURE);
  }
  /* Fill in the host name into the sockaddr_in struct. */
  name->sin_addr = *(struct in_addr *)hostInfo->h_addr;
}

// (Labb 2) Copy of server.c , readMessageFromClient();
int readMessageFromServer(int fileDescriptor) {
  char buffer[MAXMSG];
  int nOfBytes;

  nOfBytes = read(fileDescriptor, buffer, MAXMSG);
  if(nOfBytes < 0) {
    perror("Could not read data from client\n");
    printf("nOfBytes < 0");
    //exit(EXIT_FAILURE);
  }
  else
    if(nOfBytes == 0)  {
      printf("nOfBytes == 0");
      return(-1);
    }
    else 
      /* Data read */
      printf(">Incoming message from server: %s\n",  buffer);
  return(0);
}

/* writeMessage
 * Writes the string message to the file (socket) 
 * denoted by fileDescriptor.
 */
void writeMessage(int fileDescriptor, char *message) {
  int nOfBytes;
  
  nOfBytes = write(fileDescriptor, message, strlen(message) + 1);
  if(nOfBytes < 0) {
    perror("writeMessage - Could not write data\n");
    //exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[]) {
  int sock;
  struct sockaddr_in serverName;
  char hostName[hostNameLength];
  char messageString[messageLength];
  fd_set activeFdSet, readFdSet; // (Labb 2) fd_set that we use to simultaneously write incoming data as well as recieve user input.

  /* Check arguments */
  if(argv[1] == NULL) {
    perror("Usage: client [host name]\n");
    exit(EXIT_FAILURE);
  }
  else {
    strncpy(hostName, argv[1], hostNameLength);
    hostName[hostNameLength - 1] = '\0';
  }
  /* Create the socket */
  sock = socket(PF_INET, SOCK_STREAM, 0);
  if(sock < 0) {
    perror("Could not create a socket\n");
    exit(EXIT_FAILURE);
  }
  /* Initialize the socket address */
  initSocketAddress(&serverName, hostName, PORT);
  /* Connect to the server */
  if(connect(sock, (struct sockaddr *)&serverName, sizeof(serverName)) < 0) {
    perror("Could not connect to server\n");
    exit(EXIT_FAILURE);
  }
  /* Send data to the server */
  printf("\nType something and press [RETURN] to send it to the server.\n");
  printf("Type 'quit' to nuke this program.\n");
  fflush(stdin);

  // (Labb 2) We set both the sock and stdin (0) as present in set. This way we can get input from both simultaneously.
  FD_ZERO(&activeFdSet);
  FD_SET(sock, &activeFdSet);
  FD_SET(0, &activeFdSet);

  while(1) {
    readFdSet = activeFdSet;

    // (Labb 2) Selects socket
    if(select(FD_SETSIZE, &readFdSet, NULL, NULL, NULL) < 0) {
      perror("Select failed\n");
      exit(EXIT_FAILURE);
    }

    // (Labb 2) Checks if input is pending. If it has input ready, readMessage. Otherwise check user input.
    if(FD_ISSET(sock, &readFdSet))  {
      if(readMessageFromServer(sock) < 0) {
        close(sock);
      }
    }

    // (Labb 2) Check user input
    else {
      printf("\n>");
      fgets(messageString, messageLength, stdin);
      messageString[messageLength - 1] = '\0';
      if(strncmp(messageString,"quit\n",messageLength) != 0) {
        writeMessage(sock, messageString);
      }
      else {  
       close(sock);
       exit(EXIT_SUCCESS);
      }
    } 
  }
}
