/// main.cpp
/// Encoding: UTF-8
/// Tab: 4

/// A quick & dirty spagatti impl. of ns.syslang
/// NOT FOR STANDARD USAGE, MUST BE REWRITE
/// DO NOT DISTRIBUTE since this code is too dirty

/// Author: dousha

#include <cstdint>
#include <cstddef>
#include <cstring>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stack>
#include <map>
#include <vector>
#include <utility>
#include <memory>
#include <exception>

#include "op_macro.hpp"

#define FN_PTR(name) void name (const string& l, const string& r)

using namespace std;

using TablePtr = size_t;

class UD2 : public exception { 
	public: 
		const char* what() const noexcept{ 
			return "[X] Undefined Opcode.\n";
		} 
};

class LogicImpossible : public exception {
	public:
		const char* what() const noexcept{
			return "[X] Logic is not possible.\n";
		}
};

class SyntaxError : public exception {
	public:
		const char* what() const noexcept{
			return "[X] Syntax error.\n";
		}
};

class IndexNotFound : public exception {
	public:
		const char* what() const noexcept{
			return "[X] Label not found.\n";
		}
};

typedef
struct Instruction{
	const char* name;
	uint8_t param_count;
	void (*fn_ptr)(const string&, const string&);
}
instruction_t;

template<typename T>
class Register{
	public:
		Register(){
			_content = make_unique<T>();
		}

		~Register(){}

		void operator=(const Register& r){
			_content->set(r.get_content());
		}

		void operator=(const T& r){
			_content->set(r);
		}

		void set(T* r){
			_content->set(r);
		}

		void set(const T& r){
			_content->set(r);
		}

		template<typename N>
		void set(const N& n){
			_content->set(n);
		}

		// note that neither of these operations yield a result
		// but they may throw exceptions (OperationNotDefined)
		// register itself doesn't handle this exception, though
		// if this exception occured, there must be sth wrong with
		// this code
		// maybe SFINAE can do some help

		T* get_content(){
			return _content.get();
		}

		T get(){
			return _content->get();
		}

		void operator+(const Register& r){
			_content.add(r.get_content);
		}

		void operator-(const Register& r){
			_content.sub(r.get_content);
		}

		void operator*(const Register& r){
			_content.mul(r.get_content);
		}

		void operator/(const Register& r){
			_content.div(r.get_content);
		}

	private:
		unique_ptr<T> _content;
};

template<typename T>
class Number{
	// number class is a wrapper for various of number primitives
	// it may seems expensive because i can actually use those
	// primitives directly
	// but, please note in the future, more and more complicated
	// classes would be introduced
	
	public:
		Number(){
			_d = 0;
		}

		Number(T d){
			_d = d;
		}

		~Number(){}

		void add(const Number& n){
			_d += (T) n;
		}

		void sub(const Number& n){
			_d -= (T) n;
		}

		void mul(const Number& n){
			_d *= (T) n;
		}

		void div(const Number& n){
			_d /= (T) n;
		}

		void set(Number* n){
			_d = (T) *n; // why would i modify `n` anyway...
		}

		void set(const Number& n){
			_d = n.get();
		}

		void set(const T& d){
			_d = d;
		}

		T get(){
			return _d;
		}

		operator T(){
			return _d;
		}
	
	private:
		T _d;
};

template<typename I, typename V>
class Table{
	public:
		Table(){
			_content = make_unique< map<I, V> >();
		}

		~Table(){}

		V get(const I& i){
			V v;
			auto p = _content->find(i);
			if(p == _content->end())
				throw IndexNotFound();
			v = p->second;
			return v;
		}

		void put(const pair<I, V>& p){
			_content->insert(p);
		}

		void put(const I& i, const V& v){
			_content->insert(pair<I, V>(i, v));
		}
	
	private:
		unique_ptr< map<I, V> > _content;
};

