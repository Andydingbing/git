//#include <cvidef.h>

/*= R&S NRP Power Meter Include File ========================================*/

/*===========================================================================*/
/*  Please do not use global variables or arrays in the include file of      */
/*  instrument drivers that will be submitted for inclusion into the         */
/*  LabWindows Instrument Driver Library.                                    */
/*===========================================================================*/
	 
#ifndef __RSNRP_HEADER
#define __RSNRP_HEADER

#include "vpptype.h"

#if defined(__cplusplus) || defined(__cplusplus__)
extern "C" {
#endif

/*****************************************************************************/
/*= Instrument Driver Specific Error/Warning Codes ==========================*/
/*****************************************************************************/
#define VI_ERROR_INSTR_FILE_OPEN                (_VI_ERROR+0x3FFC0800L)
#define VI_ERROR_INSTR_FILE_WRITE               (_VI_ERROR+0x3FFC0801L)
#define VI_ERROR_INSTR_INTERPRETING_RESPONSE    (_VI_ERROR+0x3FFC0803L)
#define VI_ERROR_INSTR_PARAMETER9               (_VI_ERROR+0x3FFC0809L)
#define VI_ERROR_INSTR_PARAMETER10              (_VI_ERROR+0x3FFC080AL)
#define VI_ERROR_INSTR_PARAMETER11              (_VI_ERROR+0x3FFC080BL)
#define VI_ERROR_INSTR_PARAMETER12              (_VI_ERROR+0x3FFC080CL)
#define VI_ERROR_INSTR_PARAMETER13              (_VI_ERROR+0x3FFC080DL)
#define VI_ERROR_INSTR_PARAMETER14              (_VI_ERROR+0x3FFC080EL)
#define VI_ERROR_INSTR_PARAMETER15              (_VI_ERROR+0x3FFC080FL)

/*****************************************************************************/
/*= Define Instrument Specific Error/Warning Codes Here =====================*/
/*****************************************************************************/
#define VI_WARNING_INSTR_OFFSET                         (0x3FFC0900L)
#define VI_ERROR_INSTR_OFFSET                   (_VI_ERROR+0x3FFC0900L)

#define RSNRP_ERROR_INSTRUMENT_ERROR           (VI_ERROR_INSTR_OFFSET + 0xF0L)
/* Add instrument driver specific error codes here */
#define RSNRP_ERROR_INSTRUMENT_ERROR           (VI_ERROR_INSTR_OFFSET + 0xF0L)
#define RSNRP_ERROR_INVALID_CONFIGURATION      (VI_ERROR_INSTR_OFFSET + 0xF1L)
#define RSNRP_ERROR_INSTRUMENT_OPTION          (VI_ERROR_INSTR_OFFSET + 0xF2L)
#define RSNRP_ERROR_INSTRUMENT_NSUP_MODEL      (VI_ERROR_INSTR_OFFSET + 0xF3L)
#define RSNRP_ERROR_SETTINGS_CONFLICT          (VI_ERROR_INSTR_OFFSET + 0xF4L)
#define RSNRP_ERROR_TABLE_NOT_FOUND			   (VI_ERROR_INSTR_OFFSET + 0xF5L)
#define RSNRP_ERROR_SECONDARY_VALUE_INVALID	   (VI_ERROR_INSTR_OFFSET + 0xF6L)
#define RSNRP_ERROR_INVALID_FIRMWARE_VERSION   (VI_ERROR_INSTR_OFFSET + 0xF7L)

#define RSNRP_WARNING_NO_CHANNEL			   (VI_WARNING_INSTR_OFFSET + 0x01L)

/*****************************************************************************/
/*= Instrument specific defines =============================================*/
/*****************************************************************************/
#define RSNRP_SENSOR_MODE_CONTAV     0
#define RSNRP_SENSOR_MODE_BUF_CONTAV 1
#define RSNRP_SENSOR_MODE_TIMESLOT   2
#define RSNRP_SENSOR_MODE_BURST      3
#define RSNRP_SENSOR_MODE_SCOPE      4
#define RSNRP_SENSOR_MODE_TIMEGATE   5

#define RSNRP_AUTO_COUNT_TYPE_RESOLUTION 0
#define RSNRP_AUTO_COUNT_TYPE_NSR        1

#define RSNRP_TERMINAL_CONTROL_MOVING 0
#define RSNRP_TERMINAL_CONTROL_REPEAT 1

#define RSNRP_GATE_1 1
#define RSNRP_GATE_2 2
#define RSNRP_GATE_3 3
#define RSNRP_GATE_4 4

#define RSNRP_SLOPE_POSITIVE 0
#define RSNRP_SLOPE_NEGATIVE 1

#define RSNRP_TRIGGER_SOURCE_BUS       0
#define RSNRP_TRIGGER_SOURCE_EXTERNAL  1
#define RSNRP_TRIGGER_SOURCE_HOLD      2
#define RSNRP_TRIGGER_SOURCE_IMMEDIATE 3
#define RSNRP_TRIGGER_SOURCE_INTERNAL  4

#define RSNRP_SAMPLING_FREQUENCY1 1
#define RSNRP_SAMPLING_FREQUENCY2 2

#define RSNRP_MEASUREMENT_SINGLE     0
#define RSNRP_MEASUREMENT_SINGLE_REL 1
#define RSNRP_MEASUREMENT_DIFF       2
#define RSNRP_MEASUREMENT_DIFF_REL   3
#define RSNRP_MEASUREMENT_SUM        4
#define RSNRP_MEASUREMENT_SUM_REL    5
#define RSNRP_MEASUREMENT_RATIO      6
#define RSNRP_MEASUREMENT_RATIO_REL  7
#define RSNRP_MEASUREMENT_SWR        8
#define RSNRP_MEASUREMENT_REFL       9
#define RSNRP_MEASUREMENT_RLOS       10

#define RSNRP_MATH_VALUE    0
#define RSNRP_MATH_MINUS    1
#define RSNRP_MATH_PLUS     2
#define RSNRP_MATH_QUOTIENT 3
#define RSNRP_MATH_SWR      4
#define RSNRP_MATH_REFL     5
#define RSNRP_MATH_RLOSS    6

#define RSNRP_UNITS_W    0
#define RSNRP_UNITS_DBM  1
#define RSNRP_UNITS_DBUV 2
#define RSNRP_UNITS_PCT  3
#define RSNRP_UNITS_DB   4
#define RSNRP_UNITS_O    5

#define RSNRP_TYPE_MINIMUM 0
#define RSNRP_TYPE_MAXIMUM 1
#define RSNRP_TYPE_PTPEAK  2

#define RSNRP_OUTPUT_ANALOG 0
#define RSNRP_OUTPUT_TTL    1

#define RSNRP_OUT_SRC_CALC1 1
#define RSNRP_OUT_SRC_CALC2 2
#define RSNRP_OUT_SRC_CALC3 3
#define RSNRP_OUT_SRC_CALC4 4

#define RSNRP_KEY_SK1L		1
#define RSNRP_KEY_SK1R		2
#define RSNRP_KEY_SK2L		3
#define RSNRP_KEY_SK2R		4
#define RSNRP_KEY_SK3L		5
#define RSNRP_KEY_SK3R		6
#define RSNRP_KEY_SK4L		7
#define RSNRP_KEY_SK4R		8
#define RSNRP_KEY_SK5L		9
#define RSNRP_KEY_SK5R		10
#define RSNRP_KEY_SK6L		11
#define RSNRP_KEY_SK6R		12
#define RSNRP_KEY_PRESET	13
#define RSNRP_KEY_ZERO		14
#define RSNRP_KEY_FREQ		15
#define RSNRP_KEY_CONTRAST	16
#define RSNRP_KEY_POWER		17
#define RSNRP_KEY_DEL		18
#define RSNRP_KEY_ENTER		19
#define RSNRP_KEY_ESC		20
#define RSNRP_KEY_UP		21
#define RSNRP_KEY_DOWN		22
#define RSNRP_KEY_LEFT		23
#define RSNRP_KEY_RIGHT		24

#define RSNRP_SPEED_NORMAL	0
#define RSNRP_SPEED_FAST	1

#define RSNRP_WINDOW_NORMAL	0
#define RSNRP_WINDOW_ZOOMED	1

#define RSNRP_AUX_NONE	0
#define RSNRP_AUX_MIN	1
#define RSNRP_AUX_MAX	2
#define RSNRP_AUX_PTP	3

#define RSNRP_FORMAT_DIGITAL	0
#define RSNRP_FORMAT_ANALOG		1

#define RSNRP_STATCLASS_STB				0
#define RSNRP_STATCLASS_D_CONN			1
#define RSNRP_STATCLASS_D_ERR			2
#define RSNRP_STATCLASS_O_CAL			3
#define RSNRP_STATCLASS_O_MEAS			4
#define RSNRP_STATCLASS_O_TRIGGER		5
#define RSNRP_STATCLASS_O_SENSE			6
#define RSNRP_STATCLASS_O_LOWER			7
#define RSNRP_STATCLASS_O_UPPER			8
#define RSNRP_STATCLASS_Q_POWER			9
#define RSNRP_STATCLASS_Q_WINDOW		10
#define RSNRP_STATCLASS_Q_CAL			11
#define RSNRP_STATCLASS_Q_ZER			12
#define RSNRP_STATCLASS_ESR				13

#define RSNRP_SENSOR_01					2
#define RSNRP_SENSOR_02					4
#define RSNRP_SENSOR_03					8
#define RSNRP_SENSOR_04					16
#define RSNRP_ALL_SENSORS				30

#define RSNRP_WINDOW_01					2
#define RSNRP_WINDOW_02					4
#define RSNRP_WINDOW_03					8
#define RSNRP_WINDOW_04					16
#define RSNRP_WINDOW_05					32
#define RSNRP_WINDOW_06					64
#define RSNRP_WINDOW_07					128
#define RSNRP_WINDOW_08					256
#define RSNRP_ALL_WINDOWS				510

#define RSNRP_DIRECTION_PTR		0
#define RSNRP_DIRECTION_NTR		1
#define RSNRP_DIRECTION_BOTH	2
#define RSNRP_DIRECTION_NONE	3

#define RSNRP_DISPLAY_UPDATE_NORMAL		0
#define RSNRP_DISPLAY_UPDATE_SLOW		1
#define RSNRP_DISPLAY_UPDATE_FREEZE		2

#define RSNRP_CALCULATE_EVALUATE_SEQUENCE			0
#define RSNRP_CALCULATE_EVALUATE_AVERAGE			1
#define RSNRP_CALCULATE_EVALUATE_PEAK_TO_AVERAGE	2
#define RSNRP_CALCULATE_EVALUATE_PEAK				3

/*****************************************************************************/
/*= GLOBAL USER-CALLABLE FUNCTION DECLARATIONS (Exportable Functions) =======*/
/*****************************************************************************/

ViStatus _VI_FUNC rsnrp_init (ViRsrc resourceName, ViBoolean IDQuery,
				ViBoolean resetDevice, ViPSession instrSession);

ViStatus _VI_FUNC rsnrp_example_measureContAv (ViSession instrSession,
                                               ViInt32 measurement,
                                               ViInt32 resolution,
                                               ViReal64 frequency,
                                               ViReal64 *measuredValue);

ViStatus _VI_FUNC rsnrp_example_measureContAvLowLevel (ViSession instrSession,
                                                       ViInt32 measurement,
                                                       ViInt32 measuringFunction,
                                                       ViInt32 sensor1,
                                                       ViInt32 sensor2,
                                                       ViInt32 resolution,
                                                       ViReal64 frequency1,
                                                       ViReal64 frequency2,
                                                       ViReal64 *measuredValue);

ViStatus _VI_FUNC rsnrp_chans_abort (ViSession instrSession);

ViStatus _VI_FUNC rsnrp_chans_getCount (ViSession instrSession,
                                        ViInt32 *count);

ViStatus _VI_FUNC rsnrp_chans_initiate (ViSession instrSession);

ViStatus _VI_FUNC rsnrp_chans_recallStandard (ViSession instrSession,
                                              ViString standard);

ViStatus _VI_FUNC rsnrp_chans_zero (ViSession instrSession);

ViStatus _VI_FUNC rsnrp_chans_isZeroingComplete (ViSession instrSession,
                                                 ViBoolean *zeroingCompleted);

ViStatus _VI_FUNC rsnrp_chans_isMeasurementComplete (ViSession instrSession,
                                                     ViBoolean *measurementCompleted);

ViStatus _VI_FUNC rsnrp_chan_mode (ViSession instrSession, ViInt32 channel,
                                   ViInt32 measurementMode);

ViStatus _VI_FUNC rsnrp_timing_configureExclude (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViReal64 excludeStart,
                                                 ViReal64 excludeStop);

ViStatus _VI_FUNC rsnrp_timing_setTimingExcludeStart (ViSession instrSession,
                                                      ViInt32 channel,
                                                      ViReal64 excludeStart);

ViStatus _VI_FUNC rsnrp_timing_getTimingExcludeStart (ViSession instrSession,
                                                      ViInt32 channel,
                                                      ViReal64 *excludeStart);

ViStatus _VI_FUNC rsnrp_timing_setTimingExcludeStop (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 excludeStop);

ViStatus _VI_FUNC rsnrp_timing_getTimingExcludeStop (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 *excludeStop);

ViStatus _VI_FUNC rsnrp_avg_configureAvgAuto (ViSession instrSession,
                                              ViInt32 channel,
                                              ViInt32 resolution);

ViStatus _VI_FUNC rsnrp_avg_configureAvgNSRatio (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViReal64 maximumNoiseRatio,
                                                 ViReal64 upperTimeLimit);

ViStatus _VI_FUNC rsnrp_avg_configureAvgManual (ViSession instrSession,
                                                ViInt32 channel, ViInt32 count);

ViStatus _VI_FUNC rsnrp_avg_setAutoEnabled (ViSession instrSession,
                                            ViInt32 channel,
                                            ViBoolean autoEnabled);

ViStatus _VI_FUNC rsnrp_avg_getAutoEnabled (ViSession instrSession,
                                            ViInt32 channel,
                                            ViBoolean *autoEnabled);

ViStatus _VI_FUNC rsnrp_avg_setAutoMaxMeasuringTime (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 upperTimeLimit);

ViStatus _VI_FUNC rsnrp_avg_getAutoMaxMeasuringTime (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 *upperTimeLimit);

ViStatus _VI_FUNC rsnrp_avg_setAutoNoiseSignalRatio (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 maximumNoiseRatio);

ViStatus _VI_FUNC rsnrp_avg_getAutoNoiseSignalRatio (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 *maximumNoiseRatio);

ViStatus _VI_FUNC rsnrp_avg_setAutoResolution (ViSession instrSession,
                                               ViInt32 channel,
                                               ViInt32 resolution);

ViStatus _VI_FUNC rsnrp_avg_getAutoResolution (ViSession instrSession,
                                               ViInt32 channel,
                                               ViInt32 *resolution);

ViStatus _VI_FUNC rsnrp_avg_setAutoType (ViSession instrSession,
                                         ViInt32 channel, ViInt32 method);

ViStatus _VI_FUNC rsnrp_avg_getAutoType (ViSession instrSession,
                                         ViInt32 channel, ViInt32 *method);

ViStatus _VI_FUNC rsnrp_avg_setCount (ViSession instrSession,
                                      ViInt32 channel, ViInt32 count);

ViStatus _VI_FUNC rsnrp_avg_getCount (ViSession instrSession,
                                      ViInt32 channel, ViInt32 *count);

ViStatus _VI_FUNC rsnrp_avg_setEnabled (ViSession instrSession,
                                        ViInt32 channel, ViBoolean averaging);

ViStatus _VI_FUNC rsnrp_avg_getEnabled (ViSession instrSession,
                                        ViInt32 channel, ViBoolean *averaging);

ViStatus _VI_FUNC rsnrp_avg_setSlot (ViSession instrSession, ViInt32 channel,
                                     ViInt32 timeslot);

ViStatus _VI_FUNC rsnrp_avg_getSlot (ViSession instrSession, ViInt32 channel,
                                     ViInt32 *timeslot);

ViStatus _VI_FUNC rsnrp_avg_setTerminalControl (ViSession instrSession,
                                                ViInt32 channel,
                                                ViInt32 terminalControl);

ViStatus _VI_FUNC rsnrp_avg_getTerminalControl (ViSession instrSession,
                                                ViInt32 channel,
                                                ViInt32 *terminalControl);

ViStatus _VI_FUNC rsnrp_avg_reset (ViSession instrSession, ViInt32 channel);

ViStatus _VI_FUNC rsnrp_range_setAutoEnabled (ViSession instrSession,
                                              ViInt32 channel,
                                              ViBoolean autoRange);

ViStatus _VI_FUNC rsnrp_range_getAutoEnabled (ViSession instrSession,
                                              ViInt32 channel,
                                              ViBoolean *autoRange);

ViStatus _VI_FUNC rsnrp_range_setCrossoverLevel (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViReal64 crossoverLevel);

ViStatus _VI_FUNC rsnrp_range_getCrossoverLevel (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViReal64 *crossoverLevel);

ViStatus _VI_FUNC rsnrp_range_setRange (ViSession instrSession,
                                        ViInt32 channel, ViInt32 range);

ViStatus _VI_FUNC rsnrp_range_getRange (ViSession instrSession,
                                        ViInt32 channel, ViInt32 *range);

ViStatus _VI_FUNC rsnrp_corr_configureDutyCycle (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViBoolean dutyCycleState,
                                                 ViReal64 dutyCycle);

ViStatus _VI_FUNC rsnrp_corr_setDutyCycle (ViSession instrSession,
                                           ViInt32 channel, ViReal64 dutyCycle);

ViStatus _VI_FUNC rsnrp_corr_getDutyCycle (ViSession instrSession,
                                           ViInt32 channel, ViReal64 *dutyCycle);

ViStatus _VI_FUNC rsnrp_corr_setDutyCycleEnabled (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViBoolean dutyCycleState);

ViStatus _VI_FUNC rsnrp_corr_getDutyCycleEnabled (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViBoolean *dutyCycleState);

ViStatus _VI_FUNC rsnrp_corr_configureCorrections (ViSession instrSession,
                                                   ViInt32 channel,
                                                   ViBoolean offsetState,
                                                   ViReal64 offset,
                                                   ViBoolean offsetTableState,
                                                   ViString tableName,
                                                   ViBoolean sParameterEnable);

ViStatus _VI_FUNC rsnrp_chan_setCorrectionFrequency (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 frequency);

ViStatus _VI_FUNC rsnrp_chan_getCorrectionFrequency (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 *frequency);

ViStatus _VI_FUNC rsnrp_corr_setOffset (ViSession instrSession,
                                        ViInt32 channel, ViReal64 offset);

ViStatus _VI_FUNC rsnrp_corr_getOffset (ViSession instrSession,
                                        ViInt32 channel, ViReal64 *offset);

ViStatus _VI_FUNC rsnrp_corr_setOffsetEnabled (ViSession instrSession,
                                               ViInt32 channel,
                                               ViBoolean offsetState);

ViStatus _VI_FUNC rsnrp_corr_getOffsetEnabled (ViSession instrSession,
                                               ViInt32 channel,
                                               ViBoolean *offsetState);

ViStatus _VI_FUNC rsnrp_corr_setOffsetTableEnabled (ViSession instrSession,
                                                    ViInt32 channel,
                                                    ViBoolean offsetTableState);

ViStatus _VI_FUNC rsnrp_corr_getOffsetTableEnabled (ViSession instrSession,
                                                    ViInt32 channel,
                                                    ViBoolean *offsetTableState);

ViStatus _VI_FUNC rsnrp_corr_setOffsetTableSelect (ViSession instrSession,
                                                   ViInt32 channel,
                                                   ViString tableName);

ViStatus _VI_FUNC rsnrp_corr_getOffsetTableSelect (ViSession instrSession,
                                                   ViInt32 channel,
                                                   ViInt32 arraySize,
                                                   ViChar _VI_FAR selectedOffsetTable[]);

ViStatus _VI_FUNC rsnrp_corr_getOffsetTableValue (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViReal64 *frequencyDependentOffset);

ViStatus _VI_FUNC rsnrp_corr_setSParamDeviceEnabled (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViBoolean sParameterEnable);

ViStatus _VI_FUNC rsnrp_corr_getSParamDeviceEnabled (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViBoolean *sParameterCorrection);

ViStatus _VI_FUNC rsnrp_chan_configureSourceGammaCorr (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViBoolean sourceGammaCorrection,
                                                       ViReal64 magnitude,
                                                       ViReal64 phase);

ViStatus _VI_FUNC rsnrp_chan_setSourceGammaMagnitude (ViSession instrSession,
                                                      ViInt32 channel,
                                                      ViReal64 magnitude);

ViStatus _VI_FUNC rsnrp_chan_getSourceGammaMagnitude (ViSession instrSession,
                                                      ViInt32 channel,
                                                      ViReal64 *magnitude);

ViStatus _VI_FUNC rsnrp_chan_setSourceGammaPhase (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViReal64 phase);

ViStatus _VI_FUNC rsnrp_chan_getSourceGammaPhase (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViReal64 *phase);

ViStatus _VI_FUNC rsnrp_chan_setSourceGammaCorrEnabled (ViSession instrSession,
                                                        ViInt32 channel,
                                                        ViBoolean sourceGammaCorrection);

ViStatus _VI_FUNC rsnrp_chan_getSourceGammaCorrEnabled (ViSession instrSession,
                                                        ViInt32 channel,
                                                        ViBoolean *sourceGammaCorrection);

ViStatus _VI_FUNC rsnrp_chan_setContAvAperture (ViSession instrSession,
                                                ViInt32 channel,
                                                ViReal64 contAvAperture);

ViStatus _VI_FUNC rsnrp_chan_getContAvAperture (ViSession instrSession,
                                                ViInt32 channel,
                                                ViReal64 *contAvAperture);

ViStatus _VI_FUNC rsnrp_chan_setContAvSmoothingEnabled (ViSession instrSession,
                                                        ViInt32 channel,
                                                        ViBoolean contAvSmoothing);

ViStatus _VI_FUNC rsnrp_chan_getContAvSmoothingEnabled (ViSession instrSession,
                                                        ViInt32 channel,
                                                        ViBoolean *contAvSmoothing);

ViStatus _VI_FUNC rsnrp_chan_setContAvBufferedEnabled (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViBoolean contAvBufferedMode);

ViStatus _VI_FUNC rsnrp_chan_getContAvBufferedEnabled (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViBoolean *contAvBufferedMode);

ViStatus _VI_FUNC rsnrp_chan_setContAvBufferSize (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViInt32 bufferSize);

ViStatus _VI_FUNC rsnrp_chan_getContAvBufferSize (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViInt32 *bufferSize);

ViStatus _VI_FUNC rsnrp_chan_setBurstDropoutTolerance (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViReal64 dropoutTolerance);

ViStatus _VI_FUNC rsnrp_chan_getBurstDropoutTolerance (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViReal64 *dropoutTolerance);

ViStatus _VI_FUNC rsnrp_tGates_getCount (ViSession instrSession,
                                         ViInt32 *count);

ViStatus _VI_FUNC rsnrp_tGates_setSelect (ViSession instrSession,
                                          ViInt32 channel, ViInt32 selectGate);

ViStatus _VI_FUNC rsnrp_tGates_getSelect (ViSession instrSession,
                                          ViInt32 channel, ViInt32 *selectedGate);

ViStatus _VI_FUNC rsnrp_tGate_setOffsetTime (ViSession instrSession,
                                             ViInt32 channel, ViInt32 selectGate,
                                             ViReal64 offsetTime);

ViStatus _VI_FUNC rsnrp_tGate_getOffsetTime (ViSession instrSession,
                                             ViInt32 channel, ViInt32 selectGate,
                                             ViReal64 *offsetTime);

ViStatus _VI_FUNC rsnrp_tGate_setTime (ViSession instrSession,
                                       ViInt32 channel, ViInt32 selectGate,
                                       ViReal64 time);

ViStatus _VI_FUNC rsnrp_tGate_getTime (ViSession instrSession,
                                       ViInt32 channel, ViInt32 selectGate,
                                       ViReal64 *time);

ViStatus _VI_FUNC rsnrp_tslot_configureTimeSlot (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViInt32 timeSlotCount,
                                                 ViReal64 width);

ViStatus _VI_FUNC rsnrp_tslot_setTimeSlotCount (ViSession instrSession,
                                                ViInt32 channel,
                                                ViInt32 timeSlotCount);

ViStatus _VI_FUNC rsnrp_tslot_getTimeSlotCount (ViSession instrSession,
                                                ViInt32 channel,
                                                ViInt32 *timeSlotCount);

ViStatus _VI_FUNC rsnrp_tslot_setTimeSlotWidth (ViSession instrSession,
                                                ViInt32 channel, ViReal64 width);

ViStatus _VI_FUNC rsnrp_tslot_getTimeSlotWidth (ViSession instrSession,
                                                ViInt32 channel, ViReal64 *width);

ViStatus _VI_FUNC rsnrp_scope_configureScope (ViSession instrSession,
                                              ViInt32 channel,
                                              ViInt32 scopePoints,
                                              ViReal64 scopeTime,
                                              ViReal64 offsetTime,
                                              ViBoolean realtime);

ViStatus _VI_FUNC rsnrp_scope_setAverageEnabled (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViBoolean scopeAveraging);

ViStatus _VI_FUNC rsnrp_scope_getAverageEnabled (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViBoolean *scopeAveraging);

ViStatus _VI_FUNC rsnrp_scope_setAverageCount (ViSession instrSession,
                                               ViInt32 channel, ViInt32 count);

ViStatus _VI_FUNC rsnrp_scope_getAverageCount (ViSession instrSession,
                                               ViInt32 channel, ViInt32 *count);

ViStatus _VI_FUNC rsnrp_scope_setAverageTerminalControl (ViSession instrSession,
                                                         ViInt32 channel,
                                                         ViInt32 terminalControl);

ViStatus _VI_FUNC rsnrp_scope_getAverageTerminalControl (ViSession instrSession,
                                                         ViInt32 channel,
                                                         ViInt32 *terminalControl);

ViStatus _VI_FUNC rsnrp_scope_setOffsetTime (ViSession instrSession,
                                             ViInt32 channel,
                                             ViReal64 offsetTime);

ViStatus _VI_FUNC rsnrp_scope_getOffsetTime (ViSession instrSession,
                                             ViInt32 channel,
                                             ViReal64 *offsetTime);

ViStatus _VI_FUNC rsnrp_scope_setPoints (ViSession instrSession,
                                         ViInt32 channel, ViInt32 scopePoints);

ViStatus _VI_FUNC rsnrp_scope_getPoints (ViSession instrSession,
                                         ViInt32 channel, ViInt32 *scopePoints);

ViStatus _VI_FUNC rsnrp_scope_setRealtimeEnabled (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViBoolean realtime);

ViStatus _VI_FUNC rsnrp_scope_getRealtimeEnabled (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViBoolean *realtime);

ViStatus _VI_FUNC rsnrp_scope_setTime (ViSession instrSession,
                                       ViInt32 channel, ViReal64 scopeTime);

ViStatus _VI_FUNC rsnrp_scope_getTime (ViSession instrSession,
                                       ViInt32 channel, ViReal64 *scopeTime);

ViStatus _VI_FUNC rsnrp_trigger_configureInternal (ViSession instrSession,
                                                   ViInt32 channel,
                                                   ViReal64 triggerLevel,
                                                   ViInt32 triggerSlope);

ViStatus _VI_FUNC rsnrp_trigger_configureExternal (ViSession instrSession,
                                                   ViInt32 channel,
                                                   ViReal64 triggerDelay);

ViStatus _VI_FUNC rsnrp_trigger_immediate (ViSession instrSession,
                                           ViInt32 channel);

ViStatus _VI_FUNC rsnrp_trigger_setSynchronize (ViSession instrSession,
                                                ViBoolean state);

ViStatus _VI_FUNC rsnrp_trigger_getSynchronize (ViSession instrSession,
                                                ViBoolean *state);

ViStatus _VI_FUNC rsnrp_trigger_setAutoDelayEnabled (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViBoolean autoDelay);

ViStatus _VI_FUNC rsnrp_trigger_getAutoDelayEnabled (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViBoolean *autoDelay);

ViStatus _VI_FUNC rsnrp_trigger_setAutoTriggerEnabled (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViBoolean autoTrigger);

ViStatus _VI_FUNC rsnrp_trigger_getAutoTriggerEnabled (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViBoolean *autoTrigger);

ViStatus _VI_FUNC rsnrp_trigger_setCount (ViSession instrSession,
                                          ViInt32 channel, ViInt32 triggerCount);

ViStatus _VI_FUNC rsnrp_trigger_getCount (ViSession instrSession,
                                          ViInt32 channel,
                                          ViInt32 *triggerCount);

ViStatus _VI_FUNC rsnrp_trigger_setDelay (ViSession instrSession,
                                          ViInt32 channel, ViReal64 triggerDelay);

ViStatus _VI_FUNC rsnrp_trigger_getDelay (ViSession instrSession,
                                          ViInt32 channel,
                                          ViReal64 *triggerDelay);

ViStatus _VI_FUNC rsnrp_trigger_setHoldoff (ViSession instrSession,
                                            ViInt32 channel,
                                            ViReal64 triggerHoldoff);

ViStatus _VI_FUNC rsnrp_trigger_getHoldoff (ViSession instrSession,
                                            ViInt32 channel,
                                            ViReal64 *triggerHoldoff);

ViStatus _VI_FUNC rsnrp_trigger_setHysteresis (ViSession instrSession,
                                               ViInt32 channel,
                                               ViReal64 triggerHysteresis);

ViStatus _VI_FUNC rsnrp_trigger_getHysteresis (ViSession instrSession,
                                               ViInt32 channel,
                                               ViReal64 *triggerHysteresis);

ViStatus _VI_FUNC rsnrp_trigger_setLevel (ViSession instrSession,
                                          ViInt32 channel, ViReal64 triggerLevel);

ViStatus _VI_FUNC rsnrp_trigger_getLevel (ViSession instrSession,
                                          ViInt32 channel,
                                          ViReal64 *triggerLevel);

ViStatus _VI_FUNC rsnrp_trigger_setSlope (ViSession instrSession,
                                          ViInt32 channel, ViInt32 triggerSlope);

ViStatus _VI_FUNC rsnrp_trigger_getSlope (ViSession instrSession,
                                          ViInt32 channel, ViInt32 *triggerSlope);

ViStatus _VI_FUNC rsnrp_trigger_setSource (ViSession instrSession,
                                           ViInt32 channel,
                                           ViInt32 triggerSource);

ViStatus _VI_FUNC rsnrp_trigger_getSource (ViSession instrSession,
                                           ViInt32 channel,
                                           ViInt32 *triggerSource);

ViStatus _VI_FUNC rsnrp_chan_info (ViSession instrSession, ViInt32 channel,
                                   ViString infoType, ViInt32 arraySize,
                                   ViChar _VI_FAR info[]);

ViStatus _VI_FUNC rsnrp_chan_infoHeader (ViSession instrSession,
                                         ViInt32 channel,
                                         ViInt32 parameterNumber,
                                         ViInt32 arraySize, ViChar _VI_FAR header[]);

ViStatus _VI_FUNC rsnrp_chan_infosCount (ViSession instrSession,
                                         ViInt32 channel, ViInt32 *count);

ViStatus _VI_FUNC rsnrp_chan_abort (ViSession instrSession, ViInt32 channel);

ViStatus _VI_FUNC rsnrp_chan_initiate (ViSession instrSession,
                                       ViInt32 channel);

ViStatus _VI_FUNC rsnrp_chan_setInitContinuousEnabled (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViBoolean continuousInitiate);

ViStatus _VI_FUNC rsnrp_chan_getInitContinuousEnabled (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViBoolean *continuousInitiate);

ViStatus _VI_FUNC rsnrp_chan_reset (ViSession instrSession, ViInt32 channel);

ViStatus _VI_FUNC rsnrp_chan_setSamplingFrequency (ViSession instrSession,
                                                   ViInt32 channel,
                                                   ViInt32 samplingFrequency);

ViStatus _VI_FUNC rsnrp_chan_getSamplingFrequency (ViSession instrSession,
                                                   ViInt32 channel,
                                                   ViInt32 *samplingFrequency);

ViStatus _VI_FUNC rsnrp_chan_zero (ViSession instrSession, ViInt32 channel);

ViStatus _VI_FUNC rsnrp_chan_isZeroComplete (ViSession instrSession,
                                             ViInt32 channel,
                                             ViBoolean *zeroingComplete);

ViStatus _VI_FUNC rsnrp_chan_isMeasurementComplete (ViSession instrSession,
                                                    ViInt32 channel,
                                                    ViBoolean *measurementComplete);

ViStatus _VI_FUNC rsnrp_chan_selfTest (ViSession instrSession,
                                       ViInt32 channel, ViChar _VI_FAR result[]);

ViStatus _VI_FUNC rsnrp_offstbls_getCount (ViSession instrSession,
                                           ViInt32 *count);

ViStatus _VI_FUNC rsnrp_offstbl_defineName (ViSession instrSession,
                                            ViString tableName,
                                            ViInt32 tableNumber);

ViStatus _VI_FUNC rsnrp_offstbl_readName (ViSession instrSession,
                                          ViInt32 tableNumber, ViInt32 arraySize,
                                          ViChar _VI_FAR name[]);

ViStatus _VI_FUNC rsnrp_offstbl_defineElements (ViSession instrSession,
                                                ViInt32 tableNumber,
                                                ViInt32 count,
                                                ViReal64 frequencyArray[],
                                                ViReal64 magnitudeArray[]);

ViStatus _VI_FUNC rsnrp_offstbl_readElements (ViSession instrSession,
                                              ViInt32 tableNumber,
                                              ViInt32 *count,
                                              ViReal64 frequencyArray[],
                                              ViReal64 magnitudeArray[]);

ViStatus _VI_FUNC rsnrp_meass_abort (ViSession instrSession);

ViStatus _VI_FUNC rsnrp_meass_getCount (ViSession instrSession,
                                        ViInt32 *count);

ViStatus _VI_FUNC rsnrp_meass_initiate (ViSession instrSession);

ViStatus _VI_FUNC rsnrp_meass_sendSoftwareTrigger (ViSession instrSession);

ViStatus _VI_FUNC rsnrp_meass_isMeasurementComplete (ViSession instrSession,
                                                     ViBoolean *measurementCompleted);

ViStatus _VI_FUNC rsnrp_meas_abort (ViSession instrSession,
                                    ViInt32 measurement);

ViStatus _VI_FUNC rsnrp_meas_initiate (ViSession instrSession,
                                       ViInt32 measurement);

ViStatus _VI_FUNC rsnrp_meas_isMeasurementComplete (ViSession instrSession,
                                                    ViInt32 measurement,
                                                    ViBoolean *measurementComplete);

ViStatus _VI_FUNC rsnrp_meas_configureContAv (ViSession instrSession,
                                              ViInt32 measurement,
                                              ViInt32 measuringFunction,
                                              ViInt32 sensor1, ViInt32 sensor2,
                                              ViInt32 resolution);

ViStatus _VI_FUNC rsnrp_meas_configureContAvBuffered (ViSession instrSession,
                                                      ViInt32 measurement,
                                                      ViInt32 measuringFunction,
                                                      ViInt32 sensor1,
                                                      ViInt32 sensor2,
                                                      ViInt32 bufferSize,
                                                      ViInt32 resolution);

ViStatus _VI_FUNC rsnrp_meas_configureBurst (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViInt32 measuringFunction,
                                             ViInt32 sensor1, ViInt32 sensor2,
                                             ViReal64 dropoutTolerance,
                                             ViReal64 excludeStart,
                                             ViReal64 excludeStop,
                                             ViInt32 resolution);

ViStatus _VI_FUNC rsnrp_meas_configureTimeslot (ViSession instrSession,
                                                ViInt32 measurement,
                                                ViInt32 measuringFunction,
                                                ViInt32 sensor1, ViInt32 sensor2,
                                                ViReal64 timeslotWidth,
                                                ViReal64 numberOfTimeslots,
                                                ViReal64 excludeStart,
                                                ViReal64 excludeStop,
                                                ViInt32 resolution);

ViStatus _VI_FUNC rsnrp_meas_configureScope (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViInt32 sensor1, ViInt32 points,
                                             ViReal64 captureTime);

ViStatus _VI_FUNC rsnrp_meas_configureMath (ViSession instrSession,
                                            ViInt32 measurement, ViInt32 sensor1,
                                            ViInt32 operation, ViInt32 sensor2);

ViStatus _VI_FUNC rsnrp_meas_configureMathExt (ViSession instrSession,
                                               ViInt32 measurement,
                                               ViInt32 sensor1, ViInt32 eval1,
                                               ViInt32 gate1, ViInt32 operation,
                                               ViInt32 sensor2, ViInt32 eval2,
                                               ViInt32 gate2);

ViStatus _VI_FUNC rsnrp_meas_getMathPrimaryChannel (ViSession instrSession,
                                                    ViInt32 measurement,
                                                    ViInt32 *sensor1);

ViStatus _VI_FUNC rsnrp_meas_getMatchSecondaryChannel (ViSession instrSession,
                                                       ViInt32 measurement,
                                                       ViInt32 *sensor2);

ViStatus _VI_FUNC rsnrp_meas_getMathOperator (ViSession instrSession,
                                              ViInt32 measurement,
                                              ViInt32 *operation);

ViStatus _VI_FUNC rsnrp_meas_getMathPrimaryEval (ViSession instrSession,
                                                 ViInt32 measurement,
                                                 ViInt32 *eval1);

ViStatus _VI_FUNC rsnrp_meas_getMathSecondaryEval (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViInt32 *eval2);

ViStatus _VI_FUNC rsnrp_meas_getMathPrimaryGate (ViSession instrSession,
                                                 ViInt32 measurement,
                                                 ViInt32 *gate1);

ViStatus _VI_FUNC rsnrp_meas_getMathSecondaryGate (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViInt32 *gate2);

ViStatus _VI_FUNC rsnrp_limit_configureLimits (ViSession instrSession,
                                               ViInt32 measurement,
                                               ViBoolean beep,
                                               ViBoolean lowerCheck,
                                               ViReal64 lowerLimitPower,
                                               ViBoolean upperCheck,
                                               ViReal64 upperLimitPower);

ViStatus _VI_FUNC rsnrp_limit_configureLimitsRatio (ViSession instrSession,
                                                    ViInt32 measurement,
                                                    ViBoolean beep,
                                                    ViBoolean lowerCheck,
                                                    ViReal64 lowerLimitRatio,
                                                    ViBoolean upperCheck,
                                                    ViReal64 upperLimitRatio);

ViStatus _VI_FUNC rsnrp_limit_setAutoClearEnabled (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViBoolean autoClear);

ViStatus _VI_FUNC rsnrp_limit_getAutoClearEnabled (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViBoolean *autoClear);

ViStatus _VI_FUNC rsnrp_limit_setBeep (ViSession instrSession,
                                       ViInt32 measurement, ViBoolean beep);

ViStatus _VI_FUNC rsnrp_limit_getBeep (ViSession instrSession,
                                       ViInt32 measurement, ViBoolean *beepState);

ViStatus _VI_FUNC rsnrp_limit_getFail (ViSession instrSession,
                                       ViInt32 measurement,
                                       ViBoolean *limitExceeded);

ViStatus _VI_FUNC rsnrp_limit_getFailCount (ViSession instrSession,
                                            ViInt32 measurement,
                                            ViInt32 *failCount);

ViStatus _VI_FUNC rsnrp_limit_clearFailCount (ViSession instrSession,
                                              ViInt32 measurement);

ViStatus _VI_FUNC rsnrp_limit_setLowerPower (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 lowerLimitPower);

ViStatus _VI_FUNC rsnrp_limit_getLowerPower (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 *lowerLimit);

ViStatus _VI_FUNC rsnrp_limit_setLowerRatio (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 lowerLimitRatio);

ViStatus _VI_FUNC rsnrp_limit_getLowerRatio (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 *lowerLimitRatio);

ViStatus _VI_FUNC rsnrp_limit_setLowerEnabled (ViSession instrSession,
                                               ViInt32 measurement,
                                               ViBoolean lowerCheck);

ViStatus _VI_FUNC rsnrp_limit_getLowerEnabled (ViSession instrSession,
                                               ViInt32 measurement,
                                               ViBoolean *lowerCheck);

ViStatus _VI_FUNC rsnrp_limit_setUpperPower (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 upperLimitPower);

ViStatus _VI_FUNC rsnrp_limit_getUpperPower (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 *upperLimit);

ViStatus _VI_FUNC rsnrp_limit_setUpperRatio (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 upperLimitRatio);

ViStatus _VI_FUNC rsnrp_limit_getUpperRatio (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 *upperLimitRatio);

ViStatus _VI_FUNC rsnrp_limit_setUpperEnabled (ViSession instrSession,
                                               ViInt32 measurement,
                                               ViBoolean upperCheck);

ViStatus _VI_FUNC rsnrp_limit_getUpperEnabled (ViSession instrSession,
                                               ViInt32 measurement,
                                               ViBoolean *upperCheck);

ViStatus _VI_FUNC rsnrp_relative_configurePower (ViSession instrSession,
                                                 ViInt32 measurement,
                                                 ViReal64 referencePower);

ViStatus _VI_FUNC rsnrp_relative_configureRatio (ViSession instrSession,
                                                 ViInt32 measurement,
                                                 ViReal64 referenceRatio);

ViStatus _VI_FUNC rsnrp_relative_setRefValuePower (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViReal64 referencePower);

ViStatus _VI_FUNC rsnrp_relative_getRefValuePower (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViReal64 *referencePower);

ViStatus _VI_FUNC rsnrp_relative_setRefValueRatio (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViReal64 referenceRatio);

ViStatus _VI_FUNC rsnrp_relative_getRefValueRatio (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViReal64 *referencePower);

ViStatus _VI_FUNC rsnrp_relative_setRefValueState (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViBoolean relative);

ViStatus _VI_FUNC rsnrp_relative_getRefValueState (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViBoolean *reference);

ViStatus _VI_FUNC rsnrp_relative_resetRefValue (ViSession instrSession,
                                                ViInt32 measurement);

ViStatus _VI_FUNC rsnrp_meas_setUnits (ViSession instrSession,
                                       ViInt32 measurement, ViInt32 units);

ViStatus _VI_FUNC rsnrp_meas_getUnits (ViSession instrSession,
                                       ViInt32 measurement, ViInt32 *units);

ViStatus _VI_FUNC rsnrp_meas_setUnitsRatio (ViSession instrSession,
                                            ViInt32 measurement, ViInt32 units);

ViStatus _VI_FUNC rsnrp_meas_getUnitsRatio (ViSession instrSession,
                                            ViInt32 measurement, ViInt32 *units);

ViStatus _VI_FUNC rsnrp_meas_measureContAv (ViSession instrSession,
                                            ViInt32 measurement,
                                            ViInt32 measuringFunction,
                                            ViInt32 sensor1, ViInt32 sensor2,
                                            ViInt32 resolution,
                                            ViReal64 *measuredValue);

ViStatus _VI_FUNC rsnrp_meas_measureContAvBuffered (ViSession instrSession,
                                                    ViInt32 measurement,
                                                    ViInt32 measuringFunction,
                                                    ViInt32 sensor1,
                                                    ViInt32 sensor2,
                                                    ViInt32 bufferSize,
                                                    ViInt32 resolution,
                                                    ViReal64 measuredValues[]);

ViStatus _VI_FUNC rsnrp_meas_measureBurst (ViSession instrSession,
                                           ViInt32 measurement,
                                           ViInt32 measuringFunction,
                                           ViInt32 sensor1, ViInt32 sensor2,
                                           ViReal64 dropoutTolerance,
                                           ViReal64 excludeStart,
                                           ViReal64 excludeStop,
                                           ViInt32 resolution,
                                           ViReal64 *measuredValue);

ViStatus _VI_FUNC rsnrp_meas_measureTimeslot (ViSession instrSession,
                                              ViInt32 measurement,
                                              ViInt32 measuringFunction,
                                              ViInt32 sensor1, ViInt32 sensor2,
                                              ViReal64 timeslotWidth,
                                              ViReal64 numberOfTimeslots,
                                              ViReal64 excludeStart,
                                              ViReal64 excludeStop,
                                              ViInt32 resolution,
                                              ViReal64 *measuredValue);

ViStatus _VI_FUNC rsnrp_meas_measureScope (ViSession instrSession,
                                           ViInt32 measurement, ViInt32 sensor1,
                                           ViInt32 points, ViReal64 captureTime,
                                           ViReal64 *measuredValue);

ViStatus _VI_FUNC rsnrp_meas_read (ViSession instrSession,
                                   ViInt32 measurement, ViReal64 *measuredValue);

ViStatus _VI_FUNC rsnrp_meas_readArray (ViSession instrSession,
                                        ViInt32 measurement,
                                        ViReal64 measuredValues[]);

ViStatus _VI_FUNC rsnrp_meas_readTimeslot (ViSession instrSession,
                                           ViInt32 measurement,
                                           ViInt32 arraySize,
                                           ViReal64 measuredValues[]);

ViStatus _VI_FUNC rsnrp_meas_readScope (ViSession instrSession,
                                        ViInt32 measurement,
                                        ViReal64 measuredValues[]);

ViStatus _VI_FUNC rsnrp_meas_fetch (ViSession instrSession,
                                    ViInt32 measurement, ViReal64 *measuredValue);

ViStatus _VI_FUNC rsnrp_meas_fetchArray (ViSession instrSession,
                                         ViInt32 measurement,
                                         ViReal64 measuredValues[]);

ViStatus _VI_FUNC rsnrp_meas_fetchTimeslot (ViSession instrSession,
                                            ViInt32 measurement,
                                            ViInt32 arraySize,
                                            ViReal64 measuredValues[]);

ViStatus _VI_FUNC rsnrp_meas_fetchScope (ViSession instrSession,
                                         ViInt32 measurement,
                                         ViReal64 measuredValues[]);

ViStatus _VI_FUNC rsnrp_meas_fetchSecondaryValue (ViSession instrSession,
                                                  ViInt32 measurement,
                                                  ViInt32 valueType,
                                                  ViReal64 *secondaryValue);

ViStatus _VI_FUNC rsnrp_meas_resetExtremes (ViSession instrSession,
                                            ViInt32 measurement);

ViStatus _VI_FUNC rsnrp_rfOsc_setEnabled (ViSession instrSession,
                                          ViBoolean referenceOscillator);

ViStatus _VI_FUNC rsnrp_rfOsc_getEnabled (ViSession instrSession,
                                          ViBoolean *referenceOscillator);

ViStatus _VI_FUNC rsnrp_outputs_getCount (ViSession instrSession,
                                          ViInt32 *analogOutputs);

ViStatus _VI_FUNC rsnrp_output_setMode (ViSession instrSession,
                                        ViInt32 outputNumber,
                                        ViBoolean outputState,
                                        ViInt32 outputType, ViInt32 source);

ViStatus _VI_FUNC rsnrp_output_getMode (ViSession instrSession,
                                        ViInt32 outputNumber,
                                        ViBoolean *analogState,
                                        ViBoolean *TTLState,
                                        ViInt32 *analogSource,
                                        ViInt32 *TTLSource);

ViStatus _VI_FUNC rsnrp_output_setSource (ViSession instrSession,
                                          ViInt32 outputNumber,
                                          ViInt32 outputType, ViInt32 source);

ViStatus _VI_FUNC rsnrp_output_getSource (ViSession instrSession,
                                          ViInt32 outputNumber,
                                          ViInt32 outputType, ViInt32 *source);

ViStatus _VI_FUNC rsnrp_output_setState (ViSession instrSession,
                                         ViInt32 outputNumber,
                                         ViInt32 outputType,
                                         ViBoolean outputState);

ViStatus _VI_FUNC rsnrp_output_getState (ViSession instrSession,
                                         ViInt32 outputNumber,
                                         ViInt32 outputType,
                                         ViBoolean *outputState);

ViStatus _VI_FUNC rsnrp_output_setLowerPowerEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 lowerPower);

