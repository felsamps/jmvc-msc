#ifndef _SEARCHMONITOR_H
#define	_SEARCHMONITOR_H

#include "../../../include/H264AVCCommonLib.h"
#include <cstdio>
#include <string>

struct BestMatch {
    h264::Mv* mv;
    UInt refFrameId, refViewId;
    UInt bestCost, bestBits;
    
    
    BestMatch() {
        mv = new h264::Mv();
        refFrameId = -1;
        refViewId = -1;
        bestCost = 0xFFFFFFFF;
        bestBits = 0xFFFFFFFF;
    }

    void set(h264::Mv& vec, UInt frameId, UInt viewId, UInt cost, UInt bits) {
        if((cost + bits) < (bestCost + bestBits)) {
            mv->set(vec.getVer(), vec.getHor());
            this->refFrameId = frameId;
            this->refViewId = viewId;
            bestCost = cost;
            bestBits = bits;
        }
    }
};

class SearchMonitor {

private:
    static FILE *file;
    static BestMatch**** video;
    static UInt w, h, nFrames, currViewId;

    static std::string xReportSummary();
    static std::string xReportByFrame();
public:
    SearchMonitor();
    static void init(std::string name, UInt view, UInt width, UInt height, UInt numFrames);
    static void insert(UInt poc, UInt xMb, UInt yMb, h264::Mv& vec, UInt frameId, UInt viewId, UInt cost, UInt bits);
    static void reportAndClose();
};

#endif	/* _SEARCHMONITOR_H */

