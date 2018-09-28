#include <sys/syscall.h>
#include <stdio.h>
#include "procdata.h"

#define sys_log710h2018as2 356

long getprocdata(struct procdata *p){
	return (long) syscall(sys_log710h2018as2, p);
}

int main(){
	struct procdata p;
	if (!getprocdata(&p)){
		printf("Etat du processus: %lu\n", p.state);
		printf("PID: %zu\n", p.pid);
		printf("PID du parent: %zu\n", p.parent_pid);
		printf("UID: %zu\n", p.uid);
		printf("Nom du programme: %s\n", p.comm);
	}
	else{
		printf("Erreur de l'appel systeme\n");
	}
	return 0;
}
