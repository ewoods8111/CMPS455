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

#include <stdio.h>        // FA98
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "addrspace.h"   // FA98
#include "sysdep.h"   // FA98

// begin FA98

static int SRead(int addr, int size, int id);
static void SWrite(char *buffer, int size, int id);
static int SExec(char* filename);
static void SExit(int status);
void processCreator(int arg);

// end FA98

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

//Find what calls exception handler, see what gets raised.

void
ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);

	int arg1 = machine->ReadRegister(4);
	int arg2 = machine->ReadRegister(5);
	int arg3 = machine->ReadRegister(6);
	int Result;
	int i, j;
	char *ch = new char [500];
	int value;


	switch ( which )
	{
	
	case NoException :
		break;
	case SyscallException :

		// for debugging, in case we are jumping into lala-land
		// Advance program counters.
		
		machine->registers[PrevPCReg] = machine->registers[PCReg];
		machine->registers[PCReg] = machine->registers[NextPCReg];
		machine->registers[NextPCReg] = machine->registers[NextPCReg] + 4;
		
		
		switch ( type )
		{

		case SC_Halt :{
			DEBUG('t', "Shutdown, initiated by user program.\n");
			printf("HALTING\n");			
			interrupt->Halt();
			break;
}
			
		case SC_Read :
			if (arg2 <= 0 || arg3 < 0){
				printf("\nRead 0 byte.\n");
			}
			Result = SRead(arg1, arg2, arg3);
			machine->WriteRegister(2, Result);
			DEBUG('t',"Read %d bytes from the open file(OpenFileId is %d)",
			arg2, arg3);
			break;

		case SC_Write :
			for (j = 0; ; j++) {
				if(!machine->ReadMem((arg1+j), 1, &i))
					j=j-1;
				else{
					ch[j] = (char) i;
					if (ch[j] == '\0') 
						break;
				}
			}
			if (j == 0){
				printf("\nWrite 0 byte.\n");
				 SExit(1);
			} else {
				DEBUG('t', "\nWrite %d bytes from %s to the open file(OpenFileId is %d).", arg2, ch, arg3);
				SWrite(ch, j, arg3);
				machine->WriteRegister(2, 0);
			}
			break;

		case SC_Exec:
		{
		        IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts		
				/* write code for handling Exec */
				/* ----------------------------*/
				/* be careful to on and off interrupts at proper places */
				/* ----------------------------*/
				/* return the process id for the newly created process, return value
				is to write at R2 */

				printf("Process %d calls exec() system call\n", currentThread->getId());

				int fileAddr = arg1;		//Read register 4 to get address of filename
				char *filename = new char[100]; 
				
				if(!machine->ReadMem(fileAddr,1,&value)) {
					machine->WriteRegister(2, 0);
					return;
				} 
					
				i=0;
				while( value!=0 )
				{

					filename[i]=(char)value;
					fileAddr+=1;
					i++;
					if(!machine->ReadMem(fileAddr,1,&value)) {
						machine->WriteRegister(2, 0);
						return;
					}

				}
				filename[i]=(char)0;
				printf("File %s has been found\n", filename);
				int processId = SExec(filename);
				
				
				delete [] filename;
				
				machine->WriteRegister(2, processId);
				(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
				break;
		}
        case SC_Join:
        {
        	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts		
        	printf("Process %d joining on process %d.\n", currentThread->getId(), arg1);
			
			//arg1 will be the threadId for the exec'd function.
			//Current thread will become the parent
			//We need a child thread to be the alias of the actual thread
			//Set off interrupts so this is atomic
			Thread *child = PCB->GetThreadAt(arg1);
			
			child->parent = currentThread;
			currentThread->numberOfChildren++;
			child->isAChild = true;
			
			//Go to sleep until the child wakes it up
			currentThread->SaveUserState();
			currentThread->space->SaveState();
			currentThread->Sleep();
			
			printf("Process %s now has parent %s \n", child->getName(), child->parent->getName());
			machine->WriteRegister(2, 0);
			(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
            break;
		}
        case SC_Yield:
        	printf("Process %d is Yielding\n", currentThread->getId());
        	machine->WriteRegister(2, 0);
			currentThread->Yield();
            break;
        case SC_Exit:
				SExit(arg1);
            break;
            
			default :
			//Unprogrammed system calls end up here
				printf("\n\nUnprogrammed system call made. Nothing happens.\n\n");
				SExit(0);
			break;
		}         
		break;

	case ReadOnlyException :
		puts ("ReadOnlyException");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		SExit(1);
		break;
	case BusErrorException :
		puts ("BusErrorException");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		SExit(1);
		break;
	case AddressErrorException :
		puts ("Pointer out of bounds");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		SExit(1);
		break;
	case OverflowException :
		puts ("OverflowException");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		SExit(1);
		break;
	case IllegalInstrException :
		puts ("IllegalInstrException");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		SExit(1);
		break;
	case NumExceptionTypes :
		puts ("NumExceptionTypes");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		SExit(1);
		break;

		default :
		      printf("Unexpected user mode exception %d %d\n", which, type);
		      if (currentThread->getName() == "main")
		      ASSERT(FALSE);
		      SExit(1);
		break;
	}
	delete [] ch;

}


static int SRead(int addr, int size, int id)  //input 0  output 1
{
	char buffer[size+10];
	int num,Result;

	//read from keyboard, try writing your own code using console class.
	if (id == 0)
	{
		scanf("%s",buffer);

		num=strlen(buffer);
		if(num>(size+1)) {

			buffer[size+1] = '\0';
			Result = size+1;
		}
		else {
			buffer[num+1]='\0';
			Result = num + 1;
		}

		for (num=0; num<Result; num++)
		{  machine->WriteMem((addr+num), 1, (int) buffer[num]);
			if (buffer[num] == '\0')
			break; }
		return num;

	}
	//read from a unix file, later you need change to nachos file system.
	else
	{
		for(num=0;num<size;num++){
			Read(id,&buffer[num],1);
			machine->WriteMem((addr+num), 1, (int) buffer[num]);
			if(buffer[num]=='\0') break;
		}
		return num;
	}
}



static void SWrite(char *buffer, int size, int id)
{
	//write to terminal, try writting your own code using console class.
	if (id == 1)
	printf("%s", buffer);
	//write to a unix file, later you need change to nachos file system.
	if (id >= 2)
	WriteFile(id,buffer,size);
}

static int SExec(char* filename)
{
	OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
		printf("Unable to open file %s\n", filename);
		return -1;
    }
    
    printf("Reading %s \n", filename);
    Thread * t = new Thread("Exec-ed Thread");
    space = new AddrSpace(executable);  
    printf("Process %d is loading.\n", t->getId());
    //Make a new thread for the process and give it the space.
    t->space = space; 
    int processId = t->getId();
    printf("Process %d finishes the exec() system call\n", currentThread->getId());

	//Add the execed thread to the PCB
	printf("Trying to add process %d to PCB\n", processId);
	PCB->Append(t);

    delete executable;			// close file
    t->Fork(processCreator,0);
    
    return processId;
}

static void SExit(int status)
{

	//Check if we have a parent, wake it up.
	if(currentThread->parent != NULL){
		Thread *parent = currentThread->parent;
		parent->numberOfChildren--;
		parent->RestoreUserState();
		parent->space->RestoreState();
		scheduler->ReadyToRun(parent);	
	}

	//Call sever ties.  Will search the PCB to find threads with this as its parent, and set its parent to NULL
	if(currentThread->numberOfChildren > 0){
	printf("Thread %s with number children: %d \n",currentThread->getName(), currentThread->numberOfChildren);
		PCB->SeverTies(currentThread->getId());
	}

	
	if(status == 0){
		printf("Exited normally (Code 0).\n");
		machine->WriteRegister(2, 0);
	}
	else if(status == 1){
		printf("Exited abnormally (Code 1).\n");
		machine->WriteRegister(2, 1);
	}
	else
		printf("Exited with code %d.\n", status);
		machine->WriteRegister(2, status);
		
	delete (currentThread->space);
	PCB->RemoveThread(currentThread->getId());
	currentThread->Finish();
}

void processCreator(int arg)
{
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState(); 	// load page table register
	machine->Run(); 	// jump to the user progam
	ASSERT(FALSE); 		// machine->Run never returns;
}

// end FA98

