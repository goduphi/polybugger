#ifndef POLYBUGGER_H
#define POLYBUGGER_H

#include <sys/types.h>

typedef struct _breakPoint
{
	void* address;
	long data;
} breakPoint;

typedef enum _traceeStatus
{
	EXITED,
	STOPPED,
	ERROR
} traceeStatus;

long getCurrentInstructionPointer(pid_t pid);
breakPoint* createBreakPoint(pid_t pid, void* address);
void deleteBreakPoint(breakPoint* bp);
traceeStatus resumeFromBreakPoint(pid_t pid, breakPoint* bp);

#endif