ViStatus _VI_FUNC rsnrp_output_getLowerPowerEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 *lowerPower);

ViStatus _VI_FUNC rsnrp_output_setLowerRatioEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 lowerRatio);

ViStatus _VI_FUNC rsnrp_output_getLowerRatioEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 *lowerRatio);

ViStatus _VI_FUNC rsnrp_output_setUpperPowerEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 upperPower);

ViStatus _VI_FUNC rsnrp_output_getUpperPowerEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 *upperPower);

ViStatus _VI_FUNC rsnrp_output_setUpperRatioEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 upperRatio);

ViStatus _VI_FUNC rsnrp_output_getUpperRatioEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 *upperRatio);

ViStatus _VI_FUNC rsnrp_output_setPassVoltage (ViSession instrSession,
                                               ViReal64 passVoltage);

ViStatus _VI_FUNC rsnrp_output_getPassVoltage (ViSession instrSession,
                                               ViReal64 *passVoltage);

ViStatus _VI_FUNC rsnrp_output_setFailVoltage (ViSession instrSession,
                                               ViReal64 failVoltage);

ViStatus _VI_FUNC rsnrp_output_getFailVoltage (ViSession instrSession,
                                               ViReal64 *failVoltage);

ViStatus _VI_FUNC rsnrp_states_getCount (ViSession instrSession,
                                         ViInt32 *count);

