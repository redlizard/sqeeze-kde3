/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999-2000 Hans Petter Bieker<bieker@kde.org>
	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <config.h>

#include <ctype.h>
#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#if defined(__DragonFly__)
#include <sys/user.h>
#include <sys/resourcevar.h>
#endif

#if __FreeBSD_version > 500015
#include <sys/priority.h>
#endif
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/user.h>
#include <unistd.h>
#include <signal.h>

#include "../../gui/SignalIDs.h"
#include "Command.h"
#include "ProcessList.h"
#include "ccont.h"
#include "ksysguardd.h"

CONTAINER ProcessList = 0;

int fscale;

#define BUFSIZE 1024

typedef struct
{
	/* This flag is set for all found processes at the beginning of the
	 * process list update. Processes that do not have this flag set will
	 * be assumed dead and removed from the list. The flag is cleared after
	 * each list update. */
	int alive;

	/* the process ID */
	pid_t pid;

	/* the parent process ID */
	pid_t ppid;

	/* the real user ID */
	uid_t uid;

	/* the real group ID */
	gid_t gid;

	/* a character description of the process status */
	char status[16];

	/* the number of the tty the process owns */
	int ttyNo;

	/*
	 * The nice level. The range should be -20 to 20. I'm not sure
	 * whether this is true for all platforms.
	 */
	int niceLevel;

	/*
	 * The scheduling priority.
	 */
	int priority;

	/*
	 * The total amount of memory the process uses. This includes shared and
	 * swapped memory.
	 */
	unsigned int vmSize;

	/*
	 * The amount of physical memory the process currently uses.
	 */
	unsigned int vmRss;

	/*
	 * The amount of memory (shared/swapped/etc) the process shares with
	 * other processes.
	 */
	unsigned int vmLib;

	/*
	 * The number of 1/100 of a second the process has spend in user space.
	 * If a machine has an uptime of 1 1/2 years or longer this is not a
	 * good idea. I never thought that the stability of UNIX could get me
	 * into trouble! ;)
	 */
#if !defined(__DragonFly__)
	unsigned int userTime;
#else
        long userTime;
#endif

	/*
	 * The number of 1/100 of a second the process has spend in system space.
	 * If a machine has an uptime of 1 1/2 years or longer this is not a
	 * good idea. I never thought that the stability of UNIX could get me
	 * into trouble! ;)
	 */
	unsigned int sysTime;

	/* system time as multime of 100ms */
	int centStamp;

	/* the current CPU load (in %) from user space */
	double userLoad;

	/* the current CPU load (in %) from system space */
	double sysLoad;

	/* the name of the process */
	char name[64];

	/* the command used to start the process */
	char cmdline[256];

	/* the login name of the user that owns this process */
 	char userName[32];
} ProcessInfo;

static unsigned ProcessCount;

static int
processCmp(void* p1, void* p2)
{
	return (((ProcessInfo*) p1)->pid - ((ProcessInfo*) p2)->pid);
}

static ProcessInfo*
findProcessInList(int pid)
{
	ProcessInfo key;
	long index;

	key.pid = pid;
	if ((index = search_ctnr(ProcessList, processCmp, &key)) < 0)
		return (0);

	return (get_ctnr(ProcessList, index));
}

