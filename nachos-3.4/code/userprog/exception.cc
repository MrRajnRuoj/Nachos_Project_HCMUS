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

void increasePC() {
	int counter = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, counter);
	counter = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg, counter);
	machine->WriteRegister(NextPCReg, counter + 4);
}

void createFile_Handler() {
	int virtAddr;
	char* fileName;

	DEBUG('a', "\nSC_Create call ...");
	DEBUG('a', "\nReading virtual address of filename");

	virtAddr = machine->ReadRegister(4);	// đọc tham số tên file

	DEBUG('a', "\nReading filename.");
	fileName = User2System(virtAddr, MaxFileLength);	// Chuyển từ user space sang kernel space
	if (fileName == NULL)
	{
		printf("\nNot enough memory in system");
		DEBUG('a', "\nNot enough memory in system");
		machine->WriteRegister(2, -1); // trả về lỗi cho chương trình người dùng
									   
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
	machine->WriteRegister(2, 0); // trả về cho chương trình người dùng thành công

	delete fileName;
}

void printf_Handler() {
	int virtAddr = machine->ReadRegister(4);
	int oneChar = 0, i = 0;
	char c = 0;

	machine->ReadMem(virtAddr + i++, 1, &oneChar);
	while (oneChar != 0) {
		c = (char)oneChar;
		gSynchConsole->Write(&c, 1);
		machine->ReadMem(virtAddr + i++, 1, &oneChar);
	}

	machine->WriteRegister(2, 0);
}

void openFile_Handler() {
	int virtAddr = machine->ReadRegister(4);
	int type = machine->ReadRegister(5);
	char* fileName = User2System(virtAddr, MaxFileLength);	

	if (fileName == NULL) {
		printf("\nLoi: Khong du bo nho trong he thong!");
		machine->WriteRegister(2, -1); 
	} 
	else if (type < 0 || type > 3) {
		printf("\nLoi: Type khong hop le!");
		machine->WriteRegister(2, -1);
	}
	else if (fileSystem->index < 0 || fileSystem->index > 9) {
		printf("\nLoi: Khong du bo nho quan ly file!");
		machine->WriteRegister(2, -1);
	}
	else {
		if (type == 0 || type == 1) {
			OpenFile* currOpenFile = fileSystem->Open(fileName, type);
			if (currOpenFile != NULL) {
				printf("\nMo file thanh cong!");
				machine->WriteRegister(2, fileSystem->index - 1);
			}
			else {
				printf("\nLoi: File khong ton tai!");
				machine->WriteRegister(2, -1);
			}
		}
		else if (type == 2 || strcpy(fileName, "stdin") == 0) {
			printf("\nStdin mode!");
			machine->WriteRegister(2, 0);
		}
		else if (type == 3 || strcpy(fileName, "stdout") == 0) {
			printf("\nStdout mode!");
			machine->WriteRegister(2, 1);
		}
		else {
			printf("\nLoi: Tao file khong thanh cong.");
			machine->WriteRegister(2, -1);
		}
	}

	delete[] fileName;
}                                                                                                                          
void ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);

	switch (which)
	{
	case NoException:
		break;
	case PageFaultException:
		DEBUG('a' , "\n No valid translation found.");
		printf("\n\n No valid translation found.");
		ASSERT(FALSE);
		break;
	case ReadOnlyException:
		DEBUG('a', "\n Write attempted to page marked: read - only.");
		printf("\n\n Write attempted to page marked: read - only.");
		ASSERT(FALSE);
		break;
	case BusErrorException:
		DEBUG('a', "\n Translation resulted in an invalid physical address.");
		printf("\n\n Translation resulted in an invalid physical address.");
		ASSERT(FALSE);
		break;
	case AddressErrorException:
		DEBUG('a', "\n Unaligned reference or one that was beyond the end of the address space.");
		printf("\n\n Unaligned reference or one that was beyond the end of the address space.");
		ASSERT(FALSE);
		break;
	case OverflowException:
		DEBUG('a', "\n Integer overflow in add or sub.");
		printf("\n\n Integer overflow in add or sub.");
		ASSERT(FALSE);
		break;
	case IllegalInstrException:
		DEBUG('a', "\n Integer overflow in add or sub.");
		printf("\n\n Integer overflow in add or sub.");
		ASSERT(FALSE);
		break;
	case NumExceptionTypes:
		DEBUG('a', "\n Num Exception Types.");
		printf("\n\n Num Exception Types.");
		ASSERT(FALSE);
		break;
	case SyscallException:
		switch (type)
		{
		case SC_Halt:
			DEBUG('a', "\n Shutdown, initiated by user program.");
			printf("\n\n Shutdown, initiated by user program.");
			interrupt->Halt();
			break;
		case SC_Exit:
			DEBUG('a', "\n SC_Exit Exception");
			printf("\n\n SC_Exit Exception\n");
			interrupt->Halt();
			break;
		case SC_Exec:
			DEBUG('a', "\n SC_Exec Exception");
			printf("\n\n SC_Exec Exception");
			interrupt->Halt();
			break;
		case SC_Join:
			DEBUG('a', "\n SC_Join Exception");
			printf("\n\n SC_Join Exception");
			interrupt->Halt();
			break;
		case SC_Create:
			createFile_Handler();
			break;
		case SC_Open:
			openFile_Handler();
			break;
		case SC_Read:
			DEBUG('a', "\n SC_Read Exception");
			printf("\n\n SC_Read Exception");
			interrupt->Halt();
			break;
		case SC_Write:
			DEBUG('a', "\n SC_Write Exception");
			printf("\n\n SC_Write Exception");
			interrupt->Halt();
			break;
		case SC_Close:
			DEBUG('a', "\n SC_Close Exception");
			printf("\n\n SC_Close Exception");
			interrupt->Halt();
			break;
		case SC_Fork:
			DEBUG('a', "\n SC_Fork Exception");
			printf("\n\n SC_Fork Exception");
			interrupt->Halt();
			break;
		case SC_Yield:
			DEBUG('a', "\n SC_Yield Exception");
			printf("\n\n SC_Yield Exception");
			interrupt->Halt();
			break;
		case SC_Printf:
			printf_Handler();
			break;
		default:
			printf("\n Unexpected user mode exception (%d %d)", which, type);
			ASSERT(FALSE);
			break;
		}
		increasePC();
		break;
	default:
		printf("\n Unexpected user mode exception (%d %d)", which, type);
		ASSERT(FALSE);
		break; 
	}
	
}
