#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "lib.h"

#define BUFSIZE 4096 // max number of bytes we can get at once

/**
 * Struct to hold all three pieces of a URL
 */
typedef struct urlinfo_t {
  char *hostname;
  char *port;
  char *path;
} urlinfo_t;

/**
 * Tokenize the given URL into hostname, path, and port.
 *
 * url: The input URL to parse.
 *
 * Store hostname, path, and port in a urlinfo_t struct and return the struct.
*/
urlinfo_t *parse_url(char *url)
{
  // copy the input URL so as not to mutate the original
  char *hostname = strdup(url);
  char *port;
  char *path;

  urlinfo_t *urlinfo = malloc(sizeof(urlinfo_t));

  char *first_bs = strchr(hostname, '/');// Use strchr to find the first backslash in the URL
  path = first_bs + 1; //Set the path pointer to 1 character after the spot returned by strchr.
  *first_bs = '\0'; //Overwrite the backslash with a '\0' 

  char *first_colon = strchr(hostname, ':'); //Use strchr to find the first colon in the URL.
  port = first_colon + 1;//Set the port pointer to 1 character after the spot returned by strchr.
  *first_colon = '\0';//Overwrite the colon with a '\0'

  urlinfo->hostname = hostname;
  urlinfo->port = port;
  urlinfo->path = path;

  return urlinfo;
}

/**
 * Constructs and sends an HTTP request
 *
 * fd:       The file descriptor of the connection.
 * hostname: The hostname string.
 * port:     The port string.
 * path:     The path string.
 *
 * Return the value from the send() function.
*/
int send_request(int fd, char *hostname, char *port, char *path)
{
  const int max_request_size = 16384;
  char request[max_request_size];
  int rv;

  //use `sprintf` in order to construct the request from the `hostname`, `port`, and `path`
  int request_length = sprintf(request,
    "GET /%s HTTP/1.1\n"
    "Host: %s:%s\n"
    "Connection: close\n\n",

    path,
    hostname,
    port
  );
  
 // Send it all!
  rv = send(fd, request, request_length, 0);

  if (rv < 0) {
      perror("error in send_request()");
  }

  return rv;
}

int main(int argc, char *argv[])
{  
  int sockfd, numbytes;  
  char buf[BUFSIZE];

  if (argc != 2) {
    fprintf(stderr,"usage: client HOSTNAME:PORT/PATH\n");
    exit(1);
  }

  urlinfo_t *urlinfo = malloc(sizeof(urlinfo_t));
  urlinfo = parse_url(argv[1]); // Parse the input URL, parse_url defined above 
  sockfd = get_socket(urlinfo->hostname, urlinfo->port); // Initialize socket - get_socket() from lib.h
  
  send_request(sockfd, urlinfo->hostname, urlinfo->port, urlinfo->path); //construct request & send 

  while ((numbytes = recv(sockfd, buf, BUFSIZE - 1, 0)) > 0) {
    //loop until there is no more data to receive from the server
    fprintf(stdout, "%s\n", buf); // Receive the response from the server and print it to `stdout`.
  }

  free(urlinfo); //Clean up any allocated memory
  close(sockfd); // `close` any open file descriptors

  return 0;
}
