#pragma once
#include <vector>
#include <string>
#include <cstring>
#include <stack>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <chrono>
#include <sstream>
#include <condition_variable>
#include "memo.h"

using namespace std;

class Symbol { //moze biti SEG(sekcija) ili SYM(simbol)
public:
	bool isSEG;
	uint32_t num; //redni broj
	string name; //naziv simbola
	int32_t sec_num; // SYM: 0=externa, -1=absolute; a ko SEG je isto kao num
	uint32_t addr; //starts with 0x
	char flag; //G-global, L-local, kod SYM
	uint32_t sec_size; //velicina sekcije, nema kod SYM, samo kod SEG
	string flags; //W-writeable, R-readable
	Symbol(string n, uint32_t address) : isSEG(false), num(0), name(n), sec_num(0), addr(address), flag('L'), sec_size(0), flags("") {} //za SEG
	Symbol(string n, uint32_t address, int32_t sec) : isSEG(false), num(0), name(n), sec_num(-1), addr(address), flag('C'), sec_size(0), flags("") {} //za SYM
};

static int location_counter = 0;

class RelSymbol {
public:
	uint32_t addr; //adresa koju treba prepraviti, pocinje sa 0x
	char type; //A-absolute, R-relative
	uint32_t sym_num; //redni broj
	RelSymbol(uint32_t address, char t, uint32_t sym) : addr(address), type(t), sym_num(sym) {}
};

class RelTable {
public:
	std::vector<RelSymbol*> table;
	int br_sekcije; //redni broj sekcije za koju se pravi RElTAble
	std::vector<string> section_content; //pravi se posle drugog prolaza, ima velicinu sec_size
	RelTable(int num) {
		br_sekcije = num;
	}
};


class State {
public:
	uint32_t pc;
	uint32_t sp;
	vector<page_t*> pages;
	uint32_t regs[16];
	bool running;
	bool interrupted;
	bool wait_input;
	bool is_read;
	bool key_pressed;

	mutex mx;
	condition_variable uslovna;

	State() : pc(0), sp(0), running(true), interrupted(false), wait_input(false), is_read(true), key_pressed(false), mx(), uslovna() {}

};

class Core {
public:
	State* state;
};





int add_to_table(std::vector<Symbol*>& symtable, string name, uint32_t adr, vector<string>& greske);
Symbol* get_symbol(std::vector<Symbol*>& symtable, const string name);
bool exists_by_name(string name, std::vector<Symbol*>& symtab);
void add_rel_symbol(RelTable& tab, RelSymbol* rs);
RelTable* get_reltab_for_section(int sec_num, vector<RelTable*>& reltables);
void add_rel_table(int num, vector<RelTable*>& reltables);
void add_content_to_table(RelTable& tab, string cont);
void free_rel_tables(vector<RelTable*>& rt);
void to_lower(string& str);
void readFile(char* input, vector<Symbol*>& symtab, vector<RelTable*>& reltabs, vector<string>& greske);
void split(const string &s, char* delim, vector<string> &v);
void emulate(Core core);
void aritm_op(uint32_t instruction1, State& state);
void not_op(uint32_t instruction1, State& state);
void branch_op(uint32_t instruction1, State& state);
void push_op(uint32_t instruction1, State& state);
void pop_op(uint32_t instruction1, State& state);
void jmpcall_op(uint32_t instruction1, State& state);
void int_op(uint32_t instruction1, int num_interrupt, State& state);
void ret_op(uint32_t instruction1, State& state);
void load_op(uint32_t instruction1, State& state);
void store_op(uint32_t instruction1, State& state);
int prepare_emulation(vector<Symbol*>& symtab, vector<RelTable*>& reltabs, State& state, vector<string>& greske);
void delete_char(string &s, char c);
char *my_strdup(const char *str);