static int
updateProcess(int pid)
{
	static char *statuses[] = { "idle","run","sleep","stop","zombie" };
	
	ProcessInfo* ps;
	struct passwd* pwent;
	int mib[4];
	struct kinfo_proc p;
	struct rusage pru;
	size_t len;
	size_t buflen = 256;
	char buf[256];

	if ((ps = findProcessInList(pid)) == 0)
	{
		ps = (ProcessInfo*) malloc(sizeof(ProcessInfo));
		ps->pid = pid;
		ps->centStamp = 0;
		push_ctnr(ProcessList, ps);
		bsort_ctnr(ProcessList, processCmp);
	}

	ps->alive = 1;

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PID;
	mib[3] = pid;

	len = sizeof (p);
	if (sysctl(mib, 4, &p, &len, NULL, 0) == -1 || !len)
		return -1;

#if __FreeBSD_version >= 500015
        ps->pid       = p.ki_pid;
        ps->ppid      = p.ki_ppid;
        ps->uid       = p.ki_uid;    
        ps->gid       = p.ki_pgid;
        ps->priority  = p.ki_pri.pri_user;
        ps->niceLevel = p.ki_nice;
#elif defined(__DragonFly__) && __DragonFly_version >= 190000
	ps->pid       = p.kp_pid;
	ps->ppid      = p.kp_ppid;
	ps->uid       = p.kp_uid;
	ps->gid       = p.kp_pgid;
	ps->priority  = p.kp_lwp.kl_tdprio;
#else
        ps->pid       = p.kp_proc.p_pid;
        ps->ppid      = p.kp_eproc.e_ppid;
        ps->uid       = p.kp_eproc.e_ucred.cr_uid;
        ps->gid       = p.kp_eproc.e_pgid;
#if defined(__DragonFly__)
	ps->priority  = p.kp_thread.td_pri;
#else
        ps->priority  = p.kp_proc.p_priority;
#endif
        ps->niceLevel = p.kp_proc.p_nice;
#endif

        /* this isn't usertime -- it's total time (??) */
#if __FreeBSD_version >= 500015
        ps->userTime = p.ki_runtime / 10000;
#elif defined(__DragonFly__)
#if __DragonFly_version >= 190000
	if (!getrusage(p.kp_pid, &pru))
#else
	if (!getrusage(p.kp_proc.p_pid, &pru))
#endif
	{
		errx(1, "failed to get rusage info");
	}
	ps->userTime = pru.ru_utime.tv_usec / 1000; /*p_runtime / 1000*/ 
#elif __FreeBSD_version >= 300000
        ps->userTime = p.kp_proc.p_runtime / 10000;
#else
	ps->userTime = p.kp_proc.p_rtime.tv_sec*100+p.kp_proc.p_rtime.tv_usec/100;
#endif
        ps->sysTime  = 0;
        ps->sysLoad  = 0;

        /* memory, process name, process uid */
	/* find out user name with process uid */
	pwent = getpwuid(ps->uid);
	strncpy(ps->userName,pwent&&pwent->pw_name? pwent->pw_name:"????",sizeof(ps->userName));
	ps->userName[sizeof(ps->userName)-1]='\0';

	if (fscale == 0)
		ps->userLoad = 0;
	else
#if __FreeBSD_version >= 500015
		ps->userLoad = 100.0 * (double) p.ki_pctcpu / fscale;
	ps->vmSize   = p.ki_size;
	ps->vmRss    = p.ki_rssize * getpagesize();
	strlcpy(ps->name,p.ki_comm? p.ki_comm:"????",sizeof(ps->name));
	strcpy(ps->status,(p.ki_stat>=1)&&(p.ki_stat<=5)? statuses[p.ki_stat-1]:"????");
#elif defined (__DragonFly__) && __DragonFly_version >= 190000
		ps->userLoad = 100.0 * (double) p.kp_lwp.kl_pctcpu / fscale;
	ps->vmSize   = p.kp_vm_map_size;
	ps->vmRss    = p.kp_vm_rssize * getpagesize();
	strlcpy(ps->name,p.kp_comm ? p.kp_comm : "????", 
		sizeof(ps->name));
	strcpy(ps->status,(p.kp_stat>=1)&&(p.kp_stat<=5)? statuses[p.kp_stat-1]:"????");
#else
	ps->userLoad = 100.0 * (double) p.kp_proc.p_pctcpu / fscale;
	ps->vmSize   = p.kp_eproc.e_vm.vm_map.size;
	ps->vmRss    = p.kp_eproc.e_vm.vm_rssize * getpagesize();
#if defined (__DragonFly__)
	strlcpy(ps->name,p.kp_thread.td_comm ? p.kp_thread.td_comm : "????",
		sizeof(ps->name));
#else
	strlcpy(ps->name,p.kp_proc.p_comm ? p.kp_proc.p_comm : "????", sizeof(ps->name));
	strcpy(ps->status,(p.kp_proc.p_stat>=1)&&(p.kp_proc.p_stat<=5)? statuses[p.kp_proc.p_stat-1]:"????");
#endif
#endif

        /* process command line */
	/* do a sysctl to get the command line args. */

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_ARGS;
	mib[3] = pid;

	if ((sysctl(mib, 4, buf, &buflen, 0, 0) == -1) || !buflen)
		strcpy(ps->cmdline, "????");
	else
		strncpy(ps->cmdline, buf, buflen);

	return (0);
}

