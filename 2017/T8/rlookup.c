#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct route
{
    char *gateway;
    char *dest;
    char *mask;
    char *interface;
    int score;
};

struct addr
{
    char *a;
    char *b;
    char *c;
    char *d;
};

static void error(char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

static void transformNumberDot(char *p, char *buffer, int max, int iterator, int bufferSize, int count, struct addr *ret)
{
    int i = 0;

    if (iterator >= max)
    {
        return;
    }

    while (*p != '.')
    {
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
    while (i >= 0)
    {
        buffer[i] = *p;
        i--;
        p--;
    }

    switch (count)
    {
    case 0:
        ret->a = (char *)malloc(sizeof(buffer));
        if (ret->a != NULL)
            strcpy(ret->a, buffer);
        break;
    case 1:
        ret->b = (char *)malloc(sizeof(buffer));
        if (ret->b != NULL)
            strcpy(ret->b, buffer);
        break;
    case 2:
        ret->c = (char *)malloc(sizeof(buffer));
        if (ret->c != NULL)
            strcpy(ret->c, buffer);
        break;
    case 3:
        ret->d = (char *)malloc(sizeof(buffer));
        if (ret->d != NULL)
            strcpy(ret->d, buffer);
        break;
    }

    free(buffer);

    p = p + bufferSize + 2;
    transformNumberDot(&p[0], buffer, max, ++iterator, 0, ++count, ret);
}

static int analyze(struct route *in, struct addr *ip)
{
    char *buffer;
    int score = 0;

    struct addr *gateway = (struct addr *)malloc(sizeof(struct addr));
    transformNumberDot(&in->gateway[0], buffer, strlen(in->gateway), 0, 0, 0, gateway);
    printf("Gateway transformado: %s%s%s%s\n", gateway->a, gateway->b, gateway->c, gateway->d);

    struct addr *mask = (struct addr *)malloc(sizeof(struct addr));
    transformNumberDot(&in->mask[0], buffer, strlen(in->mask), 0, 0, 0, mask);
    printf("Máscara transformada: %s%s%s%s\n", mask->a, mask->b, mask->c, mask->d);

    if ((atoi(gateway->a) & atoi(mask->a)) == atoi(ip->a))
        ++score;
    if ((atoi(gateway->b) & atoi(mask->b)) == atoi(ip->b))
        ++score;
    if ((atoi(gateway->c) & atoi(mask->c)) == atoi(ip->c))
        ++score;

    return score;
}

static void bufferize(char *p, char *buffer, int max, int iterator, int i, int bufferSize, int count, struct route *route)
{
    if (iterator > max)
    {
        return;
    }

    while (*p != '/')
    {
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
    switch (count)
    {
    case 0:
        route->gateway = (char *)malloc(sizeof(buffer));
        if (route->gateway != NULL)
            strcpy(route->gateway, buffer);
        break;
    case 1:
        route->dest = (char *)malloc(sizeof(buffer));
        if (route->dest != NULL)
            strcpy(route->dest, buffer);
        break;
    case 2:
        route->mask = (char *)malloc(sizeof(buffer));
        if (route->mask != NULL)
        {
            strcpy(route->mask, buffer);
        }
        break;
    case 3:
        // route->interface = (char *)malloc(sizeof(buffer));
        // if (route->interface != NULL)
        //     strcpy(route->interface, buffer);
        break;
    }
    free(buffer);

    p = p + bufferSize + 2;

    bufferize(&p[0], buffer, max, ++iterator, 0, 0, ++count, route);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        error("São necessários, ao menos, 2 argumentos");
    }
    if (argc >= 3)
    {
        char *ip = argv[1];
        char *p;
        char *buffer;
        int i, j;
        struct route *routes[argc - 2];

        for (i = 2, j = 0; i < argc; i++, j++)
        {
            struct route *route = (struct route *)malloc(sizeof(route));

            p = argv[i];
            bufferize(&p[0], buffer, strlen(p), 0, 0, 0, 0, route);

            routes[j] = malloc(sizeof(route));
            if (routes[j] != NULL)
                routes[j] = route;
        }

        struct addr *ipNumber = (struct addr *)malloc(sizeof(struct addr));
        transformNumberDot(&ip[0], buffer, strlen(ip), 0, 0, 0, ipNumber);
        struct route *best;

        for (i = 0; i < argc - 2; i++)
        {
            routes[i]->score = analyze(routes[i], ipNumber);
            printf("score: %d\n", routes[i]->score);

            if (i == 0)
                best = routes[i];
            else if (routes[i]->score > best->score)
                best = routes[i];
        }

        printf("forwarding packet for %s to next hop %s over interface eth1\n", ip, best->dest);
    }
    return 0;
}