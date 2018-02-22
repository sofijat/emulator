#include "memo.h"
#include "emulator.h"

page_t* alloc_page() { //trebaalocirati i PAGE_SIZE memorije i mem da pokazuje na to
	page_t* page = new page_t();
	if (page == NULL) {
		return NULL;
	}
	return page;
}


void add_page(vector<page_t*>& pagetab, page_t* page) { //pagetab nece biti poredjan po base
	pagetab.push_back(page);
	return;
}

page_t* get_last_entry(vector<page_t*>& pagetab) {
	return pagetab.back();
}

uint8_t read_byte(vector<page_t*>& pagetab, uint32_t addr) {
	uint8_t byte = 0;
	for (int i = 0; i < pagetab.size(); i++) {
		page_t* pg = pagetab[i];
		if (pg->base <= addr && addr < pg->base + PAGE_SIZE) {
			addr &= (PAGE_SIZE - 1);
			byte = pg->mem[addr];
			break;
		}
	}
	return byte;
}

uint8_t read_byte_inter(uint32_t addr, State* state) {
	uint8_t byte = 0;
	
		if (addr == 0x84) {
			if (state->is_read == false) {
				state->is_read = true;
				state->uslovna.notify_one();
			}
		}
	
	for (int i = 0; i < state->pages.size(); i++) {
		page_t* pg = state->pages[i];
		if (pg->base <= addr && addr < pg->base + PAGE_SIZE) {
			addr &= (PAGE_SIZE - 1);
			byte = pg->mem[addr];
			break;
		}
	}
	return byte;
}

int write_byte(vector<page_t*>& pagetab, uint32_t addr, uint8_t num) {
	int ret = -1;
	for (int i = 0; i < pagetab.size(); i++) {
		page_t* pg = pagetab[i];
		if (pg->base <= addr && addr < pg->base + PAGE_SIZE) {
			addr &= (PAGE_SIZE - 1);
			pg->mem[addr] = num;
			ret = 0;
			break;
		}
	}
	return ret;
}

int write_byte_inter(vector<page_t*>& pagetab, uint32_t addr, uint8_t num) {
	int ret = -1;
	if (addr == 0x80) {
		//printf("%c", (char)num);
		fflush(stdout);
	}
	for (int i = 0; i < pagetab.size(); i++) {
		page_t* pg = pagetab[i];
		if (pg->base <= addr && addr < pg->base + PAGE_SIZE) {
			addr &= (PAGE_SIZE - 1);
			if (pg->flags.find('W') != string::npos) {
				pg->mem[addr] = num;
				ret = 0;
			}
			break;
		}
	}
	return ret;
}

uint16_t read_word(vector<page_t*>& pagetab, uint32_t addr) {
	uint16_t word = 0;

	for (int i = 0; i < pagetab.size(); i++) {
		page_t* page = pagetab[i];
		if (page->base <= addr && addr < page->base + PAGE_SIZE) {
			uint32_t ad = addr&(PAGE_SIZE - 1);
			if (addr + 1 >= page->base + PAGE_SIZE) continue;
			uint8_t first = page->mem[ad++];
			uint8_t second = page->mem[ad++];
			word = (second << 8) | (first);
		}
	}
	return word;
}

int write_word(vector<page_t*>& pagetab, uint32_t addr, uint16_t num) {
	int ret = -1;
	uint32_t ad = addr&(PAGE_SIZE - 1);

	for (int i = 0; i < pagetab.size(); i++) {
		page_t* pg = pagetab[i];
		if (pg->base <= addr && addr < pg->base + PAGE_SIZE) {
			if ((pg->flags).find('W') != string::npos) {
				uint8_t second = (num >> 8) & 0xFF;
				uint8_t first = num & 0xFF;
				pg->mem[ad++] = first;
				if (addr + 1 >= pg->base + PAGE_SIZE) continue;
				pg->mem[ad++] = second;
				ret = 0;
				break;
			}
		}
	}
	return ret;
}

uint32_t read_dword(vector<page_t*>& pagetab, uint32_t addr, bool& ret) {
	uint32_t word = 0;
	uint32_t ad = addr&(PAGE_SIZE - 1);
	ret = false;
	for (int i = 0; i < pagetab.size(); i++) {
		page_t* page = pagetab[i];
		if (page->base <= addr && addr < page->base + PAGE_SIZE) {
			uint8_t first = page->mem[ad++];
			if (addr + 1 >= page->base + PAGE_SIZE) continue;

			uint8_t second = page->mem[ad++];
			if (addr + 1 >= page->base + PAGE_SIZE) continue;

			uint8_t third = page->mem[ad++];
			if (addr + 1 >= page->base + PAGE_SIZE) continue;

			uint8_t fourth = page->mem[ad++];
			word = (fourth << 24) | (third << 16) | (second << 8) | (first);
			ret = true;
		}
	}

	return word;
}

int write_dword(vector<page_t*>& pagetab, uint32_t addr, uint32_t num) {
	uint32_t ad = addr&(PAGE_SIZE - 1);

	for (int i = 0; i < pagetab.size(); i++) {
		page_t* page = pagetab[i];
		if (page->base <= addr && addr < page->base + PAGE_SIZE) {
			if (page->flags.find('W') != string::npos) {
				uint8_t first = num & 0xFF;
				uint8_t second = (num >> 8) & 0xFF;
				uint8_t third = (num >> 16) & 0xFF;
				uint8_t fourth = (num >> 24) & 0xFF;

				page->mem[ad++] = first;
				if (addr + 1 >= page->base + PAGE_SIZE) continue;
				page->mem[ad++] = second;
				if (addr + 2 >= page->base + PAGE_SIZE) continue;
				page->mem[ad++] = third;
				if (addr + 3 >= page->base + PAGE_SIZE) continue;
				page->mem[ad++] = fourth;
				return 0;
			}
		}
	}
	return -1;
}