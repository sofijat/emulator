#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

using namespace std;

typedef struct page_t {
	uint32_t base; //u emuliranoj memoriji
	string flags; //RW
	uint8_t* mem; //pokazivac na alociranu memoriju u emulatoru

	page_t() : base(0), flags(""), mem(0) {}
};

static int PAGE_SIZE = 4096;
class State;

page_t* alloc_page(); //alocira jedan deskriptor
void add_page(vector<page_t*>& pagetab, page_t* page); // dodaje page u tabelu
page_t* get_last_entry(); //dohvata poslednji deskriptor koji je dodat

uint8_t read_byte(vector<page_t*>& pagetab, uint32_t addr);//pretrazuje se lista za polje base koje odgovara adresi
uint8_t read_byte_inter(uint32_t addr, State* state);

int write_byte(vector<page_t*>& pagetab, uint32_t addr, uint8_t num);
int write_byte_inter(vector<page_t*>& pagetab,uint32_t addr, uint8_t num);

uint16_t read_word(vector<page_t*>& pagetab, uint32_t addr);
int write_word(vector<page_t*>& pagetab, uint32_t addr, uint16_t num);
uint32_t read_dword(vector<page_t*>& pagetab, uint32_t addr, bool& ret);
int write_dword(vector<page_t*>& pagetab, uint32_t addr, uint32_t num);