static void
cleanupProcessList(void)
{
	ProcessInfo* ps;

	ProcessCount = 0;
	/* All processes that do not have the active flag set are assumed dead
	 * and will be removed from the list. The alive flag is cleared. */
	for (ps = first_ctnr(ProcessList); ps; ps = next_ctnr(ProcessList))
	{
		if (ps->alive)
		{
			/* Process is still alive. Just clear flag. */
			ps->alive = 0;
			ProcessCount++;
		}
		else
		{
			/* Process has probably died. We remove it from the list and
			 * destruct the data structure. i needs to be decremented so
			 * that after i++ the next list element will be inspected. */
			free(remove_ctnr(ProcessList));
		}
	}
}

/*
================================ public part ==================================
*/

void
initProcessList(struct SensorModul* sm)
{
	size_t fscalelen;
	ProcessList = new_ctnr();

	registerMonitor("ps", "table", printProcessList, printProcessListInfo, sm);
	registerMonitor("pscount", "integer", printProcessCount, printProcessCountInfo, sm);

	if (!RunAsDaemon)
	{
		registerCommand("kill", killProcess);
		registerCommand("setpriority", setPriority);
	}

	fscalelen = sizeof(fscale);
	if (sysctlbyname("kern.fscale", &fscale, &fscalelen, NULL, 0) == -1)
		fscale = 0;

	updateProcessList();
}

void
exitProcessList(void)
{
	removeMonitor("ps");
	removeMonitor("pscount");

	if (ProcessList)
		free (ProcessList);
}

int
updateProcessList(void)
{
        int mib[3];
        size_t len;
        size_t num;
        struct kinfo_proc *p;


        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_ALL;
        sysctl(mib, 3, NULL, &len, NULL, 0);
	p = malloc(len);
        sysctl(mib, 3, p, &len, NULL, 0);

	for (num = 0; num < len / sizeof(struct kinfo_proc); num++)
#if __FreeBSD_version >= 500015
		updateProcess(p[num].ki_pid);
#elif __DragonFly_version >= 190000
            /* Skip kernel threads with pid -1. Swapper with pid 0 also
             * causing problems is skipped in printProcessList() as 'kernel'
             * entry. */
            if (p[num].kp_pid >= 0)
		updateProcess(p[num].kp_pid);
#elif defined(__DragonFly__)
            if (p[num].kp_proc.p_pid >= 0)
		updateProcess(p[num].kp_proc.p_pid);
#else
		updateProcess(p[num].kp_proc.p_pid);
#endif
	free(p);
	cleanupProcessList();

	return (0);
}

void
printProcessListInfo(const char* cmd)
{
	fprintf(CurrentClient, "Name\tPID\tPPID\tUID\tGID\tStatus\tUser%%\tSystem%%\tNice\tVmSize\tVmRss\tLogin\tCommand\n");
	fprintf(CurrentClient, "s\td\td\td\td\tS\tf\tf\td\tD\tD\ts\ts\n");
}

