#include "syscall.h"

int main()
{
  int i;

  PrintString("------------------------Bang ma Ascii--------------------------\n");
  PrintString("Dec\tKy tu tuong ung\t\t\tDec\tKy tu tuong ung\n");

  for (i = 0; i < 128; i++)
  {
    PrintInt(i);
    PrintString("\t    ");
    if (i == 0)
    {
      PrintString("NULL");
    }
    else if (i == 9)
    {
      PrintString("\\t");
    }
    else if (i == 10)
    {
      PrintString("\\n");
    }
    else if (i == 11)
    {
      PrintString("VT");
    }
    else if (i == 12)
    {
      PrintString("FF");
    }
    else if (i == 13)
    {
      PrintString("SO");
    }
    else
    {
      PrintChar(i);
    }
    if (i == 0)
    {
      PrintString("\t\t|\t");
    }
    else
    {
      PrintString("\t\t\t|\t");
    }
    PrintInt(i + 128);
    PrintString("\t    ");
    PrintChar(i + 128);
    PrintChar('\n');
  }

  Halt();
}