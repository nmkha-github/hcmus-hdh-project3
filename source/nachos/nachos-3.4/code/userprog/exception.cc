// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

// Input: - User space address (int)
// - Limit of buffer (int)
// Output:- Buffer (char*)
// Purpose: Copy buffer from User memory space to System memory space
char *User2System(int virtAddr, int limit)
{
    int i; // index
    int oneChar;
    char *kernelBuf = NULL;
    kernelBuf = new char[limit + 1]; // need for terminal string
    if (kernelBuf == NULL)
        return kernelBuf;
    memset(kernelBuf, 0, limit + 1);
    // printf("\n Filename u2s:");
    for (i = 0; i < limit; i++)
    {
        machine->ReadMem(virtAddr + i, 1, &oneChar);
        kernelBuf[i] = (char)oneChar;
        // printf("%c",kernelBuf[i]);
        if (oneChar == 0)
            break;
    }
    return kernelBuf;
}

// Input: - User space address (int)
// - Limit of buffer (int)
// - Buffer (char[])
// Output:- Number of bytes copied (int)
// Purpose: Copy buffer from System memory space to User memory space
int System2User(int virtAddr, int len, char *buffer)
{
    if (len < 0)
        return -1;
    if (len == 0)
        return len;
    int i = 0;
    int oneChar = 0;
    do
    {
        oneChar = (int)buffer[i];
        machine->WriteMem(virtAddr + i, 1, oneChar);
        i++;
    } while (i < len && oneChar != 0);
    return i;
}

// Di chuyen thanh ghi ve sau 4 byte de nap lenh ke tiep
// Cac buoc:
// B1: Ghi vao PrevPcReg <- gia tri PCReg hien tai
// B2: Ghi vao PCReg hien tai <- gia tri NextPCReg
// B3: Di chuyen thanh ghi ve sau 4 byte (ghi vao NextPCReg <- gia tri NextPCReg + 4)
void Increase_ProgramCounter()
{
    /*
    machine/mipssim.cc cuoi ham "void Machine::OneInstruction(Instruction *instr)"

     // Advance program counters.
        registers[PrevPCReg] = registers[PCReg];	// for debugging, in case we
                            // are jumping into lala-land
        registers[PCReg] = registers[NextPCReg];
        registers[NextPCReg] = pcAfter;
    */
    int pcAfter = machine->ReadRegister(NextPCReg) + 4;
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, pcAfter);
}

void Exception_ReadInt()
{

    const int maxlen = 11; // max len(int) = 11 do int: [-2147483648 , 2147483647]
    char num_string[maxlen] = {0};
    long long ret = 0;
    char c = ' ';
    // remove front space
    while (c == ' ')
    {
        ptrSynchConsole->Read(&c, 1);
    }
    int length = 0;
    // check sign
    bool sign = 0;
    if (c == '-')
    {
        sign = 1;
    }
    else if (c == '+')
    {
        sign = 0;
    }
    else if (c >= '0' && c <= '9')
    {
        num_string[length] = c;
        length++;
    }

    // read full number
    ptrSynchConsole->Read(&c, 1);
    while (c <= '9' && c >= '0' && length < maxlen)
    {
        num_string[length] = c;
        length++;
        ptrSynchConsole->Read(&c, 1);
    }

    for (int i = 0; i < length; i++)
    {
        ret = ret * 10 + 1ll * (num_string[i] - '0');
    }
    if (ret > 2147483647)
    {
        ret = 0;
    }
    ret = sign ? (-ret) : ret;

    machine->WriteRegister(2, (int)ret);
}

void Exception_Halt()
{
    DEBUG('a', "\n Shutdown, initiated by user program.");
    printf("\n");
    interrupt->Halt();
}

void Exception_PrintInt()
{
    int n = machine->ReadRegister(4); //đọc số đã được gán sẵn
    const int maxlen = 11;            // giá trị độ dài tối đa kiểu int
    char num_string[maxlen] = {0};
    int tmp[maxlen] = {0}, i = 0, j = 0;
    if (n < 0)
    {                          // xử lý số âm
        n = -n;                // nếu âm chuyển về dương để xử lý
        num_string[i++] = '-'; // chuỗi số xuất ra thêm dấu âm
    }
    do
    { // chuyển số sang mảng int để dễ thao tác
        tmp[j++] = n % 10;
        n /= 10;
    } while (n);
    while (j) // chuyển mảng int sang char
        num_string[i++] = '0' + (char)tmp[--j];
    ptrSynchConsole->Write(num_string, i); // ghi ra màn hình char và độ dài mảng
    machine->WriteRegister(2, 0);
}

