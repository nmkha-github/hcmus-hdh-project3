#include "syscall.h"

int main()
{
    int number;
    PrintString("Nhap so nguyen: ");
    number = ReadInt();
    PrintString("So nguyen da nhap: ");
    PrintInt(number);
    Halt();
}