ViStatus _VI_FUNC rsnrp_state_defineName (ViSession instrSession,
                                          ViString setupName,
                                          ViInt32 setupNumber);

ViStatus _VI_FUNC rsnrp_state_readName (ViSession instrSession,
                                        ViInt32 setupNumber, ViChar _VI_FAR name[]);

ViStatus _VI_FUNC rsnrp_state_recall (ViSession instrSession,
                                      ViInt32 setupNumber);

ViStatus _VI_FUNC rsnrp_state_save (ViSession instrSession,
                                    ViInt32 setupNumber);

ViStatus _VI_FUNC rsnrp_display_getPIXMap (ViSession instrSession,
                                           ViChar _VI_FAR pixelMapData[]);

ViStatus _VI_FUNC rsnrp_display_hideMessage (ViSession instrSession);

ViStatus _VI_FUNC rsnrp_display_hideQuery (ViSession instrSession);

ViStatus _VI_FUNC rsnrp_display_setIlluminationEnabled (ViSession instrSession,
                                                        ViBoolean illumination);

ViStatus _VI_FUNC rsnrp_display_getIlluminationEnabled (ViSession instrSession,
                                                        ViBoolean *illumination);

ViStatus _VI_FUNC rsnrp_display_isQueryDone (ViSession instrSession,
                                             ViBoolean *queryDone);

