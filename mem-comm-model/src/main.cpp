/* 
 * File:   main.cpp
 * Author: felsamps
 *
 * Created on May 30, 2012, 9:51 AM
 */

#include <stdlib.h>
#include <iostream>

#include "../inc/CommFile.h"

using namespace std;

int main(int argc, char** argv) {
	
	CommFile rCommFile("comm.txt");
	while(!rCommFile.isEmpty()) {
		CommEntry *entry = rCommFile.getNextComm();
	}
	return (EXIT_SUCCESS);
}

