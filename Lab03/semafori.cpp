#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <cstdlib>
#include "time.h"

class Meduspremnik
{
public:
    int first, last, count;
    std::vector<char> ms;
    Meduspremnik(int size)
    {
        this->first = 0;
        this->last = 0;
        this->count = 0;
        this->ms.resize(size);
        for(auto &d: ms) {
            d = '-';
        }
    }

    void insert(char a)
    {

        if (count == ms.size())
        {
            first = (first + 1) % ms.size();
        }
        else
        {
            count++;
        }
        ms[last] = a;
        last = (last + 1) % ms.size();
    }

    char get()
    {
        if (count == 0)
            return '0';

        char value = ms[first];
        ms[first] = '-';
        first = (first + 1) % ms.size();
        count--;

        return value;
    }
};

int ums_size, ims_size;
int bud, brd, bid;
sem_t *ispis_semafor;
std::list<sem_t *> ums_bsem_semafori;
std::list<sem_t *> ims_bsem_semafori;
std::list<sem_t *> ums_osem_semafori;
std::list<Meduspremnik> ums;
std::list<Meduspremnik> ims;
time_t start;

double getTimePassed() {
    return difftime(time(0), start);
}

void print_ms()
{
    double time = getTimePassed();
    std::cout << "t=" << time << "\t";
    std::cout << "UMS[]: ";
    for (auto ms : ums)
    {
        for (auto data : ms.ms)
        {
            std::cout << data;
        }
        std::cout << " ";
    }
    std::cout << std::endl;
    std::cout << "t=" << time << "\t";
    std::cout << "IMS[]: ";
    for (auto ms : ims)
    {
        for (auto data : ms.ms)
        {
            std::cout << data;
        }
        std::cout << " ";
    }
    std::cout << std::endl
              << std::endl;
}

char dohvati_ulaz(int I)
{
    return 'a' + rand() % 26; // 'a' - 'z'
}

int obradi_ulaz(int I, char U)
{
    return rand() % brd; // Indeks nekog UMS
}

char obradi(char P, int &t)
{
    int sleep_time = rand() % 5 + 2;
    sleep(sleep_time);
    t = rand() % bid;
    return P + ('A' - 'a');
}

void *ulazna_dretva(void *I)
{
    int id = *((int *)I);
    while (1)
    {

        int sleep_time = rand() % 5 + 1;
        sleep(sleep_time);

        char U = dohvati_ulaz(id);
        int T = obradi_ulaz(id, U);

        auto it_ums_osem = ums_osem_semafori.begin();
        std::advance(it_ums_osem, T);

        auto it_ums_bsem = ums_bsem_semafori.begin();
        std::advance(it_ums_bsem, T);

        sem_wait(*it_ums_bsem);

        // KO
        auto ums_it = ums.begin();
        std::advance(ums_it, T);
        ums_it->insert(U);

        sem_post(*it_ums_bsem);

        sem_wait(ispis_semafor);

        std::cout << "t=" << getTimePassed() << "\tU" << id
                  << ": dohvati_ulaz(" << id << ")->"
                  << U << "; obradi_ulaz(" << U << ")->"
                  << T << "; " << T << "-> UMS[" << T << "]"
                  << std::endl;
        print_ms();
        // KO
        sem_post(ispis_semafor);
        // Postavi semafore
        sem_post(*it_ums_bsem);
        sem_post(*it_ums_osem);
    }
    return nullptr;
}

void *radna_dretva(void *I)
{
    int id = *((int *)I);
    while (1)
    {
        auto it_ims_bsem = ims_bsem_semafori.begin();
        std::advance(it_ims_bsem, id);
        auto it_ums_osem = ums_osem_semafori.begin();
        std::advance(it_ums_osem, id);
        auto it_ums_bsem = ums_bsem_semafori.begin();
        std::advance(it_ums_bsem, id);

        sem_wait(*it_ums_osem);
        sem_wait(*it_ums_bsem);
        auto ums_it = ums.begin();
        std::advance(ums_it, id);
        char P = ums_it->get();
        sem_post(*it_ums_bsem);

        int t;
        char R = obradi(P, t);

        sem_wait(*it_ims_bsem);
        auto ims_it = ims.begin();
        std::advance(ims_it, t);
        ims_it->insert(R);
        sem_post(*it_ims_bsem);
    
        sem_wait(ispis_semafor);
                std::cout << "t=" << getTimePassed() << "\tR" << id
                  << ": uzimam iz UMS[" << id << "]->"
                  << P << " i obradujem->" << R
                  << std::endl;
        print_ms();
        sem_post(ispis_semafor);
    }
}