ViStatus _VI_FUNC rsnrp_display_showMessage (ViSession instrSession,
                                             ViString message);

ViStatus _VI_FUNC rsnrp_display_showQuery (ViSession instrSession,
                                           ViString message, ViBoolean blocking,
                                           ViUInt32 timeout);

ViStatus _VI_FUNC rsnrp_display_setUpdate (ViSession instrumentHandle,
                                           ViInt32 update);

ViStatus _VI_FUNC rsnrp_display_getUpdate (ViSession instrumentHandle,
                                           ViInt32 *update);

ViStatus _VI_FUNC rsnrp_display_setShowErrorlist (ViSession instrumentHandle,
                                                  ViBoolean state);

ViStatus _VI_FUNC rsnrp_display_getShowErrorlist (ViSession instrumentHandle,
                                                  ViBoolean *state);

ViStatus _VI_FUNC rsnrp_service_setBatteryAuto (ViSession instrSession,
                                                ViBoolean autoState);

ViStatus _VI_FUNC rsnrp_service_getBatteryAuto (ViSession instrSession,
                                                ViBoolean *autoState);

ViStatus _VI_FUNC rsnrp_service_getDetectorTemperature (ViSession instrSession,
                                                        ViInt32 channel,
                                                        ViReal64 *temperature);

