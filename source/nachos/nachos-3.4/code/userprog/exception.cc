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
        gSynchConsole->Read(&c, 1);
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
    gSynchConsole->Read(&c, 1);
    while (c <= '9' && c >= '0' && length < maxlen)
    {
        num_string[length] = c;
        length++;
        gSynchConsole->Read(&c, 1);
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
    int n = machine->ReadRegister(4); //?????c s??? ???? ???????c g??n s???n
    const int maxlen = 11;            // gi?? tr??? ????? d??i t???i ??a ki???u int
    char num_string[maxlen] = {0};
    int tmp[maxlen] = {0}, i = 0, j = 0;
    if (n < 0)
    {                          // x??? l?? s??? ??m
        n = -n;                // n???u ??m chuy???n v??? d????ng ????? x??? l??
        num_string[i++] = '-'; // chu???i s??? xu???t ra th??m d???u ??m
    }
    do
    { // chuy???n s??? sang m???ng int ????? d??? thao t??c
        tmp[j++] = n % 10;
        n /= 10;
    } while (n);
    while (j) // chuy???n m???ng int sang char
        num_string[i++] = '0' + (char)tmp[--j];
    gSynchConsole->Write(num_string, i); // ghi ra m??n h??nh char v?? ????? d??i m???ng
    machine->WriteRegister(2, 0);
}

void Exception_ReadChar()
{
    char ch = 0;
    gSynchConsole->Read(&ch, 1); // ?????c 1 k?? t??? t??? console
    machine->WriteRegister(2, (int)ch);
}

void Exception_PrintChar()
{
    char ch = (char)machine->ReadRegister(4); // l???y k?? t??? ???? ???????c g??n s???n
    gSynchConsole->Write(&ch, 1);             // ghi 1 k?? t??? ra m??n h??nh console
    machine->WriteRegister(2, 0);
}

void Exception_ReadString()
{
    int addr, maxLength;
    char *buffer;
    addr = machine->ReadRegister(4);
    maxLength = machine->ReadRegister(5);

    gSynchConsole->Read(buffer, maxLength);

    System2User(addr, maxLength, buffer);
}

void Exception_PrintString()
{
    int addr;
    char *buffer;
    addr = machine->ReadRegister(4);

    buffer = User2System(addr, 255);

    gSynchConsole->Write(buffer, strlen(buffer));
    delete[] buffer;
}

void Exception_CreateFile()
{
    int addr;
    char *fileName;

    addr = machine->ReadRegister(4);

    fileName = User2System(addr, 255);

    if (fileName == NULL)
    {
        printf("\nKhong du bo nho");
        DEBUG('a', "\nKhong du bo nho");
        machine->WriteRegister(2, -1); // N???u l???i th?? ghi v??o thanh ghi s??? 2 gi?? tr??? -1
    }

    if (strlen(fileName) == 0)
    {
        printf("\nTen file khong duoc trong");
        DEBUG('a', "\nTen file khong duoc trong");
        machine->WriteRegister(2, -1); // N???u l???i th?? ghi v??o thanh ghi s??? 2 gi?? tr??? -1
    }

    // Create file with size = 0
    // D??ng ?????i t?????ng fileSystem c???a l???p OpenFile ????? t???o file
    if (fileSystem->Create(fileName, 0) == -1)
    {
        printf("\nXay ra loi khi tao file (%s)", fileName);
        machine->WriteRegister(2, -1);
    }

    machine->WriteRegister(2, 0); // N???u kh??ng c?? l???i ghi v??o thanh ghi s??? 2
    delete[] fileName;            // Gi???i ph??ng v??ng nh??? cho filename
}

void Exception_Open()
{
    int addr;
    int type; 
    char* filename;

    //L???y c??c tham s??? t??? thanh ghi
    addr = machine->ReadRegister(4); 
    type = machine->ReadRegister(5); 
    filename = User2System(addr, 255);

    int freeSlot = fileSystem->FindFreeSlot();
    
    if (freeSlot == -1){
        machine->WriteRegister(2, -1); //Tr??? l???i v??? cho ng?????i d??ng
    }

    switch (type){
    case 0: 
        fileSystem->openFileTable[freeSlot] = fileSystem->Open(filename, type);
        if (fileSystem->openFileTable[freeSlot] != NULL)
        {
            machine->WriteRegister(2, freeSlot); //tr??? v??? v??? tr?? c??n tr???ng
        }
        break;
    case 1:
        fileSystem->openFileTable[freeSlot] = fileSystem->Open(filename, type);
        if (fileSystem->openFileTable[freeSlot] != NULL) //Mo file thanh cong
        {
            machine->WriteRegister(2, freeSlot); //tr??? v??? v??? tr?? c??n tr???ng
        }
        break;
    case 2:
        machine->WriteRegister(2, 0);
        break;
    case 3:
        machine->WriteRegister(2, 1);
        break;
    }
 
    delete[] filename;
}

void Exception_Close()
{
    int fileId;
    fileId = machine->ReadRegister(4); 
    //Ch??? x??? l?? trong ph???m vi b???ng m?? t??? file
    if (fileId >= 0 && fileId <= 10) 
    {
        if (fileSystem->openFileTable[fileId]) 
        {
            machine->WriteRegister(2, 0);   //Tr??? k???t qu??? th??nh c??ng
            delete[] fileSystem->openFileTable[fileId]; 
        }
    }
    machine->WriteRegister(2, -1);  // Tr??? l???i
}

