#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void obradi_sigterm(int sig);
void obradi_sigint(int sig);
void obradi_prekid(int sig);
void obrada_prekida(int sig);
void blokiraj_odblokiraj_signale(int blokiraj);
std::string printStates();

int nije_kraj = 1;

int tp = 0;
int kz[] = {0, 0, 0, 0, 0};
int kon[] = {0, 0, 0, 0, 0};

int main() {
        struct sigaction act;
        act.sa_flags = 0;

        act.sa_handler = obradi_sigterm;
        sigemptyset(&act.sa_mask);
        sigaction(SIGTERM, &act, NULL);

        act.sa_handler = obradi_sigint;
        sigemptyset(&act.sa_mask);
        sigaction(SIGINT, &act, NULL);

        act.sa_handler = obradi_prekid;
        sigemptyset(&act.sa_mask);
        sigaction(1, &act, NULL);

        act.sa_handler = obradi_prekid;
        sigemptyset(&act.sa_mask);
        sigaction(2, &act, NULL);

        act.sa_handler = obradi_prekid;
        sigemptyset(&act.sa_mask);
        sigaction(3, &act, NULL);

        act.sa_handler = obradi_prekid;
        sigemptyset(&act.sa_mask);
        sigaction(4, &act, NULL);

        printf("Program s PID=%ld krenuo s radom\n", (long)getpid());
        int i = 1;
        while (nije_kraj) {
                sleep(1);
        }
        printf("Program s PID=%ld zavrsio s radom\n", (long)getpid());
        return 0;
}

void obradi_sigterm(int sig) {
        printf("Primio signal SIGTERM, pospremam prije izlaska iz programa\n");
        nije_kraj = 0;
}

void obradi_sigint(int sig) {
        printf("Primio signal SIGINT, prekidam rad\n");
        exit(1);
}

void obradi_prekid(int sig) {
        blokiraj_odblokiraj_signale(1);
        kz[sig] = 1;
        printf("Primljen prekid %d\t\t%s\n", sig, printStates().c_str());
        int i;
        for (i = 4; i > 0 && kz[i] == 0; i--)
                ;
        while (i > tp) {
                kz[i] = 0;
                kon[i] = tp;
                tp = i;
                printf("Pocetak obrade %d\t\t%s\n", tp, printStates().c_str());
                blokiraj_odblokiraj_signale(0);
                obrada_prekida(i);
                blokiraj_odblokiraj_signale(1);
                printf("Kraj obrade %d\t\t\t%s\n", tp, printStates().c_str());
                tp = kon[i];
                kon[i] = 0;
                for (i = 4; i > 0 && kz[i] == 0; i--);
        }
}

void obrada_prekida(int sig) {
        for (int i = 0; i <= 10; i++) {
                sleep(1);
        }
}

void blokiraj_odblokiraj_signale(int blokiraj) {
        sigset_t signali;
        sigemptyset(&signali);
        sigaddset(&signali, SIGTERM);
        sigaddset(&signali, SIGINT);
        sigaddset(&signali, 1);
        sigaddset(&signali, 2);
        sigaddset(&signali, 3);
        sigaddset(&signali, 4);
        if (blokiraj)
                pthread_sigmask(SIG_BLOCK, &signali, NULL);
        else
                pthread_sigmask(SIG_UNBLOCK, &signali, NULL);
}

std::string printStates() {
        std::string states = "kz: [";
        for (int i = 1; i < 5; ++i) {
                states += std::to_string(kz[i]);
                if (i < 4)
                        states += ", ";
        }
        states += "] kon: [";
        for (int i = 1; i < 5; ++i) {
                states += std::to_string(kon[i]);
                if (i < 4)
                        states += ", ";
        }
        states += "] tp: " + std::to_string(tp);
        return states;
}