ViStatus _VI_FUNC rsnrp_service_SMBReadWord (ViSession instrSession,
                                             ViInt32 i2CAddress,
                                             ViInt32 SMBusCommand,
                                             ViUInt16 *word);

ViStatus _VI_FUNC rsnrp_service_SMBReadString (ViSession instrSession,
                                               ViInt32 i2CAddress,
                                               ViInt32 SMBusCommand,
                                               ViChar _VI_FAR string[]);

ViStatus _VI_FUNC rsnrp_service_SMBWriteWord (ViSession instrSession,
                                              ViInt32 i2CAddress,
                                              ViInt32 SMBusCommand,
                                              ViUInt16 word);

ViStatus _VI_FUNC rsnrp_service_startSimulation (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViInt32 blockCount);

ViStatus _VI_FUNC rsnrp_service_setSimulationValues (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViInt32 valueCount[],
                                                     ViReal64 values[]);

ViStatus _VI_FUNC rsnrp_service_stopSimulation (ViSession instrSession,
                                                ViInt32 channel);

ViStatus _VI_FUNC rsnrp_system_beepImmediate (ViSession instrSession);

ViStatus _VI_FUNC rsnrp_system_beepStop (ViSession instrSession);

ViStatus _VI_FUNC rsnrp_system_setBeepTime (ViSession instrSession,
                                            ViReal64 beepTime);

