#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void error(char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

static void bufferize(char *p, char *buffer, int max, int iterator, int i, int bufferSize)
{
    if (iterator > max)
    {
        return;
    }

    while (*p != '/')
    {
        printf("%c\n", *p);
        bufferSize++;
        p++;
        iterator++;
    }
    iterator++;

    buffer = (char *)malloc(bufferSize + 1);

    if (buffer == NULL)
        error("Erro na alocação de memória");

    i = bufferSize - 1;
    p--;
    buffer[bufferSize] = '\0';
    while (i >= 0)
    {
        buffer[i] = *p;
        i--;
        p--;
    }
    printf("%s (%d)\n", buffer, bufferSize + 1);
    free(buffer);

    p = p + bufferSize + 2;

    bufferize(&p[0], buffer, max, ++iterator, 0, 0);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("São necessários 2 argumentos");
    }
    if (argc == 2)
    {
        char *arg1 = argv[1];
        char *p = argv[1];
        char *buffer;
        int max = strlen(arg1);

        char *i = &argv[1][0];

        // while (*i != '\0')
        // {
        //     printf("%c\n", *i);
        //     i++;
        // }

        bufferize(&p[0], buffer, max, 0, 0, 0);
    }
    return 0;
}