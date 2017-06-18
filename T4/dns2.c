#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

# define PORT 53
# define BUFFSIZE 20

static void error(char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in sin;
    sin.sin_port = htons(PORT);
    struct addrinfo hints, *res, *p;
    int status, sockfd;
    int numbytes;
    char ipstr[INET6_ADDRSTRLEN];
    char buffer[BUFFSIZE] = "Olá destinatário!";


    if (argc != 2)
    {
        error("Passe o domínio");
    }

    char *ip = "127.0.0.1";

    if (argc >= 3)
        ip = argv[2];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (inet_aton((char *)&ip, &(sin.sin_addr)) != 0)
        error("IP inválido");

    if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        error("Erro ao obter informações do servidor destino");
    }

    printf("Endereço IP do %s:\n", argv[1]);

    for (p = res; p != NULL; p = p->ai_next)
    {
        void *addr;
        char *ipver;

        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            continue;
        }

        if (p->ai_family == AF_INET)
        {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }
        else
        {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("%s: %s\n", ipver, ipstr);
        break;
    }

    if (p == NULL)
    {
        error("Falha ao criar o socket");
    }

    printf("%s\n", p->ai_addr);

    if ((numbytes = sendto(sockfd, buffer, BUFFSIZE, 0, p->ai_addr, p->ai_addrlen)) == -1)
    {
        error("Erro ao enviar socket");
    }

    freeaddrinfo(res);
    return 0;
}