/* 
 * File:   Debugger.cpp
 * Author: felsamps
 * 
 * Created on May 25, 2012, 1:57 PM
 */

#include "Debugger.h"

FILE* Debugger::dbgFile;

Debugger::Debugger() {
}

void Debugger::initFile(std::string name) {
	dbgFile = fopen(name.c_str(), "w");
}

void Debugger::closeFile() {
	fclose(dbgFile);
}

void Debugger::print(const char* str, ...) {
	va_list args;
	char buffer[BUFSIZ];

	va_start(args, str);
	vsprintf(buffer, str, args);
	fprintf(dbgFile, "%s\n", buffer);
	va_end(args);
}

