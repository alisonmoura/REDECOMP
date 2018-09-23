/* hw2-simple-client.c: program to connect to web server. */
/* compile with: gcc -Wall -o hw2-simple-client hw2-simple-client.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <string.h>

/* maximum size of a printed address -- if not defined, we define it here */
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif /* INET6_ADDRSTRLEN */
#define BUFSIZE 1000

bool http = false;
bool https = false;

/*
 * Fonte: StackOverflow
 *
 *
 * Description:
 *   Find and replace text within a string.
 *
 * Parameters:
 *   src  (in) - pointer to source string
 *   from (in) - pointer to search text
 *   to   (in) - pointer to replacement text
 *
 * Returns:
 *   Returns a pointer to dynamically-allocated memory containing string
 *   with occurences of the text pointed to by 'from' replaced by with the
 *   text pointed to by 'to'.
 */
char *replace(const char *src, const char *from, const char *to)
{
    /*
    * Find out the lengths of the source string, text to replace, and
    * the replacement text.
    */
    size_t size = strlen(src) + 1;
    size_t fromlen = strlen(from);
    size_t tolen = strlen(to);
    /*
    * Allocate the first chunk with enough for the original string.
    */
    char *value = malloc(size);
    /*
    * We need to return 'value', so let's make a copy to mess around with.
    */
    char *dst = value;
    /*
    * Before we begin, let's see if malloc was successful.
    */
    if (value != NULL)
    {
        /*
       * Loop until no matches are found.
       */
        for (;;)
        {
            /*
          * Try to find the search text.
          */
            const char *match = strstr(src, from);
            if (match != NULL)
            {
                /*
             * Found search text at location 'match'. :)
             * Find out how many characters to copy up to the 'match'.
             */
                size_t count = match - src;
                /*
             * We are going to realloc, and for that we will need a
             * temporary pointer for safe usage.
             */
                char *temp;
                /*
             * Calculate the total size the string will be after the
             * replacement is performed.
             */
                size += tolen - fromlen;
                /*
             * Attempt to realloc memory for the new size.
             */
                temp = realloc(value, size);
                if (temp == NULL)
                {
                    /*
                * Attempt to realloc failed. Free the previously malloc'd
                * memory and return with our tail between our legs. :(
                */
                    free(value);
                    return NULL;
                }
                /*
             * The call to realloc was successful. :) But we'll want to
             * return 'value' eventually, so let's point it to the memory
             * that we are now working with. And let's not forget to point
             * to the right location in the destination as well.
             */
                dst = temp + (dst - value);
                value = temp;
                /*
             * Copy from the source to the point where we matched. Then
             * move the source pointer ahead by the amount we copied. And
             * move the destination pointer ahead by the same amount.
             */
                memmove(dst, src, count);
                src += count;
                dst += count;
                /*
             * Now copy in the replacement text 'to' at the position of
             * the match. Adjust the source pointer by the text we replaced.
             * Adjust the destination pointer by the amount of replacement
             * text.
             */
                memmove(dst, to, tolen);
                src += fromlen;
                dst += tolen;
            }
            else /* No match found. */
            {
                /*
             * Copy any remaining part of the string. This includes the null
             * termination character.
             */
                strcpy(dst, src);
                break;
            }
        }
    }
    return value;
}

/* print a system error and exit the program */
static void error(char *s)
{
    perror(s);
    exit(1);
}
static void usage(char *program)
{
    printf("usage: %s hostname [port]\n", program);
    exit(1);
}
static char *separteURL(char *url)
{
    char *token;

    if (strstr(url, "http://") != NULL)
    {
        http = true;
        token = replace(url, "http://", "");
        printf(" %s\n", token);
    }
    else if (strstr(url, "https://") != NULL)
    {
        https = false;
        token = replace(url, "https://", "");
        printf(" %s\n", token);
    }else{
        return url;
    }

    return token;
}
static char *build_request(char *hostname)
{
    char header1[] = "GET / HTTP/1.1\r\nHost: ";
    char header2[] = "\r\n\r\n";
    /* add 1 to the total length, so we have room for the null character --
* the null character is never sent, but is needed to make this a C string */
    int tlen = strlen(header1) + strlen(hostname) + strlen(header2) + 1;
    char *result = malloc(tlen);
    if (result == NULL)
        return NULL;
    snprintf(result, tlen, "%s%s%s", header1, hostname, header2);
    return result;
}
/* must be executed inline, so must be defined as a macro */
#define next_loop(a, s) \
    {                   \
        if (s >= 0)     \
            close(s);   \
        a = a->ai_next; \
        continue;       \
    }
int main(int argc, char **argv)
{
    int sockfd;
    struct addrinfo *addrs;
    struct addrinfo hints;
    char *port = "80"; /* default is to connect to the http port, port 80 */
    if ((argc != 2) && (argc != 3))
        usage(argv[0]);
    char *hostname = argv[1];
    if (argc == 3)
        port = argv[2];
    hostname = separteURL(hostname);
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(hostname, port, &hints, &addrs) != 0)
        error("getaddrinfo");
    struct addrinfo *original_addrs = addrs;
    while (addrs != NULL)
    {
        char buf[BUFSIZE];
        char prt[INET6_ADDRSTRLEN] = "unable to print";
        int af = addrs->ai_family;
        struct sockaddr_in *sinp = (struct sockaddr_in *)addrs->ai_addr;
        struct sockaddr_in6 *sin6p = (struct sockaddr_in6 *)addrs->ai_addr;
        if (af == AF_INET)
            inet_ntop(af, &(sinp->sin_addr), prt, sizeof(prt));
        else if (af == AF_INET6)
            inet_ntop(af, &(sin6p->sin6_addr), prt, sizeof(prt));
        else
        {
            printf("unable to print address of family %d\n", af);
            next_loop(addrs, -1);
        }
        if ((sockfd = socket(af, addrs->ai_socktype, addrs->ai_protocol)) < 0)
        {
            perror("socket");
            next_loop(addrs, -1);
        }
        printf("trying to connect to address %s, port %s\n", prt, port);
        if (connect(sockfd, addrs->ai_addr, addrs->ai_addrlen) != 0)
        {
            perror("connect");
            next_loop(addrs, sockfd);
        }
        printf("connected to %s\n", prt);
        char *request = build_request(hostname);
        if (request == NULL)
        {
            printf("memory allocation (malloc) failed\n");
            next_loop(addrs, sockfd);
        }
        if (send(sockfd, request, strlen(request), 0) != strlen(request))
        {
            perror("send");
            next_loop(addrs, sockfd);
        }
        free(request); /* return the malloc'd memory */
        /* sometimes causes problems, and not needed
        shutdown (sockfd, SHUT_WR); */
        int count = 0;
        while (1)
        {
            /* use BUFSIZE - 1 to leave room for a null character */
            int rcvd = recv(sockfd, buf, BUFSIZE - 1, 0);
            count++;
            if (rcvd <= 0)
                break;
            buf[rcvd] = '\0';
            printf("%s", buf);
        }
        printf("data was received in %d recv calls\n", count);
        next_loop(addrs, sockfd);
    }
    freeaddrinfo(original_addrs);
    return 0;
}