ViStatus _VI_FUNC rsnrp_system_getBeepTime (ViSession instrSession,
                                            ViReal64 *beepTime);

ViStatus _VI_FUNC rsnrp_system_setBeepOnOverload (ViSession instrumentHandle,
                                                  ViBoolean state);

ViStatus _VI_FUNC rsnrp_system_getBeepOnOverload (ViSession instrumentHandle,
                                                  ViBoolean *state);

ViStatus _VI_FUNC rsnrp_system_setKeyclick (ViSession instrSession,
                                            ViBoolean state);

ViStatus _VI_FUNC rsnrp_system_getKeyclick (ViSession instrSession,
                                            ViBoolean *state);

ViStatus _VI_FUNC rsnrp_system_setClockTime (ViSession instrSession,
                                             ViInt32 hour, ViInt32 minute,
                                             ViInt32 second);

ViStatus _VI_FUNC rsnrp_system_getClockTime (ViSession instrSession,
                                             ViInt32 *hour, ViInt32 *minute,
                                             ViInt32 *second);

ViStatus _VI_FUNC rsnrp_system_setDate (ViSession instrSession, ViInt32 year,
                                        ViInt32 month, ViInt32 day);

ViStatus _VI_FUNC rsnrp_system_getDate (ViSession instrSession,
                                        ViInt32 *year, ViInt32 *month,
                                        ViInt32 *day);

