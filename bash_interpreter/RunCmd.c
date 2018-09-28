#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[], char *envp[]){
	if (argc > 1){
		int i,ret,status;
		char **new_argv = malloc(sizeof(char*)*(argc-1));
		for (i=0;i<argc;i++){
			new_argv[i] = argv[i+1];
		}
		pid_t child_pid = fork();
		struct rusage rs;
		struct timeval start,end,wc,cpu_time;
		gettimeofday(&start,0);
		switch (child_pid) {
			case -1:
				printf("Error on fork() call");
				exit(1);
				break;
			case 0:
				ret = execvp(new_argv[0], new_argv);
				if (ret < 0){
					printf("Error: command not found");
					exit(1);
				}
				break;
			default:
				ret = waitpid(child_pid,&status,0);
				if (ret < 0){
					printf("Error on wait() call");
					exit(1);
				}
				gettimeofday(&end,0);
				timersub(&end,&start,&wc);
				ret = getrusage(RUSAGE_CHILDREN,&rs);
				if (ret == 0){
					printf("Wall-clock time: %.3f ms\n", (float)wc.tv_usec / 1000);
					timeradd(&rs.ru_utime, &rs.ru_stime, &cpu_time);
					printf("CPU time: %.3f ms\n", (float)cpu_time.tv_usec / 1000);
					printf("Interruptions involontaires: %ld\n", rs.ru_nivcsw);
					printf("Interruptions volontaires: %ld\n", rs.ru_nvcsw);
					printf("Défauts de page: %ld\n", rs.ru_majflt);
					printf("Défauts de page (satisfaits par cache): %ld\n", rs.ru_minflt);
				}
				break;
		}
	}
	return 0;
}
