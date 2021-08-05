#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

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
		printf("Starting debug session...\n");
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
		int status;
		waitpid(pid, &status, 0);

		int data[8];
		ptrace(PTRACE_PEEKDATA, pid, 0, data);
		printf("%d\n", data[0]);
	}
	return EXIT_SUCCESS;
}
