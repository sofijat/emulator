#include "emulator.h"

int add_to_table(std::vector<Symbol*>& symtable, string name, uint32_t adr, vector<string>& greske) {
	string n;
	n = name;
	if (n == "") {
		greske.push_back("Nije moguce ubaciti simbol bez naziva u tabelu simbola");
		return -1;
	} 
	Symbol* sym = new Symbol(n, adr);
	sym->name = name;
	//sym->num = ++state.index;

	symtable.push_back(sym);
	return 0; //sve ok
}

void delete_char(string &s, char c) {
	string novi = "";
	for (int i = 0; i < s.length(); i++) {
		if (s[i] != c) {
			novi += s[i];
		}
	}
	s = novi;
}

char *my_strdup(const char *str) {
	size_t len = strlen(str);
	char *x = (char *)malloc(len + 1); /* 1 for the null terminator */
	if (!x) return NULL; /* malloc could not allocate memory */
	memcpy(x, str, len + 1); /* copy the string into the new buffer */
	return x;
}

Symbol* get_symbol(std::vector<Symbol*>& symtable, const string name) {
	for (int i = 0; i < symtable.size(); i++) {
		if (name == symtable[i]->name) {
			return symtable[i];
		}
	}
	return NULL;
}

/*Symbol* get_symbol_by_num(std::vector<Symbol*>& symtable, uint32_t num) {
for (int i = 0; i < symtable.size(); i++) {
if (num == symtable[i]->num) {
return symtable[i];
}
}
return NULL;
}*/

bool exists_by_name(string name, std::vector<Symbol*>& symtab) {
	int ret = false;
	for (int i = 0; i < symtab.size(); i++) {
		if (symtab[i]->name == name) {
			ret = true;
			break;
		}
	}
	return ret;
}

void add_rel_symbol(RelTable& tab, RelSymbol* rs) {
	if (rs != NULL) {
		tab.table.push_back(rs);
	}
	else return;
}

RelTable* get_reltab_for_section(int sec_num, vector<RelTable*>& reltables) {
	for (int i = 0; i < reltables.size(); i++) {
		if (reltables[i]->br_sekcije == sec_num) return reltables[i];
	}
	return NULL;
}

void add_rel_table(int num, vector<RelTable*>& reltables) {
	RelTable* rt = new RelTable(num);
	if (rt != NULL) reltables.push_back(rt);
}

void add_content_to_table(RelTable& tab, string cont) {
	if (!cont.size() == 0) {
		tab.section_content.push_back(cont);
	}
}

void free_rel_tables(vector<RelTable*>& rt) {
	if (rt.size() == 0) return;
	for (int i = 0; i < rt.size(); i++) {
		for (int j = 0; j < rt[i]->table.size(); j++) {
			RelSymbol* rs = rt[i]->table[j];
			delete rs;
		}
	}
}

void to_lower(string& str) {
	for (int i = 0; i < str.size(); i++) {
		char r = tolower(str[i]);
		str[i] = r;
	}
}

