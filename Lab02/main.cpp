#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <sstream>
#include <termios.h>
#include <iostream>
#include <list>
#include <string.h>

#define MAXARGS 10

class Proces
{

public:
	pid_t pid;
	std::string name;
	Proces(pid_t pid_c, std::string naziv_c) : pid(pid_c), name(naziv_c) {}
};

struct sigaction prije;
std::list<Proces> still_running;

bool is_running(pid_t pid)
{
	for (const Proces &proces : still_running)
	{
		if (proces.pid == pid)
			return true;
	}
	return false;
}

void ukloni_proces_iz_running(pid_t pid_zavrsio)
{
	for (auto it = still_running.begin(); it != still_running.end(); ++it)
	{
		if (it->pid == pid_zavrsio)
		{
			still_running.erase(it);
			break;
		}
	}
}

void obradi_dogadjaj(int sig) {
	pid_t pid = getpid();
	printf("\n[signal SIGINT] proces %d primio signal %d\n", (int)pid, sig);
}

void obradi_signal_zavrsio_neki_proces_dijete(int id)
{
	pid_t pid_zavrsio = waitpid(-1, NULL, WNOHANG);
	if (pid_zavrsio > 0)
		if (kill(pid_zavrsio, 0) == -1)
		{
			ukloni_proces_iz_running(pid_zavrsio);
			printf("[roditelj %d] dijete %d zavrsilo s radom\n", (int)getpid(), pid_zavrsio);
		}
}

pid_t pokreni_program(char *naredba[], int u_pozadini)
{
	pid_t pid_novi;
	if ((pid_novi = fork()) == 0)
	{
		printf("[dijete %d] krenuo s radom\n", (int)getpid());
		sigaction(SIGINT, &prije, NULL);
		setpgid(pid_novi, pid_novi);
		if (!u_pozadini)
			tcsetpgrp(STDIN_FILENO, getpgid(pid_novi));
		execvp(naredba[0], naredba);
		exit(1);
	}
	return pid_novi;
}

std::string cwd()
{
	static const long size = pathconf(".", _PC_PATH_MAX);
	static char *cwd = nullptr;
	static char *buf = (char *)malloc((size_t)size);
	if (buf != nullptr)
	{
		cwd = getcwd(buf, (size_t)size);
	}
	if (cwd)
		return cwd;
	else
		return "";
}

int main()
{
	struct sigaction act;
	size_t vel_buf = 128;
	char buffer[vel_buf];
	pid_t pid_novi;

	printf("[roditelj %d] krenuo s radom\n", (int)getpid());

	act.sa_handler = obradi_dogadjaj;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, &prije);
	act.sa_handler = obradi_signal_zavrsio_neki_proces_dijete;
	sigaction(SIGCHLD, &act, NULL);
	act.sa_handler = SIG_IGN;
	sigaction(SIGTTOU, &act, NULL);

	struct termios shell_term_settings;
	tcgetattr(STDIN_FILENO, &shell_term_settings);

	tcsetpgrp(STDIN_FILENO, getpgid(0));

	do
	{
		if (fgets(buffer, vel_buf, stdin) != NULL)
		{
			int background = 0;
			char *argv[MAXARGS];
			int argc = 0;
			argv[argc] = strtok(buffer, " \t\n");
			while (argv[argc] != NULL)
			{

				char *value = strtok(NULL, " \t\n");
				if (value != NULL)
				{

					std::string s_value(value);
					if (s_value == "&")
					{
						background = 1;
						continue;
					}
				}
				argc++;
				argv[argc] = value;
			}

			if (strcmp(argv[0], "pwd") == 0 && argc == 1)
			{
				std::cout << cwd() << std::endl;
			}

			else if (strcmp(argv[0], "cd") == 0 && argc == 2)
			{
				chdir(argv[1]);
				std::cout << "cwd:  " << cwd() << std::endl;
			}

			else if (strcmp(argv[0], "ps") == 0 && argc == 1)
			{
				if (still_running.size() > 0)
				{
					std::cout << "PID\time" << std::endl;
					for (auto &proces : still_running)
					{
						std::cout << proces.pid << "\t" << proces.name << std::endl;
					}
				}
			}

			else if (strcmp(argv[0], "kill") == 0 && argc == 3)
			{
				pid_t pid = std::stoi(argv[1]);
				if (is_running(pid))
				{
					kill(pid, std::stoi(argv[2]));
				}
				else
					std::cout << "Proces s PID-om " << pid << " nije pokrenut od strane ove ljuske." << std::endl;
			}

			else if (strcmp(argv[0], "exit") == 0 && argc == 1)
			{
				for (auto &proces : still_running)
				{
					printf("[roditelj %d] dijete %d zavrsilo s radom\n", (int)getpid(), proces.pid);
					kill(proces.pid, SIGINT);
				}
				break;
			}

			else
			{
				pid_novi = pokreni_program(argv, background);
				Proces running = Proces(pid_novi, argv[0]);
				still_running.push_back(running);
				pid_t pid_zavrsio;
				if (!background)
				{
					do
					{
						pid_zavrsio = waitpid(pid_novi, NULL, 0);
						if (pid_zavrsio > 0)
						{
							if (kill(pid_novi, 0) == -1)
							{
								tcsetpgrp(STDIN_FILENO, getpgid(0));
								tcsetattr(STDIN_FILENO, 0, &shell_term_settings);
								printf("[roditelj %d] dijete %d zavrsilo s radom\n", (int)getpid(), pid_zavrsio);
								ukloni_proces_iz_running(pid_zavrsio);
							}
							else
							{
								pid_novi = (pid_t)0;
							}
						}
						else
						{
							break;
						}
					} while (pid_zavrsio <= 0);
				}
			}
		}
	} while (strncmp(buffer, "exit", 4) != 0);
	return 0;
}
