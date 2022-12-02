#include "syscall.h"

int main()
{
  char *s;
  int maxLength;
  PrintString("Nhap do dai toi da cua xau: ");
  maxLength = ReadInt();
  PrintString("Nhap xau: ");
  ReadString(s, maxLength);
  PrintString("Xau da nhap: ");
  PrintString(s);
  Halt();
}