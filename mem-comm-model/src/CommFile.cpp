/* 
 * File:   CommFile.cpp
 * Author: felsamps
 * 
 * Created on May 30, 2012, 9:54 AM
 */

#include "../inc/CommFile.h"

CommFile::CommFile(string name) {
	file.open(name.c_str(), fstream::in);
	while (!file.eof()){
		CommEntry *entry = new CommEntry();
		file >> entry->rw >> entry->view >> entry->frame;
		comm.push_back(entry);
	}
}

CommEntry* CommFile::getNextComm() {
	if(!comm.empty()) {
		CommEntry *returnable = comm.front();
		comm.pop_front();
		return returnable;
	}
	return NULL;
}

bool CommFile::isEmpty() {
	return comm.empty();
}


