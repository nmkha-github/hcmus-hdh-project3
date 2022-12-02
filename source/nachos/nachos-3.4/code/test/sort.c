#include "syscall.h"

int a[110];

int main()
{
    int i, j, tmp, n;

    PrintString("Nhap do dai mang: ");
    n = ReadInt();

    while (n < 0 || n > 100)
    {
        PrintString("Nhap lai do dai mang (0 <= n <= 100): ");
        n = ReadInt();
    }

    for (i = 0; i < n; i++)
    {
        PrintString("a[");
        PrintInt(i);
        PrintString("]= ");
        a[i] = ReadInt();
    }

    PrintString("Mang da nhap: ");
    for (i = 0; i < n; i++)
    {
        PrintInt(a[i]);
        PrintChar(' ');
    }
    PrintChar('\n');

    // sort array
    for (i = 0; i < n; i++)
        for (j = 0; j < i; j++)
            if (a[i] < a[j])
            {
                tmp = a[i];
                a[i] = a[j];
                a[j] = tmp;
            }

    PrintString("Mang sau khi sap xep tang dan: ");
    for (i = 0; i < n; i++)
    {
        PrintInt(a[i]);
        PrintChar(' ');
    }
    PrintChar('\n');

    Halt();
}