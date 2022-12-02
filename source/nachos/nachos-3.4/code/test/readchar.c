#include "syscall.h"

int main()
{
    char ch;
    PrintString("Nhap ki tu: ");
    ch = ReadChar();
    PrintString("Ky tu da nhap: ");
    PrintChar(ch);
    Halt();
}