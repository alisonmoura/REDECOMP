#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    int day, month, year;
} Date;

int main()
{
    Date *date;
    printf( "O tamanho é: %i\n", sizeof(date));
}