void readFile(char* input, vector<Symbol*>& symtab, vector<RelTable*>& reltabs, vector<string>& greske) { //formira SymTab, RelTables, content
	ifstream infile(input);
	string line;
	if (!infile) {
		greske.push_back("Neuspesno otvaranje fajla");
		return;
	}

	vector<vector<string>> vect;
	while (getline(infile, line)) {
		vector<string> tokens;
		split(line, "\t\n\v\r ", tokens); //mozda i neki \v\r

		if (tokens.size() == 0) continue;
		vect.push_back(tokens);
	}

	/*for (int k = 0; k < vect.size(); k++) {
		for (int l = 0; l < vect[k].size(); l++) {
			cout << vect[k][l] << " | ";
		}
		cout << "\n";
	}*/

	for (int i = 0; i < vect.size(); i++)
		for (int j = 0; j < vect[i].size(); j++) {
			delete_char(vect[i][j], '\r');
			delete_char(vect[i][j], '\v');
			delete_char(vect[i][j], '\n');
			delete_char(vect[i][j], '\t');
		}

	bool in_symtable = false;
	bool in_relcont = false;

	if (vect.size() == 0) return;
	for (int i = 0; i < vect.size(); i++) {
		if (vect[i][0][0] == '#') {
			if (vect[i][0] == "#TabelaSimbola") {
				in_symtable = true;
				in_relcont = false;
			}
			else if (vect[i][0] == "#end") break;
			else {
				in_symtable = false;
				in_relcont = false;
				uint32_t pos = vect[i][0].find(".");
				string naz = "";
				if (pos != string::npos) naz = vect[i][0].substr(pos);
				Symbol* sym = get_symbol(symtab, naz);
				RelTable* rt = NULL;
				if (sym != NULL)rt = new RelTable(sym->sec_num);
				if (rt != NULL) reltabs.push_back(rt);
			}
		}
		else if (vect[i][0][0] == '.') {
			in_relcont = true;
			in_symtable = false;
		}

		else { //ako nema #'

			if (in_symtable == true) {
				bool isSEG;
				uint32_t num; //redni broj
				string name; //naziv simbola
				int32_t sec_num; // SYM: 0=externa, -1=absolute; a ko SEG je isto kao num
				uint32_t addr; //starts with 0x
				char flag; //G-global, L-local, kod SYM
				uint32_t sec_size; //velicina sekcije, nema kod SYM, samo kod SEG
				string flags; //W-writeable, R-readable, O-ORGed
				if (vect[i][0] == "SEG") {
					isSEG = true;
					num = stoi(vect[i][1]);
					name = vect[i][2];
					sec_num = stoi(vect[i][3]);
					addr = strtol(vect[i][4].c_str(), NULL, 16); //ili conert to num
					sec_size = strtol(vect[i][5].c_str(), NULL, 16);
					flags = vect[i][6];
					add_to_table(symtab, name, addr, greske);
					Symbol* symb = get_symbol(symtab, name);
					symb->flags = flags;
					symb->sec_num = sec_num;
					symb->sec_size = sec_size;
					symb->isSEG = isSEG;
					symb->num = num;
				}
				else if (vect[i][0] == "SYM") {
					isSEG = false;
					num = stoi(vect[i][1]);
					name = vect[i][2];
					sec_num = stoi(vect[i][3]);
					addr = strtol(vect[i][4].c_str(), NULL, 16);
					const char *ch = vect[i][5].c_str();
					flag = ch[0];
					add_to_table(symtab, name, addr, greske);
					Symbol* symb = get_symbol(symtab, name);
					symb->sec_num = sec_num;
					symb->isSEG = isSEG;
					symb->num = num;
					symb->flag = flag;
				}
			}
			else if (in_symtable == false && in_relcont == false) { //ako je simbol iz reltab
				uint32_t addr; //adresa koju treba prepraviti, pocinje sa 0x
				char type; //A-absolute, R-relative
				uint32_t sym_num; //redni broj
				addr = strtol(vect[i][0].c_str(), NULL, 16);
				const char* ch = vect[i][1].c_str();
				type = ch[0];
				sym_num = stoi(vect[i][2]);
				RelSymbol* rs = new RelSymbol(addr, type, sym_num);
				RelTable* rt = reltabs.back();
				add_rel_symbol(*rt, rs);
			}
			else if (in_relcont == true && in_symtable == false) {//sadrzaj reltab

				RelTable* rt = reltabs.back();
				for (int j = 0; j < vect[i].size(); j++) {
					string byte = vect[i][j];
					add_content_to_table(*rt, byte);
				}
			}
		}
	}
}

void split(const string &s, char* delim, vector<string> &v) {
	char * dup = my_strdup(s.c_str());
	bool first = true;
	char * token;
	char* delimfirst = delim;

	token = strtok(dup, delimfirst);
	if (token != NULL) v.push_back(string(token));
	else return;

	while (token != NULL) {
		token = strtok(NULL, delimfirst);//def 5
		if (token != NULL) {
			v.push_back(token);
		}
		else return;
	}
}

