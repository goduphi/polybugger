#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#include "polybugger.h"

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		printf("Format error: %s <Executable>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	pid_t pid = fork();
	if(pid == -1)
	{
		perror("fork() error:");
		exit(EXIT_FAILURE);
	}
	
	if(pid == 0)
	{
		long ptraceResult = ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		if(ptraceResult == -1)
		{
			perror("ptrace() error");
			exit(EXIT_FAILURE);
		}

		int execveResult = execve(argv[1], &argv[1], NULL);
		if(execveResult == -1)
		{
			perror("execve() error");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		printf("Starting debug session...\n");
		int status;

		// This call to waitpid makes sure that the tracer waits until the
		// child process stops on its first instruction as specified by
		// PTRACE_TRACEME
		waitpid(pid, &status, 0);
		
		printf("Child is now at 0x%08lx\n", getCurrentInstructionPointer(pid));
		
		uint64_t bpAddr = 0x00000000004000d4;
		breakPoint* bp = createBreakPoint(pid, (void*)bpAddr);
		printf("Created breakpoint\n");
		ptrace(PTRACE_CONT, pid, 0, 0);
		waitpid(pid, &status, 0);

		while(true)
		{
			printf("Process stopped at 0x%16lx\n", getCurrentInstructionPointer(pid));
			traceeStatus ts = resumeFromBreakPoint(pid, bp);

			if(ts == ERROR)
				break;
			else if(ts == EXITED)
			{
				printf("Child exited\n");
				break;
			}
			else if(ts == STOPPED)
				continue;
		}

		deleteBreakPoint(bp);
	}

		
	return EXIT_SUCCESS;
}
