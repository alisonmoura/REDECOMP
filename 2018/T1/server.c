#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <sys/uio.h> // sendfile no MAC
// #include <sys/sendfile.h> // sendfile no Linux
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

// Func. para calcular tamanho do arquivo
unsigned long fsize(char* file)
{
    FILE * f = fopen(file, "r");
    fseek(f, 0, SEEK_END);
    unsigned long len = (unsigned long)ftell(f);
    fclose(f);
    return len;
}

// Página HTML como resposta
char webpage[] = 
"HTTP/1.1 200 Ok\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>Servidor Web en C</title>\r\n"
"<style>body { background-color: #A9D0F5 }</style></head>\r\n"
"<body><center><h1>Hola mundo!</h1><br>\r\n"
"<img src=\"icono.png\"/></center></body></html>\r\n";

int main(int argc, char *argv[]) 
	{
	struct sockaddr_in server_addr, client_addr; 
	socklen_t sin_len =sizeof(client_addr);
	int fd_server, fd_client;
	char buf[2048];
	int fdimg;
	int on = 1;
        
    // Criando socket
    fd_server = socket(AF_INET, SOCK_STREAM, 0); 
        
    if (fd_server < 0) {
      perror("socket");
      exit(1);	
    }
        
    // Opçoes do socket
    setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
	
    // Famíla
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);
	
    if (bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
    {
      perror("bind");
      close(fd_server);
      exit(1);
    }

    if (listen(fd_server, 10) == -1)
    {
      perror("listen");
      close(fd_server);
      exit(1);
    }	
	
    // Loop
    while(1) {
      
      // Accept da conexao com o cliente
      fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);
      if (fd_client == 1)
      {
        perror("Falha na conexao...\n");
        continue;
      }	
      printf("Conexao estabelecida.\n");

      if (!fork()) {
        close(fd_server);
        memset(buf, 0, 2048);
        read(fd_client, buf, 2047);
        
        if(!strncmp(buf, "GET /favicon.ico", 16))
        {
          fdimg = open("favicon.ico", O_RDONLY);
          sendfile(fd_client, fdimg, NULL, fsize("favicon.ico"));
          close(fdimg);
        } else if(!strncmp(buf, "GET /icono.png", 14)) {
          fdimg = open("icono.png", O_RDONLY);
          sendfile(fd_client, fdimg, NULL, fsize("icono.png"));
          close(fdimg);
        } else  {
          write(fd_client, webpage, sizeof(webpage) -1);
        }
        
        close(fd_client);
        printf("Cerrando...\n");
        exit(0);
        
      }
      close(fd_client);
	}	

	return 0;
}