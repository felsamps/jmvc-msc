#include "TimeMeter.h"

struct timeval TimeMeter::inicio, TimeMeter::final;
int TimeMeter::tmili;
FILE* TimeMeter::fp;

TimeMeter::TimeMeter() {
}

void TimeMeter::init(int view) {
	if(view == 0) {
		fp = fopen("time.txt", "w");
	}
	else {
		fp = fopen("time.txt", "a");
	}
}

void TimeMeter::close() {
	fprintf(fp, "\n");
	fclose(fp);
}

void TimeMeter::startMeasure() {
	gettimeofday(&inicio, NULL);
}

void TimeMeter::finishMeasure() {
	gettimeofday(&final, NULL);
    tmili = (int) (1000 * (final.tv_sec - inicio.tv_sec) + (final.tv_usec - inicio.tv_usec) / 1000);
	fprintf(fp, "%d ", tmili);
}