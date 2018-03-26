#include "syscall.h"
#define MAXLENGTH 256

int main() {
	char buffer[MAXLENGTH];

	Printf("\t\tCHUONG TRINH ECHO!");
	Printf("\n\n");
	Printf("Nhap chuoi: ");
	Scanf(buffer, MAXLENGTH );
	Printf("Chuoi vua nhap: ");
	Printf(buffer);
		
	return 0;
}