ViStatus _VI_FUNC rsnrp_system_info (ViSession instrSession,
                                     ViString infoType, ViInt32 arraySize,
                                     ViChar _VI_FAR info[]);

ViStatus _VI_FUNC rsnrp_system_infoHeader (ViSession instrSession,
                                           ViInt32 parameterNumber,
                                           ViInt32 arraySize,
                                           ViChar _VI_FAR header[]);

ViStatus _VI_FUNC rsnrp_system_infosCount (ViSession instrSession,
                                           ViInt32 *count);

ViStatus _VI_FUNC rsnrp_system_keyCharValue (ViSession instrSession,
                                             ViInt32 simulatedKey);

ViStatus _VI_FUNC rsnrp_system_keyNumeric (ViSession instrSession,
                                           ViInt32 simulatedKey);

ViStatus _VI_FUNC rsnrp_system_remoteState (ViSession instrSession,
                                            ViBoolean *remoteState);

ViStatus _VI_FUNC rsnrp_system_setSpeed (ViSession instrSession,
                                         ViInt32 speed);

ViStatus _VI_FUNC rsnrp_system_getSpeed (ViSession instrSession,
                                         ViInt32 *speed);

ViStatus _VI_FUNC rsnrp_error_setExtendedMessage (ViSession instrSession,
                                                  ViBoolean state);

