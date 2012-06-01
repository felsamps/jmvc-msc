#include <stdlib.h>
#include <iostream>

#include "../inc/CommFile.h"
#include "../inc/RecVideo.h"

using namespace std;

int main(int argc, char** argv) {
	
	CommFile rCommFile("comm.txt");
	while(!rCommFile.isEmpty()) {
		CommEntry *entry = rCommFile.getNextComm();
	}

	RecVideo rRecVideo("recon", 3, 5, 640, 480);

	return (EXIT_SUCCESS);
}