void get_op(shared_ptr<vector<instruction_t>> ins, 
					const char* name,
					instruction_t** ptr){
	for(size_t i = 0; i < ins->size(); i++){
		if(strcmp(ins->at(i).name, name) == 0){
			*ptr = &(ins->at(i));
			return;
		}
	}
	throw UD2(); 
}

/// ^^^ type def ^^^
/// vvv Environment vvv
/// XXX: Use `class Environment` instead

bool running = true;

/// NOTE: now registers can be get with get_reg(const string&) function
Register<Number<double> > ax; // XXX: we would use a register vector in the future
Register<Number<size_t> > ip; // XXX: it's getting worse
Register<Number<size_t> > bp; // XXX: template classes is hard to be stroed
Register<Number<size_t> > sp; // XXX: =_=

Table<string, size_t> fns; // XXX: i believe it's not working

stack<size_t> sys_stk; // XXX: :-|
stack<TablePtr> param_stk; // XXX: i promise i will use a env class next time

/// ^^^ Environment ^^^
/// vvv helper fn vvvv

void split(const string& s, char d, vector<string>& v){
	stringstream ss;
	ss.str(s);
	string i;
	while(std::getline(ss, i, d)){
		if(!i.empty()) v.push_back(i);
	}
}

void split(const string& s, char d, vector<string>& v, int count){
	stringstream ss, us;
	ss.str(s);
	string u;
	while(std::getline(ss, u, d)){
		if(!u.empty()){
			if(count){
				v.push_back(u);
				--count;
			}else{
				us << u << " ";
			}
		}
	}
	if(!us.str().empty())
		v.push_back(us.str());
}

int toDigit(const char c){
	if('0' <= c && c <= '9'){
		return c - 48;
	}
	else if(
		('A' <= c && c <= 'Z') 
		|| ('a' <= c && c <= 'z')){
		return tolower(c) - 97;
	}
	else{
		throw LogicImpossible();
	}
}

string trim(const string& s){
	return s.substr(s.find_first_not_of(' '), s.find_last_not_of(' ') + 1);
}

template<typename T>
T toNumber(const string& s, bool isHex = false){
	if(isHex){
		// (0[xh])?.....
		string u = trim(s);
		int exp = 0;
		T result = 0;
		if(u[0] == '0'){
			u = u.substr(2);
		}
		for(auto i = u.rbegin();
			i < u.rend();
			i++){
			result += toDigit(*i) * (1 << (exp * 4));
			++exp;
		}
		return result;
	}else{
		stringstream ss;
		T d;
		ss << s;
		ss >> d;
		return d;
	}
}

string escape(const string& s){
	string escaped;
	for(int i = 0; i < s.size(); i++){
		if(s[i] == '\\'){
			switch(s[i + 1]){
				case 'n':
					escaped.push_back('\n');
					break;
				case 't':
					escaped.push_back('\t');
					break;
				case 'r':
					escaped.push_back('\r');
					break;
				case 'b':
					escaped.push_back('\b');
					break;
				case 'v':
					escaped.push_back('\v');
					break;
				case '\\':
					escaped.push_back('\\');
					break;
				case 'x':
					// DBG
					escaped.push_back(
						(char) toNumber<int>(
							s.substr(i + 2, 2), true
						)
					);
					i += 2;
					break;
				// these cases below should trigger out a warning
				case '\0':
					escaped.push_back('\\');
					break;
				case ' ':
					escaped.push_back('\\');
					break;
				default:
					break;
			}
			++i;
		}else{
			escaped.push_back(s[i]);
		}
	}
	return escaped;
}

void* get_reg(const string& s){
	if(s.empty()) return nullptr;
	else if(s == "ax") return (void*) &ax;
	else if(s == "ip") return (void*) &ip;
	else if(s == "bp") return (void*) &bp;
	else if(s == "sp") return (void*) &sp;
	else throw LogicImpossible();
}

/// ^^^ helper fn ^^^
/// vvv functions vvv

