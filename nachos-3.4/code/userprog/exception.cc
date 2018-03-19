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

#define MaxFileLength 32

void increasePC() {
	int counter = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, counter);
	counter = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg, counter);
	machine->WriteRegister(NextPCReg, counter + 4);
}

void
ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);

	//if ((which == SyscallException) && (type == SC_Halt)) {
	//	DEBUG('a', "Shutdown, initiated by user program.\n");
	//	interrupt->Halt();
	//}
	//else {
	//	printf("Unexpected user mode exception %d %d\n", which, type);
	//	ASSERT(FALSE);
	//}

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
			printf("\n\n SC_Exit Exception");
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
		{
			int virtAddr;
			char* fileName;
			
			DEBUG('a', "\n SC_Create call ...");
			DEBUG('a', "\n Reading virtual address of filename");

			virtAddr = machine->ReadRegister(4);

			DEBUG('a', "\n Reading filename.");
			fileName = User2System(virtAddr, MaxFileLength);
			if (fileName == NULL)
			{
				printf("\n Not enough memory in system");
				DEBUG('a', "\n Not enough memory in system");
				machine->WriteRegister(2, -1); // trả về lỗi cho chương
											   // trình người dùng
				delete fileName;
				return;
			}
			DEBUG('a', "\n Finish reading filename.");

			if (!fileSystem->Create(fileName, 0))
			{
				printf("\n Error create file '%s'", fileName);
				machine->WriteRegister(2, -1);
				delete fileName;
				return;
			}
			machine->WriteRegister(2, 0); // trả về cho chương trình
										  // người dùng thành công
			
			delete fileName;
			break;
		}
		case SC_Open:
			DEBUG('a', "\n SC_Open Exception");
			printf("\n\n SC_Open Exception");
			interrupt->Halt();
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

/*
* Input:
*	- User space address (int)
*	- Limit of buffer (int)
* Output:
*	- Buffer (char*)
* Purpose: Copy buffer from User memory space to System memory space
*/

char* User2System(int virtAddr, int limit)
{
	int i;	// index
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit + 1];	// need for terminal string
	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf, 0, limit + 1);

	for (i = 0; i < limit; i++)
	{
		machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;

		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}


/*
* Input:
*	- User space address (int)
*	- Limit of buffer (int)
*	- Buffer (char[])
* Output:
*	- Number of bytes copied (int)
* Purpose: Copy buffer from System memory space to User memory space
*/

int System2User(int virtAddr, int len, char* buffer)
{
	if (len < 0) return -1;
	if (len == 0)return len;
	int i = 0;
	int oneChar = 0;
	do {
		oneChar = (int)buffer[i];
		machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}