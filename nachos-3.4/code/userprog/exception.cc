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

#define MaxFileLength 256

void increasePC() {	// tang Program Counter
	int counter = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, counter);
	counter = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg, counter);
	machine->WriteRegister(NextPCReg, counter + 4);
}

/* int Create(char *name)
 * Input:
 *	- name: ten file can tao
 * Output:
 *	- int: ket qua, 0: tao file thanh cong, -1: tao file that bai
 */
void createFile_Handler() {
	int virtAddr;
	char* fileName;

	DEBUG('a', "\nSC_Create call ...");
	DEBUG('a', "\nReading virtual address of filename");

	virtAddr = machine->ReadRegister(4);	// doc tham so ten file

	DEBUG('a', "\nReading filename.");
	fileName = User2System(virtAddr, MaxFileLength);	// Chuyen tu user space sang kernel space
	if (fileName == NULL)
	{
		printf("\nNot enough memory in system");
		DEBUG('a', "\nNot enough memory in system");
		machine->WriteRegister(2, -1); // tra ve loi cho chuon trinh nguoi dung

		delete fileName;
		return;
	}
	DEBUG('a', "\nFinish reading filename.");

	if (!fileSystem->Create(fileName, 0))
	{
		printf("\nError create file '%s'", fileName);
		machine->WriteRegister(2, -1);
		delete fileName;
		return;
	}
	machine->WriteRegister(2, 0); // Tra ve thanh cong

	delete fileName;
}


/* void Printf(char* buffer)
 * Input: 
 *	- buffer: chuoi can in ra console
 * Output: void
 */
void printf_Handler() {
	int virtAddr = machine->ReadRegister(4);	// Doc tham so luu chuoi buffer can in
	char* buff = User2System(virtAddr, 256);

	if (buff == NULL) {
		return;
	}
	gSynchConsole->Write(buff, strlen(buff));	// Ghi ra console chuoi buffer

	delete[] buff;
}

/* void Scanf(char* buffer, int length)
 * Input: 
 *	- buffer: luu chuoi doc duoc tu console
 *	- length: do dai toi da co the doc duoc
 * Output: void
 */
void scanf_Handler() {
	int buffAddr = machine->ReadRegister(4);	
	int length = machine->ReadRegister(5);		
	char* buff;

	if (length < 0) {
		return;
	}

	// Khoi tao chuoi rong
	buff = new char[length];
	if (buff == NULL) {
		return;
	}
	memset(buff, 0, length);

	gSynchConsole->Read(buff, length);		// Doc chuoi tu console
	System2User(buffAddr, length, buff);	// Luu chuoi sang user space

	delete[] buff;
}

/* OpenFileID Open(char *name, int type)
 * Input:
 *	- name: ten file can mo
 *	- type: loai chuc nang, 0: doc va ghi, 1: chi doc, 2: input console, 3: output console
 * Output: 
 *	- OpenFileID: vi tri trong bang mo ta file
 */
void openFile_Handler() {
	int virtAddr = machine->ReadRegister(4);		// Doc tham so ten file
	int type = machine->ReadRegister(5);			// Doc type
	char* fileName = User2System(virtAddr, MaxFileLength);	// Doc ten file

	if (fileName == NULL) {
		printf("\nLoi: Khong du bo nho trong he thong!");
		machine->WriteRegister(2, -1);
	}
	else if (type < 0 || type > 3) {
		printf("\nLoi: Type khong hop le!");
		machine->WriteRegister(2, -1);
	}
	else {
		if (type == 0 || type == 1) {	// Xu ly mo file binh thuong de doc hoac ghi
			OpenFile* openFile = fileSystem->Open(fileName, type);
			if (openFile != NULL) {
				int fID = fileSystem->AddToTable(openFile);	// Them vao bang mo ta file
				machine->WriteRegister(2, fID);
			}
			else {
				printf("\nLoi: File khong ton tai!");
				machine->WriteRegister(2, -1);
			}
		}
		else if (type == 2 || strcpy(fileName, "stdin") == 0) {	// Xu ly cho input console
			machine->WriteRegister(2, 0);
		}
		else if (type == 3 || strcpy(fileName, "stdout") == 0) {	// Xy ly cho output console
			machine->WriteRegister(2, 1);
		}
		else {
			printf("\nLoi OpenFile: Mo file khong thanh cong.");
			machine->WriteRegister(2, -1);
		}
	}

	delete[] fileName;
}

