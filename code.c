#include <stdio.h>

int *uninit();

int main()
{
    printf("%p\n", uninit());
    *uninit() = 10;
    uninit();
    return 0;
}

int *uninit()
{
    int m;
    printf("%d\n", m);
    int *tmp = &m;
    return &m;
}