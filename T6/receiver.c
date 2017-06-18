#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT_NUMBER 2001
#define BUFSIZE 102

static void error(char *s)
{
    printf("ERR: %s\n", s);
    exit(1);
}

static long file_size(char *file_name)
{
    struct stat st;
    if (stat(file_name, &st) == 0)
        return st.st_size;
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        error("Deve ser informado o arquivo na linha de comando!");
    }

    char basePath[53] = "/home/alisonmoura/Documents/REDCOMP/Trabalhos/T6/tmp/";
    char *fileName = argv[1];
    char *host = "127.0.0.1";

    int newSize = (sizeof(fileName) + sizeof(basePath) + 1);
    char *filePath = (char *)malloc(newSize);

    strcpy(filePath, basePath);
    strcat(filePath, fileName);

    printf("%s\n", filePath);

    FILE *fp = fopen(filePath, "ab");

    if (file_size(filePath) > 0)
        error("O arquivo já existe\n");
    else
        printf("O arquivo não existe\n");

    int sockfd;
    int clientlen;
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    struct hostent *hostp;
    char buf[BUFSIZE];
    char *hostaddrp;
    int optval;
    int n;
    int ack = 1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("ao abrir o socket");

    optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
               (const void *)&optval, sizeof(int));

    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)PORT_NUMBER);

    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        error("binding");

    clientlen = sizeof(clientaddr);
    while (1)
    {

        bzero(buf, BUFSIZE);
        n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &clientlen);

        if (n < 0)
            error("recvfrom");

        hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);

        if (hostp == NULL)
            error("gethostbyaddr");

        hostaddrp = inet_ntoa(clientaddr.sin_addr);

        if (hostaddrp == NULL)
            error("inet_ntoa\n");

        printf("Dados recebidos de: %s (%s)\n", hostp->h_name, hostaddrp);
        printf("Recebido %d/%d bytes: %s\n", strlen(buf), n, buf);
        printf("Escrevendo no arquivo...\n");

        char *strAck = (char *)malloc(2);
        sprintf(strAck, "%d", ack);

        printf("Enviando ACK: %s\n", strAck);
        n = sendto(sockfd, strAck, sizeof(ack), 0, (struct sockaddr *)&clientaddr, clientlen);

        printf("Buf size %d\n", sizeof(buf));
        if (sizeof buf < 102 && sizeof buf > 2)
        {
            printf("Arquivo recebido\n");
            exit(0);
        }

        //Não está escrevendo no arquivo
        fputs((char *)&buf, fp);

        if (n < 0)
            error("sendto");

        ack++;
    }

    printf("Fechando o arquivo");
    fclose(fp);
    free(filePath);
}