void
printProcessList(const char* cmd)
{
	ProcessInfo* ps;

	ps = first_ctnr(ProcessList); /* skip 'kernel' entry */
	for (ps = next_ctnr(ProcessList); ps; ps = next_ctnr(ProcessList))
	{
		fprintf(CurrentClient, "%s\t%ld\t%ld\t%ld\t%ld\t%s\t%.2f\t%.2f\t%d\t%d\t%d\t%s\t%s\n",
			   ps->name, (long)ps->pid, (long)ps->ppid,
			   (long)ps->uid, (long)ps->gid, ps->status,
			   ps->userLoad, ps->sysLoad, ps->niceLevel,
			   ps->vmSize / 1024, ps->vmRss / 1024, ps->userName, ps->cmdline);
	}
}

void
printProcessCount(const char* cmd)
{
	fprintf(CurrentClient, "%d\n", ProcessCount);
}

void
printProcessCountInfo(const char* cmd)
{
	fprintf(CurrentClient, "Number of Processes\t1\t65535\t\n");
}

void
killProcess(const char* cmd)
{
	int sig, pid;

	sscanf(cmd, "%*s %d %d", &pid, &sig);
	switch(sig)
	{
	case MENU_ID_SIGABRT:
		sig = SIGABRT;
		break;
	case MENU_ID_SIGALRM:
		sig = SIGALRM;
		break;
	case MENU_ID_SIGCHLD:
		sig = SIGCHLD;
		break;
	case MENU_ID_SIGCONT:
		sig = SIGCONT;
		break;
	case MENU_ID_SIGFPE:
		sig = SIGFPE;
		break;
	case MENU_ID_SIGHUP:
		sig = SIGHUP;
		break;
	case MENU_ID_SIGILL:
		sig = SIGILL;
		break;
	case MENU_ID_SIGINT:
		sig = SIGINT;
		break;
	case MENU_ID_SIGKILL:
		sig = SIGKILL;
		break;
	case MENU_ID_SIGPIPE:
		sig = SIGPIPE;
		break;
	case MENU_ID_SIGQUIT:
		sig = SIGQUIT;
		break;
	case MENU_ID_SIGSEGV:
		sig = SIGSEGV;
		break;
	case MENU_ID_SIGSTOP:
		sig = SIGSTOP;
		break;
	case MENU_ID_SIGTERM:
		sig = SIGTERM;
		break;
	case MENU_ID_SIGTSTP:
		sig = SIGTSTP;
		break;
	case MENU_ID_SIGTTIN:
		sig = SIGTTIN;
		break;
	case MENU_ID_SIGTTOU:
		sig = SIGTTOU;
		break;
	case MENU_ID_SIGUSR1:
		sig = SIGUSR1;
		break;
	case MENU_ID_SIGUSR2:
		sig = SIGUSR2;
		break;
	}
	if (kill((pid_t) pid, sig))
	{
		switch(errno)
		{
		case EINVAL:
			fprintf(CurrentClient, "4\t%d\n", pid);
			break;
		case ESRCH:
			fprintf(CurrentClient, "3\t%d\n", pid);
			break;
		case EPERM:
			fprintf(CurrentClient, "2\t%d\n", pid);
			break;
		default:
			fprintf(CurrentClient, "1\t%d\n", pid);	/* unknown error */
			break;
		}

	}
	else
		fprintf(CurrentClient, "0\t%d\n", pid);
}

void
setPriority(const char* cmd)
{
	int pid, prio;

	sscanf(cmd, "%*s %d %d", &pid, &prio);
	if (setpriority(PRIO_PROCESS, pid, prio))
	{
		switch(errno)
		{
		case EINVAL:
			fprintf(CurrentClient, "4\n");
			break;
		case ESRCH:
			fprintf(CurrentClient, "3\n");
			break;
		case EPERM:
		case EACCES:
			fprintf(CurrentClient, "2\n");
			break;
		default:
			fprintf(CurrentClient, "1\n");	/* unknown error */
			break;
		}
	}
	else
		fprintf(CurrentClient, "0\n");
}
