#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <asm-generic/current.h>
#include <asm/errno.h>

asmlinkage long sys_log710h2018as2(struct procdata *upd) {
	struct task_struct *p = current;
	struct procdata kpd;
	kpd.state = p->state;
	kpd.pid = p->pid;
	kpd.parent_pid = p->real_parent->pid;
	kpd.uid = p->cred->uid.val;
	strncpy(kpd.comm, p->comm, 16);
	if (copy_to_user(upd, &kpd, sizeof kpd))
		return -EFAULT;
	return 0;
}
