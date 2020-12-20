#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int* a;

void seconde();
void chgmt_var();
int temperature();

int main()
{
    chgmt_var();
    printf("\n%d", a);
    return 0;
}

void chgmt_var()
{
    for (int i=0;i<=4;i++)
    {
        seconde();
        printf("%d",temperature(i));
        a = i;
    }
}

int temperature(int val)
{
    return val;
}

void seconde ()
{
    Sleep(500);
}
