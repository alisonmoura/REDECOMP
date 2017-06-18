#include <stdio.h>
#include <netinet/in.h>

/* gives htons and friends */
struct hw3
{
    char x;
    char y;
    short z;
    int w;
};

static void print_buffer(char *buffer, int size)
{
    for (int i = 0; i < size; i++)
    {
        printf("%02x\t", buffer[i] & 0xff);
    }
    printf("\n");
}

static void print_struct(struct hw3 *sp)
{
    printf("sp->x is 0x%02x\t", sp->x);
    printf("sp->y is 0x%02x\t", sp->y);
    printf("sp->z is 0x%02x\t", sp->z);
    printf("sp->w is 0x%02x\t", sp->w);
    printf("\n");
}

int main()
{
    char buffer[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
    print_buffer(buffer, sizeof(buffer));

    struct hw3 *sp = (struct hw3 *)buffer;
    print_struct(sp);
}