/* void CloseFile(OpenFileID id)
 * Input:
 *	- id: id cua file (vi tri trong bang mo ta file)
 * Output: void
 */
void closeFile_Handler() {
	int fileID = machine->ReadRegister(4);

	if (fileID < 0 || fileID > 9) {
		machine->WriteRegister(2, -1);
	}
	else {
		fileSystem->DelFromTable(fileID);	// Xoa khoi bang mo ta file
		machine->WriteRegister(2, 0);
	}
}

/* int Read(char *buffer, int charcount, OpenFileID id)
 * Input:
 *	- buffer: chuoi doc duoc tu file
 *	- charcount: so byte toi da co the doc duoc
 *	- id: id cua file (vi tri trong bang mo ta file)
 * Output: 
 *	- int: so byte da doc thanh cong
 */
void readFile_Handler() {
	int virtAddr = machine->ReadRegister(4);
	int charcount = machine->ReadRegister(5);
	int fID = machine->ReadRegister(6);
	char* buffer = new char[charcount];

	if (charcount < 1) {
		printf("\nLoi: Doc it nhat 1 ki tu!");
		machine->WriteRegister(2, -1);
	}
	else if (fID < 0 || fID > 9 || fileSystem->openFileTable[fID] == NULL) {
		printf("\nLoi: File khong ton tai!");
		machine->WriteRegister(2, -1);
	}
	else if (fID == 1) {	// stdout
		printf("\nLoi: Khong the doc \"Console Output\"!");
		machine->WriteRegister(2, -1);
	}
	else {
		int size;
		if (fID == 0) {	// Doc qua console input
			size = gSynchConsole->Read(buffer, charcount);
			System2User(virtAddr, size, buffer);
			if (size == charcount) {
				machine->WriteRegister(2, -2);
			}
			else {
				machine->WriteRegister(2, size);
			}
		}
		else {	// Doc file
			size = fileSystem->openFileTable[fID]->Read(buffer, charcount);	// Doc file va luu vao buffer
			int fileLength = fileSystem->openFileTable[fID]->Length();
			System2User(virtAddr, size, buffer);		// Chuyen buffer tu system space sang user space
			if (size == fileLength) {	// Doc den cuoi file
				machine->WriteRegister(2, -2);
			}
			else {
				machine->WriteRegister(2, size);
			}
		}
	}

	delete[] buffer;
}

/* int Write(char *buffer, int charcount, OpenFileID id)
 * Input:
 *	- buffer: chuoi duoc ghi vao file
 *	- charcount: so byte toi da co the ghi duoc
 *	- id: id cua file (vi tri trong bang mo ta file)
 * Output:
 *	- int: so byte da ghi thanh cong
 */
void writeFile_Handler() {
	int virtAddr = machine->ReadRegister(4);
	int charcount = machine->ReadRegister(5);
	int fID = machine->ReadRegister(6);
	char* buffer;

	if (charcount < 1) {
		printf("\nLoi: Ghi it nhat 1 ki tu!");
		machine->WriteRegister(2, -1);
	}
	else if (fID < 0 || fID > 9 || fileSystem->openFileTable[fID] == NULL) {
		printf("\nLoi: File khong ton tai!");
		machine->WriteRegister(2, -1);
	}
	else if (fID == 0) {	// stdin mode
		printf("\nLoi: Khong the ghi \"Console Input\"!");
		machine->WriteRegister(2, -1);
	}
	else if (fileSystem->openFileTable[fID]->type == 1 || fileSystem->openFileTable[fID]->type == 2) {
		printf("\nLoi: Khong the ghi vao file chi doc!");
		machine->WriteRegister(2, -1);
	}
	else {
		int size;
		buffer = User2System(virtAddr, charcount);
		if (fID == 1) {	// Ghi vao console
			size = gSynchConsole->Write(buffer, charcount);
			if (size == charcount) {
				machine->WriteRegister(2, -2);
			}
			else {	
				machine->WriteRegister(2, size);
			}
		}
		else { // Ghi file
			size = fileSystem->openFileTable[fID]->Write(buffer, charcount);
			int buffLength = strlen(buffer);

			if (size == buffLength) {	// Ghi het chuoi buffer vao file
				printf("\nSize: %d", size);
				machine->WriteRegister(2, -2);
			}
			else {
				machine->WriteRegister(2, size);
			}
		}
	}

	delete[] buffer;
}

