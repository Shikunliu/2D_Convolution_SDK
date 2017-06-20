/*
 * AxiTimerHelper.h
 *
 *  Created on: 2017Äê6ÔÂ19ÈÕ
 *      Author: baohaochun
 */

#ifndef SRC_AXITIMERHELPER_H_
#define SRC_AXITIMERHELPER_H_

//Needed header files
#include "xil_types.h"
#include "xtmrctr.h"
#include "xparameters.h"

class AxiTimerHelper{

public:
	AxiTimerHelper();
	virtual ~AxiTimerHelper();
	unsigned int getElapsedTicks();
	double getElapsedTimerInSeconds();
	unsigned int startTimer();
	unsigned int stopTimer();
	double getClockPeriod();
	double getTimerClockFreq();

private:
	XTmrCtr m_AxiTimer;
	unsigned int m_tickCounter1;
	unsigned int m_tickCounter2;
	double m_clockPeriodSeconds;
	double m_timerClockFreq;
};

#endif /* SRC_AXITIMERHELPER_H_ */