void emulate(Core core) {
	State* state = core.state;
	uint32_t instruction1;
	char c;
	uint8_t opcode;
	auto x = chrono::steady_clock::now();
	bool ret;

	while (state->running == true) {
		unique_lock<mutex> lck(state->mx);
		instruction1 = read_dword(state->pages, state->pc, ret);
		if (ret == false) {
			state->running = false;
			continue;
		}
		state->pc += 4;
		opcode = (instruction1 >> 24) & 0xFF;

		char str[100];
		sprintf(str, "%x", opcode);
		opcode = strtol(str, NULL, 10);

		if (opcode == 30 || opcode == 31 || opcode == 32 || opcode == 33 || opcode == 34 || opcode == 35 || opcode == 36 || opcode == 37 || opcode == 39 || opcode == 0x3A) {
			aritm_op(instruction1, *state);
		}
		else if (opcode == 38) {
			not_op(instruction1, *state);
		}
		else if (opcode == 4 || opcode == 5 || opcode == 6 || opcode == 7 || opcode == 8 || opcode == 9) {
			branch_op(instruction1, *state);
		}
		else if (opcode == 20) {
			push_op(instruction1, *state);
		}
		else if (opcode == 21) {
			pop_op(instruction1, *state);
		}
		else if (opcode == 2 || opcode == 3) {
			jmpcall_op(instruction1, *state);
		}
		else if (opcode == 0) {
			int r0 = (instruction1 >> 16) & 0x1F;
			if (state->regs[r0] == 0) {
				state->running = false;
				continue;
			}
			int_op(instruction1, -1, *state);
			continue;
		}
		else if (opcode == 1) {
			ret_op(instruction1, *state);
		}
		else if (opcode == 10) {
			load_op(instruction1, *state);
		}
		else if (opcode == 11) {
			store_op(instruction1, *state);
		}
		else {
			int_op(0, 3, *state);
			//GRESKA, kod operacije ne odgovara nijednom postojecem
			//break;
		}

		if (state->wait_input == false) {
			//unique_lock<mutex> lck(state->mx);
			state->wait_input = true;
			if (state->is_read == false) state->uslovna.wait(lck);
			
			lck.unlock();  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			c=getchar();
			lck.lock();
			
			state->is_read = false;
			int wr=write_byte_inter(state->pages, 0x84, c);
			state->wait_input = false;
			if (c == '\n') {
				state->is_read = true;
				state->running = false;
				core.state->uslovna.notify_all();
				//continue;
			}
			else {
				state->key_pressed = true;
				state->running = false;
				read_byte_inter(0x84, core.state);
			}

		}
		if (state->interrupted == false && opcode != 1) {
			if (state->key_pressed == true) {
				state->key_pressed = false;
				
				int_op(0, 5, *state);
			}
			else {
				auto new_x = chrono::steady_clock::now();
				if (chrono::time_point_cast<chrono::microseconds>(new_x) - chrono::time_point_cast<chrono::microseconds>(x) > chrono::microseconds(100)) {
					x = chrono::steady_clock::now();
					//cout << "timer\n";
					int_op(0, 4, *state);
				}
			}
		}
	}
}


void aritm_op(uint32_t instruction1, State& state) {
	int r0, r1, r2;
	r0 = (instruction1 >> 16) & 0x1F;
	r1 = (instruction1 >> 11) & 0x1F;
	r2 = (instruction1 >> 6) & 0x1F;
	uint8_t opcode = (instruction1 >> 24) & 0xFF;
	char str[100];
	sprintf(str, "%x", opcode);
	opcode = strtol(str, NULL, 10);

	if (opcode == 30) {
		state.regs[r0] = state.regs[r1] + state.regs[r2];
	}
	else if (opcode == 31) {
		state.regs[r0] = state.regs[r1] - state.regs[r2];
	}
	else if (opcode == 32) {
		state.regs[r0] = state.regs[r1] * state.regs[r2];
	}
	else if (opcode == 33) {
		state.regs[r0] = state.regs[r1] / state.regs[r2];
	}
	else if (opcode == 34) {
		state.regs[r0] = state.regs[r1] % state.regs[r2];
	}
	else if (opcode == 35) {
		state.regs[r0] = state.regs[r1] & state.regs[r2];
	}
	else if (opcode == 36) {
		state.regs[r0] = state.regs[r1] | state.regs[r2];
	}
	else if (opcode == 37) {
		state.regs[r0] = state.regs[r1] ^ state.regs[r2];
	}
	else if (opcode == 39) {
		state.regs[r0] = state.regs[r1] << state.regs[r2];
	}
	else if (opcode == 0x3A) {
		state.regs[r0] = state.regs[r1] >> state.regs[r2];
	}
}