/* int Seek(int pos, OpenFileID id)
 * Input:
 *	- pos: vi tri chuyen con tro file 
 *	- id: id cua file (vi tri trong bang mo ta file)
 * Output:
 *	- int: vi tri da chuyen den
 */
void seek_Handler(){
	int pos = machine->ReadRegister(4);
	int fID = machine->ReadRegister(5);

	if (fID < 0 || fID > 9 || fileSystem->openFileTable[fID] == NULL) {
		printf("\nLoi SeekFile: File khong ton tai!");
		machine->WriteRegister(2, -1);
	}
	else if (fID == 0 || fID == 1) {
		printf("\nLoi SeekFile: Khong the Seek tren Console!");
		machine->WriteRegister(2, -1);
	}
	else if (pos < 0 || pos > fileSystem->openFileTable[fID]->Length()) {
		machine->WriteRegister(2, -1);
	}
	else {
		if (pos == -1) {	// Chuyen den cuoi file
			pos = fileSystem->openFileTable[fID]->Length();
		}
		fileSystem->openFileTable[fID]->Seek(pos);
		machine->WriteRegister(2, pos);
	}
}

void ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);

	switch (which)
	{
	case NoException:
		break;
	case PageFaultException:
		printf("\n\nNo valid translation found.(%d, %d)", which, type);
		ASSERT(FALSE);
		break;
	case ReadOnlyException:
		printf("\n\nWrite attempted to page marked: read - only.(%d, %d)", which, type);
		ASSERT(FALSE);
		break;
	case BusErrorException:
		printf("\n\nTranslation resulted in an invalid physical address.(%d, %d)", which, type);
		ASSERT(FALSE);
		break;
	case AddressErrorException:
		printf("\n\nUnaligned reference or one that was beyond the end of the address space.(%d, %d)", which, type);
		ASSERT(FALSE);
		break;
	case OverflowException:
		printf("\n\nInteger overflow in add or sub.(%d, %d)", which, type);
		ASSERT(FALSE);
		break;
	case IllegalInstrException:
		printf("\n\nUnimplemented or reserved instr. (%d, %d)", which, type);
		ASSERT(FALSE);
		break;
	case NumExceptionTypes:
		printf("\n\nNum Exception Types.(%d, %d)", which, type);
		ASSERT(FALSE);
		break;
	case SyscallException:
		switch (type)
		{
		case SC_Halt:
			printf("\n\nShutdown, initiated by user program.");
			interrupt->Halt();
			break;
		case SC_Exit:
			printf("\n\nSC_Exit Exception\n");
			interrupt->Halt();
			break;
		case SC_Exec:
			printf("\n\nSC_Exec Exception");
			interrupt->Halt();
			break;
		case SC_Join:
			printf("\n\nSC_Join Exception");
			interrupt->Halt();
			break;
		case SC_Create:
			createFile_Handler();
			break;
		case SC_Open:
			openFile_Handler();
			break;
		case SC_Read:
			readFile_Handler();
			break;
		case SC_Write:
			writeFile_Handler();
			break;
		case SC_Close:
			closeFile_Handler();
			break;
		case SC_Fork:
			printf("\n\nSC_Fork Exception");
			interrupt->Halt();
			break;
		case SC_Yield:
			printf("\n\nSC_Yield Exception");
			interrupt->Halt();
			break;
		case SC_Printf:
			printf_Handler();
			break;
		case SC_Scanf:
			scanf_Handler();
			break;
		case SC_Seek:
			seek_Handler();
			break;
		default:
			printf("\nUnexpected user mode exception (%d %d)", which, type);
			ASSERT(FALSE);
			break;
		}
		
		increasePC();
		break;
	default:
		printf("\nUnexpected user mode exception (%d %d)", which, type);
		ASSERT(FALSE);
		break;
	}

}