void *izlazna_dretva(void *I)
{
    int id = *((int *)I);
    while (1)
    {
        sleep(2);
        auto it_access = ims_bsem_semafori.begin();
        std::advance(it_access, id);

        sem_wait(*it_access);

        // KO

        auto ims_it = ims.begin();
        std::advance(ims_it, id);

        char K = ims_it->get();

        // KO
        sem_post(*it_access);
    }
}

int main()
{
    // Vrijeme
    start = time(0);

    // Input
    std::cout << "BUD: ";
    std::cin >> bud;
    std::cout << "BRD: ";
    std::cin >> brd;
    std::cout << "BID: ";
    std::cin >> bid;
    std::cout << "UMS_SIZE: ";
    std::cin >> ums_size;
    std::cout << "IMS_SIZE: ";
    std::cin >> ims_size;

    // Dretve
    pthread_t *u_threads = new pthread_t[bud];
    pthread_t *r_threads = new pthread_t[brd];
    pthread_t *i_threads = new pthread_t[bid];

    // Ispis semafor
    ispis_semafor = new sem_t;
    sem_init(ispis_semafor, 0, 1);

    // Stvaranje dretvi

    // Ulazne dretve
    for (int i = 0; i < bud; i++)
    {
        int *id = (int *)malloc(sizeof(int));
        *id = i;
        pthread_create(&u_threads[i], nullptr, ulazna_dretva, id);
    }

    // Radne dretve
    for (int i = 0; i < brd; i++)
    {
        int *id = (int *)malloc(sizeof(int));
        *id = i;

        // Stvaranje UMS
        Meduspremnik m(ums_size);
        for (auto &val : m.ms)
        {
            val = '-';
        }
        ums.push_back(m);

        // Stvaranje semafora za kontrolu pristupa ulaznih dretvi
        sem_t *sem_a = new sem_t;
        sem_init(sem_a, 0, 1);
        ums_bsem_semafori.push_back(sem_a);

        // Stvaranje semafora za kontrolu pristupa radnih dretvi
        sem_t *sem_n = new sem_t;
        sem_init(sem_n, 0, 0);
        ums_osem_semafori.push_back(sem_n);

        // Stvaranje Dretve
        pthread_create(&r_threads[i], nullptr, radna_dretva, id);
    }

    // Izlazne dretve
    for (int i = 0; i < bid; i++)
    {
        int *id = (int *)malloc(sizeof(int));
        *id = i;

        // Stvaranje IMS
        Meduspremnik m(ims_size);
        for (auto &val : m.ms)
        {
            val = '-';
        }
        ims.push_back(m);

        // Stvaranje Semafora za kontrolu pristupa radnih dretvi
        sem_t *sem_a = new sem_t;
        sem_init(sem_a, 0, 1);
        ims_bsem_semafori.push_back(sem_a);

        // Stvaranje Dretve
        pthread_create(&i_threads[i], nullptr, izlazna_dretva, id);
    }

    // Cleanup
    for (int i = 0; i < bud; i++)
    {
        pthread_join(u_threads[i], nullptr);
    }
    for (int i = 0; i < brd; i++)
    {
        pthread_join(r_threads[i], nullptr);
    }
    for (int i = 0; i < bid; i++)
    {
        pthread_join(i_threads[i], nullptr);
    }

    sem_destroy(ispis_semafor);
    delete ispis_semafor;
    for (auto sem : ims_bsem_semafori)
    {
        sem_destroy(sem);
        delete sem;
    }
    for (auto sem : ums_bsem_semafori)
    {
        sem_destroy(sem);
        delete sem;
    }
    for (auto sem : ums_osem_semafori)
    {
        sem_destroy(sem);
        delete sem;
    }
    delete[] u_threads;
    delete[] r_threads;
    delete[] i_threads;

    return 0;
}
