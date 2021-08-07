#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <fcntl.h>

#include "polybugger.h"

#define MAX_USER_INPUT_CHARS	80

void checkCommandLineArguments(int argc, char* argv[])
{
	if(argc < 2)
	{
		printf("Format error: %s <Executable>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[])
{
	checkCommandLineArguments(argc, argv);	

	// These variables are for the parsing the DWARF format inside of elf

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
	
		// Use objdump -d executable to find out the address for the breakpoint	
		uint64_t bpAddr = 0x4000f2;
		breakPoint* bp = createBreakPoint(pid, (void*)bpAddr);
		printf("Created breakpoint at 0x%12lx\n", bpAddr);
		// Continue to the next break point
		ptrace(PTRACE_CONT, pid, 0, 0);
		waitpid(pid, &status, 0);
	
		char userInput[MAX_USER_INPUT_CHARS + 1];
		memset(userInput, 0, sizeof(userInput));

		while(true)
		{
			printf("Process stopped at 0x%12lx\n", getCurrentInstructionPointer(pid));

			if(WIFSTOPPED(status))
			{
				printf("> ");
				scanf("%s", userInput);

				if(strncmp(userInput, "c", 1) == 0 && strlen(userInput) == 1)
				{	
					if(ptrace(PTRACE_SINGLESTEP, pid, 0, 0) < 0)
					{
						perror("ptrace() error");
						exit(EXIT_FAILURE);
					}
					waitpid(pid, &status, 0);
				}
			}
			
			if(WIFEXITED(status))
			{
				printf("Child exited.\n");
				break;
			}
		}

		deleteBreakPoint(bp);
	}

	return EXIT_SUCCESS;
}
