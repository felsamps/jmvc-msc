#ifndef _SEARCHMONITOR_H
#define	_SEARCHMONITOR_H

#include "../../../include/H264AVCCommonLib.h"
#include <cstdio>
#include <string>
#include <algorithm>
#include <map>

struct BestMatch {
    std::map<Int, h264::Mv*> mvList;
    std::map<Int, UInt> costList;
    UInt refFrameId, refViewId;
    UInt bestCost, bestBits;
    Int bestRef;
    
    BestMatch() {
        bestRef = 0;
        costList.clear();
        mvList.clear();
        refFrameId = -1;
        refViewId = -1;
        bestCost = 0xFFFFFFFF;
        bestBits = 0xFFFFFFFF;
    }

    void set(h264::Mv& vec, UInt frameId, UInt viewId, Int refId, UInt cost, UInt bits) {
        h264::Mv *mv = new h264::Mv(vec.getVer(), vec.getHor());
        mvList[refId] = mv;
        costList[refId] = cost;
        if((cost + bits) < (bestCost + bestBits)) {
            refFrameId = frameId;
            refViewId = viewId;
            bestRef = refId;
            bestCost = cost;
            bestBits = bits;
        }
    }
};

class SearchMonitor {

private:
    static FILE *file, *fileByFrame;
    static FILE *fileMvMe, *fileMvDe;
    static FILE *fileMvdMe, *fileMvdDe;
    static FILE *fileCostMe, *fileCostDe;
    static BestMatch**** video;
    static UInt w, h, nFrames, currViewId;
    static std::vector<std::map<std::pair<UInt,UInt>, Int> > refFrames;
    static Int meRefCounter, deRefCounter;

    static void xReportRefFrame();
    static void xReportMvTracing();
    static h264::Mv* xCalcMvd(UInt f, UInt x, UInt y, Int idx);
    static h264::Mv* xGenPredictedMv02(UInt f, Int x, Int y, Int idx);

public:
    SearchMonitor();
    static void init( UInt view, UInt width, UInt height, UInt numFrames);
    static void initCounters();
    static void insertRefFrame(UInt currFrame, UInt viewId, UInt framePOC);
    static void insert(UInt currFrameId, UInt xMb, UInt yMb, h264::Mv& vec, UInt frameId, UInt viewId, UInt cost, UInt bits);
    static void reportAndClose();
};

#endif	/* _SEARCHMONITOR_H */

