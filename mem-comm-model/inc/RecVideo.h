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
    RecVideo(const RecVideo&);
    Pel operator()(Int v, Int f, Int x, Int y);
    Pel get(Int v, Int f, Int x, Int y);
    
    void setH(UInt h);
    UInt getH() const;
    void setW(UInt w);
    UInt getW() const;
    void setNumFrames(UInt numFrames);
    UInt getNumFrames() const;
    void setNumViews(UInt numViews);
    UInt getNumViews() const;

};

#endif	/* _RECVIDEO_H */

