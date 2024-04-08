#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <termios.h>
#include <iostream>

#define MAXARGS 10

int main() {

	size_t vel_buf = 128;
	char buffer[vel_buf];
	do
	{
		if(fgets(buffer, vel_buf, stdin) != NULL)
		{
			char *argv[MAXARGS];
			int argc = 0;
			argv[argc] = strtok(buffer, " \t\n");
			while (argv[argc] != NULL)
			{
				argc++;
				argv[argc] = strtok(NULL, " \t\n");
			}

			if(strcmp(argv[0], "cd") == 0)
			{
				std::cout << "Changing directory" << std::endl;
			}
		}
	} while (strncmp(buffer, "exit", 4) != 0);
}