void Exception_Read()
{
    int addr;
    int charCount;
    int id;

    int firstPositionInFile;
    int lastPositionInFile;
    char *buffer;

    //L???y gi?? tr??? tham s??? t??? thanh ghi
    addr = machine->ReadRegister(4);
    charCount = machine->ReadRegister(5); 
    id = machine->ReadRegister(6);  

    // N???u n???m ngo??i b???ng m?? t??? th?? tr??? l???i
    if (id < 0 || id > 14)
    {
        printf("\nFile ID nam ngoai bang mo ta");
        machine->WriteRegister(2, -1);
        return;
    }

    //Ki???m tra file t???n t???i
    if (fileSystem->openFileTable[id] == NULL)
    {
        printf("\nFile khong ton tai.");
        machine->WriteRegister(2, -1);
        return;
    }

    if (fileSystem->openFileTable[id]->type == 3) 
    {
        printf("\nKhong the read file stdout.");
        machine->WriteRegister(2, -1);
        return;
    }

    buffer = User2System(addr, charCount); 
    //file stdin 
    if (fileSystem->openFileTable[id]->type == 2)
    {
        // S??? byte th???c s??? ?????c ???????c
        int size = gSynchConsole->Read(buffer, charCount); 
        System2User(addr, size, buffer); 
        machine->WriteRegister(2, size); 
        delete[] buffer;
        return;
    }

    firstPositionInFile = fileSystem->openFileTable[id]->currentPos();
    fileSystem->openFileTable[id]->Read(buffer, charCount);
    lastPositionInFile = fileSystem->openFileTable[id]->currentPos();

    //S??? byte th???c s??? = lastPositionInFile - firstPositionInFile
    int size = lastPositionInFile - firstPositionInFile;

    //Tr?????ng h???p file kh??c r???ng
    if (size > 0){
        System2User(addr, size, buffer); 
        machine->WriteRegister(2, size);
        delete[] buffer;
        return;
    }

    //Tr?????ng h???p c??n l???i file null tr??? v??? -2
    machine->WriteRegister(2, -2); 
    delete[] buffer;
}

void Exception_Write()
{
    int addr;
    int charCount;
    int id;

    int firstPositionInFile;
    int lastPositionInFile;
    char *buffer;

    //L???y gi?? tr??? tham s??? t??? thanh ghi
    addr = machine->ReadRegister(4);
    charCount = machine->ReadRegister(5); 
    id = machine->ReadRegister(6);  

    // N???u n???m ngo??i b???ng m?? t??? th?? tr??? l???i
    if (id < 0 || id > 14)
    {
        printf("\nFile ID nam ngoai bang mo ta");
        machine->WriteRegister(2, -1);
        return;
    }

    //Ki???m tra file t???n t???i
    if (fileSystem->openFileTable[id] == NULL)
    {
        printf("\nFile khong ton tai.");
        machine->WriteRegister(2, -1);
        return;
    }

    if (fileSystem->openFileTable[id]->type == 1 || fileSystem->openFileTable[id]->type == 2) 
    {
        printf("\nKhong the viet file stdin hoac file chi doc.");
        machine->WriteRegister(2, -1);
        return;
    }

    firstPositionInFile = fileSystem->openFileTable[id]->currentPos();
    fileSystem->openFileTable[id]->Write(buffer, charCount);
    lastPositionInFile = fileSystem->openFileTable[id]->currentPos();

    //S??? byte th???c s??? = lastPositionInFile - firstPositionInFile
    int size = lastPositionInFile - firstPositionInFile;

    //X??t v???i file ch??? ?????c v?? vi???t th?? tr??? v??? s??? byte th???t s???
    if (fileSystem->openFileTable[id]->type == 0)
    {
        if (size > 0)
        {
            machine->WriteRegister(2, size);   //Tr??? v??? s??? byte th???t s???
            delete[] buffer;
            return;
        }
    }

    //V???i file stdout
    if (fileSystem->openFileTable[id]->type == 3) 
    {
        int i;
        for (i = 0; buffer[i] != '\0'; i++)
        {
            gSynchConsole->Write(buffer + i, 1);
        }
        machine->WriteRegister(2, i - 1); // Tra ve so byte thuc su write duoc
        delete[] buffer;
    }
}

void Exception_Exec()
{
    int addr;
    addr = machine->ReadRegister(4);	// doc dia chi ten chuong trinh tu thanh ghi r4
    char* name;
    name = User2System(addr, 255); // Lay ten chuong trinh, nap vao kernel

    if (name == NULL)
    {
        DEBUG('a', "\nKhong du bo nho");
        printf("\nKhong du bo nho");
        machine->WriteRegister(2, -1);
        return;
    }

    OpenFile *openFile = fileSystem->Open(name);
    if (openFile == NULL)
    {
        printf("\n Khong mo duoc file chuong trinh.");
        machine->WriteRegister(2,-1);
        return;
    }

    delete openFile;

    // id c???a ti???u tr??nh
    int id = pTab->ExecUpdate(name); 
    machine->WriteRegister(2, id);

    delete[] name;
}

void Exception_Join()
{

}

void Exception_Exit()
{
    
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
        case SC_CreateFile:
            Exception_CreateFile();
            Increase_ProgramCounter();
            return;
        case SC_Open:
            Exception_Open();
            Increase_ProgramCounter();
            return;
        case SC_Close:
            Exception_Close();
            Increase_ProgramCounter();
            return;
        case SC_Exec:
            Exception_Exec();
            Increase_ProgramCounter();
            return;
        case SC_Join:
            Exception_Join();
            Increase_ProgramCounter();
            return;
        case SC_Exit:
            Exception_Exit();
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
