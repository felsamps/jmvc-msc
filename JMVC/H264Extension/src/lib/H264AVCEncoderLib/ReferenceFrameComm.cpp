/* 
 * File:   ReferenceFrameComm.cpp
 * Author: felsamps
 * 
 * Created on May 28, 2012, 1:03 PM
 */

#include <list>

#include "ReferenceFrameComm.h"

FILE* ReferenceFrameComm::commFile;
std::list<std::string> ReferenceFrameComm::comm;

ReferenceFrameComm::ReferenceFrameComm() {
}

void ReferenceFrameComm::init(int view) {
	if(view == 0) {
		commFile = fopen("comm.txt", "w");
	}
	else {
		commFile = fopen("comm.txt", "a");
	}
}

void ReferenceFrameComm::insertComm(std::string rw, int viewId, int framePOC) {
	char temp[10];
	sprintf(temp, " %d %d", viewId, framePOC);
	std::string commStr(temp);
	commStr = rw + commStr;
	comm.push_back(commStr);
}

void ReferenceFrameComm::reportAndClose() {
	while(!comm.empty()) {
		fprintf(commFile, "%s\n", comm.front().c_str());
		comm.pop_front();
	}
	fclose(commFile);
}

