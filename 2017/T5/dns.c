#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 53
#define A 1
#define MX 15
#define AAAA 28

struct DNS_HEADER
{
    unsigned short id; // Trans. ID

    unsigned char rd : 1;     // recursion desired
    unsigned char tc : 1;     // truncated message
    unsigned char aa : 1;     // authoritive answer
    unsigned char opcode : 4; // purpose of message
    unsigned char qr : 1;     // query/response flag

    unsigned char rcode : 4; // response code
    unsigned char cd : 1;    // checking disabled
    unsigned char ad : 1;    // authenticated data
    unsigned char z : 1;     // reserved
    unsigned char ra : 1;    // recursion available

    unsigned short q_count;    // Number of Questions
    unsigned short ans_count;  // Number of Answers
    unsigned short auth_count; // Number of Authority
    unsigned short add_count;  // Number of Addicional
};

struct QUESTION
{
    unsigned short qtype;  // Query Type : A = 1, MX = 15, AAAA = 28
    unsigned short qclass; // Query Class
};

static void get_qname(unsigned char *from, unsigned char *to)
{
    int lock = 0, i;

    strcat((char *)from, ".");
    for (i = 0; i < strlen((char *)from); i++)
    {
        if (from[i] == '.')
        {
            for (*to++ = i - lock; lock < i; lock++)
            {
                *to++ = from[lock];
            }
            lock++;
        }
    }
    *to++ = '\0';
}

static void print_buffer(char *buffer, int size)
{
    for (int i = 0; i < size; i++)
    {
        printf("%02x\t", buffer[i] & 0xff);
    }
    printf("\n");
}

static void error(char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char **argv)
{
    int sockfd, res;
    unsigned char buffer[65536], *query;
    struct sockaddr_in src_addr, dest_addr;
    struct hostent *h;

    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(PORT);

    struct DNS_HEADER *dns = NULL;
    struct QUESTION *qinfo = NULL;

    if (argc < 2)
        error("Forneça ao menos o domínio de destino");

    char *domain = argv[1];
    char *ip = "127.0.0.1";

    if (argc >= 3)
        ip = argv[2];

    if (inet_aton((char *)&ip, &(src_addr.sin_addr)) != 0)
        error("IP inválido");

    if ((h = gethostbyname(domain)) == NULL)
        error("Servidor destino de DNS inválido!");

    if (h->h_addrtype == AF_INET)
        printf("É Ipv4\n");
    else if (h->h_addrtype == AF_INET6)
        printf("É Ipv6\n");

    dest_addr.sin_family = h->h_addrtype;
    memcpy((char *)&dest_addr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    dest_addr.sin_port = htons(PORT);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        error("Erro ao criar socket");

    dns = (struct DNS_HEADER *)&buffer;

    dns->id = (unsigned short)htons(getpid());
    dns->qr = 0;
    dns->opcode = 0;
    dns->aa = 0;
    dns->tc = 0;
    dns->rd = 1;
    dns->ra = 0;
    dns->z = 0;
    dns->ad = 0;
    dns->cd = 0;
    dns->rcode = 0;
    dns->q_count = htons(1);
    dns->ans_count = 0;
    dns->auth_count = 0;
    dns->add_count = 0;

    query = (unsigned char *)&buffer[sizeof(struct DNS_HEADER)];
    get_qname(domain, query);
    qinfo = (struct QUESTION *)&buffer[sizeof(struct DNS_HEADER) + (strlen((const char *)query) + 1)];
    qinfo->qtype = htons(AAAA);
    qinfo->qclass = htons(1);

    printf("Enviando socket...");
    if (sendto(sockfd, (char *)buffer, sizeof(struct DNS_HEADER) + (strlen((const char *)query) + 1) + sizeof(struct QUESTION), 0,
               (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
        error("Erro ao enviar socket");
    printf("OK\n");

    printf("Lendo retorno...");
    size_t nread = read(sockfd, buffer, sizeof buffer);
    if (nread == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }
    printf("OK\n");

    printf("Received %ld bytes: %s\n", (long)nread, buffer);

    printf("Recebendo resposta...\n");
    if (recvfrom(sockfd, (char *)buffer, sizeof(buffer), 0, (struct sockaddr *)&dest_addr, (socklen_t *)sizeof(dest_addr)) < 0)
        error("Erro receber resposta");

    printf("Resposta recebida!\n");

    print_buffer(buffer, sizeof buffer);

    dns = (struct DNS_HEADER *)buffer;
    char *reader = &buffer[sizeof(struct DNS_HEADER) + (strlen((const char *)query) + 1) + sizeof(struct QUESTION)];

    // printf("Resposta:\n");
    // printf("Perguntas: %d\n", ntohs(dns->q_count));
    // printf("Respostas: %d\n", ntohs(dns->ans_count));
    // printf("Servidores com autoridade: %d\n", ntohs(dns->auth_count));
    // printf("Registros adicionais: %d\n", ntohs(dns->add_count));

    return 0;
}