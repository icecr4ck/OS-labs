#include <sys/types.h>

struct procdata {
	long state;
	pid_t pid;
	pid_t parent_pid;
	uid_t uid;
	char comm[16];
};
