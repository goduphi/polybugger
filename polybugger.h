#ifndef POLYBUGGER_H
#define POLYBUGGER_H

#include <sys/types.h>

typedef struct _breakPoint
{
	void* address;
	long data;
} breakPoint;

long getCurrentInstructionPointer(pid_t pid);
breakPoint* createBreakPoint(pid_t pid, void* address);
void deleteBreakPoint(breakPoint* bp);
void continueToBreakPoint(pid_t pid, int* status);
void freeBreakPoints(breakPoint* bp[], uint8_t breakPointCounter);

#endif
