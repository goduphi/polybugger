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
#define COMMAND_NUMBER		4
#define MAX_COMMAND_SIZE	11

typedef enum
{
	BREAK, CONTINUE, STEP, READ
} currentCommand;

char commands[][MAX_COMMAND_SIZE] = { "break", "continue", "step", "read" };

void checkCommandLineArguments(int argc, char* argv[])
{
	if(argc < 2)
	{
		printf("Format error: %s <Executable>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
}

bool isValidCommand(const char* command, currentCommand* c)
{
	for(uint8_t i = 0; i < COMMAND_NUMBER; i++)
	{
		if(strncmp(command, commands[i], strlen(commands[i])) == 0 &&
		   strlen(commands[i]) == strlen(command))
		{
			*c = (currentCommand)i;
			return true;
		}
	}
	return false;
}

uint64_t getAddress(char input[], currentCommand* c)
{
	if(strlen(input) == 0)
		return false;
	input[strlen(input) - 1] = '\0';
	char* token = strtok(input, " ");
	if(!isValidCommand(token, c))
		return 0x0;
	if((token = strtok(NULL, " ")) == NULL)
		return 0x0; 
	return strtol(token, NULL, 16);
}

void continueToBreakPoint(pid_t pid, int* status)
{
	if(ptrace(PTRACE_CONT, pid, 0, 0) < 0)
	{
		perror("ptrace() error");
		exit(EXIT_FAILURE);
	}
	waitpid(pid, status, 0);
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

		char userInput[MAX_USER_INPUT_CHARS + 1];
		currentCommand c;
		breakPoint* bp;
		
		while(true)
		{
			printf("Program/Instruction counter [%lx]\n>> ", getCurrentInstructionPointer(pid));
			fgets(userInput, MAX_USER_INPUT_CHARS + 1, stdin);
			uint64_t address = getAddress(userInput, &c);

			switch(c)
			{
			case BREAK:
				printf("Creating breakpoint at 0x%lx\n", address);
				bp = createBreakPoint(pid, (void*)address);
				printf("Break point created at 0x%lx\n", address);
				break;
			case CONTINUE:
				continueToBreakPoint(pid, &status);
				break;
			case STEP:
				if(!WIFSTOPPED(status))
					break;
				if(ptrace(PTRACE_SINGLESTEP, pid, 0, 0) < 0)
				{
					perror("ptrace() error");
				}
				waitpid(pid, &status, 0);				
				break;
			case READ:
				{
				long data;
				if((data = ptrace(PTRACE_PEEKTEXT, pid, address, 0)) < 0)
				{
					perror("ptrace() error at read");
				}
				printf("Data at 0x%lx is %lx\n", address, data);	
				}
				break;
			}

			if(WIFEXITED(status))
			{
				printf("Child exited\n");
				break;
			}
		}

		deleteBreakPoint(bp);
	}

	return EXIT_SUCCESS;
}
