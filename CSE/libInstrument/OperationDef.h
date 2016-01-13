#ifndef _OPERATOR_DEF_H_
#define _OPERATOR_DEF_H_

enum DetectorTypes    { DetAUTO, DetNormal, DetAverage, DetPeak, DetSample, DetNegPeak };
enum PeakSearchTypes  { PeakHighest, PeakNextHigh, PeakNextRight, PeakNextLeft };
enum MarkerTypes      { MarkerNormal, MarkerDelta, MarkerOff };
enum MarkerMoveTypes  { MarkerToCF, MarkerToRefLvl };
enum CalibrationTypes { AUTO_ON, AUTO_OFF, CAL_NOW };

#endif