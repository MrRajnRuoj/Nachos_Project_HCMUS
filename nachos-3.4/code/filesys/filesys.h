// filesys.h 
//	Data structures to represent the Nachos file system.
//
//	A file system is a set of files stored on disk, organized
//	into directories.  Operations on the file system have to
//	do with "naming" -- creating, opening, and deleting files,
//	given a textual file name.  Operations on an individual
//	"open" file (read, write, close) are to be found in the OpenFile
//	class (openfile.h).
//
//	We define two separate implementations of the file system. 
//	The "STUB" version just re-defines the Nachos file system 
//	operations as operations on the native UNIX file system on the machine
//	running the Nachos simulation.  This is provided in case the
//	multiprogramming and virtual memory assignments (which make use
//	of the file system) are done before the file system assignment.
//
//	The other version is a "real" file system, built on top of 
//	a disk simulator.  The disk is simulated using the native UNIX 
//	file system (in a file named "DISK"). 
//
//	In the "real" implementation, there are two key data structures used 
//	in the file system.  There is a single "root" directory, listing
//	all of the files in the file system; unlike UNIX, the baseline
//	system does not provide a hierarchical directory structure.  
//	In addition, there is a bitmap for allocating
//	disk sectors.  Both the root directory and the bitmap are themselves
//	stored as files in the Nachos file system -- this causes an interesting
//	bootstrap problem when the simulated disk is initialized. 
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef FS_H
#define FS_H

#include "copyright.h"
#include "openfile.h"

#ifdef FILESYS_STUB 		// Temporarily implement file system calls as 
							// calls to UNIX, until the real file system
							// implementation is available
class FileSystem {
public:
	OpenFile** openFileTable;	// Bang mo ta file

	FileSystem(bool format) {
		/* Khoi tao bang mo ta file va them vao 2 phan tu dau 
		 * cho inputconsole va output console
		 */
		this->openFileTable = new OpenFile*[10];

		for (int i = 0; i < 10; ++i) {
			this->openFileTable[i] = NULL;
		}

		// Khoi tao bang OpenFile voi 2 vi tri dau cho stdin va stdout
		this->Create("stdin", 0);
		this->Create("stdout", 0);
		this->openFileTable[0] = this->Open("stdin", 2);
		this->openFileTable[1] = this->Open("stdout", 3);
	}

	~FileSystem() {
		// Xoa bang mo ta file

		for (int i = 0; i < 10; ++i) {
			delete openFileTable[i];
			openFileTable[i] = NULL;
		}
		delete[] openFileTable;
	}

	bool Create(char *name, int initialSize) {
		int fileDescriptor = OpenForWrite(name);

		if (fileDescriptor == -1) return FALSE;
		Close(fileDescriptor);
		return TRUE;
	}

	OpenFile* Open(char *name) {
		int fileDescriptor = OpenForReadWrite(name, FALSE);

		if (fileDescriptor == -1) return NULL;
		return new OpenFile(fileDescriptor);
	}

	OpenFile* Open(char* name, int type) {
		int fileDescriptor = OpenForReadWrite(name, FALSE);
		if (fileDescriptor == -1) return NULL;

		return new OpenFile(fileDescriptor, type);
	}

	bool Remove(char *name) { return Unlink(name) == 0; }

	// Ham them OpenFile vao bang mo ta file
	int AddToTable(OpenFile* openFile) {
		int result = -1;
		for (int i = 2; i < 10; i++) {
			if (openFileTable[i] == NULL) {
				openFileTable[i] = openFile;
				result = i;
				break;
			}
		}
			
		return result;
	}

	// Ham xoa file dang mo khoi bang mo ta file
	void DelFromTable(int fID) {
		if (fID > 1 && fID < 10) {
			if (openFileTable[fID] != NULL) {
				delete openFileTable[fID];
				openFileTable[fID] = NULL;
			}
		}
	}
};

#else // FILESYS
class FileSystem {
public:
	OpenFile** openFileTable;		// Bang mo ta file

	FileSystem(bool format);		// Initialize the file system.
									// Must be called *after* "synchDisk" 
									// has been initialized.
									// If "format", there is nothing on
									// the disk, so initialize the directory
									// and the bitmap of free blocks.

	bool Create(char *name, int initialSize);	// Create a file (UNIX creat)

	OpenFile* Open(char *name); 			// Open a file (UNIX open)

	OpenFile* Open(char *name, int type);	// Mo file voi type tuong ung

	bool Remove(char *name);  				// Delete a file (UNIX unlink)

	void List();							// List all the files in the file system

	void Print();							// List all the files and their contents

	int AddToTable(OpenFile* openFile);		// Ham them OpenFile vao bang mo ta file

	void DelFromTable(int fID);				// Ham xoa file dang mo khoi bang mo ta file

private:
	OpenFile* freeMapFile;		// Bit map of free disk blocks,
								// represented as a file
	OpenFile* directoryFile;	// "Root" directory -- list of 
								// file names, represented as a file
};

#endif // FILESYS

#endif // FS_H