void not_op(uint32_t instruction1, State& state) {
	int r0, r1;
	r0 = (instruction1 >> 16) & 0x1F;
	r1 = (instruction1 >> 11) & 0x1F;
	state.regs[r0] = ~state.regs[r1];
}

void branch_op(uint32_t instruction1, State& state) {
	int r0, r1;
	uint32_t opcode = (instruction1 >> 24) & 0xFF;
	char str[100];
	sprintf(str, "%x", opcode);
	opcode = strtol(str, NULL, 10);
	uint32_t address;
	r0 = (instruction1 >> 16) & 0x1F;
	r1 = (instruction1 >> 11) & 0x1F;
	uint8_t addr_mode = (instruction1 >> 21) & 0x7;
	bool ret;

	if (addr_mode == 6) {//memdir
		address = read_dword(state.pages, state.pc, ret);
		state.pc += 4;
	}
	else if (addr_mode == 2) { //regind
		address = state.regs[r0];
		address = read_dword(state.pages, address, ret);
	}
	else if (addr_mode == 7) { //regindpom
		address = read_dword(state.pages, state.pc, ret);
		state.pc += 4;
		if (r0 == 16) address += state.pc;
		else if (r0 == 17) address += state.pc;
		else address += state.regs[r0];
		address = read_dword(state.pages, address, ret);
	}

	if (opcode == 4) {
		if (state.regs[r1] == 0) state.pc = address;
	}
	else if (opcode == 5) {
		if (state.regs[r1] != 0) state.pc = address;
	}
	else if (opcode == 6) {
		if (state.regs[r1] > 0) state.pc = address;
	}
	else if (opcode == 7) {
		if (state.regs[r1] >= 0) state.pc = address;
	}
	else if (opcode == 8) {
		if (state.regs[r1] < 0) state.pc = address;
	}
	else if (opcode == 9) {
		if (state.regs[r1] <= 0) state.pc = address;
	}
}

void push_op(uint32_t instruction1, State& state) {
	int r0;
	r0 = (instruction1 >> 16) & 0x1F;
	state.sp += 1;
	write_dword(state.pages, state.sp, state.regs[r0]);
	state.sp += 3;
}


void pop_op(uint32_t instruction1, State& state) {
	int r0;
	bool ret;
	r0 = (instruction1 >> 16) & 0x1F;
	state.sp -= 3;
	state.regs[r0] = read_dword(state.pages, state.sp, ret);
	state.sp -= 1;
}

void jmpcall_op(uint32_t instruction1, State& state) {
	int r0;
	bool ret;
	uint32_t opcode = (instruction1 >> 24) & 0xFF;
	char str[100];
	sprintf(str, "%x", opcode);
	opcode = strtol(str, NULL, 10);
	uint32_t address;
	r0 = (instruction1 >> 16) & 0x1F;
	uint8_t addr_mode = (instruction1 >> 21) & 0x7;

	if (addr_mode == 6) {//memdir
		address = read_dword(state.pages, state.pc, ret);
		state.pc += 4;
	}
	else if (addr_mode == 2) { //regind
		address = state.regs[r0];
		address = read_dword(state.pages, address, ret);
	}
	else if (addr_mode == 7) { //regindpom
		address = read_dword(state.pages, state.pc, ret);
		state.pc += 4;
		if (r0 == 16) address += state.pc;
		else if (r0 == 17) address += state.pc;
		else address += state.regs[r0];
		address = read_dword(state.pages, address, ret);
	}
	if (opcode == 2) {//jmp
		state.pc = address;
	}
	else if (opcode == 3) {//call, pc ide na stek
		state.sp += 1;
		write_dword(state.pages, state.sp, state.pc);
		state.sp += 3;
		state.pc = address;
	}
}

