#include "Debugger.h"

FILE* Debugger::dbgFile;

Debugger::Debugger() {
}

void Debugger::initFile(std::string name, int view) {
	if(view == 0) {
		dbgFile = fopen(name.c_str(), "w");
	}
	else {
		dbgFile = fopen(name.c_str(), "a");
	}
}

void Debugger::closeFile() {
	fclose(dbgFile);
}

void Debugger::print(const char* str, ...) {
	va_list args;
	char buffer[BUFSIZ];

	va_start(args, str);
	vsprintf(buffer, str, args);
	fprintf(dbgFile, "%s", buffer);
	va_end(args);
}

