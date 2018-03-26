#include "syscall.h"

int main() {
	char buffer[258];

	int fid = Open("text.txt", 2);
	int result = Seek(3, fid);
	if (result < 0) {
	return 0;
	}
	Read(buffer, 5, fid);
	Printf(buffer);
	CloseFile(fid);
	
	return 0;
}