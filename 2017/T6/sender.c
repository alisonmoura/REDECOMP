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

void handle_alarm(int sig)
{
    printf("Reenviando pacote");
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        error("Deve ser informado o arquivo na linha de comando!");
    }

    char *filePath = argv[1];
    char *hostname = "127.0.0.1";
    char *package = (char *)malloc(102);
    FILE *fp = fopen(filePath, "rb");
    long fileSize = file_size(filePath);
    int *header = 0;
    int n = fileSize / 100 + 1;
    int bytesToRead;
    int bytesToLast = fileSize / 100;
    char lasToReaded[bytesToLast];
    int sockfd, ret;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    int ack = 1;
    char *strAck;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    server = gethostbyname(hostname);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(PORT_NUMBER);

    printf("=======================================================\n");
    printf("Endreço do arquivo: %s\n", filePath);
    printf("=======================================================\n");
    printf("Será enviado um arquivo com %d bytes\n", fileSize);
    printf("=======================================================\n");
    printf("Serão enviados %d pacotes com 100 bytes e 1 pacote com %d bytes\n", fileSize / 100, fileSize % 100);
    printf("=======================================================\n");

    do
    {
        if (n == 1)
        {
            bytesToRead = fileSize % 100 + fileSize / 100;
            free(package);
            package = (char *)malloc(bytesToRead);
        }
        else
            bytesToRead = 100;

        char readed[bytesToRead];

        printf("=======================================================\n");
        printf("Bytes a serem lidos: %d bytes\n", bytesToRead);
        printf("=======================================================\n");

        fgets(readed, bytesToRead, (FILE *)fp);
        strcpy(package, "0");
        strcat(package, readed);
        strcat(package, "\0");

        printf("=======================================================\n");
        printf("Enviando o pacote: %s\t Quantidade lida: %d bytes\n", package, strlen(package));
        printf("=======================================================\n");

        serverlen = sizeof(serveraddr);
        ret = sendto(sockfd, package, strlen(package), 0, (struct sockaddr *)&serveraddr, serverlen);
        if (ret < 0)
            error("ERROR in sendto");

        ret = recvfrom(sockfd, package, strlen(package), 0, (struct sockaddr *)&serveraddr, &serverlen);
        if (ret < 0)
            error("ERROR in recvfrom");

        // signal(SIGALRM, handle_alarm);
        // alarm(1);

        strAck = (char *)malloc(sizeof package);
        strcpy(strAck, package);
        printf("ACK recebido: %s\n", strAck);
        printf("ACK esperado: %d\n", ack);

        if (atoi(strAck) != ack)
        {
            while (atoi(strAck) != ack)
            {
                printf("Reenviar pacote: %s\n", strAck);
                ret = sendto(sockfd, package, strlen(package), 0, (struct sockaddr *)&serveraddr, serverlen);
                if (ret < 0)
                    error("ERROR in sendto");

                ret = recvfrom(sockfd, package, strlen(package), 0, (struct sockaddr *)&serveraddr, &serverlen);
                if (ret < 0)
                    error("ERROR in recvfrom");

                sleep(2);
            }
        }

        n--;
        ack++;

    } while (n > 0);

    if (ack == (fileSize / 100 + 2))
    {
        printf("Arquivo trasmitido corretamente\n");
    }

    free(package);
    fclose(fp);
}