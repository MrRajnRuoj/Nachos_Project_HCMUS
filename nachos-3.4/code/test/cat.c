#include "syscall.h"
#define MAXLENGTH 256
int main() {
	char fileName[MAXLENGTH], buffer[MAXLENGTH];
	int fileID;

	Printf("\t\tCHUONG TRINH CAT\n\n");
	Printf("Nhap ten file: ");
	Scanf(fileName, MAXLENGTH);
	fileID = Open(fileName, 0);
	if (fileID < 2 || fileID > 9) {
		return 0;
	}

	Read(buffer, MAXLENGTH, fileID);
	CloseFile(fileID);
	
	Printf("Noi dung:\n");
	Printf(buffer);

	return 0;	
}