void int_op(uint32_t instruction1, int num_interrupt, State& state) {
	if (num_interrupt == -1) {
		int r0 = (instruction1 >> 16) & 0x1F;
		num_interrupt = state.regs[r0];
	}
	bool ret;
	state.interrupted = true;
	state.sp += 1;
	write_dword(state.pages, state.sp, state.pc);
	state.sp += 3;
	state.pc = read_dword(state.pages, num_interrupt * 4, ret);
}

void ret_op(uint32_t instruction1, State& state) {
	uint32_t ret_adr;
	bool ret;
	state.sp -= 3;
	ret_adr = read_dword(state.pages, state.sp, ret);
	state.sp -= 1;
	state.pc = ret_adr;
	if (state.interrupted == true) state.interrupted = false;
}

void store_op(uint32_t instruction1, State& state) {
	bool ret;
	int r0, r1;
	uint8_t type;
	uint32_t opcode = (instruction1 >> 24) & 0xFF;
	r0 = (instruction1 >> 16) & 0x1F;
	r1 = (instruction1 >> 11) & 0x1F;
	uint8_t addr_mode = (instruction1 >> 21) & 0x7;
	type = (instruction1 >> 3) & 0x7;

	if (addr_mode == 0) {//regdir
		state.regs[r0] = state.regs[r1];
	}
	else if (addr_mode == 6) {//memdir
		uint32_t value = read_dword(state.pages, state.pc, ret);
		state.pc += 4;
		if (type == 0) write_dword(state.pages, value, state.regs[r1]);
		else if (type == 1) write_word(state.pages, value, state.regs[r1]);
		else if (type == 2) write_byte_inter(state.pages, value, state.regs[r1]);
	}
	else if (addr_mode == 2) { //regind
		if (type == 0) write_dword(state.pages, state.regs[r0], state.regs[r1]);
		else if (type == 1) write_word(state.pages, state.regs[r0], state.regs[r1]);
		else if (type == 2) write_byte_inter(state.pages, state.regs[r0], state.regs[r1]);
	}
	else if (addr_mode == 7) {
		uint32_t value = read_dword(state.pages, state.pc, ret);
		state.pc += 4;
		if (r0 == 16) value += state.sp;
		else if (r0 == 17) value += state.pc;
		else value += state.regs[r0];

		if (type == 0) write_dword(state.pages, value, state.regs[r1]);
		else if (type == 1) write_word(state.pages, value, state.regs[r1]);
		else if (type == 2) write_byte_inter(state.pages, value, state.regs[r1]);
	}
}

