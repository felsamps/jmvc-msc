#ifndef _RECVIDEO_H
#define	_RECVIDEO_H

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include "TypeDefs.h"
using namespace std;

class RecVideo {
private:
    UInt numViews, numFrames, w, h;
    vector<ifstream*> reconYuv;
    Pel*** pYRecFrames;

    void xReadYuv();
public:

    RecVideo(string name, UInt numViews, UInt numFrames, UInt w, UInt h);

    Pel operator()(Int v, Int f, Int x, Int y);

};

#endif	/* _RECVIDEO_H */

