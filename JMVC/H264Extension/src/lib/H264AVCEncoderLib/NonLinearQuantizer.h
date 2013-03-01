#ifndef NONLINEARQUANTIZER_H
#define	NONLINEARQUANTIZER_H

#include <vector>
#include <map>

#include "H264AVCEncoder.h"

using namespace std;

class NonLinearQuantizer {
private:
    fstream quantFile;
    vector<double> levels;
    map<XPel, XPel> fQuant, iQuant;
    int numOfLevels;

    void xParseQuantFile();
    void xGenerateQuantTable();
    
public:
    NonLinearQuantizer(std::string fileName);
    void report();

    XPel quantize(XPel pel);
    XPel invQuantize(XPel pel);

    int getNumOfLevels();

};

#endif	/* NONLINEARQUANTIZER_H */

