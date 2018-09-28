#include <sys/syscall.h>
#include <stdio.h>

#define sys_log710h2018as1 355

long foncTestAS1(void){
	return (long) syscall(sys_log710h2018as1);
}

int main(){
	printf("Code de retour du syscall: %d\n", foncTestAS1());
	return 0;
}
