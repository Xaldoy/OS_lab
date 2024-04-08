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

class Proces
{
public:
	pid_t pid;
	std::string naziv;
	Proces(pid_t pid, std::string &naziv) : pid(pid), naziv(naziv) {}
}

int
main()
{

	size_t vel_buf = 128;
	char buffer[vel_buf];
	std::list<Proces> still_running;

	do
	{
		if (fgets(buffer, vel_buf, stdin) != NULL)
		{
			char *argv[MAXARGS];
			int argc = 0;
			argv[argc] = strtok(buffer, " \t\n");
			while (argv[argc] != NULL)
			{
				argc++;
				argv[argc] = strtok(NULL, " \t\n");
			}

			if (strcmp(argv[0], "cd") == 0 && argc == 2)
			{
				chdir(argv[1]);
				std::cout << "Changed to: " << getcwd() << std::endl;
			}

			if (strcmp(argv[0], "ps") == 0 && argc == 1)
			{
				std::cout << "PID\time" << std::endl;
				foreach (Proces &proces : still_running)
				{
					std::cout << proces.pid << "\t" << proces.name << std::endl;
				}
			}

			if (strcmp(argv[0], "kill" == 0) && argc == 2)
			{
				std::cout << "killing " << argv[1] << std::endl;
			}
		}
	} while (strncmp(buffer, "exit", 4) != 0);
}
