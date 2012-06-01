#include <stdlib.h>
#include <iostream>

#include "../inc/CommFile.h"
#include "../inc/RecVideo.h"
#include "../inc/Statistical.h"

using namespace std;

int main(int argc, char** argv) {
	
	CommFile rCommFile("comm.txt");
	while(!rCommFile.isEmpty()) {
		CommEntry *entry = rCommFile.getNextComm();
	}

	RecVideo* rRecVideo = new RecVideo("recon", 3, 5, 640, 480);

	Statistical stats(rRecVideo);
	stats.approach01();
	

	return (EXIT_SUCCESS);
}