void load_op(uint32_t instruction1, State& state) {
	int r0, r1;
	uint8_t type;
	uint32_t opcode = (instruction1 >> 24) & 0xFF;
	r0 = (instruction1 >> 16) & 0x1F;
	r1 = (instruction1 >> 11) & 0x1F;
	uint8_t addr_mode = (instruction1 >> 21) & 0x7;
	type = (instruction1 >> 3) & 0x7;
	uint32_t value;
	bool ret;

	if (addr_mode == 4) {//immed
		value = state.pc;
		if (type == 0) state.regs[r1] = read_dword(state.pages, value, ret);
		else if (type == 1 || type == 5)state.regs[r1] = read_word(state.pages, value);
		else if (type == 7 || type == 3)state.regs[r1] = read_byte_inter(value, &state);
		state.pc += 4;
	}
	else if (addr_mode == 0) {//regdir
		if (r0 == 16) state.regs[r1] = state.sp;
		else state.regs[r1] = state.regs[r0];
	}
	else if (addr_mode == 6) {//memdir
		uint32_t value = read_dword(state.pages, state.pc, ret);
		state.pc += 4;
		if (type == 0) state.regs[r1] = read_dword(state.pages, value, ret);
		else if (type == 1 || type == 5)state.regs[r1] = read_word(state.pages, value);
		else if (type == 7 || type == 3)state.regs[r1] = read_byte_inter(value, &state);
	}
	else if (addr_mode == 2) {//regind
		if (type == 0) state.regs[r1] = read_dword(state.pages, state.regs[r0], ret);
		else if (type == 1 || type == 5)state.regs[r1] = read_word(state.pages, state.regs[r0]);
		else if (type == 7 || type == 3)state.regs[r1] = read_byte_inter(value, &state);
	}
	else if (addr_mode == 7) {//regindpom
		uint32_t value = read_dword(state.pages, state.pc, ret);
		state.pc += 4;
		if (r0 == 16) value += state.pc;
		else if (r0 == 17) value += state.sp;
		else value += state.regs[r0];

		if (type == 0) state.regs[r1] = read_dword(state.pages, value, ret);
		else if (type == 1 || type == 5)state.regs[r1] = read_word(state.pages, value);
		else if (type == 7 || type == 3)state.regs[r1] = read_byte_inter(value, &state);
	}
}


