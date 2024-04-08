#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <termios.h>

#define MAXARGS 5

struct sigaction prije;

void obradi_dogadjaj(int sig)
{
        printf("\n[signal SIGINT] prcoces %d primio signal %d\n", (int)getpid(), sig);
}

void obradi_signal_zavrsio_neki_proces_dijete(int id)
{
        pid_t pid_zavrsio = waitpid(-1, NULL, WNOHANG);
        if (pid_zavrsio > 0)
                if (kill(pid_zavrsio, 0) == -1)
                        printf("\n[roditelj %d - SIGCHLD + waitpid] dijete %d zavrsilo s radom\n", (int)getpid(), pid_zavrsio);
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
                        tcsetpgrp(STDIN_FILENO, getpid(pid_novi));
                execvp(naredba[0], naredba);
                perror("Nisam pokrenuo program!");
                exit(1);
        }
        return pid_novi;
}

int main()
{
        struct sigaction act;
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

        char *naredba_echo[] = {"echo", "-e", "Jedan\nDva\nTri", NULL};
        pid_novi = pokreni_program(naredba_echo, 0);
        waitpid(pid_novi, NULL, 0);

        tcsetpgrp(STDIN_FILENO, getpid(0));

        size_t vel_buf = 128;
        char buffer[vel_buf];

        do
        {
                printf("[roditelj] unesi naredbu: ");

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

                        printf("[roditelj] pokrecem program\n");
                        pid_novi = pokreni_program(argv, 0);
                        printf("[roditelj] cekam da zavrsi\n");
                        pid_t pid_zavrsio;
                        do
                        {
                                pid_zavrsio = waitpid(pid_novi, NULL, 0);
                                if (pid_zavrsio > 0)
                                {
                                        if (kill(pid_novi, 0) == -1)
                                        {
                                                printf("[roditelj] dijete %d zavrsilo s radom\n", pid_zavrsio);

                                                tcsetpgrp(STDIN_FILENO, getpid(0));
                                                tcsetattr(STDIN_FILENO, 0, &shell_term_settings);
                                        }
                                        else
                                        {
                                                pid_novi = (pid_t)0;
                                        }
                                        else
                                        {
                                                printf("[roditelj] waitpid gotov ali ne daje informaciju\n");
                                                break;
                                        }
                                }
                        } while (pid_zavrsio <= 0);
                }
        } while (strncmp(buffer, "exit", 4) != 0);
        return 0;
}