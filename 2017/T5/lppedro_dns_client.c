#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <resolv.h>
#include <unistd.h>

#define PORT 53

// https://tools.ietf.org/html/rfc1035 página 25, 4.1.1
struct DNS_HEADER {
  unsigned short id;         // identification number

  unsigned char rd : 1;      // recursion desired
  unsigned char tc : 1;      // truncated message
  unsigned char aa : 1;      // authoritive answer
  unsigned char opcode : 4;  // purpose of message
  unsigned char qr : 1;      // query/response flag

  unsigned char rcode : 4;   // response code
  unsigned char cd : 1;      // checking disabled
  unsigned char ad : 1;      // authenticated data
  unsigned char z : 1;       // reserved
  unsigned char ra : 1;      // recursion available

  unsigned short q_count;    // number of question entries
  unsigned short ans_count;  // number of answer entries
  unsigned short auth_count; // number of authority entries
  unsigned short add_count;  // number of resource entries
};
 
// https://tools.ietf.org/html/rfc1035 página 27, 4.1.2
struct QUESTION {
  // aqui fica qname, que possui tamanho constante (entre o fim de DNS_HEADER e o qtype de QUESTION)
  unsigned short qtype;
  unsigned short qclass;
};

void get_qname(unsigned char *from, unsigned char *to);
static void error(char *s);


int main(int argc, char *argv[]) {
  int sd, rc;
  unsigned char buf[65536], *qname;
  struct sockaddr_in cli_addr, remote_addr;
  struct hostent *h;

  struct DNS_HEADER *dns = NULL;
  struct QUESTION *qinfo = NULL;

  char *domain = argv[1];
  char *dns_address;

  if(argc != 3) {
    error("Parâmetros inválidos\nUse: ./dns [domain] [dns_server]");
  }

  // ----------------- CRIANDO O SOCKET COM DESTINO PARA O SERVIDOR DNS -----------------

  dns_address = argv[2];
  if((h = gethostbyname(dns_address)) == NULL) {
    error("Servidor destino de DNS inválido!");
  }

  remote_addr.sin_family = h->h_addrtype;
  memcpy((char *) &remote_addr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  remote_addr.sin_port = htons(PORT);

  if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    error("Erro ao criar socket");
  }

  // ----------------- CONSTRUINDO A REQUISIÇÃO DNS -----------------

  dns = (struct DNS_HEADER *) &buf;

  // https://tools.ietf.org/html/rfc1035 p�gina 25, 4.1.1
  dns->id = (unsigned short) htons(getpid()); // usar o pid como id
  dns->qr = 0;     // tipo query
  dns->opcode = 0; // tipo de query padr�o
  dns->aa = 0;     // sem autoridade
  dns->tc = 0;     // mensagem n�o � truncada
  dns->rd = 1;
  dns->ra = 0;
  dns->z = 0;
  dns->ad = 0;
  dns->cd = 0;
  dns->rcode = 0;
  dns->q_count = htons(1); // 1 pergunta
  dns->ans_count = 0;
  dns->auth_count = 0;
  dns->add_count = 0;

  // QUESTION/qname come�a depois do final da DNS_HEADER, ent�o calcular o final de DNS_HEADER e criar o ponteiro
  qname = (unsigned char *) &buf[sizeof(struct DNS_HEADER)];

  // https://tools.ietf.org/html/rfc1035 p�gina 27, 4.1.2, QNAME
  // a parte do QNAME deve ser separada em labels com seu tamanho
  // exemplo: www.google.com vira 3www6google3com
  get_qname(domain, qname);

  // qinfo (qtype e qclass) vem depois de qname, ent�o calcular o tamanho de qname e criar o ponteiro
  qinfo = (struct QUESTION *) &buf[sizeof(struct DNS_HEADER) + (strlen((const char *) qname) + 1)];

  // https://tools.ietf.org/html/rfc1035 p�gina 11, 3.2.2 & 3.2.3, (Q)TYPE values: A = 1, MX = 15
  // https://tools.ietf.org/html/rfc3596 p�gina 2, 2.1, AAAA = 28
  qinfo->qtype = htons(1); 
  
  // https://tools.ietf.org/html/rfc1035 p�gina 12, 3.2.4 & 3.2.5, (Q)CLASS values: IN - 1 the Internet
  qinfo->qclass = htons(1);

  // ----------------- ENVIANDO A REQUISI��O DNS -----------------

  // o tamanho total � o tamanho de DNS_HEADER + o tamanho din�mico de QNAME + o tamanho da parte de tamanho constante de QUESTION (qtype e qclass)
  rc = sendto(sd,
              (char *)buf, 
              sizeof(struct DNS_HEADER) + (strlen((const char *) qname) + 1) + sizeof(struct QUESTION),
              0,
              (struct sockaddr *) &remote_addr,
              sizeof(remote_addr));
  if(rc < 0) {
    error("Error no envio do buffer");
  }

  // ----------------- RECEBENDO A HEADER DA RESPOSTA -----------------

  int size = sizeof(remote_addr);
  if(recvfrom(sd, (char *) buf, sizeof(buf), 0, (struct sockaddr *) &remote_addr, (socklen_t *) &size) < 0) {
    error("Erro ao receber resposta");
  }

  // https://tools.ietf.org/html/rfc1035 p�gina 25
  // https://tools.ietf.org/html/rfc6895 p�gina 2
  dns = (struct DNS_HEADER *) buf;
  char *reader = &buf[sizeof(struct DNS_HEADER) + (strlen((const char *) qname) + 1) + sizeof(struct QUESTION)];

  printf("Resposta:\n");
  printf("Perguntas: %d\n", ntohs(dns->q_count));
  printf("Respostas: %d\n", ntohs(dns->ans_count));
  printf("Servidores com autoridade: %d\n", ntohs(dns->auth_count));
  printf("Registros adicionais: %d\n", ntohs(dns->add_count));

  // ----------------- DAQUI PARA BAIXO INTERPRETAR O BODY DA RESPOSTA -----------------

  return 0;
}


void get_qname(unsigned char *from, unsigned char *to) {
  int lock = 0 , i;

  strcat((char *) from, ".");
  for(i = 0; i < strlen((char *) from); i++) {
    if(from[i] == '.') {
      for(*to++ = i - lock; lock < i ; lock++) {
        *to++ = from[lock];
      }
      lock++;
    }
  }
  *to++ = '\0';
}

static void error(char * s) {
  printf("Error: %s\n", s);
  exit(1);
}
