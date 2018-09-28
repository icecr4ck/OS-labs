#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

struct bg_task{
	int num;
	int pid;
	char *cmd;
	int active;
};

int nb_tasks=0;
int nb_tasks_active=0;
struct bg_task tasks[100];

int sig_caught=0;

void sig_handler(int s){
	printf("\nUtilisez la commande exit pour terminer le programme.\n");
	sig_caught=1;
}

int split_cmd(char *cmd, char **res){
	int i=0;
	int is_task = 0;
	char *token, *pos, *str;
	if ((pos=strchr(cmd, '\n')) != NULL)
		*pos = '\0';
	str = strdup(cmd);
	while ((token = strsep(&str," ")) != NULL){
		if (strcmp(token,"&") == 0){
			is_task = 1;
			res[i] = NULL;
		}
		else{
			res[i] = token;
		}
		i++;
	}
	res[i] = NULL;
	return is_task;
}

void exec_cmd(char **cmd, int is_task){
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
			if (!is_task){
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
			}
			else {
				tasks[nb_tasks].num = nb_tasks+1;
				tasks[nb_tasks].pid = child_pid;
				tasks[nb_tasks].cmd = malloc(strlen(cmd[0])+1);
				strcpy(tasks[nb_tasks].cmd, cmd[0]);
				tasks[nb_tasks].active = 1;
				printf("[%d] %d\n", tasks[nb_tasks].num, tasks[nb_tasks].pid);
				nb_tasks++;
				nb_tasks_active++;
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
		printf("Nouveau répertoire: %s", full_path);
	}
}

void check_tasks(){
	int status, i=0, ret=0;
	for (i=0;i<nb_tasks;i++){
		if (tasks[i].active){
			if (waitpid(tasks[i].pid, &status, WNOHANG)){
				tasks[i].active = 0;
				free(tasks[i].cmd);
				nb_tasks_active--;
			}
		}
	}
}

void show_tasks(){
	int i=0;
	for (i=0;i<nb_tasks;i++){
		if (tasks[i].active){
			printf("[%d] %d %s\n", tasks[i].num, tasks[i].pid, tasks[i].cmd);
		}
	}
}

int main(int argc, char *argv[], char *envp[]){
	char buf[100];
	int exit=0;
	int is_task=0;
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = sig_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	while(!exit){
		check_tasks();
		printf("%s", "Log710H2018%>");
		fgets(buf, sizeof(buf), stdin);
		if (strcmp(buf,"\n") != 0 && !sig_caught){
			char *cmd[100];
			is_task = split_cmd(buf, cmd);
			if (strcmp(cmd[0],"cd") == 0){
				change_dir(cmd);
			}
			else if (strcmp(cmd[0],"exit") == 0){
				if (nb_tasks_active){
					printf("Il reste encore des tâches non finies en arrière-plan.\n");
				}
				else {
					exit = 1;
				}
			}
			else if (strcmp(cmd[0],"aptaches") == 0){
				show_tasks();
			}
			else{
				exec_cmd(cmd, is_task);
			}
		}
		sig_caught=0;
	}
	return 0;
}
