/*
	kmod, the new module loader (replaces kerneld)
	Kirk Petersen
*/

#define __KERNEL_SYSCALLS__

#include <linux/sched.h>
#include <linux/types.h>
#include <linux/unistd.h>

/*
	kmod_unload_delay and modprobe_path are set via /proc/sys.
*/
int kmod_unload_delay = 60;
char modprobe_path[256] = "/sbin/modprobe";
static char module_name[64] = "";
static char * argv[] = { modprobe_path, "-s", "-k", module_name, NULL };
static char * envp[] = { "HOME=/", "TERM=linux", "PATH=/usr/bin:/bin", NULL };

/*
	kmod_queue synchronizes the kmod thread and the rest of the system
	kmod_unload_timer is what we use to unload modules
	after kmod_unload_delay seconds
*/
static struct wait_queue * kmod_queue = NULL;
static struct timer_list kmod_unload_timer;

/*
	It is not easy to implement a full fork in kernel-space on some
	systems (Alpha), and it is not necessary for us here.  This is 
	a new thread that does the exec.
*/
static int kmod_exec_modprobe(void * data)
{
	sigemptyset(&current->blocked);
	execve(modprobe_path, argv, envp);
	printk(KERN_ERR "kmod: failed to load module %s\n", module_name);
	return 0;
}

/*
	kmod_thread is the thread that does most of the work.  kmod_unload and
	request_module tell it to wake up and do work.
*/
static int kmod_thread(void * data)
{
	int pid;

	/*
		Initialize basic thread information
	*/
	current->session = 1;
	current->pgrp = 1;
	sprintf(current->comm, "kmod");
	sigfillset(&current->blocked);

	/*
		This is the main kmod_thread loop.  It first sleeps, then
		handles requests from request_module or kmod_unload.
	*/

	while (1) {
		interruptible_sleep_on(&kmod_queue);

		/*
			If request_module woke us up, we should try to
			load module_name.  If not, kmod_unload woke us up,
			do call delete_module.
			(if somehow both want us to do something, ignore the
			 delete_module request)
		*/
		if (module_name[0] == '\0') {
			delete_module(NULL);
		} else {
			pid = kernel_thread(kmod_exec_modprobe, NULL, SIGCHLD);
			if (pid > 0) {
				waitpid(pid, NULL, 0);
				module_name[0] = '\0';
				wake_up(&kmod_queue);
			} else {
				printk(KERN_ERR "kmod: fork failed, errno %d\n", -pid);
			}
		}
	}

	return 0;	/* Never reached. */
}

/*
	kmod_unload is the function that the kernel calls when
	the kmod_unload_timer expires
*/
void kmod_unload(unsigned long x)
{
	/*
		wake up the kmod thread, which does the work
		(we can't call delete_module, as it locks the kernel and
		 we are in the bottom half of the kernel (right?))
		once it is awake, reset the timer
	*/
	wake_up(&kmod_queue);
	kmod_unload_timer.expires = jiffies + (kmod_unload_delay * HZ);
	add_timer(&kmod_unload_timer);
}

int kmod_init(void)
{
	printk("Starting kmod\n");

	/*
	 * CLONE_FS means that our "cwd" will follow that of init.
	 * CLONE_FILES just saves some space (we don't need any
	 * new file descriptors). Ditto for CLONE_SIGHAND.
	 */
	kernel_thread(kmod_thread, NULL, CLONE_FILES | CLONE_FS | CLONE_SIGHAND);

	kmod_unload_timer.next = NULL;
	kmod_unload_timer.prev = NULL;
	kmod_unload_timer.expires = jiffies + (5 * 60 * HZ);
	kmod_unload_timer.data = 0L;
	kmod_unload_timer.function = kmod_unload;
	add_timer(&kmod_unload_timer);

	return 0;
}

/*
	request_module, the function that everyone calls when they need a
	module to be loaded
*/
int request_module(const char * name)
{
	/* first, copy the name of the module into module_name */
	/* then wake_up() the kmod daemon */
	/* wait for the kmod daemon to finish (it will wake us up) */

	/*
		kmod_thread is sleeping, so start by copying the name of
		the module into module_name.  Once that is done, wake up
		kmod_thread.
	*/
	strncpy(module_name, name, sizeof(module_name));
	module_name[sizeof(module_name)-1] = '\0';
	wake_up(&kmod_queue);

	/*
		Now that we have told kmod_thread what to do, we want to
		go to sleep and let it do its work.  It will wake us up,
		at which point we will be done (the module will be loaded).
	*/
	interruptible_sleep_on(&kmod_queue);
	return 0;
}