FN_PTR(nop){
	return;	
}

FN_PTR(mov){
	/// MOV dest, src
	/// `dest` must be a register
	if(trim(l)[0] == '%'){
		/// FIXME: use Environment.Registers.get(string& s) instead!
		if(trim(l).substr(1) == "ax"){
			ax.get_content()->set(toNumber<double>(trim(r).substr(1)));
		}else{
			throw SyntaxError();
		}
	}else{
		throw SyntaxError();
	}
}

FN_PTR(add){
	// ADD dest, src
	// NOTE: if both `dest` and `src` are numric immediates, the answer is 
	// stored in `ax` register
	OPERATION_MACRO(l, r, ax, +)
}

FN_PTR(sub){
	// SUB dest, src
	// NOTE: if both `dest` and `src` are numric immediates, the answer is 
	// stored in `ax` register
	OPERATION_MACRO(l, r, ax, -)
}

FN_PTR(mul){
	// MUL dest, src
	// NOTE: if both `dest` and `src` are numric immediates, the answer is 
	// stored in `ax` register
	OPERATION_MACRO(l, r, ax, *)
}

FN_PTR(div){
	// DIV dest, src
	// NOTE: if both `dest` and `src` are numric immediates, the answer is 
	// stored in `ax` register
	OPERATION_MACRO(l, r, ax, /)
}

FN_PTR(push){
	// PUSH reg
	// PUSH imm
	// Register content is treated as TablePtr (aka size_t)
	if(trim(l)[0] == '%'){
		param_stk.push(
			(
				(Register< Number<size_t> >*)
				get_reg(trim(l).substr(1))
			)->get());
		sp.set<size_t>((size_t) sp.get());
	}
	else if(trim(l)[0] == '$'){
		param_stk.push(toNumber<size_t>(trim(l).substr(1)));
	}
	else{
		throw SyntaxError();
	}
}

FN_PTR(pop){
	// POP reg
	// Register content is treated as TablePtr (aka size_t)
	// `ip` can be changed
	if(trim(l)[0] == '%'){
		((Register< Number<size_t> >*) 
			get_reg(trim(l).substr(1)))->set(param_stk.top());
		param_stk.pop();
		sp.set<size_t>((size_t) sp.get());
	}
	else{
		throw SyntaxError();
	}
}

FN_PTR(hlt){
	cout << endl;
	cout << "[VM] VM is exitting..." << endl;
	running = false;
}

FN_PTR(print){
	// PRINT $str
	// PRINT %reg
	// PRINT @ptr
	switch(trim(l)[0]){
		case '%':
			// register
			// PRINT %ax => value of ax register
			// XXX: use tuple or map instead of this
			if(trim(l).substr(1) == "ax"){
				cout << (double) *(ax.get_content());
			}
			else{
				cout << ((Register< Number<size_t> >*)
				get_reg(trim(l).substr(1)))->get();
			}
			break;
		case '$':
			// string immediates
			// PRINT $Hello world! -> l = "$Hello", r = "world!"
			cout << escape(l.substr(1));
			if(!r.empty()) cout << " " << escape(r);
			break;
		case '@':
			// pointer
			
			break;
		default:
			throw SyntaxError();
			break;
	}
}

FN_PTR(jmp){
	// JMP (NEAR|FAR) callsign
	// `l` can be only "NEAR" or "FAR"
	// `r` is call sign
	if(trim(l) == "NEAR"){
		// "NEAR" means in the same file
		ip.set<size_t>(
			fns.get(trim(r))
			);
	}
	else if(trim(l) == "FAR"){
		// "FAR" means in other files
		// TODO: This will need to be completed

	}
	else{
		throw SyntaxError();
	}
}

