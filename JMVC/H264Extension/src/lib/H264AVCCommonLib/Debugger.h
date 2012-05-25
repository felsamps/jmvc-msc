/* 
 * File:   Debugger.h
 * Author: felsamps
 *
 * Created on May 25, 2012, 1:57 PM
 */

#ifndef _DEBUGGER_H
#define	_DEBUGGER_H

#include <cstdarg>
#include <string>
#include <cstdio>

class Debugger {
private:
    static FILE *dbgFile;
    
public:
    Debugger();

    static void initFile(std::string name);
    static void closeFile();
    static void print(const char* str, ...);
};

#endif	/* _DEBUGGER_H */

