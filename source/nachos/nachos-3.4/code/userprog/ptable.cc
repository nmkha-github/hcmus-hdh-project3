#include "ptable.h"
#include "system.h"
#include "openfile.h"

PTable::PTable(int size)
{

  if (size < 0)
    return;

  psize = size;
  bm = new BitMap(size);
  bmsem = new Semaphore("bmsem", 1);

  for (int i = 0; i<MAX_PROCESS; i++){
		pcb[i] = 0;
  }

	bm->Mark(0);

	pcb[0] = new PCB(0);
	pcb[0]->SetFileName("./test/scheduler");
	pcb[0]->parentID = -1;
}

PTable::~PTable()
{
    if( bm != 0 )
	delete bm;
    
    for (int i = 0; i<psize; i++){
		if(pcb[i] != 0)
			delete pcb[i];
    }
		
	if( bmsem != 0)
		delete bmsem;
}

int PTable::ExecUpdate(char* name)
{
  //Gọi mutex->P(); để giúp tránh tình trạng nạp 2 tiến trình cùng 1 lúc.
	bmsem->P();
	
	if(name == NULL)
	{
		printf("\nTen chuong trinh bi NULL\n");
		bmsem->V();
		return -1;
	}

	// So sánh tên chương trình và tên của currentThread để chắc chắn rằng chương trình này không gọi thực thi chính nó.
	if( strcmp(name,"./test/scheduler") == 0 || strcmp(name,currentThread->getName()) == 0 )
	{
		printf("\nChuong trinh dang thuc thi chinh no\n");		
		bmsem->V();
		return -1;
	}

	// Tìm slot trống trong bảng Ptable.
	int index = this->GetFreeSlot();

    // Check if have free slot
	if(index < 0)
	{
		printf("\nKhong con slot trong\n");
		bmsem->V();
		return -1;
	}

	//Nếu có slot trống thì khởi tạo một PCB mới với processID chính là index của slot này
	pcb[index] = new PCB(index);
	pcb[index]->SetFileName(name);

	// parrentID là processID của currentThread
    	pcb[index]->parentID = currentThread->processID;

	
	// Gọi thực thi phương thức Exec của lớp PCB.
	int pid = pcb[index]->Exec(name,index);

	// Gọi bmsem->V()
	bmsem->V();
	// Trả về kết quả thực thi của PCB->Exec.
	return pid;
}

int PTable::JoinUpdate(int id)
{
	if(id < 0)
	{
		return -1;
	}

	// Kiểm tra tiến trình đang chạy có phải là tiến trình cha của tiến trình tham gia không
	if(currentThread->processID != pcb[id]->parentID)
	{
		return -1;
	}

    	// Tăng numwait và gọi JoinWait() để chờ tiến trình con thực hiện.
	// Sau khi tiến trình con thực hiện xong, tiến trình đã được giải phóng
	pcb[pcb[id]->parentID]->IncNumWait();
	
	pcb[id]->JoinWait();

	// Xử lý exitcode.	
	int ec = pcb[id]->GetExitCode();
        // ExitRelease() để cho phép tiến trình con thoát.
	pcb[id]->ExitRelease();

    // Successfully
	return ec;
}
int PTable::ExitUpdate(int exitcode)
{              
    // Nếu tiến trình gọi là main process thì gọi Halt().
	int id = currentThread->processID;
	if(id == 0)
	{
		
		currentThread->FreeSpace();		
		interrupt->Halt();
		return 0;
	}
    
  if(IsExist(id) == false)
	{
		printf("\nPTable::ExitUpdate: This %d is not exist. Try again?", id);
		return -1;
	}

	// Ngược lại gọi SetExitCode để đặt exitcode cho tiến trình gọi.
	pcb[id]->SetExitCode(exitcode);
	pcb[pcb[id]->parentID]->DecNumWait();
    
    // Gọi JoinRelease để giải phóng tiến trình cha đang đợi nó(nếu có) 
    //và ExitWait() để xin tiến trình cha cho phép thoát.
	pcb[id]->JoinRelease();
    // 
	pcb[id]->ExitWait();
	
	Remove(id);
	return exitcode;
}

// Find free slot in order to save the new process infom
int PTable::GetFreeSlot()
{
	return bm->Find();
}

// Check if Process ID is Exist
bool PTable::IsExist(int pid)
{
	return bm->Test(pid);
}

// Remove proccess ID out of table
// When it ends
void PTable::Remove(int pid)
{
	bm->Clear(pid);
	if(pcb[pid] != 0)
		delete pcb[pid];
}

char* PTable::GetFileName(int id)
{
	return (pcb[id]->GetFileName());
}
