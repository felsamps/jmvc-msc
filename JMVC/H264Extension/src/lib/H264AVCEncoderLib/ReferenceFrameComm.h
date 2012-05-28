/* 
 * File:   ReferenceFrameComm.h
 * Author: felsamps
 *
 * Created on May 28, 2012, 1:03 PM
 */

#ifndef _REFERENCEFRAMECOMM_H
#define	_REFERENCEFRAMECOMM_H

#include <cstdio>
#include <string>
#include <list>


class ReferenceFrameComm {
private:
    static FILE* commFile;
    static std::list<std::string> comm;

public:

    ReferenceFrameComm();
    static void init(int view);
    static void insertComm(std::string rw, int viewId, int framePOC);
    static void reportAndClose();
    
};

#endif	/* _REFERENCEFRAMECOMM_H */

