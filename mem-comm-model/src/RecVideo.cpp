#include "../inc/RecVideo.h"

RecVideo::RecVideo(string name, UInt numViews, UInt numFrames, UInt w, UInt h) {
	this->w = w;
	this->h = h;
	this->numFrames = numFrames;
	this->numViews = numViews;
	reconYuv.clear();

	for (int i = 0; i < numViews; i++) {
		ostringstream oss;
		oss << name << "_" << i << ".yuv";
		string fName = YUV_FOLDER + oss.str();
		
		ifstream *ifs = new ifstream(fName.c_str(), fstream::binary);
		cout << ifs->
		reconYuv.push_back(ifs);
	}

	this->xReadYuv();
}

void RecVideo::xReadYuv() {
	pYRecFrames = new Pel**[numViews];
	for (int v = 0; v < numViews; v++) {
		pYRecFrames[v] = new Pel*[numFrames];
		for (int f = 0; f < numFrames; f++) {
			pYRecFrames[v][f] = new Pel[w*h];
			ifstream* ifs = reconYuv[v];
			ifs->read(pYRecFrames[v][f], w*h);
			ifs->seekg(w*h/2, ios::cur);
		}
	}		
}

Pel RecVideo::operator ()(Int v, Int f, Int x, Int y) {
	return pYRecFrames[v][f][x+y*w];
}