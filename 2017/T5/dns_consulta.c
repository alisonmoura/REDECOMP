//https://github.com/wfelipe/simple-dns/blob/master/src/client.c

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#ifndef __DNS_PROTOCOL_H__
#define __DNS_PROTOCOL_H__

#include <sys/types.h>

/* QR
 * a one bit field that specifies whether this message is a query
 * or a response
 */
#define QR_QUERY 0
#define QR_RESPONSE 1

/* OPCODE
 * a four bit field that specifies kind of query in this message
 */
#define OPCODE_QUERY 0 /* a standard query */
#define OPCODE_IQUERY 1 /* an inverse query */
#define OPCODE_STATUS 2 /* a server status request */
/* 3-15 reserved for future use */

/* AA
 * one bit, valid in responses, and specifies that the responding
 * name server is an authority for the domain name in question
 * section
 */
#define AA_NONAUTHORITY 0
#define AA_AUTHORITY 1

struct dns_header
{
	u_int16_t id; /* a 16 bit identifier assigned by the client */
	u_int16_t qr:1;
	u_int16_t opcode:4;
	u_int16_t aa:1;
	u_int16_t tc:1;
	u_int16_t rd:1;
	u_int16_t ra:1;
	u_int16_t z:3;
	u_int16_t rcode:4;
	u_int16_t qdcount;
	u_int16_t ancount;
	u_int16_t nscount;
	u_int16_t arcount;
};


struct dns_packet
{
	struct dns_header header;
//	struct dns_question question;
	char *data;
	u_int16_t data_size;
};

struct dns_response_packet
{
	char *name;
	u_int16_t type;
	u_int16_t class;
	u_int32_t ttl;
	u_int16_t rdlength;
	char *rdata;
};

struct dns_question
{
	char *qname;
	u_int16_t qtype;
	u_int16_t qclass;
};

void dns_print_header (struct dns_header *header);
void dns_print_packet (struct dns_packet *packet);

int dns_request_parse (struct dns_packet *pkt, void *data, u_int16_t size);
int dns_header_parse (struct dns_header *header, void *data);
int dns_question_parse (struct dns_packet *pkt);

#endif

int main (int argc, char **argv)
{
	struct dns_packet *packet;
	struct sockaddr_in sin;
	int sin_len = sizeof (sin);
	int sock;
	char buf[256];
	int buf_len = 0;

	sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	memset ((char *) &sin, 0, sizeof (sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons (53);
	inet_aton ("127.0.0.1", &sin.sin_addr);
	// inet_aton ("201.6.0.103", &sin.sin_addr);

	packet = calloc (1, sizeof (struct dns_packet));
	packet->header.id = 2048;

	memcpy (buf, &packet->header, 12);
	buf_len = 12;
	memcpy (buf + buf_len, "www.uol.com.br", sizeof ("www.uol.com.br"));
	buf_len += sizeof ("www.uol.com.br");

	sendto (sock, buf, buf_len, 0, (struct sockaddr *) &sin, sin_len);
	recv (sock, buf, 255, 0);

	printf ("%s\n", buf);

	return 0;
}