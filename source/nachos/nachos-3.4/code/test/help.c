#include "syscall.h"

int main()
{
  PrintString("------------------------------Thong tin nhom-----------------------------\n");
  PrintString("-Thanh vien 1:  Nguyen Minh Kha \t 20120502\n");
  PrintString("-Thanh vien 2:  Nguyen Quoc Anh \t 19120450\n");
  PrintString("-Thanh vien 3:  Le Dao Thao Tien \t 20120591\n");
  PrintString("-Thanh vien 4:  Phan Dinh Anh Quan \t 20120635\n");
  PrintString("-Thanh vien 5:  Chau Nhat Tuyet \t 20120622\n");

  PrintString("----------------------------Mo ta chuong trinh----------------------------\n");
  PrintString("-Chuong trinh sort: sap xep mang tang dan input tu user.\n");
  PrintString("-Chuong trinh ascii: in ra 256 ma ascii dau tien va ki tu tuong ung.\n");
  Halt();
}