void Exception_ReadChar()
{
    char ch = 0;
    ptrSynchConsole->Read(&ch, 1); // đọc 1 ký tự từ console
    machine->WriteRegister(2, (int)ch);
}

void Exception_PrintChar()
{
    char ch = (char)machine->ReadRegister(4); // lấy ký tự đã được gán sẵn
    ptrSynchConsole->Write(&ch, 1);           // ghi 1 ký tự ra màn hình console
    machine->WriteRegister(2, 0);
}

void Exception_ReadString()
{
    int addr, maxLength;
    char *buffer;
    addr = machine->ReadRegister(4);
    maxLength = machine->ReadRegister(5);

    ptrSynchConsole->Read(buffer, maxLength);

    System2User(addr, maxLength, buffer);
}

void Exception_PrintString()
{
    int addr;
    char *buffer;
    addr = machine->ReadRegister(4);

    buffer = User2System(addr, 255);

    ptrSynchConsole->Write(buffer, strlen(buffer));
    delete[] buffer;
}

void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    switch (which)
    {
    case NoException:
        return;
    case SyscallException:
        switch (type)
        {
        case SC_Halt:
            Exception_Halt();
            return;
        // code mau cong hai so
        case SC_Sub:
            int op1, op2, result;
            op1 = machine->ReadRegister(4);
            op2 = machine->ReadRegister(5);
            result = op1 - op2;
            machine->WriteRegister(2, result);
            interrupt->Halt();
            return;
        case SC_ReadInt:
            Exception_ReadInt();
            Increase_ProgramCounter();
            return;
        case SC_PrintInt:
            Exception_PrintInt();
            Increase_ProgramCounter();
            return;
        case SC_ReadChar:
            Exception_ReadChar();
            Increase_ProgramCounter();
            return;
        case SC_PrintChar:
            Exception_PrintChar();
            Increase_ProgramCounter();
            return;
        case SC_ReadString:
            Exception_ReadString();
            Increase_ProgramCounter();
            return;
        case SC_PrintString:
            Exception_PrintString();
            Increase_ProgramCounter();
            return;
        default:
            printf("\n Unexpected user mode exception (%d %d)", which,
                   type);
            interrupt->Halt();
            return;
        }
        return;

    case PageFaultException:
        DEBUG('a', "\nKhong tim thay dinh vi du lieu trong RAM. ");
        printf("\n\nKhong tim thay dinh vi du lieu trong RAM. ");
        interrupt->Halt();
        return;
    case ReadOnlyException:
        DEBUG('a', "\nTrang dang co gang ghi vao duoc danh dau la 'chi doc'.");
        printf("\n\nTrang dang co gang ghi vao duoc danh dau la 'chi doc'.");
        interrupt->Halt();
        return;
    case BusErrorException:
        DEBUG('a', "\nChuong trinh dang co truy cap bo nho khong the xac dinh dia chi vat ly.");
        printf("\n\nChuong trinh dang co truy cap bo nho khong the xac dinh dia chi vat ly.");
        interrupt->Halt();
        return;
    case AddressErrorException:
        DEBUG('a', "\nChuong trinh dang tham chieu den mot khong gian dia chi bat hop phap.");
        printf("\n\nChuong trinh dang tham chieu den mot khong gian dia chi bat hop phap.");
        interrupt->Halt();
        return;
    case OverflowException:
        DEBUG('a', "\nLoi tran so do phep tinh cong hoac tru.");
        printf("\n\nLoi tran so do phep tinh cong hoac tru.");
        interrupt->Halt();
        return;
    case IllegalInstrException:
        DEBUG('a', "\nLenh dang co gang thuc thi khong duoc ho tro.");
        printf("\n\nLenh dang co gang thuc thi khong duoc ho tro.");
        interrupt->Halt();
        return;
    case NumExceptionTypes:
        DEBUG('a', "\nChuong trinh gap loi ngoai le so.");
        printf("\n\nChuong trinh gap loi ngoai le so.");
        interrupt->Halt();
        return;
    default:
        DEBUG('a', "\nChuong trinh gap loi ngoai le khong mong muon.");
        printf("\n\nChuong trinh gap loi ngoai le khong mong muon.");
        return;
    }
}
