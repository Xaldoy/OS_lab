#include <iostream>
#include <list>
#include <unistd.h>
#include <vector>
#include <pthread.h>

#define BR_PISACA 4
#define BR_CITACA 10
#define BR_BRISACA 2

std::list<char> lista;
pthread_mutex_t m;
pthread_cond_t red_citaca;
pthread_cond_t red_pisaca;
pthread_cond_t red_brisaca;
int br_citaca_ceka = 0;
int br_citaca_cita = 0;
int br_brisaca_brise = 0;
int br_brisaca_ceka = 0;
int br_pisaca_ceka = 0;
int br_pisaca_pise = 0;

void print_data()
{
    std::cout << "aktivnih: citaca=" << br_citaca_cita
              << ", pisaca=" << br_pisaca_pise
              << ", brisaca=" << br_brisaca_brise
              << std::endl;
    std::cout << "ceka: citaca=" << br_citaca_ceka
              << ", pisaca=" << br_pisaca_ceka
              << ", brisaca=" << br_brisaca_ceka
              << std::endl;
    std::cout << "Lista: ";
    for (auto &d : lista)
    {
        std::cout << d << " ";
    }
    std::cout << std::endl
              << std::endl;
}

void *citac(void *p)
{
    int sleep_time;
    int id = *((int *)p);
    while (1)
    {
        pthread_mutex_lock(&m);
        int index = rand() % (lista.size() + 1);
        std::cout << "čitač " << id << " želi čitati element " << index << " liste" << std::endl;
        print_data();
        br_citaca_ceka++;
        while (br_brisaca_brise + br_brisaca_ceka > 0)
            pthread_cond_wait(&red_citaca, &m);
        br_citaca_cita++;
        br_citaca_ceka--;
        auto it = lista.begin();
        std::advance(it, index);
        char y = (lista.size() > 0 && it != lista.end()) ? *it : '-';
        std::cout << "čitač " << id << " čita element " << index << " iz liste (vrijednosti " << y << ")" << std::endl;
        print_data();
        pthread_mutex_unlock(&m);

        sleep_time = rand() % 5 + 3;
        sleep(sleep_time);

        pthread_mutex_lock(&m);
        br_citaca_cita--;
        if (br_citaca_cita == 0 && br_brisaca_ceka > 0)
        {
            pthread_cond_signal(&red_brisaca);
        }
        std::cout << "čitač " << id << " više ne koristi listu" << std::endl;
        print_data();
        pthread_mutex_unlock(&m);

        sleep_time = rand() % 5 + 3;
        sleep(sleep_time);
    }
}

void *pisac(void *p)
{
    int sleep_time;
    int id = *((int *)p);
    while (1)
    {
        char c = rand() % ('z' - 'a' + 1) + 'a';
        pthread_mutex_lock(&m);
        std::cout << "pisač " << id << " želi dodati vrijednost " << c << " na kraj liste" << std::endl;
        print_data();
        br_pisaca_ceka++;
        while (br_brisaca_ceka + br_brisaca_brise + br_pisaca_pise > 0)
            pthread_cond_wait(&red_pisaca, &m);
        br_pisaca_ceka--;
        br_pisaca_pise++;
        lista.push_back(c);
        std::cout << "pisač " << id << " dodao vrijednost " << c << " na kraj liste " << std::endl;
        print_data();
        pthread_mutex_unlock(&m);

        sleep_time = rand() % 2 + 1;
        sleep(sleep_time);

        pthread_mutex_lock(&m);
        br_pisaca_pise--;
        if (br_pisaca_pise == 0 && br_pisaca_ceka > 0)
        {
            pthread_cond_signal(&red_pisaca);
        }
        if(br_brisaca_brise == 0 && br_brisaca_ceka > 0) {
            pthread_cond_signal(&red_brisaca);
        }

        std::cout << "pisač " << id << " više ne koristi listu" << std::endl;
        print_data();
        pthread_mutex_unlock(&m);

        sleep_time = rand() % 5 + 3;
        sleep(sleep_time);
    }
}

void *brisac(void *p)
{
    int sleep_time;
    int id = *((int *)p);
    while (1)
    {
        pthread_mutex_lock(&m);
        int index = rand() % lista.size();
        std::cout << "brisač " << id << " želi obrisati element " << index << " liste" << std::endl;
        print_data();
        br_brisaca_ceka++;
        while (br_pisaca_pise + br_citaca_cita + br_brisaca_brise > 0)
            pthread_cond_wait(&red_brisaca, &m);
        br_brisaca_brise++;
        br_brisaca_ceka--;
        if (index < lista.size())
        {
            auto it = lista.begin();
            std::advance(it, index);
            char c = *it;
            lista.erase(it);
            std::cout << "brisač " << id << " obrisao element " << index << " iz liste (vrijednosti " << c << ")" << std::endl;
            print_data();
        }
        else
            std::cout << "brisač " << id << " pokušao obrisati element " << index << " koji više ne postoji" << std::endl << std::endl;

        pthread_mutex_unlock(&m);

        sleep_time = rand() % 2 + 1;
        sleep(sleep_time);

        pthread_mutex_lock(&m);
        br_brisaca_brise--;
        if(br_brisaca_brise == 0 && br_brisaca_ceka) pthread_cond_signal(&red_brisaca);
        if(br_brisaca_brise == 0 && br_pisaca_ceka) pthread_cond_signal(&red_pisaca);
        if(br_citaca_ceka == 0 && br_citaca_ceka) pthread_cond_broadcast(&red_citaca);
        std::cout << "brisač " << id << " više ne koristi listu" << std::endl;
        print_data();
        pthread_mutex_unlock(&m);

        sleep_time = rand() % 10 + 5;
        sleep(sleep_time);
    }
}

int main()
{
    pthread_t *pisaci = new pthread_t[BR_PISACA];
    pthread_t *citaci = new pthread_t[BR_CITACA];
    pthread_t *brisaci = new pthread_t[BR_BRISACA];

    pthread_mutex_init(&m, NULL);
    pthread_cond_init(&red_pisaca, NULL);
    pthread_cond_init(&red_citaca, NULL);
    pthread_cond_init(&red_brisaca, NULL);

    for (int i = 0; i < BR_PISACA; i++)
    {
        int *id = (int *)malloc(sizeof(int));
        *id = i;
        pthread_create(&pisaci[i], nullptr, pisac, id);
    }
    sleep(5);
    for (int i = 0; i < BR_CITACA; i++)
    {
        int *id = (int *)malloc(sizeof(int));
        *id = i;
        pthread_create(&citaci[i], nullptr, citac, id);
    }
    for (int i = 0; i < BR_BRISACA; i++)
    {
        int *id = (int *)malloc(sizeof(int));
        *id = i;
        pthread_create(&brisaci[i], nullptr, brisac, id);
    }
    for (int i = 0; i < BR_PISACA; i++)
    {
        pthread_join(pisaci[i], nullptr);
    }
    for (int i = 0; i < BR_CITACA; i++)
    {
        pthread_join(citaci[i], nullptr);
    }
    for (int i = 0; i < BR_BRISACA; i++)
    {
        pthread_join(brisaci[i], nullptr);
    }
    return 0;
}
