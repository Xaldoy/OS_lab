#include <iostream>
#include <vector>
#include <random>
#include <Windows.h>
#include <iomanip>
#include <limits>

constexpr auto N = 5;
constexpr auto M = 7;

constexpr auto ADR_SIZE = 16;

std::random_device rd;
std::default_random_engine eng(rd());

class Okvir
{
public:
	int64_t content;
	int proces_id;
	int16_t* tablica_ref;
	Okvir(int64_t _content, int _proces_id, int16_t* _tablica_ref) : content(_content), proces_id(_proces_id), tablica_ref(_tablica_ref) {}
	Okvir() : content(0), proces_id(0), tablica_ref(nullptr) {}
};

int64_t disk[N][ADR_SIZE];
Okvir spremnik[M];
int16_t tablica[N][ADR_SIZE];
int proces[N];

int t = 0;

uint16_t get_random()
{
	std::uniform_int_distribution<uint16_t> distr(0, UINT16_MAX);
	return distr(eng);
}

static uint16_t get_lru(int16_t zapis)
{
	return (zapis & 0x1F);
}

static bool is_prisutan(int16_t zapis)
{
	return (zapis & 0x20) >> 5;
}


static int pronadi_okvir_lru()
{
	int indeks_lru = 0;;

	for (int indeks = 0; indeks < M; indeks++)
	{
		if (t - get_lru(*spremnik[indeks].tablica_ref) > t - get_lru(*spremnik[indeks_lru].tablica_ref))
		{
			indeks_lru = indeks;
		}
	}

	Okvir okvir = spremnik[indeks_lru];

	std::cout << "\t\tIzbacujem stranicu " << std::hex << std::setfill('0') << std::showbase << indeks_lru << " iz procesa " << okvir.proces_id << std::endl;
	std::cout << "\t\tlru izbacene stranice: " << std::hex << std::setfill('0') << std::showbase << get_lru(*okvir.tablica_ref) << std::endl;

	return indeks_lru;
}

static std::pair<bool, int> pronadi_okvir()
{
	for (int indeks = 0; indeks < M; indeks++)
	{
		if (spremnik[indeks].tablica_ref == nullptr) return std::make_pair(false, indeks);
		if (!is_prisutan(*spremnik[indeks].tablica_ref))
		{
			return std::make_pair(true, indeks);
		}
	}

	return std::make_pair(true,pronadi_okvir_lru());
}


static int dohvati_fizicku_adresu(int proces_id, int logicka_adresa)
{
	uint16_t indeks_stranice = (logicka_adresa >> 6 & 0XF);
	int16_t& zapis_tablice = tablica[proces_id][indeks_stranice];

	int fizicka_adresa = (zapis_tablice & 0xFFC0) >> 6;

	if (!is_prisutan(zapis_tablice))
	{
		std::cout << "\tPromasaj!" << std::endl;
		std::pair <bool, int> pronadi_okvir_result = pronadi_okvir();
		fizicka_adresa = pronadi_okvir_result.second;
		std::cout << "\t\tdodijeljen okvir: " << std::hex << std::setfill('0') << std::showbase << fizicka_adresa << std::endl;
		if(pronadi_okvir_result.first)
			disk[proces_id][indeks_stranice] = spremnik[fizicka_adresa].content;
		int16_t zapis = fizicka_adresa << 6 | t | 0x20;
		zapis_tablice = zapis;
		spremnik[fizicka_adresa].tablica_ref = &zapis_tablice;
	}
	std::cout << "\tfiz. adresa: " << std::hex << std::setfill('0') << fizicka_adresa << std::endl;
	std::cout << "\tzapis tablice: " << std::hex << std::setfill('0') << std::showbase << zapis_tablice << std::endl;
	std::cout << "\tsadrzaj adrese: " << std::dec << disk[proces_id][indeks_stranice] << std::endl;

	return fizicka_adresa;
}

static int dohvati_sadrzaj(int proces_id, int fizicka_adresa)
{
	return (int)spremnik[fizicka_adresa].content;
}

static void zapisi_sadrzaj(int proces_id, int fizicka_adresa, int content)
{
	spremnik[fizicka_adresa].content = content;
	spremnik[fizicka_adresa].proces_id = proces_id;
}

void print_start_proces(int proces_id, int16_t log_adr)
{
	std::cout << "--------------------------" << std::endl;
	std::cout << "proces: " << std::dec << proces_id << std::endl;
	std::cout << "\tt: " << std::dec << t << std::endl;
	std::cout << "\tlog. adresa: " << std::hex << std::setfill('0') << std::showbase << log_adr << std::endl;
	std::cout << "\tadresa stranice: " << std::hex << std::setfill('0') << std::showbase << (log_adr >> 6 & 0XF) << std::endl;
}

void increment_time()
{
	t++;
	if (t < 32) return;

	t = 0;
	for (auto& tab : tablica)
	{
		for (auto& zapis : tab)
		{
			zapis &= 0xFFBF;
		}
	}
}

int main()
{
	for (int i = 0; i < N; i++)
	{
		proces[i] = i;
		std::fill(std::begin(tablica[i]), std::end(tablica[i]), 0);
	}

	while (true)
	{
		for (auto proces_id : proces)
		{
			uint16_t log_adr = get_random();
			print_start_proces(proces_id, log_adr);
			int fizicka_adresa = dohvati_fizicku_adresu(proces_id, log_adr);
			
			int content = dohvati_sadrzaj(proces_id, fizicka_adresa);
			content++;
			zapisi_sadrzaj(proces_id, fizicka_adresa, content);

			increment_time();
			Sleep(300);
			std::cout << std::endl << std::endl;
		}
	}
	return 0;
}