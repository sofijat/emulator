#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stack>
#include <thread>
#include "memo.h"
#include "emulator.h"

using namespace std;



int main(int argc, char* argv[]) {
	
	State* state = new State();
	Core core;
	core.state = state;
	vector<RelTable*> reltabs;
	vector<Symbol*> symtab;
	vector<string> has_content;
	vector<string> greske;
	
	
	readFile(argv[1], symtab, reltabs, greske);
	
	
	Symbol* startni = get_symbol(symtab, "START");
	if (startni == NULL) {
		greske.push_back("Nije definisan simbol START");
		for (int n = 0; n < greske.size(); n++) {
			ofstream errout("errors.txt");
			errout << greske[n] << "\n";
		}
		return 1;
	}
	if (startni->flag == 'L') {
		cout << "Start mora da bude globalan!";
		for (int n = 0; n < greske.size(); n++) {
			ofstream errout("errors.txt");
			errout << greske[n] << "\n";
		}
		return 1;
	}
	int prep = prepare_emulation(symtab, reltabs, *core.state, greske);
	if (prep != 0) {
		greske.push_back("Priprema emulacije nije uspela");
	}
	bool ret;
	uint32_t sp = read_dword(state->pages, 0x0, ret);

	//isto mogu neke greske za sp

	uint32_t startadr = startni->addr;
	state->pc = startadr;

	state->sp = sp;
	//oslobodi reltab i symtab
	if (greske.size() > 0) {
		ofstream errout("errors.txt");
		for (int n = 0; n < greske.size(); n++) {
			errout << greske[n] << "\n";
		}
	}

	thread t1(emulate, core);
	thread t2(emulate, core);
	t1.join();
	t2.join();

	for (int i = 0; i < 16; i++) {
		string br = (state->regs[i] != 3452816845) ? to_string(state->regs[i]) : "empty";
		cout <<"R"<<i<<"="<<br << " ";
	}
	cout << '\n';

	for (int i = 0; i < symtab.size(); i++) {
		delete symtab[i];
	}
	for (int i = 0; i < reltabs.size(); i++) {
		for (int j = 0; j < reltabs[i]->table.size(); j++) {
			delete reltabs[i]->table[j];
		}
	}

	return 0;
}

