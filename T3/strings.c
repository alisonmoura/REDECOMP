#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void error(char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

static void bufferize(char *p, char *buffer, int max, int iterator)
{
    if (iterator > max)
    {
        printf("Acabou\n");
        return;
    }

    int i = 0;
    int bufferSize = 0;

    while (*p != '/')
    {
        bufferSize++;
        p++;
    }

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
    printf("%s (%d)\n", buffer, bufferSize);
    free(buffer);

    p = p + bufferSize + 1;

    bufferize(&p[0], buffer, max, ++iterator);
    return;
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
        char *p = &arg1[0];
        char *buffer;
        char *s = "/";
        int max = strlen(arg1);

        char *token;

        token = strtok(arg1, s);

        while (token != NULL)
        {
            printf(" %s\n", token);
            token = strtok(NULL, s);
        }
    }
    return 0;
}