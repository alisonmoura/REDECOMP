/* hw2-simple-client.c: program to connect to web server. */
/* compile with: gcc -Wall -o hw2-simple-client hw2-simple-client.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

/* maximum size of a printed address -- if not defined, we define it here */
#ifndef INET6_ADDRSTRLEN
  #define INET6_ADDRSTRLEN 46
#endif /* INET6_ADDRSTRLEN */

#define BUFSIZE 1000

/* print a system error and exit the program */
static void error (char * s) {
  perror (s);
  exit (1);
}

static void usage (char * program) {
  printf ("usage: %s hostname [port]\n", program);
  exit (1);
}

static void removeProtocol (char *s) {
  char http[] = "http://";
  char https[] = "https://";

  if( strstr(s,http) ) {
    while( (s=strstr(s,http)) )
      memmove(s,s+strlen(http),1+strlen(s+strlen(http)));

  } else {
    while( (s=strstr(s,https)) )
      memmove(s,s+strlen(https),1+strlen(s+strlen(https)));
  }
}

static char * build_request (char * hostname) {
  char *full_hostname = hostname;
  char host[100];
  char path[100];
  char method [] = "GET ";
  char request [] = " HTTP/1.1\r\nHost: ";
  char sufix [] = "\r\n\r\n";

  removeProtocol(full_hostname);
  sscanf(full_hostname, "%99[^\n^/]/%99[^\n]", host, path);

  if ( (strlen(path) < 1) ) {
   strcpy(path, "/");
  }

  /* add 1 to the total length, so we have room for the null character --
  * the null character is never sent, but is needed to make this a C string */
  int total_length = strlen(host) + strlen(path) + strlen(method) + strlen(request) + strlen(sufix) + 1;
  char * result = malloc (total_length);

  if (result == NULL) {
    return NULL;
  }

  snprintf (result, total_length, "%s%s%s%s%s", method, path, request, host, sufix);

  printf("%s", result);

  return result;
}

/* must be executed inline, so must be defined as a macro */
#define next_loop(a, s) { if (s >= 0) close (s); a = a->ai_next; continue; }

int main (int argc, char ** argv) {
  int sockfd;
  struct addrinfo * addrs;
  struct addrinfo hints;
  char * port = "80"; /* default is to connect to the http port, port 80 */

  if ((argc != 2) && (argc != 3)) {
    usage (argv [0]);
  }

  char * hostname = argv [1];

  if (argc == 3) {
    port = argv [2];
  }

  bzero (&hints, sizeof (hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo (hostname, port, &hints, &addrs) != 0) {
    error ("getaddrinfo");
  }

  struct addrinfo * original_addrs = addrs;

  while (addrs != NULL) {
    char buf [BUFSIZE];
    char prt [INET6_ADDRSTRLEN] = "unable to print";
    int af = addrs->ai_family;
    struct sockaddr_in * sinp = (struct sockaddr_in *) addrs->ai_addr;
    struct sockaddr_in6 * sin6p = (struct sockaddr_in6 *) addrs->ai_addr;

    if (af == AF_INET){
      inet_ntop (af, &(sinp->sin_addr), prt, sizeof (prt));
    } else if (af == AF_INET6) {
      inet_ntop (af, &(sin6p->sin6_addr), prt, sizeof (prt));
    } else {
      printf ("unable to print address of family %d\n", af);
      next_loop (addrs, -1);
    }

    if ((sockfd = socket (af, addrs->ai_socktype, addrs->ai_protocol)) < 0) {
      perror ("socket");
      next_loop (addrs, -1);
    }

    printf ("trying to connect to address %s, port %s\n", prt, port);

    if (connect (sockfd, addrs->ai_addr, addrs->ai_addrlen) != 0) {
      perror ("connect");
      next_loop (addrs, sockfd);
    }

    printf ("connected to %s\n", prt);
    char * request = build_request (hostname);

    if (request == NULL) {
      printf ("memory allocation (malloc) failed\n");
      next_loop (addrs, sockfd);
    }
    if (send (sockfd, request, strlen (request), 0) != strlen (request)) {
      perror ("send");
      next_loop (addrs, sockfd);
    }

    free (request); /* return the malloc’d memory */
                    /* sometimes causes problems, and not needed
                    shutdown (sockfd, SHUT_WR); */
    int count = 0;
    while (1) {
      /* use BUFSIZE - 1 to leave room for a null character */
      int rcvd = recv (sockfd, buf, BUFSIZE - 1, 0);
      count++;
      if (rcvd <= 0) {
        break;
      }
      buf [rcvd] = '\0';
      printf ("%s", buf);
    }
    printf ("data was received in %d recv calls\n", count);
    next_loop (addrs, sockfd);
  }

  freeaddrinfo (original_addrs);

  return 0;
}