#ifndef _STATISTICAL_H
#define	_STATISTICAL_H

#include "RecVideo.h"

using namespace std;

class Statistical {
private:
    RecVideo* rRecVideo;
    ofstream roResidualFile, roReconFile;
public:
    Statistical(RecVideo* rRecVideo);

    void approach01();
    void xSamplesHandle();
    void xBlockScan(UInt v, UInt f, UInt x, UInt y);

};

#endif	/* _STATISTICAL_H */

