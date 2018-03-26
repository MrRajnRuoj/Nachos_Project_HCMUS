#include "syscall.h"
#define MAXLENGTH 256

int main() {
	char sourceName[MAXLENGTH], destName[MAXLENGTH], buffer[MAXLENGTH];
	int sourceID, destID;

	Printf("\t\tCHUONG TRINH COPY\n\n");
	Printf("Nhap ten file nguon: ");
	Scanf(sourceName, MAXLENGTH);
	Printf("Nhap ten file dich: ");
	Scanf(destName, MAXLENGTH);
	
	sourceID = Open(sourceName, 0);
	if (sourceID < 2 || sourceID > 9)
		return 0;
	destID = Open(destName, 0);
	if (destID < 2 || destID > 9)
		return 0;

	Read(buffer, MAXLENGTH, sourceID);
	Write(buffer, MAXLENGTH, destID);

	CloseFile(sourceID);
	CloseFile(destID);
	
	Printf("Copy hoan tat!");
	return 0;
}
