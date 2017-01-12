/// main.cpp
/// A test sample
/// NOT FOR STANDARD TESTS
/// DO NOT DISTRIBUTE
/// Encoding: UTF-8
/// Tab: 4

#include <cstdint>
#include <cstring>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <utility>
#include <memory>
#include <exception>

#define FN_PTR(name) void name (const string& l, const string& r)

bool running = true;

using namespace std;

class UD2 : public exception { 
	public: 
		const char* what() const noexcept{ 
			return "[X] Undefined Opcode\n";
		} 
};

typedef
struct Instruction{
	const char* name;
	uint8_t param_count;
	void (*fn_ptr)(const string&, const string&);
}
instruction_t;

instruction_t* get_op(shared_ptr<vector<instruction_t>> ins, 
					const char* name){
	instruction_t* ip = nullptr;
	for(auto i : *ins){
		if(strncmp(i.name,name,6) == 0){
			ip = &i;
			return ip;
		};
	}
	throw UD2(); 
}

/// vvv functions vvv

FN_PTR(nop){
	return;	
}

FN_PTR(add){
	
}

FN_PTR(sub){
	
}

FN_PTR(mul){

}

FN_PTR(div){

}

FN_PTR(push){

}

FN_PTR(pop){

}

FN_PTR(hlt){
	cout << "HLT" << endl;
	running = false;
}

FN_PTR(print){
	// PRINT str
	// PRINT reg
	switch(l[0]){
		case '%':
			// register
			
			break;
		case '$':
			// string immediates

			break;
		case '@':
			// pointer
			
			break;
		default:
			cerr << "Syntax error: Bad parameter to PRINT." << endl;
			break;
	}
}

/// ^^^ functions ^^^

void fn_init(shared_ptr<vector<instruction_t>> ins){
	ins->push_back({"NOP", 0, nop});
	ins->push_back({"ADD", 2, add});
	ins->push_back({"SUB", 2, sub});
	ins->push_back({"MUL", 2, mul});
	ins->push_back({"DIV", 2, div});
	ins->push_back({"PUSH", 1, push});
	ins->push_back({"POP", 1, pop});
	ins->push_back({"HLT", 0, hlt});
	ins->push_back({"PRINT", 1, print});
}

void split(const string& s, char d, vector<string>& v){
	stringstream ss;
	ss.str(s);
	string i;
	while(std::getline(ss, i, d)){
		if(!i.empty()) v.push_back(i);
	}
}

void split(const string& s, char d, vector<string>& v, int count){
	stringstream ss;
	ss.str(s);
	string i;
	while(std::getline(ss, i, d) && count){
		if(!i.empty()) v.push_back(i);
		--count;
	}
	std::getline(ss, i);
	if(!i.empty()) v.push_back(i);
}

void run_script(char* file){
	ifstream f(file);
	string s;
	vector<string> v;
	auto ins = make_shared<vector<instruction_t>>();
	fn_init(ins);
	while(f.good() && !f.eof() && running){
		getline(f, s);
		if(s.empty()) continue;
		v.clear();
		split(s, ' ', v, 2);
		switch(v.size()){
			case 3:
				get_op(ins, v.at(0).c_str())
					->fn_ptr(
						v.at(1).c_str(), 
						v.at(2).c_str());
				break;
			case 2:
				get_op(ins, v.at(0).c_str())
					->fn_ptr(
						v.at(1).c_str(), 
						"");
				break;
			case 1:
				get_op(ins, v.at(0).c_str())
					->fn_ptr("", "");
				break;
			default:
				cout << "[!] Unreachable code occured." << endl;
				break;
		}
	}
}

int main(int argc, char** argv){
	if(argc == 2){
		run_script(argv[1]);
		return 0;
	}
	else return -1;
}

