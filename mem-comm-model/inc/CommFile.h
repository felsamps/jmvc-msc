/* 
 * File:   CommFile.h
 * Author: felsamps
 *
 * Created on May 30, 2012, 9:54 AM
 */

#ifndef _COMMFILE_H
#define	_COMMFILE_H

#include <list>
#include <string>
#include <fstream>

#include "TypeDefs.h"

using namespace std;

struct CommEntry {
    char rw;
    UInt view, frame;

};

class CommFile {
private:
    fstream file;
    list<CommEntry*> comm;

public:
    CommFile(string name);
    CommEntry* getNextComm();
    bool isEmpty();

};

#endif	/* _COMMFILE_H */

