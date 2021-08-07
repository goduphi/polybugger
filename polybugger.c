#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/ptrace.h>

#include "polybugger.h"

/*
 * Use rip instead of the eip register due to the machine being x86 64
 */

long getCurrentInstructionPointer(pid_t pid)
{
	struct user_regs_struct r;
	ptrace(PTRACE_GETREGS, pid, 0, &r);
	return r.rip;
}

// A static function makes sure that it is only visible to the polybugger.c file
static void enableBreakPoint(pid_t pid, breakPoint* bp)
{
	bp->data = ptrace(PTRACE_PEEKTEXT, pid, (void*)bp->address, 0);
	// Write back a value of 0xCC in the lower byte of the instruction to set a break point
	ptrace(PTRACE_POKEDATA, pid, bp->address, (bp->data & ~0xFF) | 0xCC);
}

static void disableBreakPoint(pid_t pid, breakPoint* bp)
{
	long instruction = ptrace(PTRACE_PEEKTEXT, pid, bp->address, 0);
	// Check if there was a breakpoint to begin with
	assert((instruction & 0xFF) == 0xCC);
	ptrace(PTRACE_POKEDATA, pid, bp->address, (instruction & ~0xFF) | (bp->data & 0xFF));
}

breakPoint* createBreakPoint(pid_t pid, void* address)
{
	breakPoint* bp = (breakPoint*)malloc(sizeof(breakPoint));
	bp->address = address;
	enableBreakPoint(pid, bp);
	return bp;
}

void deleteBreakPoint(breakPoint* bp)
{
	free(bp);
}

traceeStatus resumeFromBreakPoint(pid_t pid, breakPoint* bp)
{
	struct user_regs_struct r;
	int status;

	// Get the current instruction pointer and check if the
	// child stopped at the expected address
	ptrace(PTRACE_GETREGS, pid, 0, &r);
	// The instruction counter always points to the next instruction
	// to be executed
	assert(r.rip == (long)bp->address + 1);

	// Rewind the instruction pointer back by 1 and do a PTRACE_SINGLESTEP
	// This allows the process to execute the instruction where it got halted
	// Before this though, disable the break point
	
	r.rip = (long)bp->address;
	ptrace(PTRACE_SETREGS, pid, 0, &r);
	disableBreakPoint(pid, bp);
	if(ptrace(PTRACE_SINGLESTEP, pid, 0, 0) < 0)
	{
		perror("ptrace() error");
		return ERROR;
	}
	waitpid(pid, &status, 0);
	
	if(WIFEXITED(status))
		return EXITED;

	enableBreakPoint(pid, bp);

	if(ptrace(PTRACE_CONT, pid, 0, 0) < 0)
	{
		perror("ptrace() error");
		return ERROR;
	}

	waitpid(pid, &status, 0);

	if(WIFEXITED(status))
		return EXITED;
	else if(WIFSTOPPED(status))
		return STOPPED;
	
	return ERROR;
}
