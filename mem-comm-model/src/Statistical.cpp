#include "../inc/Statistical.h"

Statistical::Statistical(RecVideo* rpRecVideo) {
	this->rRecVideo = rpRecVideo;
}

void Statistical::approach01() {
	roReconFile.open("../../../outputs/recon_samples.dat", fstream::out);
	roResidualFile.open("../../../outputs/residual_samples.dat", fstream::out);

	xSamplesHandle();

	roReconFile.close();
	roResidualFile.close();
}

void Statistical::xSamplesHandle() {
	for (int v = 0; v < rRecVideo->getNumViews(); v++) {
		for (int f = 0; f < rRecVideo->getNumFrames(); f++) {
			for (int x = 0; x < rRecVideo->getW(); x+=16) {
				for (int y = 0; y < rRecVideo->getH(); y+=16) {
					xBlockScan(v, f, x, y);
				}
			}
		}
	}
}

void Statistical::xBlockScan(UInt v, UInt f, UInt x, UInt y) {
	for (int xx = 0; xx < 16; xx++) {
		for (int yy = 0; yy < 16; yy++) {
			Int xPel = (xx%2) ? xx+x-1 : xx+x;
			Int yPel = (yy%2) ? yy+y-1 : yy+y;
			
			if(yPel%2==0 && xPel%2==0) {				
				roReconFile << (Int)rRecVideo->get(v, f, xPel, yPel) << endl;
			}			
			roResidualFile << (Int) (rRecVideo->get(v, f, x+xx, y+yy) - rRecVideo->get(v, f, xPel, yPel)) << endl;
		}
	}

}