FN_PTR(call){
	// CALL (NEAR|FAR) label
	if(trim(l) == "NEAR"){
		sys_stk.push((size_t) ip.get());
		sys_stk.push((size_t) bp.get());
		bp.set<size_t>((size_t) sp.get());
		ip.set<size_t>(fns.get(trim(r)));
	}
	else if(trim(l) == "FAR"){
		// TODO
	}
	else{
		throw SyntaxError();
	}
}

FN_PTR(ret){
	// RET
	// Return from a call
	// if the stack is empty, treated as HLT
	// note you can change `ip` by manipulating stack
	// be careful when doing this
	if(sys_stk.empty()){
		hlt("", "");
		return;
	}
	bp.set<size_t>(sys_stk.top());
	sys_stk.pop();
	ip.set<size_t>(sys_stk.top());
	sys_stk.pop();
}

/// ^^^ functions ^^^

void fn_init(shared_ptr<vector<instruction_t>> ins){
	ins->push_back({"NOP", 0, nop});
	ins->push_back({"MOV", 2, mov});
	ins->push_back({"ADD", 2, add});
	ins->push_back({"SUB", 2, sub});
	ins->push_back({"MUL", 2, mul});
	ins->push_back({"DIV", 2, div});
	ins->push_back({"PUSH", 1, push});
	ins->push_back({"POP", 1, pop});
	ins->push_back({"HLT", 0, hlt});
	ins->push_back({"PRINT", 1, print});
	ins->push_back({"JMP", 2, jmp});
	ins->push_back({"CALL", 2, call});
	ins->push_back({"RET", 0, ret});
}


void run_script(char* file){
	ifstream f(file);
	string s;
	vector<string> v, script;
	auto ins = make_shared<vector<instruction_t>>();
	fn_init(ins);
	instruction_t* curIns = nullptr;
	ax.get_content()->set((double) 0.0);
	ip.set<size_t>((size_t) 0);

	// first scan
	while(f.good() && !f.eof()){
		getline(f, s);
		if(!s.empty()){
			v.clear();
			split(s, ' ', v);
			if(trim(v.at(0)) == "@FN"){
				fns.put(trim(v.at(1)), (size_t) *(ip.get_content()));
			}
		}
		script.push_back(s);
		ip.set<size_t>((size_t) ip.get() + 1); // maybe should be done more gracefully
	}

	f.close();

	// second run
	for(ip.set<size_t>((size_t) 0);
		(size_t) ip.get() < script.size();
		ip.set<size_t>((size_t) ip.get() + 1)
		){
		if(!running) break;
		s = script.at((size_t) ip.get());
		if(s.empty()){
			continue;
		}
		v.clear();
		split(s, ' ', v, 2);
		
		if(trim(v.at(0))[0] == '@' || trim(v.at(0))[0] == '#') continue;
		
		get_op(ins, trim(v.at(0)).c_str(), &curIns);

		try{
			switch(v.size()){
				case 3:
					if(curIns->param_count <= 2)
						curIns->fn_ptr(
							v.at(1).c_str(), 
							v.at(2).c_str());
					else
						throw SyntaxError();
					break;
				case 2:
					if(curIns->param_count <= 1)
						curIns->fn_ptr(
							v.at(1).c_str(), 
							"");
					else
						throw SyntaxError();
					break;
				case 1:
					if(curIns->param_count == 0)
						curIns->fn_ptr("", "");
					else
						throw SyntaxError();
					break;
				default:
					throw LogicImpossible();
					break;
			}
		}
		catch(SyntaxError){
			cerr << "At: " 
				<< (size_t) *(ip.get_content()) + 1
				<< ": " 
				<< script.at((size_t) *(ip.get_content()))  
				<< endl
				<< "[X] Syntax error." 
				<< endl;
			exit(-1);
		}
		catch(IndexNotFound){
			cerr << "At: "
				<< (size_t) *(ip.get_content()) + 1
				<< ": "
				<< script.at((size_t) *(ip.get_content()))
				<< endl
				<< "[X] Label not found."
				<< endl;
			exit(-1);
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

