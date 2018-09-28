#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

void split_cmd(char *cmd, char **res){
	int i=0;
	char *token, *pos, *str;
	if ((pos=strchr(cmd, '\n')) != NULL)
		*pos = '\0'; // Remplace le caractere de nouvelle ligne par le caractère NULL
	str = strdup(cmd);
	while ((token = strsep(&str," ")) != NULL){
		res[i] = token;
		i++;
	}
	res[i] = NULL;
}

void exec_cmd(char **cmd){
	int ret,status;
	pid_t child_pid = fork();
	struct timeval start,end,wc,cpu_time;
	struct rusage prev_rs;
	getrusage(RUSAGE_CHILDREN,&prev_rs);	
	struct rusage rs;
	gettimeofday(&start,0);
	switch (child_pid) {
		case -1:
			printf("Error on fork() call\n");
			exit(1);
			break;
		case 0:
			ret = execvp(cmd[0], cmd);
			if (ret < 0){
				printf("Error: command not found (%d)\n", errno);
				exit(1);
			}
			break;
		default:
			ret = waitpid(child_pid,&status,0);
			if (ret < 0){
				printf("Error on wait() call\n");
				exit(1);
			}
			gettimeofday(&end,0);
			timersub(&end,&start,&wc);
			ret = getrusage(RUSAGE_CHILDREN,&rs);
			if (ret == 0){
				printf("Wall-clock time: %.3f ms\n", (float)wc.tv_usec / 1000);
				timersub(&rs.ru_utime, &prev_rs.ru_utime, &rs.ru_utime);
				timersub(&rs.ru_stime, &prev_rs.ru_stime, &rs.ru_stime);
				timeradd(&rs.ru_utime, &rs.ru_stime, &cpu_time);
				printf("CPU time: %.3f ms\n", (float)cpu_time.tv_usec / 1000);
				printf("Interruptions involontaires: %ld\n", rs.ru_nivcsw-prev_rs.ru_nivcsw);
				printf("Interruptions volontaires: %ld\n", rs.ru_nvcsw-prev_rs.ru_nvcsw);
				printf("Défauts de page: %ld\n", rs.ru_majflt-prev_rs.ru_majflt);
				printf("Défauts de page (satisfaits par cache): %ld\n", rs.ru_minflt-prev_rs.ru_minflt);
			}
			break;
	}
}

void change_dir(char **cmd){
	char full_path[100];
	if (chdir(cmd[1]) < 0){
		printf("Error: path not valid\n");
		exit(1);
	}
	else{
		getcwd(full_path, sizeof(full_path));
		printf("Nouveau répertoire: %s\n", full_path);
	}
}

int main(int argc, char *argv[], char *envp[]){
	char buf[100];
	int exit=0;
	while(!exit){
		printf("%s", "Log710H2018%>");
		fgets(buf, 100, stdin);
		if (strcmp(buf,"\n") != 0){
			char *cmd[100];
			split_cmd(buf, cmd);
			if (strcmp(cmd[0],"cd") == 0){
				change_dir(cmd);
			}
			else if (strcmp(cmd[0],"exit") == 0){
				exit = 1;
			}
			else{
				exec_cmd(cmd);
			}
		}
	}
	return 0;
}
