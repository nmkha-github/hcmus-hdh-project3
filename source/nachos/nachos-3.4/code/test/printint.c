#include "syscall.h"

int main()
{
    int number = 999999;
    PrintString("Test print int");
    PrintInt(number);
    Halt();
}