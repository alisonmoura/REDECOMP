#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

static void error(char *s)
{
    perror(s);
    exit(1);
}
int main(int argc, char **argv)
{
    int port = 53;
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    struct addrinfo *addrs, *p;
    struct addrinfo hints;
    int sockfd;

    if (argc < 2)
        error("Forneça ao menos o domínio de destino");

    char *domain = argv[1];
    char *ip = "127.0.0.1";

    if (argc >= 3)
        ip = argv[2];

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (inet_aton((char *)&ip, &(sin.sin_addr)) != 0)
        error("IP inválido");

    /**
    * Pega informações do servidor destino da requisição
    * O primeiro parâmetro da função é o endereço do servidor destino (domínio/host ou IP)
    * O segundo parâmetro pode ser o serviço "http, ftp, telnet, smtp" ou a porta
    * O terceiro parâmetro é o endereço de memória do ponteiro de uma struc addinfo que receberá as informações coletadas do servidor
    */
    if (getaddrinfo(domain, (char *)&port, &hints, &addrs) != 0)
        error("Erro ao abter informações do endereço");

    /**
    * Looping para tentar acessar o destino
    */
    for (p = addrs; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("listener: socket");
            continue;
        }
        if (bind(sockfd, (struct sockaddr *)&sin, sizeof sin) == -1)
        {
            close(sockfd);
            perror("listener: bind");
            continue;
        }
        sleep(2);
        printf("Requisição enviada!\n");
        break;
    }
}