int prepare_emulation(vector<Symbol*>& symtab, vector<RelTable*>& reltabs, State& state, vector<string>& greske) {
	for (int i = 0; i < symtab.size(); i++) {
		if (symtab[i]->sec_num == 0) {
			greske.push_back("Simbol nije definisan");
			return -1;
		}
	}
	page_t* first_page;
	int ret = 0;


	for (int i = 0; i < symtab.size(); i++) {
		if (symtab[i]->isSEG == true) {
			//ovo je sve ako nadje O
			int segment_page = symtab[i]->addr & 0xFFFFF000;
			if (symtab[i]->flags.find('O') != string::npos) {

				int pages_needed;
				uint32_t base;
				RelTable* rel = NULL;

				for (int j = 0; j < symtab.size(); j++) {
					if (symtab[j]->isSEG == true && symtab[j]->flags.find('O') != string::npos) {
						if (j != i) {
							int segment_page2 = symtab[j]->addr & 0xFFFFF000;
							if (segment_page == segment_page2) {
								greske.push_back("Adrese dve sekcije se poklapaju");
								return -1;
							}
						}
					}
				}

				base = symtab[i]->addr & 0xFFFFF000;
				int page_taken = (symtab[i]->addr & 0x0FFF) + symtab[i]->sec_size;
				pages_needed = (int)(page_taken / PAGE_SIZE) + 1;



				vector<page_t*> cur_pages;
				for (int j = 0; j < pages_needed; j++) {
					page_t* page = alloc_page();
					page->flags = symtab[i]->flags;

					page->base = base + j*PAGE_SIZE;
					add_page(state.pages, page);
					page->mem = new uint8_t[PAGE_SIZE]; //ili uint8_t
					cur_pages.push_back(page);

				}

				int page_index = 0;
				first_page = cur_pages[page_index];

				//ako je u pitanju sadrzaj reltable, treba da ga prepise u memoriju


				for (int j = 0; j < reltabs.size(); j++) {
					if (reltabs[j]->br_sekcije == symtab[i]->sec_num) {
						rel = reltabs[j];
						break;
					}
				}

				if (rel != NULL) {
					uint32_t k;
					k = symtab[i]->addr;

					if (rel->section_content.size()>0) {
						for (int l = 0; l < symtab[i]->sec_size; l++) {
							first_page->mem[k] = strtol(rel->section_content[l].c_str(), NULL, 16); //mora konverzija, jer je meni content=string
							k++;
							if (k >= PAGE_SIZE) {
								k = 0;
								first_page = cur_pages[++page_index]; //mora sledeca stranica kad se jedna popuni
							}
						}
					}
				}
				for (int j = 0; j < symtab.size(); j++) {
					if (symtab[j]->isSEG == false && symtab[j]->flag == 'G' && symtab[j]->sec_num == symtab[i]->sec_num) {
						symtab[j]->addr += symtab[i]->addr;
					}
				}
			}

		}//if symtab[i]->isSEG
	}//for


	//BEZ ORG
	for (int i = 0; i < symtab.size(); i++) {
		if (symtab[i]->isSEG) {
			int segment_page = symtab[i]->addr & 0xFFFFF000;

			if (symtab[i]->flags.find('O') == string::npos) {
				int pages_needed;
				uint32_t base;
				RelTable* rel = NULL;

				page_t* last;
				if (state.pages.size()>0) {
					last = state.pages.back();
					base = (last->base & 0xFFFFF000) + PAGE_SIZE;
				}
				else base = 0;
				symtab[i]->addr = base;
				for (int j = 0; j < reltabs.size(); j++) {
					if (reltabs[j]->br_sekcije == symtab[i]->sec_num) {
						rel = reltabs[j];
						break;
					}
				}

				if (rel) {
					for (int j = 0; j < rel->table.size(); j++)
						rel->table[j]->addr += symtab[i]->addr;
				}
				pages_needed = (int)symtab[i]->sec_size / PAGE_SIZE + 1;


				vector<page_t*> cur_pages;
				for (int j = 0; j < pages_needed; j++) {
					page_t* page = alloc_page();
					page->flags = symtab[i]->flags;

					page->base = base + j*PAGE_SIZE;
					add_page(state.pages, page);
					page->mem = new uint8_t[PAGE_SIZE]; //ili uint8_t
					cur_pages.push_back(page);

				}

				int page_index = 0;
				first_page = cur_pages[page_index];

				if (rel) {
					uint32_t k;
					k = 0;

					if (rel->section_content.size()>0) {
						for (int l = 0; l < symtab[i]->sec_size; l++) {
							first_page->mem[k] = strtol(rel->section_content[l].c_str(), NULL, 16); //mora konverzija, jer je meni content=string
							k++;
							if (k >= PAGE_SIZE) {
								k = 0;
								first_page = cur_pages[++page_index]; //mora sledeca stranica kad se jena popuni
							}
						}
					}
				}
				for (int j = 0; j < symtab.size(); j++) {
					if (symtab[j]->isSEG == false && symtab[j]->flag == 'G' && symtab[j]->sec_num == symtab[i]->sec_num) {
						symtab[j]->addr += symtab[i]->addr;
					}
				}

			}
		}
	}



	for (int i = 0; i < symtab.size(); i++) {
		if (symtab[i]->isSEG == true) {
			page_t* page;
			Symbol* segm = symtab[i];

			RelTable* rt = NULL;
			for (int j = 0; j < reltabs.size(); j++) {
				if (reltabs[j]->br_sekcije == segm->sec_num) {
					rt = reltabs[j];
					break;
				}
			}
			if (rt == NULL) continue;
			for (int j = 0; j < rt->table.size(); j++) {
				uint32_t sym_num = rt->table[j]->sym_num;
				
				uint32_t addr = rt->table[j]->addr;
				Symbol* sym;
				for (int k = 0; k < symtab.size(); k++) {
					if (symtab[k]->num == sym_num) {
						sym = symtab[k];
						break;
					}

				}


				uint8_t first, second, third, fourth;
				first = read_byte(state.pages, addr);
				second = read_byte(state.pages, addr + 1);
				third = read_byte(state.pages, addr + 2);
				fourth = read_byte(state.pages, addr + 3);

				int32_t val = (fourth << 24) | (third << 16) | (second << 8) | (first << 0);
				val += sym->addr;
				if (rt->table[j]->type == 'R') val -= addr;
				write_byte(state.pages, addr, val & 0xFF);
				write_byte(state.pages, addr + 1, (val >> 8) & 0xFF);
				write_byte(state.pages, addr + 2, (val >> 16) & 0xFF);
				write_byte(state.pages, addr + 3, (val >> 24) & 0xFF);
			}

		}
	}
	return ret;
}