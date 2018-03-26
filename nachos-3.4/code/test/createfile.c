#include "syscall.h"
#include "copyright.h"

int main() {
	char filename[256];
	char* defaultFileName = "NewFile.txt";
	int result;

	Printf("Nhap ten file (Enter):");
	Scanf(filename, 256);
	if (filename[0] != 0) {
		result = Create(filename);
	}
	else {
		result = Create(defaultFileName);
	}
	
	if(result == -1) {
		Printf("Loi: Tao file khong thanh cong!");
	}
	else {
		Printf("Tao file thanh cong!");
	}

	return 0;
}