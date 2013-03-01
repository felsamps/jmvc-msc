#include "NonLinearQuantizer.h"

using namespace std;

double abs(double l0) {
    return (l0 < 0) ? -l0 : l0;
}

XPel closestToZero(double l0, double l1) {
    if((l0 > 0 && l1 < 0) || (l0 < 0 && l1 > 0)) {
        return 0;
    }
    return abs(l0) < abs(l1) ? l0 : l1;
}

NonLinearQuantizer::NonLinearQuantizer(std::string fileName) {
    this->quantFile.open(fileName.c_str(), fstream::in);
    this->levels.clear();
    this->fQuant.clear();
    this->iQuant.clear();

    xParseQuantFile();
    xGenerateQuantTable();
}

void NonLinearQuantizer::xParseQuantFile() {
    this->levels.push_back(-129.00);
    while(!this->quantFile.eof()) {
        double level;
        this->quantFile >> level;
        this->levels.push_back(level);
    }
    this->levels.pop_back();
    this->levels.push_back(128.00);

    this->numOfLevels = this->levels.size() - 1;
}

void NonLinearQuantizer::xGenerateQuantTable() {
    Pel l = -(this->numOfLevels/2);
    int idx = 0;
    for (int sample = -128; sample < 128; sample++) {
        if((double)sample >= this->levels[idx+1]) {
            l++;
            idx++;
        }		
        this->fQuant[(Pel)sample] = l;
    }
    idx = 0;
    for (Pel sample = -(this->numOfLevels/2); sample <= (this->numOfLevels/2); sample ++) {
        this->iQuant[sample] = closestToZero(this->levels[idx], this->levels[idx+1]);
        idx++;
    }
}

XPel NonLinearQuantizer::quantize(XPel pel) {
    return this->fQuant[pel];
}

XPel NonLinearQuantizer::invQuantize(XPel pel) {
    return this->iQuant[pel];
}