ViStatus _VI_FUNC rsnrp_error_getExtendedMessage (ViSession instrSession,
                                                  ViBoolean *state);

ViStatus _VI_FUNC rsnrp_system_SCPIversion (ViSession instrSession,
                                            ViInt32 arraySize,
                                            ViChar _VI_FAR systemVersion[]);

ViStatus _VI_FUNC rsnrp_windows_getCount (ViSession instrSession,
                                          ViInt32 *count);

ViStatus _VI_FUNC rsnrp_windows_setSelected (ViSession instrSession,
                                             ViInt32 window);

ViStatus _VI_FUNC rsnrp_windows_getSelected (ViSession instrSession,
                                             ViInt32 *selectedWindow);

ViStatus _VI_FUNC rsnrp_windows_setSize (ViSession instrSession,
                                         ViInt32 windowSize);

ViStatus _VI_FUNC rsnrp_windows_getSize (ViSession instrSession,
                                         ViInt32 *windowSize);

ViStatus _VI_FUNC rsnrp_window_configureWindowAnalog (ViSession instrSession,
                                                      ViInt32 window,
                                                      ViInt32 resolution,
                                                      ViInt32 units,
                                                      ViReal64 lowerScale,
                                                      ViReal64 upperScale);

ViStatus _VI_FUNC rsnrp_window_configureWindowDigital (ViSession instrSession,
                                                       ViInt32 window,
                                                       ViInt32 resolution,
                                                       ViInt32 auxiliaryValue);

ViStatus _VI_FUNC rsnrp_window_configureWindowTrace (ViSession instrSession,
                                                     ViInt32 window,
                                                     ViReal64 lowerScale,
                                                     ViReal64 upperScale);

ViStatus _VI_FUNC rsnrp_window_setAuxiliaryValue (ViSession instrSession,
                                                  ViInt32 window,
                                                  ViInt32 auxiliaryValue);

ViStatus _VI_FUNC rsnrp_window_getAuxiliaryValue (ViSession instrSession,
                                                  ViInt32 window,
                                                  ViInt32 *auxiliaryValue);

ViStatus _VI_FUNC rsnrp_window_setFormat (ViSession instrSession,
                                          ViInt32 window, ViInt32 format);

ViStatus _VI_FUNC rsnrp_window_getFormat (ViSession instrSession,
                                          ViInt32 window, ViInt32 *format);

ViStatus _VI_FUNC rsnrp_window_meterAuto (ViSession instrSession,
                                          ViInt32 window);

ViStatus _VI_FUNC rsnrp_window_setMeterLowerPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 lowerPower);

ViStatus _VI_FUNC rsnrp_window_getMeterLowerPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 *lowerPower);

ViStatus _VI_FUNC rsnrp_window_setMeterLowerRatio (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 lowerRatio);

ViStatus _VI_FUNC rsnrp_window_getMeterLowerRatio (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 *lowerRatio);

ViStatus _VI_FUNC rsnrp_window_setMeterUpperPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 upperPower);

ViStatus _VI_FUNC rsnrp_window_getMeterUpperPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 *upperPower);

ViStatus _VI_FUNC rsnrp_window_setMeterUpperRatio (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 upperRatio);

ViStatus _VI_FUNC rsnrp_window_getMeterUpperRatio (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 *upperRatio);

ViStatus _VI_FUNC rsnrp_window_setResolution (ViSession instrSession,
                                              ViInt32 window, ViInt32 resolution);

ViStatus _VI_FUNC rsnrp_window_getResolution (ViSession instrSession,
                                              ViInt32 window,
                                              ViInt32 *resolution);

ViStatus _VI_FUNC rsnrp_window_setTimeslot (ViSession instrSession,
                                            ViInt32 window, ViInt32 timeslot);

ViStatus _VI_FUNC rsnrp_window_getTimeslot (ViSession instrSession,
                                            ViInt32 window, ViInt32 *timeslot);

ViStatus _VI_FUNC rsnrp_window_setTitle (ViSession instrSession,
                                         ViInt32 window, ViString title);

ViStatus _VI_FUNC rsnrp_window_getTitle (ViSession instrSession,
                                         ViInt32 window, ViInt32 arraySize,
                                         ViChar _VI_FAR title[]);

ViStatus _VI_FUNC rsnrp_window_setTraceLowerPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 lowerPower);

ViStatus _VI_FUNC rsnrp_window_getTraceLowerPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 *lowerPower);

ViStatus _VI_FUNC rsnrp_window_setTraceUpperPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 upperPower);

ViStatus _VI_FUNC rsnrp_window_getTraceUpperPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 *upperPower);

ViStatus _VI_FUNC rsnrp_window_setVisible (ViSession instrSession,
                                           ViInt32 window, ViBoolean visible);

ViStatus _VI_FUNC rsnrp_window_getVisible (ViSession instrSession,
                                           ViInt32 window, ViBoolean *visible);

ViStatus _VI_FUNC rsnrp_status_checkCondition (ViSession instrSession,
                                               ViInt32 statusClass,
                                               ViUInt32 mask, ViBoolean *state);

ViStatus _VI_FUNC rsnrp_status_catchEvent (ViSession instrSession,
                                           ViInt32 statusClass, ViUInt32 mask,
                                           ViInt32 direction);

ViStatus _VI_FUNC rsnrp_status_checkEvent (ViSession instrSession,
                                           ViInt32 statusClass, ViUInt32 mask,
                                           ViUInt32 resetMask, ViBoolean *events);

ViStatus _VI_FUNC rsnrp_status_enableEventNotification (ViSession instrSession,
                                                        ViInt32 statusClass,
                                                        ViUInt32 mask);

ViStatus _VI_FUNC rsnrp_status_disableEventNotification (ViSession instrSession,
                                                         ViInt32 statusClass,
                                                         ViUInt32 mask);

ViStatus _VI_FUNC rsnrp_status_preset (ViSession instrSession);

ViStatus _VI_FUNC rsnrp_errorCheckState (ViSession instrSession,
				ViBoolean stateChecking);
ViStatus _VI_FUNC rsnrp_writeInstrData (ViSession instrSession,
				ViString writeBuffer);
ViStatus _VI_FUNC rsnrp_readInstrData (ViSession instrSession,
				ViInt32 numberBytesToRead,
				ViChar _VI_FAR readBuffer[],
				ViPInt32 numBytesRead);
ViStatus _VI_FUNC rsnrp_queryInstrData (ViSession instrSession,
				ViString writeBuffer,
				ViInt32 numberBytesToRead,
				ViChar _VI_FAR readBuffer[],
				ViPInt32 numBytesRead);
ViStatus _VI_FUNC rsnrp_reset (ViSession instrSession);
ViStatus _VI_FUNC rsnrp_self_test (ViSession instrSession,
				ViPInt16 selfTestResult,
				ViChar _VI_FAR selfTestMessage[]);
ViStatus _VI_FUNC rsnrp_error_query (ViSession instrSession, ViPInt32 errorCode,
				ViChar _VI_FAR errorMessage[]);
ViStatus _VI_FUNC rsnrp_error_message (ViSession instrSession,
				ViStatus statusCode,
				ViChar _VI_FAR message[]);
ViStatus _VI_FUNC rsnrp_revision_query (ViSession instrSession,
				ViChar _VI_FAR instrumentDriverRevision[],
				ViChar _VI_FAR firmwareRevision[]);
ViStatus _VI_FUNC rsnrp_close (ViSession instrSession);


#if defined(__cplusplus) || defined(__cplusplus__)
}
#endif

/*****************************************************************************/
/*=== END INCLUDE FILE ======================================================*/
/*****************************************************************************/

#endif
