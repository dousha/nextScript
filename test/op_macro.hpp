#pragma once

#define OPERATION_MACRO(l, r, ax, op) \
	double lv = 0; \
	double rv = 0; \
	Number<double> ntemp; \
	switch(trim(l)[0]){ \
		case '$': \
			switch(trim(r)[0]){ \
				case '%': \
					lv = toNumber<double>(l.substr(1)); \
					rv = (double) *(ax.get_content()); \
					ntemp.set(lv op rv); \
					ax.set(&ntemp); \
					break; \
				case '$': \
					lv = toNumber<double>(l.substr(1)); \
					rv = toNumber<double>(r.substr(1)); \
					ntemp.set(lv op rv); \
					ax.set(&ntemp); \
					break; \
				default: \
					throw SyntaxError(); \
					break; \
			} \
			break; \
		case '%': \
			if(trim(l).substr(1) == "ax") \
				lv = (double) *(ax.get_content()); \
			else \
				throw SyntaxError(); \
			switch(trim(r)[0]){ \
				case '%': \
					if(trim(r).substr(1) == "ax"){ \
						rv = (double) *(ax.get_content()); \
						ntemp.set(lv op rv); \
						ax.set(&ntemp); \
					} else { \
						throw SyntaxError(); \
					} \
					break; \
				case '$': \
					rv = toNumber<double>(r.substr(1)); \
					ntemp.set(lv op rv); \
					ax.set(&ntemp); \
					break; \
				default: \
					throw SyntaxError(); \
					break; \
			} \
			break; \
		default: \
			throw SyntaxError(); \
			break; \
	}

