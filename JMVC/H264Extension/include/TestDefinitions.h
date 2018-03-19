#ifndef TESTDEFINITIONS_H
#define	TESTDEFINITIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

#define REF_COMM_EN 0
#define SW_USAGE_EN 1
#define DEBUGGER_EN 0
#define MONITOR_EN 0
#define TIME_METER_EN 0
    
/* SW_ANALYSIS == 1 --> pixel accuracy analysis
   SW_ANALYSIS == 0 --> block accuracy analysis*/
#if SW_USAGE_EN
    #define SW_ANALYSIS 0
    #define SW_SEARCH_MAP 0
#endif

#if MONITOR_EN
    #define REF_DIRECTION_EN 0
    #define PR_ANALYSIS_EN 1
#endif
    
#ifdef	__cplusplus
}
#endif

#endif	/* TESTDEFINITIONS_H */

