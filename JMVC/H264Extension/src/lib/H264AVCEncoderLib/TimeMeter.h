#ifndef TIMEMETER_H
#define	TIMEMETER_H

#include <cstdio>
#include <sys/time.h>

class TimeMeter {
private:
	static struct timeval inicio, final;
    static int tmili;
	static FILE* fp;
	
public:
	TimeMeter();
	static void init(int view);
	
	static void startMeasure();
	static void finishMeasure();
	
	static void close();

};

#endif	/* TIMEMETER_H */

