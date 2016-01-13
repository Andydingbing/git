/*= R&S NRP Power Meter =====================================================*/
/* LabWindows/CVI 6.0 VXIpnp Instrument Driver                               */
/* Original Release: February 2003                                           */
/* By: Martin Koutny                                                         */
/*                                                                           */
/* Should you have any technical questions please contact the hotline of     */
/* Rohde & Schwarz Vertriebs-GmbH Rohde & Schwarz Support Center             */
/*                                                                           */
/* e-mail: CustomerSupport@rsd.rohde-schwarz.com                             */
/*                                                                           */
/* Modification History:                                                     */
/*  Rev 2.2, 08/2004, CVI 6.0 by Martin Krcmar                               */
/*      Fixed function rsnrp_tslot_setTimeSlotCount                          */
/*                                                                           */
/*  Rev 2.1, 01/2004, CVI 7.0 by Karel Pisala                                */
/*      Added new function for firmware revision 3.01:                       */
/*        rsnrp_display_setUpdate                                            */
/*        rsnrp_display_getUpdate                                            */
/*        rsnrp_display_setShowErrorlist                                     */
/*        rsnrp_display_getShowErrorlist                                     */
/*        rsnrp_error_setExtendedMessage                                     */
/*        rsnrp_error_getExtendedMessage                                     */
/*        rsnrp_system_setBeepOnOverload                                     */
/*        rsnrp_system_getBeepOnOverload                                     */
/*        rsnrp_system_setKeyclick                                           */
/*        rsnrp_system_getKeyclick                                           */
/*        rsnrp_meas_configureMathExt                                        */
/*        rsnrp_meas_getMathPrimaryEval                                      */
/*        rsnrp_meas_getMathSecondaryEval                                    */
/*        rsnrp_meas_getMathPrimaryGate                                      */
/*        rsnrp_meas_getMathSecondaryGate                                    */
/*        rsnrp_trigger_setSynchronize                                       */
/*        rsnrp_trigger_getSynchronize                                       */
/*      Updated function:                                                    */
/*        rsnrp_meas_configureMath                                           */
/*      Changed:                                                             */
/*        Checking of connected sensors                                      */
/*                                                                           */
/*  Rev 2.0, 05/2003, CVI 6.0 by Jiri Kominek                                */
/*      Initial Release, except core functions all is new                    */
/*                                                                           */
/*  Rev 1.0, 02/2003, CVI 6.0 by M.K.                                        */
/*      Beta Release - Only Core functions are supported                     */
/*                                                                           */
/*===========================================================================*/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include "rsnrp.h"
#include "visa.h"
/*= DEFINES =================================================================*/
#define RSNRP_REVISION     "Rev 2.2, 08/2004, CVI 6.0" /* Instrument driver revision */
#define BUFFER_SIZE             1024L   /* File I/O buffer size */

#define RSNRP_DEFAULT_TIMEOUT   10000   /* Default Timeout value in ms (10 sec) */
#define IEEE_ERROR_BITS         0x3C    /* AND mask for ESR checking */
#define MAX_MEASUREMENTS        8    /* Maximum number of calc. blocks */
#define MAX_CHANNELS            4   /* Maximum number of channels */
#define MAX_OUTPUTS             2   /* Maximum number of analog outputs */
#define MAX_MEM_PLACE           19 /* Maximum number of memory storage places */

/*= Useful Macros ===========================================================*/
#define REG(command, sreMask, statusMask)  { command, sreMask, statusMask},

/* Error checking macro */
#define CHECKERR(fCal) if (rsnrp_status = (fCal), rsnrp_status < VI_SUCCESS) \
    return rsnrp_status; else

/* Error checking macro with clean up (rsnrp_init only) */
#define CHECKERR_CU(fCal) if (rsnrp_status = (fCal), rsnrp_status < VI_SUCCESS) \
    return rsnrp_initCleanUp (rmSession, instrSession, rsnrp_status); else

/* Check presency of instrument's option (when error checking is enabled) */
#define CHECKOPT(option) if (rsnrp_status = (instrPtr -> instrOption & (option)) ? \
    VI_SUCCESS : (instrPtr -> errorChecking) ? RSNRP_ERROR_INSTRUMENT_OPTION : VI_SUCCESS, \
    rsnrp_status < VI_SUCCESS) \
    return rsnrp_status; else

/* Is the result of scanning (sscanf) function OK? If not, return error code. */
#define CHECKSCAN(fCal) if ((fCal) < 0) \
    return VI_ERROR_INSTR_INTERPRETING_RESPONSE; else

/* Compare firmware version. Firmware version must be greater or equal then specified version */
/* firmware version is stored in integer (major * 100 + minor)                                */
/*    eg. 301 is version 3.01, 310 is version 3.10                                            */
/*  if you want specified version 3.1 you must set CHECKFWREVISION(3,10)                      */
#define CHECKFWREVISION(major, minor) \
    if ((instrPtr->firmwareVersion) < (major * 100 + minor)) \
        return RSNRP_ERROR_INVALID_FIRMWARE_VERSION; \
    else
        

/* Declare and get pointer to session structure */
#define GETSESSIONDATA(instrSession) rsnrp_instrRange instrPtr;\
    if ((rsnrp_status = viGetAttribute(instrSession,VI_ATTR_USER_DATA, &instrPtr)) < VI_SUCCESS)\
        return rsnrp_status;

/*--- Instrument Options (See Default Instrument Setup for more details) ---*/
#define RSNRP_OPT             0x0   /* No options included */
#define RSNRP_OPT_B1          0x1   /* Test generator installed */
#define RSNRP_OPT_B2          0x2   /* Second channel interface installed */
#define RSNRP_OPT_B3          0x4   /* Battery installed */
#define RSNRP_OPT_B4          0x8   /* Ethernet interface installed */
#define RSNRP_OPT_B5          0x10  /* Four channel interface installed */
#define RSNRP_OPT_B6          0x20  /* Sensor A/B rear socket */

/*****************************************************************************/
/*= INSTRUMENT-DEPENDENT COMMAND ARRAYS =====================================*/
/*****************************************************************************/
static ViString directionArr[] = {"PTR","NTR",VI_NULL};
static ViString switchArr[] = {"OFF","ON",VI_NULL};
static ViString measModeArr[] = {"\"POW:AVG\"", "\"POW:AVG\"", "\"POW:TSL:AVG\"",
                                "\"POW:BURS:AVG\"", "\"XTIM:POW\"",
                                "\"POW:TGAT:AVG\"", VI_NULL};
static ViString autoTypeArr[] = {"RES","NSR",VI_NULL};
static ViString tControlArr[] = {"MOV","REP",VI_NULL};
static ViString slopeArr[] = {"POS","NEG",VI_NULL};
static ViString trgSourceArr[] = {"BUS","EXT","HOLD","IMM","INT",VI_NULL};
static ViString sampFreqArr[] = {"UNUSED","FREQ1","FREQ2",VI_NULL};
static ViString measFuncArr[] = {"AVG","REL","DIFF","DIFF:REL","SUM","SUM:REL",
                                 "RAT","RAT:REL","SWR","REFL","RLOS",VI_NULL};
static ViReal64 lowLimitArray[] = {-9.99999E5, -120.0, -13.0103, -100.0, -200.0,-1.0E20};
static ViReal64 highLimitArray[] = {9.99999E5, 90.0, 196.897, 999.99, 200.0,1.0E20};
static ViString unitsArr[] = {"W","DBM","DBUV","DB", "DPCT","O",VI_NULL};
static ViString typeArr[] = {"MIN","MAX","PTP",VI_NULL};
static ViString keyArr[] = {"SK1L","SK1R","SK2L","SK2R","SK3L","SK3R","SK4L",
                            "SK4R","SK5L","SK5R","SK6L","SK6R","PRES","ZERO",
                            "FREQ","CONT","POW","DEL","ENT","ESC","UP","DOWN",
                            "LEFT","RIGH",VI_NULL};
static ViString speedArr[] = {"NORM","FAST",VI_NULL};
static ViString windowArr[] = {"NORM","ZOOM",VI_NULL};
static ViString auxArr[] = {"OFF","MIN","MAX","PTP",VI_NULL}; // {"NONE","MIN","MAX","PTP",VI_NULL};
static ViString formatArr[] = {"DIG","ANAL",VI_NULL};
static ViString outpTypeArr[] = {"REC","TTL",VI_NULL};
static ViString displayUpdateArr[] = {"NORM", "SLOW", "FRE", VI_NULL};
/*===========================================================================*/

/*****************************************************************************/
/*= INSTRUMENT-DEPENDENT STATUS/RANGE STRUCTURE  ============================*/
/*****************************************************************************/
/* rsnrp_stringValPair is used in the rsnrp_error_message function           */
/* rsnrp_statusDataRanges is used to track session dependent status & ranges */
/*===========================================================================*/
typedef struct
{
    const ViString      command;
    ViInt16             sreMask;
    ViInt16             statusMask;
} SRegEntry;

typedef SRegEntry NrpRegTable[];
/*
 * table for registers update
 */
static NrpRegTable gs_NrpRegs = 
{
REG("",                 -1,     -1)
REG("STAT:DEV?",        2,      -1)
REG("STAT:DEV?",        2,      -1)
REG("STAT:OPER:CAL?",   128,    1)
REG("STAT:OPER:MEAS?",  128,    16)
REG("STAT:OPER:TRIG?",  128,    32)
REG("STAT:OPER:SENS?",  128,    1024)
REG("STAT:OPER:LLIM?",  128,    2048)
REG("STAT:OPER:ULIM?",  128,    4096)
REG("STAT:QUES:POW?",   8,      8)
REG("STAT:QUES:WIND?",  8,      16)
REG("STAT:QUES:CAL?",   8,      256)
REG("STAT:QUES:POW?",   8,      8)
REG("*ESR?",            32,     -1)
{ NULL, 0, 0}
};

typedef struct  rsnrp_stringValPair
{
   ViStatus stringVal;
   ViString stringName;
}  rsnrp_tStringValPair;
 
/* This structure holds information which channels are associated with
   particular measurement */
typedef struct rsnrp_measurement
{
    ViInt32 channel1;
    ViInt32 channel2;
} rsnrp_tMeasurement;

/* This structure holds the states of each event status register */
typedef struct rsnrp_cache
{
    ViUInt32    reserved1;
    ViUInt32    device_connected;
    ViUInt32    device_error;
    ViUInt32    calibrating;
    ViUInt32    measuring;
    ViUInt32    trigger;
    ViUInt32    sense;
    ViUInt32    lowerLimit;
    ViUInt32    upperLimit;
    ViUInt32    q_power;
    ViUInt32    q_window;
    ViUInt32    q_calibration;
    ViUInt32    q_zero;
    ViUInt32    esr;
} rsnrp_tEventCache;

struct rsnrp_statusDataRanges
{
/* TRUE if the checking after each function should be checked */
    ViBoolean   errorChecking;
/* 0x0   No options included
   0x1   Test generator installed
   0x2   Second channel interface installed
   0x4   Battery installed
   0x8   Ethernet interface installed
   0x10  Four channel interface installed
   0x20  Sensor A/B rear socket */
    ViInt32     instrOption;
/* Number of available channels */
    ViInt32     channels;
/* Bit array of connected channels */   
    ViUInt32    channelMask;
/* Events cache */
    rsnrp_tEventCache   events;
/* Meaurement-channel mapping table */
    rsnrp_tMeasurement  measurements[MAX_MEASUREMENTS+1];
/* Count of simulation values pairs */
    ViInt32     simulationCount[MAX_CHANNELS];
/* Indicates whether the query window is blocking or not */
    ViBoolean   nonBlockingQuery;
/* Temporary storage */
    ViUInt32    temporaryEsr;
    ViUInt32    temporaryEse;
/* Firmware version number (Major * 100 + Minor) */
    ViUInt32    firmwareVersion;
};

typedef struct rsnrp_statusDataRanges *rsnrp_instrRange;

/*****************************************************************************/
/*= UTILITY ROUTINE DECLARATIONS (Non-Exportable Functions) =================*/
/*****************************************************************************/
ViStatus rsnrp_sendCmd (ViSession instrSession, ViString cmd);
ViStatus rsnrp_sendCmdChannel (ViSession instrSession, ViString cmd, 
                               ViInt32 channel);
ViStatus rsnrp_setViInt32 (ViSession instrSession,
                           ViString cmd,
                           ViInt32 value,
                           ViInt32 min,
                           ViInt32 max,
                           ViStatus parameterErrorCode);
ViStatus rsnrp_setViInt32Channel (ViSession instrSession,
                           ViString cmd,
                           ViInt32 channel,
                           ViInt32 value,
                           ViInt32 min,
                           ViInt32 max,
                           ViStatus parameterErrorCode);
ViStatus rsnrp_getViInt32 (ViSession instrSession,
                           ViString cmd,
                           ViInt32 *value,
                           ViStatus parameterErrorCode);
ViStatus rsnrp_getViInt32Channel (ViSession instrSession,
                           ViString cmd,
                           ViInt32 channel,
                           ViInt32 *value,
                           ViStatus parameterErrorCode);
ViStatus rsnrp_getViBoolean (ViSession instrSession,
                           ViString cmd,
                           ViBoolean *value,
                           ViStatus parameterErrorCode);
ViStatus rsnrp_getViBooleanChannel (ViSession instrSession,
                           ViString cmd,
                           ViInt32 channel,
                           ViBoolean *value,
                           ViStatus parameterErrorCode);
ViStatus rsnrp_setViReal64 (ViSession instrSession,
                            ViString cmd,
                            ViReal64 value,
                            ViReal64 min,
                            ViReal64 max,
                            ViStatus parameterErrorCode);
ViStatus rsnrp_setViReal64Channel (ViSession instrSession,
                            ViString cmd,
                            ViInt32  channel,
                            ViReal64 value,
                            ViReal64 min,
                            ViReal64 max,
                            ViStatus parameterErrorCode);
ViStatus rsnrp_getViReal64 (ViSession instrSession,
                            ViString cmd,
                            ViReal64 *value,
                            ViStatus parameterErrorCode);
ViStatus rsnrp_getViReal64Channel (ViSession instrSession,
                            ViString cmd,
                            ViInt32 channel,
                            ViReal64 *value,
                            ViStatus parameterErrorCode);
ViStatus rsnrp_getViReal64Array (ViSession instrSession,
                            ViString cmd,
                            ViReal64 *value,
                            ViInt32  count,
                            ViStatus parameterErrorCode);
ViStatus rsnrp_getViReal64ArrayChannel (ViSession instrSession,
                                        ViString cmd,
                                        ViInt32 channel,
                                        ViReal64 *value,
                                        ViInt32  count,
                                        ViStatus parameterErrorCode);
ViStatus rsnrp_getViReal64BinArrayChannel (ViSession instrSession,
                                        ViString cmd,
                                        ViInt32 channel,
                                        ViReal64 *value,
                                        ViInt32  count,
                                        ViStatus parameterErrorCode);
ViStatus rsnrp_setEnum (ViSession instrSession,
                        ViString cmd,
                        ViInt32 value,
                        ViString _VI_FAR list[],
                        ViStatus parameterErrorCode);
ViStatus rsnrp_setEnumChannel (ViSession instrSession,
                        ViString cmd,
                        ViInt32 channel,
                        ViInt32 value,
                        ViString _VI_FAR list[],
                        ViStatus parameterErrorCode);
ViStatus rsnrp_getEnum (ViSession instrSession,
                        ViString cmd,
                        ViInt32 *value,
                        ViString _VI_FAR list[],
                        ViStatus parameterErrorCode);
ViStatus rsnrp_getEnumChannel (ViSession instrSession,
                        ViString cmd,
                        ViInt32 channel,
                        ViInt32 *value,
                        ViString _VI_FAR list[],
                        ViStatus parameterErrorCode);
ViStatus rsnrp_setViString (ViSession instrSession,
                            ViString cmd,
                            ViString value,
                            ViStatus parameterErrorValue);
ViStatus rsnrp_setViStringChannel (ViSession instrSession,
                            ViString cmd,
                            ViInt32 channel,
                            ViString value,
                            ViStatus parameterErrorValue);
ViStatus rsnrp_getViString (ViSession instrSession,
                            ViString _VI_FAR cmd,
                            ViString _VI_FAR value,
                            ViInt32 buffer_size,
                            ViStatus parameterErrorCode);
ViStatus rsnrp_getViStringChannel (ViSession instrSession,
                            ViString _VI_FAR cmd,
                            ViInt32 channel,
                            ViString _VI_FAR value,
                            ViInt32 buffer_size,
                            ViStatus parameterErrorCode);
ViStatus rsnrp_checkResolution (ViReal64 resolution, ViStatus statusIfError);
ViStatus rsnrp_checkIndexInList (ViSession instrSession,
                                 ViInt32 index,
                                 ViString list[],
                                 ViStatus statusIfError);
ViStatus rsnrp_getIndexFromString (ViSession instrSession,
                                   ViString string,
                                   ViString *list,
                                   ViInt32 *index);
void rsnrp_delay (ViReal64 numberOfSeconds);
ViStatus rsnrp_itemToConst (ViChar **array, ViString stringItem, ViPInt32 itemNo);
ViBoolean rsnrp_invalidMask (ViUInt32 mask, ViInt32 class);
ViUInt32 rsnrp_translateMask (ViUInt32 mask, ViInt32 class);
ViStatus rsnrp_invalidChannelRange (ViSession instrSession, ViInt32 channel,
                                    ViStatus errorCode);
ViBoolean rsnrp_invalidViBooleanRange (ViBoolean val);
ViBoolean rsnrp_invalidViInt32Range (ViInt32 val, ViInt32 min, ViInt32 max);
ViBoolean rsnrp_invalidViReal64Range (ViReal64 val, ViReal64 min,
                                       ViReal64 max);
ViBoolean rsnrp_invalidViString (ViString val, ViUInt32 maxLength);
ViStatus rsnrp_waitOnVisaEvent (ViSession instrSession,
                                 ViEventType eventTypeIn,
                                 ViUInt32 tmoIn, ViPUInt16 STB);
ViStatus rsnrp_initCleanUp (ViSession openRMSession,
                             ViPSession openInstrSession,
                             ViStatus currentStatus);
ViStatus rsnrp_readToFile (ViSession instrSession,
                            ViString filename, ViUInt32 readBytes,
                            ViPUInt32 retCount);
ViStatus rsnrp_writeFromFile (ViSession instrSession, ViString filename,
                               ViUInt32 writeBytes, ViPUInt32 retCount);
ViStatus rsnrp_defaultInstrSetup (ViSession openInstrSession);

/*****************************************************************************/
/*------ INSERT INSTRUMENT-DEPENDENT UTILITY ROUTINE DECLARATIONS HERE ------*/
/*****************************************************************************/
ViStatus rsnrp_selectTable (ViSession instrSession, ViInt32 tableNumber);
ViStatus rsnrp_updateCache (ViSession instrSession, ViInt32 reg, ViUInt32 resetMask,
                            ViUInt32 setMask);
ViStatus rsnrp_updateRegister (ViSession instrSession, ViString reg, ViUInt32 resetMask,
                            ViUInt32 setMask);
ViStatus rsnrp_sendHighLevelCommand (ViSession  instrSession, 
                                     ViString   cmd,
                                     ViInt32    measurement,
                                     ViInt32    measuringFunction,
                                     ViString   parameterList,
                                     ViReal64   *result,
                                     ViInt32    count);
ViStatus rsnrp_readDataUnknownLength(ViSession instrSession,
                                     ViChar **outBuffer,
                                     ViUInt32 *count);
ViStatus rsnrp_getMathExpression(ViSession instrSession, ViInt32 measurement,
                                 ViInt32* operand1, ViInt32* operation, 
                                 ViInt32* operand2);
ViStatus rsnrp_getMathExExpression(ViSession instrSession, ViInt32 measurement,
                                   ViInt32* eval1, ViInt32* eval2, 
                                   ViInt32* gate1, ViInt32* gate2);
ViStatus rsnrp_checkStatus (ViSession instrSession);
ViStatus rsnrp_sendMathCommand(ViSession instrSession, ViInt32 measurement,
                               ViInt32 sensor1, ViInt32 operation,
                               ViInt32 sensor2);

/*****************************************************************************/
/*====== USER-CALLABLE FUNCTIONS (Exportable Functions) =====================*/
/*****************************************************************************/

/*===========================================================================*/
/* Function: Initialize                                                      */
/* Purpose:  This function opens the instrument, queries the instrument      */
/*           for its ID, and initializes the instrument to a known state.    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_init (ViRsrc resourceName, ViBoolean IDQuery,
                               ViBoolean resetDevice, ViPSession instrSession)
{
    ViStatus            rsnrp_status = VI_SUCCESS;
    ViSession           rmSession = 0;
    ViChar              buffer[BUFFER_SIZE];
    rsnrp_instrRange   instrPtr;

    /*- Check input parameter ranges ----------------------------------------*/
    if (rsnrp_invalidViBooleanRange (IDQuery))
        return VI_ERROR_PARAMETER2;
    if (rsnrp_invalidViBooleanRange (resetDevice))
        return VI_ERROR_PARAMETER3;

    /*- Open instrument session ---------------------------------------------*/
    CHECKERR( viOpenDefaultRM (&rmSession));

    if ((rsnrp_status = viOpen (rmSession, resourceName, VI_LOAD_CONFIG,
                                 VI_NULL, instrSession)) < 0)
        {
        viClose (rmSession);
        return rsnrp_status;
        }

    /* Determine if the structure has been initialized for the current VISA  */
    /* Session and malloc if it has not.                                     */
    CHECKERR_CU( viGetAttribute (*instrSession, VI_ATTR_USER_DATA, &instrPtr));

    if (instrPtr == NULL)
    {
        instrPtr = malloc (sizeof (struct rsnrp_statusDataRanges));
        /* clear the memory */
        memset(instrPtr,0,sizeof (struct rsnrp_statusDataRanges));
    }

    CHECKERR_CU(viSetAttribute (*instrSession, VI_ATTR_USER_DATA, (ViUInt32)instrPtr));

    /*- Configure VISA Formatted I/O ----------------------------------------*/
    CHECKERR_CU( viSetAttribute (*instrSession, VI_ATTR_TMO_VALUE,
                                 RSNRP_DEFAULT_TIMEOUT));
    CHECKERR_CU( viSetBuf (*instrSession, VI_READ_BUF|VI_WRITE_BUF, 4000));
    CHECKERR_CU( viSetAttribute (*instrSession, VI_ATTR_WR_BUF_OPER_MODE,
                                 VI_FLUSH_ON_ACCESS));
    CHECKERR_CU( viSetAttribute (*instrSession, VI_ATTR_RD_BUF_OPER_MODE,
                                 VI_FLUSH_ON_ACCESS));

    CHECKERR_CU( viSetAttribute (*instrSession, VI_ATTR_TERMCHAR_EN, VI_TRUE));
    CHECKERR_CU( viSetAttribute (*instrSession, VI_ATTR_TERMCHAR, '\n'));

//  CHECKERR_CU( viSetAttribute (*instrSession, VI_ATTR_SEND_END_EN, VI_FALSE));
    CHECKERR_CU( viWrite(*instrSession,(ViPBuf)"*CLS",5,VI_NULL));
    /*- Identification Query ------------------------------------------------*/
    if (IDQuery)
    {

        CHECKERR_CU( viQueryf (*instrSession, "*IDN?", "%*[^,],%[^,],%*[\r\n]",
                               buffer));

        /* Check if instrument is connected or not */
        if (strncmp (buffer, "NRP", 17))
            return rsnrp_initCleanUp (rmSession, instrSession, VI_ERROR_FAIL_ID_QUERY);
    }
    
    /*- Reset instrument ----------------------------------------------------*/
    if (resetDevice)
        CHECKERR_CU( rsnrp_reset (*instrSession));
    else  /*- Send Default Instrument Setup ---------------------------------*/
        CHECKERR_CU( rsnrp_defaultInstrSetup (*instrSession));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: ContAv Measurement Example                                      */
/* Purpose:  This function shows how to use correction frequency setting     */
/*           and how to perform measurement using high level function        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_example_measureContAv (ViSession instrSession,
                                               ViInt32 measurement,
                                               ViInt32 resolution,
                                               ViReal64 frequency,
                                               ViReal64 *measuredValue)
{
    ViStatus    rsnrp_status;
    
    rsnrp_status = rsnrp_chan_setCorrectionFrequency (instrSession, 
                                                      measurement,
                                                      frequency);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        rsnrp_status = VI_ERROR_PARAMETER4;

    CHECKERR(rsnrp_status); 

    rsnrp_status = rsnrp_meas_measureContAv (instrSession, measurement,
                                             RSNRP_MEASUREMENT_SINGLE,
                                             measurement, measurement,
                                             resolution, measuredValue);
    if (rsnrp_status == VI_ERROR_PARAMETER7)
        rsnrp_status = VI_ERROR_PARAMETER5;

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: ContAv Low Level Measurement Example                            */
/* Purpose:  This function shows how to use correction frequency setting     */
/*           and how to perform measurement using high level function        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_example_measureContAvLowLevel (ViSession instrSession,
                                                       ViInt32 measurement,
                                                       ViInt32 measuringFunction,
                                                       ViInt32 sensor1,
                                                       ViInt32 sensor2,
                                                       ViInt32 resolution,
                                                       ViReal64 frequency1,
                                                       ViReal64 frequency2,
                                                       ViReal64 *measuredValue)
{
    ViStatus    rsnrp_status;
    ViBoolean   measurement_complete;
    
    if (!measuredValue)
        return VI_ERROR_INSTR_PARAMETER9;
        
    /* configure the calc block's parameters */
    CHECKERR(rsnrp_meas_configureContAv(instrSession,
                                        measurement,
                                        measuringFunction,
                                        sensor1,
                                        sensor2,
                                        resolution));
    
    /* set other parameters for selected sensors */
    rsnrp_status = rsnrp_chan_setCorrectionFrequency (instrSession, 
                                                      sensor1,
                                                      frequency1);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        rsnrp_status = VI_ERROR_PARAMETER7;

    CHECKERR(rsnrp_status); 

    rsnrp_status = rsnrp_chan_setCorrectionFrequency (instrSession, 
                                                      sensor2,
                                                      frequency2);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        rsnrp_status = VI_ERROR_PARAMETER8;

    CHECKERR(rsnrp_status); 

    CHECKERR(rsnrp_meas_initiate(instrSession,measurement));

    do
    {
        CHECKERR(rsnrp_meas_isMeasurementComplete(instrSession,
                                                  measurement,
                                                  &measurement_complete));
    } while (!measurement_complete);

    return rsnrp_meas_fetch(instrSession, measurement, measuredValue);
}

/*===========================================================================*/
/* Function: Abort                                                           */
/* Purpose:  This function immediately sets all the sensors to the IDLE      */
/*           state.                                                          */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chans_abort (ViSession instrSession)
{
    ViStatus    rsnrp_status;
    
    GETSESSIONDATA(instrSession);

    /* Clear all events in measuring register */
    instrPtr->events.measuring = 0;
    
    CHECKERR(rsnrp_sendCmd(instrSession, "ABOR:ALL"));
    
    /* read the measuring status event register - clear all events*/
    return rsnrp_updateCache (instrSession, RSNRP_STATCLASS_O_MEAS, 65535, 0);
}

/*===========================================================================*/
/* Function: Count                                                           */
/* Purpose:  This function returns number of available channels.             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chans_getCount (ViSession instrSession,
                                        ViInt32 *count)
{
    ViStatus    rsnrp_status;
    
    GETSESSIONDATA(instrSession);
    
    *count = instrPtr->channels;
            
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Initiate                                                        */
/* Purpose:  This function starts a single-shot measurement on all channels. */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chans_initiate (ViSession instrSession)
{
    ViStatus    rsnrp_status;
    
    GETSESSIONDATA(instrSession);

    /* read the status to the cache to clear the measuring status register
       and clear all events in measuring register, they are no longer valid */
    CHECKERR(rsnrp_updateCache (instrSession, RSNRP_STATCLASS_O_MEAS, 65535, 0));

    return rsnrp_sendCmd(instrSession, "INIT:ALL");
}

/*===========================================================================*/
/* Function: Recall Standard                                                 */
/* Purpose:  This control selects the desired standard to be preset.         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chans_recallStandard (ViSession instrSession,
                                              ViString standard)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if(!standard)
        return VI_ERROR_PARAMETER2;

    sprintf(buffer,"'%s'",standard);
    return rsnrp_setViString(instrSession, "SYST:STAN:PRES", buffer, 0);
}

/*===========================================================================*/
/* Function: Zero                                                            */
/* Purpose:  This function performs zeroing using the signal at the sensor   */
/*           input.                                                          */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chans_zero (ViSession instrSession)
{
    return rsnrp_sendCmd(instrSession, "CAL:ALL:ZERO:AUTO ONCE");
}

/*===========================================================================*/
/* Function: Get Zeroing Complete State                                      */
/* Purpose:  This function returns the summary status of zeroing on all      */
/*           channels.                                                       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chans_isZeroingComplete (ViSession instrSession,
                                                 ViBoolean *zeroingCompleted)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViUInt32    the_mask = 0;
    
    GETSESSIONDATA(instrSession);
    
    if (!zeroingCompleted)
        return VI_ERROR_PARAMETER2;
    
    /* create the mask from number of channels */
    /* mask is in same order as connected sensors mask */
    /* only shift to left by 1 */
    the_mask = instrPtr->channelMask << 1;

    /* try the fastest way - are all measurements already in cache? If not,
       read the state from instrument, if yes, clear and return */
    if ((instrPtr->events.calibrating & the_mask) == the_mask)
    {
        instrPtr->events.calibrating &= ~the_mask;
        *zeroingCompleted = VI_TRUE;
        return rsnrp_status;
    }
    
    /* read and update the cache */
    CHECKERR(rsnrp_updateCache(instrSession, RSNRP_STATCLASS_O_CAL, 0, 0));

    /* check the cache with new values */
    if ((instrPtr->events.calibrating & the_mask) == the_mask)
    {
        instrPtr->events.calibrating &= ~the_mask;
        *zeroingCompleted = VI_TRUE;
    }

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Measurement Complete State                                  */
/* Purpose:  This function returns the summary status of measurements on all */
/*           channels.                                                       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chans_isMeasurementComplete (ViSession instrSession,
                                                     ViBoolean *measurementCompleted)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViUInt32    the_mask = 0;
    
    GETSESSIONDATA(instrSession);
    
    if (!measurementCompleted)
        return VI_ERROR_PARAMETER2;
    
    /* create the mask from number of channels */
    /* mask is in same order as connected sensors mask */
    /* only shift to left by 1 */
    the_mask = instrPtr->channelMask << 1;

    /* try the fastest way - are all measurements already in cache? If not,
       read the state from instrument, if yes, clear and return */
    if ((instrPtr->events.measuring & the_mask) == the_mask)
    {
        instrPtr->events.measuring &= ~the_mask;
        *measurementCompleted = VI_TRUE;
        return rsnrp_status;
    }
    
    /* read and update the cache */
    CHECKERR(rsnrp_updateCache(instrSession, RSNRP_STATCLASS_O_MEAS, 0, 0));

    /* check the cache with new values */
    if ((instrPtr->events.measuring & the_mask) == the_mask)
    {
        instrPtr->events.measuring &= ~the_mask;
        *measurementCompleted = VI_TRUE;
    }

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Measurement Mode                                                */
/* Purpose:  This function sets the sensor to one of the measurement modes.  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_mode (ViSession instrSession, ViInt32 channel,
                                   ViInt32 measurementMode)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));

    /* enable or disable buffering */
    
    rsnrp_status = rsnrp_chan_setContAvBufferedEnabled(instrSession,
                                                 channel,
                                                 (ViBoolean)(measurementMode == RSNRP_SENSOR_MODE_BUF_CONTAV));
    if ((rsnrp_status == RSNRP_ERROR_INSTRUMENT_ERROR) && (measurementMode == RSNRP_SENSOR_MODE_BUF_CONTAV))
        return rsnrp_status;

    return rsnrp_setEnumChannel(instrSession, 
                                "SENS%ld:FUNC",
                                channel,
                                measurementMode,
                                measModeArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Configure Exclude                                               */
/* Purpose:  This function configures times that is to be excluded at the    */
/*           beginning and at the end of the integration.                    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_timing_configureExclude (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViReal64 excludeStart,
                                                 ViReal64 excludeStop)
{
    ViStatus    rsnrp_status;
    
    CHECKERR (rsnrp_timing_setTimingExcludeStart(instrSession, 
                                                 channel,
                                                 excludeStart));
    return rsnrp_timing_setTimingExcludeStop(instrSession,
                                             channel, 
                                             excludeStop);
}

/*===========================================================================*/
/* Function: Set Timing Exclude Start                                        */
/* Purpose:  This function sets a time that is to be excluded at the         */
/*           beginning of the integration.                                   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_timing_setTimingExcludeStart (ViSession instrSession,
                                                      ViInt32 channel,
                                                      ViReal64 excludeStart)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession,
                                    "SENS%ld:TIM:EXCL:STAR",
                                    channel,
                                    excludeStart,
                                    0.0, 0.0,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Timing Exclude Start                                        */
/* Purpose:  This function reads a time that is to be excluded at the        */
/*           beginning of the integration.                                   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_timing_getTimingExcludeStart (ViSession instrSession,
                                                      ViInt32 channel,
                                                      ViReal64 *excludeStart)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession,
                                    "SENS%ld:TIM:EXCL:STAR?",
                                    channel,
                                    excludeStart,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Timing Exclude Stop                                         */
/* Purpose:  This function sets a time that is to be excluded at the end of  */
/*           the integration.                                                */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_timing_setTimingExcludeStop (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 excludeStop)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession,
                                    "SENS%ld:TIM:EXCL:STOP",
                                    channel,
                                    excludeStop, 
                                    0.0, 0.0,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Timing Exclude Stop                                         */
/* Purpose:  This function reads a time that is to be excluded at the end of */
/*           the integration.                                                */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_timing_getTimingExcludeStop (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 *excludeStop)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                    "SENS%ld:TIM:EXCL:STOP?", 
                                    channel,
                                    excludeStop,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Configure Averaging Auto                                        */
/* Purpose:  This function configures all parameters necessary for automatic */
/*           detection of filter bandwidth.                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_configureAvgAuto (ViSession instrSession,
                                              ViInt32 channel,
                                              ViInt32 resolution)
{
    ViStatus    rsnrp_status;

    CHECKERR(rsnrp_avg_setAutoResolution(instrSession, channel, resolution));
    CHECKERR(rsnrp_avg_setAutoType (instrSession, channel,
                                    RSNRP_AUTO_COUNT_TYPE_RESOLUTION));
    CHECKERR(rsnrp_avg_setTerminalControl (instrSession,
                                           channel,
                                           RSNRP_TERMINAL_CONTROL_REPEAT));
    CHECKERR(rsnrp_avg_setAutoEnabled(instrSession,channel,VI_TRUE));
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Configure Averaging NS Ratio                                    */
/* Purpose:  This function configures all parameters necessary for setting   */
/*           the noise ratio in the measurement result and automatic         */
/*           detection of filter bandwidth.                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_configureAvgNSRatio (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViReal64 maximumNoiseRatio,
                                                 ViReal64 upperTimeLimit)
{
    ViStatus    rsnrp_status;

    CHECKERR(rsnrp_avg_setAutoNoiseSignalRatio(instrSession,
                                               channel,
                                               maximumNoiseRatio));
    if ((rsnrp_status = rsnrp_avg_setAutoMaxMeasuringTime(instrSession,
                                                          channel,
                                                          upperTimeLimit)) == VI_ERROR_PARAMETER3)
        return VI_ERROR_PARAMETER4;                                                       

    CHECKERR(rsnrp_avg_setAutoType (instrSession, channel,
                                    RSNRP_AUTO_COUNT_TYPE_NSR));
    CHECKERR(rsnrp_avg_setTerminalControl (instrSession,
                                           channel,
                                           RSNRP_TERMINAL_CONTROL_REPEAT));
    CHECKERR(rsnrp_avg_setAutoEnabled(instrSession,channel,VI_TRUE));
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Configure Averaging Manual                                      */
/* Purpose:  This function configures all parameters necessary for manual    */
/*           setting of filter bandwidth.                                    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_configureAvgManual (ViSession instrSession,
                                                ViInt32 channel, ViInt32 count)
{
    ViStatus    rsnrp_status;

    CHECKERR(rsnrp_avg_setCount(instrSession,channel,count));
    CHECKERR(rsnrp_avg_setTerminalControl (instrSession,
                                           channel,
                                           RSNRP_TERMINAL_CONTROL_REPEAT));
    CHECKERR(rsnrp_avg_setAutoEnabled(instrSession,channel,VI_FALSE));
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Auto Averaging State                                        */
/* Purpose:  This function can be used to automatically determine a value for*/
/*           filter bandwidth.                                               */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_setAutoEnabled (ViSession instrSession,
                                            ViInt32 channel,
                                            ViBoolean autoEnabled)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession,
                                "SENS%ld:AVER:COUN:AUTO",
                                channel,
                                autoEnabled,
                                switchArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Auto Averaging State                                        */
/* Purpose:  This function queries the setting of automatic switchover of    */
/*           filter bandwidth.                                               */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_getAutoEnabled (ViSession instrSession,
                                            ViInt32 channel,
                                            ViBoolean *autoEnabled)
{
    ViStatus    rsnrp_status;
        
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    CHECKERR(rsnrp_getViBooleanChannel(instrSession, 
                                       "SENS%ld:AVER:COUN:AUTO?", 
                                       channel, 
                                       autoEnabled, 
                                       VI_ERROR_PARAMETER3));
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Auto Max Measuring Time                                     */
/* Purpose:  This function sets an upper time limit can be set.              */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_setAutoMaxMeasuringTime (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 upperTimeLimit)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                    "SENS%ld:AVER:COUN:AUTO:MTIME",
                                    channel, 
                                    upperTimeLimit, 
                                    0.0, 0.0, 
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Auto Max Measuring Time                                     */
/* Purpose:  This function queries an upper time limit.                      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_getAutoMaxMeasuringTime (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 *upperTimeLimit)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession,
                                    "SENS%ld:AVER:COUN:AUTO:MTIME?",
                                    channel, 
                                    upperTimeLimit, 
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Auto Noise Signal Ratio                                     */
/* Purpose:  This function sets the maximum noise ratio in the measurement   */
/*           result.                                                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_setAutoNoiseSignalRatio (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 maximumNoiseRatio)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                    "SENS%ld:AVER:COUN:AUTO:NSR",
                                    channel, 
                                    maximumNoiseRatio, 
                                    0.0, 0.0, 
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Auto Noise Signal Ratio                                     */
/* Purpose:  This function queries the maximum noise signal ratio value.     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_getAutoNoiseSignalRatio (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 *maximumNoiseRatio)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession,
                                    "SENS%ld:AVER:COUN:AUTO:NSR?",
                                    channel,
                                    maximumNoiseRatio,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Auto Resolution                                             */
/* Purpose:  This function defines the number of significant places          */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_setAutoResolution (ViSession instrSession,
                                               ViInt32 channel,
                                               ViInt32 resolution)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViInt32Channel(instrSession,
                                   "SENS%ld:AVER:COUN:AUTO:RES",
                                   channel, 
                                   resolution, 1, 4, 
                                   0);
}

/*===========================================================================*/
/* Function: Get Auto Resolution                                             */
/* Purpose:  This function reads the number of significant places            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_getAutoResolution (ViSession instrSession,
                                               ViInt32 channel,
                                               ViInt32 *resolution)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViInt32Channel(instrSession,
                                   "SENS%ld:AVER:COUN:AUTO:RES?",
                                   channel,
                                   resolution,
                                   VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Auto Type                                                   */
/* Purpose:  This function selects a method by which the automatic filter    */
/*           length switchover can operate.                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_setAutoType (ViSession instrSession,
                                         ViInt32 channel, ViInt32 method)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession,
                                "SENS%ld:AVER:COUN:AUTO:TYPE",
                                channel,
                                method, 
                                autoTypeArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Auto Type                                                   */
/* Purpose:  This function returns a method by which the automatic filter    */
/*           length switchover can operate.                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_getAutoType (ViSession instrSession,
                                         ViInt32 channel, ViInt32 *method)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getEnumChannel(instrSession,
                                "SENS%ld:AVER:COUN:AUTO:TYPE?",
                                channel, 
                                method,
                                autoTypeArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Averaging Count                                             */
/* Purpose:  This function sets the filter bandwidth.                        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_setCount (ViSession instrSession,
                                      ViInt32 channel, ViInt32 count)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViInt32Channel(instrSession,
                                   "SENS%ld:AVER:COUN",
                                   channel, 
                                   count,
                                   0, 0, 
                                   VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Averaging Count                                             */
/* Purpose:  This function gets the filter bandwidth.                        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_getCount (ViSession instrSession,
                                      ViInt32 channel, ViInt32 *count)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViInt32Channel(instrSession,
                                   "SENS%ld:AVER:COUN?",
                                   channel, 
                                   count,
                                   VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Averaging State                                             */
/* Purpose:  This function switches the filter function of a sensor on or    */
/*           off.                                                            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_setEnabled (ViSession instrSession,
                                        ViInt32 channel, ViBoolean averaging)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession, 
                                "SENS%ld:AVER",
                                channel, 
                                averaging,
                                switchArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Averaging State                                             */
/* Purpose:  This function returns the state of the filter function of a     */
/*           sensor.                                                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_getEnabled (ViSession instrSession,
                                        ViInt32 channel, ViBoolean *averaging)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViBooleanChannel(instrSession, 
                                     "SENS%ld:AVER?",
                                     channel, 
                                     averaging, 
                                     VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Timeslot                                                    */
/* Purpose:  This function sets a timeslot whose measured value is used to   */
/*           automatically determine the filter length.                      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_setSlot (ViSession instrSession, ViInt32 channel,
                                     ViInt32 timeslot)
{
    ViStatus    rsnrp_status;
    ViInt32     max_slot = 0;

    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    CHECKERR(viPrintf(instrSession, "SENS%ld:TSL:COUN?",channel));
    CHECKERR(viScanf(instrSession, "%ld", &max_slot));
    
    return rsnrp_setViInt32Channel(instrSession, 
                                   "SENS%ld:AVER:COUN:AUTO:SLOT",
                                   channel, 
                                   timeslot, 
                                   1, max_slot, 
                                   VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Timeslot                                                    */
/* Purpose:  This function returns a timeslot whose measured value is used to*/
/*           automatically determine the filter length.                      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_getSlot (ViSession instrSession, ViInt32 channel,
                                     ViInt32 *timeslot)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViInt32Channel(instrSession, 
                                   "SENS%ld:AVER:COUN:AUTO:SLOT?",
                                   channel, 
                                   timeslot, 
                                   VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Terminal Control                                            */
/* Purpose:  This function determines whether a new result is calculated     */
/*           immediately after a new measured value is available or only     */
/*           after an entire range of new values is available for the filter.*/
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_setTerminalControl (ViSession instrSession,
                                                ViInt32 channel,
                                                ViInt32 terminalControl)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession, 
                                "SENS%ld:AVER:TCON",
                                channel, 
                                terminalControl, 
                                tControlArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Terminal Control                                            */
/* Purpose:  This function returns the type of terminal control.             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_getTerminalControl (ViSession instrSession,
                                                ViInt32 channel,
                                                ViInt32 *terminalControl)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getEnumChannel(instrSession, 
                                "SENS%ld:AVER:TCON?",
                                channel, 
                                terminalControl, 
                                tControlArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Reset                                                           */
/* Purpose:  This function initializes the digital filter by deleting the    */
/*           stored measured values.                                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_avg_reset (ViSession instrSession, ViInt32 channel)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    return rsnrp_sendCmdChannel(instrSession,
                                "SENS%ld:AVER:RES",
                                channel);
}

/*===========================================================================*/
/* Function: Set Auto Range State                                            */
/* Purpose:  This function sets the automatic selection of a measurement     */
/*           range to ON or OFF.                                             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_range_setAutoEnabled (ViSession instrSession,
                                              ViInt32 channel,
                                              ViBoolean autoRange)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession, 
                                "SENS%ld:RANG:AUTO",
                                channel, 
                                autoRange, 
                                switchArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Auto Range State                                            */
/* Purpose:  This function returns the state of automatic selection of a     */
/*           measurement range.                                              */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_range_getAutoEnabled (ViSession instrSession,
                                              ViInt32 channel,
                                              ViBoolean *autoRange)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViBooleanChannel(instrSession, 
                                "SENS%ld:RANG:AUTO?",
                                channel,
                                autoRange,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Crossover Level                                             */
/* Purpose:  This function sets the cross-over level.                        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_range_setCrossoverLevel (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViReal64 crossoverLevel)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                "SENS%ld:RANG:CLEV",
                                channel, 
                                crossoverLevel, 
                                0.0, 0.0,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Crossover Level                                             */
/* Purpose:  This function reads the cross-over level.                       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_range_getCrossoverLevel (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViReal64 *crossoverLevel)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                "SENS%ld:RANG:CLEV?",
                                channel,
                                crossoverLevel,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Range                                                       */
/* Purpose:  This function selects a measurement range in which the          */
/*           corresponding sensor is to perform a measurement.               */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_range_setRange (ViSession instrSession,
                                        ViInt32 channel, ViInt32 range)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViInt32Channel(instrSession, 
                                "SENS%ld:RANG",
                                channel, 
                                range, 
                                0, 2,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Range                                                       */
/* Purpose:  This function returns a measurement range in which the          */
/*           corresponding sensor is to perform a measurement.               */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_range_getRange (ViSession instrSession,
                                        ViInt32 channel, ViInt32 *range)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViInt32Channel(instrSession, 
                                "SENS%ld:RANG?",
                                channel, 
                                range, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Configure Duty Cycle                                            */
/* Purpose:  This function configures all duty cycle parameters.             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_configureDutyCycle (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViBoolean dutyCycleState,
                                                 ViReal64 dutyCycle)
{
    ViStatus    rsnrp_status;

    CHECKERR(rsnrp_corr_setDutyCycle(instrSession, channel, dutyCycle));
    return rsnrp_corr_setDutyCycleEnabled(instrSession,
                                          channel,
                                          dutyCycleState); 
}

/*===========================================================================*/
/* Function: Set Duty Cycle                                                  */
/* Purpose:  This function informs the R&S NRP about the duty cycle of the   */
/*           power to be measured.                                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_setDutyCycle (ViSession instrSession,
                                           ViInt32 channel, ViReal64 dutyCycle)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                "SENS%ld:CORR:DCYC",
                                channel, 
                                dutyCycle, 
                                0.0, 0.0,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Duty Cycle                                                  */
/* Purpose:  This function gets the size of duty cycle of the power to be    */
/*           measured.                                                       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_getDutyCycle (ViSession instrSession,
                                           ViInt32 channel, ViReal64 *dutyCycle)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                "SENS%ld:CORR:DCYC?",
                                channel, 
                                dutyCycle, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Duty Cycle State                                            */
/* Purpose:  This function switches measured-value correction for a specific */
/*           duty cycle on or off.                                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_setDutyCycleEnabled (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViBoolean dutyCycleState)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession, 
                                "SENS%ld:CORR:DCYC:STAT",
                                channel,
                                dutyCycleState,
                                switchArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Duty Cycle State                                            */
/* Purpose:  This function gets the setting of duty cycle.                   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_getDutyCycleEnabled (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViBoolean *dutyCycleState)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViBooleanChannel(instrSession, 
                                "SENS%ld:CORR:DCYC:STAT?",
                                channel,
                                dutyCycleState,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Configure Corrections                                           */
/* Purpose:  This function configures all correction parameters.             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_configureCorrections (ViSession instrSession,
                                                   ViInt32 channel,
                                                   ViBoolean offsetState,
                                                   ViReal64 offset,
                                                   ViBoolean offsetTableState,
                                                   ViString tableName,
                                                   ViBoolean sParameterEnable)
{
    ViStatus    rsnrp_status;

    rsnrp_status = rsnrp_corr_setOffset (instrSession, channel, offset);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        return VI_ERROR_PARAMETER4;

    CHECKERR (rsnrp_corr_setOffsetEnabled (instrSession, channel, offsetState));

    rsnrp_status = rsnrp_corr_setOffsetTableEnabled (instrSession, channel, offsetTableState);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        return VI_ERROR_PARAMETER5;

    rsnrp_status = rsnrp_corr_setOffsetTableSelect (instrSession, channel, tableName);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        return VI_ERROR_PARAMETER6;
    
    rsnrp_status = rsnrp_corr_setSParamDeviceEnabled (instrSession, channel, sParameterEnable);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        rsnrp_status = VI_ERROR_PARAMETER7;

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Correction Frequency                                        */
/* Purpose:  This function informs the R&S NRP about the frequency of the    */
/*           power to be measured since this frequency is not automatically  */
/*           determined.                                                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_setCorrectionFrequency (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 frequency)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                "SENS%ld:FREQ",
                                channel, 
                                frequency, 
                                0.0, 0.0,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Correction Frequency                                        */
/* Purpose:  This function queries the instrument for the frequency of the   */
/*           power to be measured since this frequency is not automatically  */
/*           determined.                                                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_getCorrectionFrequency (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViReal64 *frequency)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                "SENS%ld:FREQ?",
                                channel, 
                                frequency, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Offset                                                      */
/* Purpose:  With this function a fixed offset value can be defined for      */
/*           multiplying (logarithmically adding) the measured value of a    */
/*           sensor.                                                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_setOffset (ViSession instrSession,
                                        ViInt32 channel, ViReal64 offset)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                "SENS%ld:CORR:OFFS",
                                channel, 
                                offset, 
                                0.0, 0.0,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Offset                                                      */
/* Purpose:  This function gets a fixed offset value defined for multiplying */
/*           (logarithmically adding) the measured value of a sensor.        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_getOffset (ViSession instrSession,
                                        ViInt32 channel, ViReal64 *offset)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                "SENS%ld:CORR:OFFS?",
                                channel, 
                                offset, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Offset State                                                */
/* Purpose:  This function switches the offset correction on or off.         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_setOffsetEnabled (ViSession instrSession,
                                               ViInt32 channel,
                                               ViBoolean offsetState)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession, 
                                "SENS%ld:CORR:OFFS:STAT",
                                channel,
                                offsetState,
                                switchArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Offset State                                                */
/* Purpose:  This function returns the offset correction on or off.          */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_getOffsetEnabled (ViSession instrSession,
                                               ViInt32 channel,
                                               ViBoolean *offsetState)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViBooleanChannel(instrSession, 
                                "SENS%ld:CORR:OFFS:STAT?",
                                channel,
                                offsetState,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Offset Table State                                          */
/* Purpose:  This function activates and deactivates the current             */
/*           frequency-dependent offset table. Only one table can be active  */
/*           at a time.                                                      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_setOffsetTableEnabled (ViSession instrSession,
                                                    ViInt32 channel,
                                                    ViBoolean offsetTableState)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession, 
                                "SENS%ld:CORR:FDOT:STAT",
                                channel,
                                offsetTableState,
                                switchArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Offset Table State                                          */
/* Purpose:  This function gets the state of activation of offset table.     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_getOffsetTableEnabled (ViSession instrSession,
                                                    ViInt32 channel,
                                                    ViBoolean *offsetTableState)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViBooleanChannel(instrSession, 
                                "SENS%ld:CORR:FDOT:STAT?",
                                channel,
                                offsetTableState,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Select Offset Table                                             */
/* Purpose:  The name of the current frequency-dependent offset table can be */
/*           set by this function.                                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_setOffsetTableSelect (ViSession instrSession,
                                                   ViInt32 channel,
                                                   ViString tableName)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    sprintf(buffer,"SENS%%ld:CORR:FDOT '%s'",tableName);
    return rsnrp_sendCmdChannel(instrSession, 
                                buffer,
                                channel);
}

/*===========================================================================*/
/* Function: Selected Offset Table                                           */
/* Purpose:  This function returns the name of selected offset table.        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_getOffsetTableSelect (ViSession instrSession,
                                                   ViInt32 channel,
                                                   ViInt32 arraySize,
                                                   ViChar _VI_FAR selectedOffsetTable[])
{
    ViStatus    rsnrp_status;
    ViChar*     pbuffer;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    if (!selectedOffsetTable)
        return VI_ERROR_PARAMETER3;
        
    pbuffer = (ViChar*)malloc(arraySize+2);
    
    rsnrp_status = rsnrp_getViStringChannel(instrSession, 
                                "SENS%ld:CORR:FDOT?",
                                channel,
                                pbuffer,
                                arraySize,
                                VI_ERROR_PARAMETER3);
    if (rsnrp_status == VI_SUCCESS)
        sscanf(pbuffer,"\"%[^\"]",selectedOffsetTable);
    
    free(pbuffer);
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Offset Table Value                                          */
/* Purpose:  This function gets the respective frequency-dependent           */
/*           correction factor determined by means of the active tables      */
/*           (if necessary by interpolation).                                */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_getOffsetTableValue (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViReal64 *frequencyDependentOffset)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                "SENS%ld:CORR:FDOF?",
                                channel, 
                                frequencyDependentOffset, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set S-Parameter Device State                                    */
/* Purpose:  This function instructs the sensor to perform a measured-value  */
/*           correction by means of the stored s-parameter device.           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_setSParamDeviceEnabled (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViBoolean sParameterEnable)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession, 
                                "SENS%ld:CORR:SPD:STAT",
                                channel,
                                sParameterEnable,
                                switchArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get S-Parameter Device State                                    */
/* Purpose:  This function returns the state of a measured-value correction  */
/*           by means of the stored s-parameter device.                      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_corr_getSParamDeviceEnabled (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViBoolean *sParameterCorrection)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViBooleanChannel(instrSession, 
                                "SENS%ld:CORR:SPD:STAT?",
                                channel,
                                sParameterCorrection,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Configure Source Gamma Corr                                     */
/* Purpose:  This function sets the parameters of the reflection coefficient */
/*           for measured-value correction.                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_configureSourceGammaCorr (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViBoolean sourceGammaCorrection,
                                                       ViReal64 magnitude,
                                                       ViReal64 phase)
{
    ViStatus    rsnrp_status;

    rsnrp_status = rsnrp_chan_setSourceGammaMagnitude(instrSession,channel,magnitude);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        return VI_ERROR_PARAMETER4;

    rsnrp_status = rsnrp_chan_setSourceGammaPhase(instrSession,channel,phase);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        return VI_ERROR_PARAMETER5;

    return rsnrp_chan_setSourceGammaCorrEnabled(instrSession,channel,sourceGammaCorrection);
}

/*===========================================================================*/
/* Function: Set Source Gamma Magnitude                                      */
/* Purpose:  This function sets the magnitude of the reflection coefficient  */
/*           for measured-value correction.                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_setSourceGammaMagnitude (ViSession instrSession,
                                                      ViInt32 channel,
                                                      ViReal64 magnitude)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                "SENS%ld:SGAM",
                                channel, 
                                magnitude, 
                                0.0, 0.0,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Source Gamma Magnitude                                      */
/* Purpose:  This function reads the magnitude of the reflection coefficient */
/*           for measured-value correction.                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_getSourceGammaMagnitude (ViSession instrSession,
                                                      ViInt32 channel,
                                                      ViReal64 *magnitude)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                "SENS%ld:SGAM?",
                                channel, 
                                magnitude, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Source Gamma Phase                                          */
/* Purpose:  This function sets the phase angle of the reflection coefficient*/
/*           for measured-value correction.                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_setSourceGammaPhase (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViReal64 phase)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                "SENS%ld:SGAM:PHAS",
                                channel, 
                                phase, 
                                0.0, 0.0,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Source Gamma Phase                                          */
/* Purpose:  This function reads the phase angle of the reflection           */
/*           coefficient for measured-value correction.                      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_getSourceGammaPhase (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViReal64 *phase)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                "SENS%ld:SGAM:PHAS?",
                                channel, 
                                phase, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Source Gamma Corr State                                     */
/* Purpose:  This function switches the measured-value correction of the     */
/*           reflection coefficient effect of the source gamma ON or OFF.    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_setSourceGammaCorrEnabled (ViSession instrSession,
                                                        ViInt32 channel,
                                                        ViBoolean sourceGammaCorrection)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession, 
                                "SENS%ld:SGAM:CORR:STAT",
                                channel,
                                sourceGammaCorrection,
                                switchArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Source Gamma Corr State                                     */
/* Purpose:  This function reads the state of source gamma correction.       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_getSourceGammaCorrEnabled (ViSession instrSession,
                                                        ViInt32 channel,
                                                        ViBoolean *sourceGammaCorrection)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViBooleanChannel(instrSession, 
                                "SENS%ld:SGAM:CORR:STAT?",
                                channel,
                                sourceGammaCorrection,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set ContAv Aperture                                             */
/* Purpose:  This function determines the integration time for a single      */
/*           measurement in the ContAv mode.                                 */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_setContAvAperture (ViSession instrSession,
                                                ViInt32 channel,
                                                ViReal64 contAvAperture)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                "SENS%ld:APER",
                                channel, 
                                contAvAperture, 
                                0.0, 0.0,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get ContAv Aperture                                             */
/* Purpose:  This function queries the value of ContAv mode aperture size.   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_getContAvAperture (ViSession instrSession,
                                                ViInt32 channel,
                                                ViReal64 *contAvAperture)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                "SENS%ld:APER?",
                                channel, 
                                contAvAperture, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set ContAv Smoothing State                                      */
/* Purpose:  This function activates digital lowpass filtering of the sampled*/
/*           video signal.                                                   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_setContAvSmoothingEnabled (ViSession instrSession,
                                                        ViInt32 channel,
                                                        ViBoolean contAvSmoothing)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession, 
                                "SENS%ld:SMO:STAT",
                                channel,
                                contAvSmoothing,
                                switchArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get ContAv Smoothing State                                      */
/* Purpose:  This function gets the state of digital lowpass filtering of the*/
/*           sampled video signal.                                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_getContAvSmoothingEnabled (ViSession instrSession,
                                                        ViInt32 channel,
                                                        ViBoolean *contAvSmoothing)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViBooleanChannel(instrSession, 
                                "SENS%ld:SMO:STAT?",
                                channel,
                                contAvSmoothing,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Buffered ContAv State                                       */
/* Purpose:  This function switches on the buffered ContAv mode, after which */
/*           data blocks rather than single measured values are then         */
/*           returned.                                                       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_setContAvBufferedEnabled (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViBoolean contAvBufferedMode)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession, 
                                "SENS%ld:BUFF:STAT",
                                channel,
                                contAvBufferedMode,
                                switchArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Buffered ContAv State                                       */
/* Purpose:  This function returns the state of ContAv Buffered Measurement  */
/*           Mode.                                                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_getContAvBufferedEnabled (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViBoolean *contAvBufferedMode)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViBooleanChannel(instrSession, 
                                "SENS%ld:BUFF:STAT?",
                                channel,
                                contAvBufferedMode,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set ContAv Buffer Size                                          */
/* Purpose:  This function sets the number of desired values for the buffered*/
/*           ContAv mode.                                                    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_setContAvBufferSize (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViInt32 bufferSize)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViInt32Channel(instrSession, 
                                "SENS%ld:BUFFer:SIZE",
                                channel, 
                                bufferSize,
                                1, 4000000,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get ContAv Buffer Size                                          */
/* Purpose:  This function returns the number of desired values for the      */
/*           buffered ContAv mode.                                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_getContAvBufferSize (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViInt32 *bufferSize)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViInt32Channel(instrSession, 
                                "SENS%ld:BUFF:SIZE?",
                                channel, 
                                bufferSize,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Burst Drop-Out Tolerance                                    */
/* Purpose:  This function sets the drop-out tolerance parameter.            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_setBurstDropoutTolerance (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViReal64 dropoutTolerance)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                "SENS%ld:BURS:DTOL",
                                channel, 
                                dropoutTolerance, 
                                0.0, 0.0,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Burst Drop-Out Tolerance                                    */
/* Purpose:  This function returns the drop-out tolerance parameter.         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_getBurstDropoutTolerance (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViReal64 *dropoutTolerance)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                "SENS%ld:BURS:DTOL?",
                                channel, 
                                dropoutTolerance, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Count                                                           */
/* Purpose:  This function puts always number 4 into count parameter.        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_tGates_getCount (ViSession instrSession,
                                         ViInt32 *count)
{
    ViStatus    rsnrp_status = VI_SUCCESS;

    if (count)
        *count = 4;
    else
        rsnrp_status = VI_ERROR_PARAMETER2;

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Select                                                          */
/* Purpose:  This function selects one of the four gates in which the average*/
/*           power is measured if the Timegate mode is selected.             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_tGates_setSelect (ViSession instrSession,
                                          ViInt32 channel, ViInt32 selectGate)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViInt32Channel(instrSession, 
                                "SENS%ld:TGAT:SEL",
                                channel, 
                                selectGate, 
                                1, 4,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Selected                                                        */
/* Purpose:  This function reads the number of selected gate in which the    */
/*           average power is measured if the Timegate mode is selected.     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_tGates_getSelect (ViSession instrSession,
                                          ViInt32 channel, ViInt32 *selectedGate)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViInt32Channel(instrSession, 
                                "SENS%ld:TGAT:SEL?",
                                channel, 
                                selectedGate, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Timegate Offset Time                                        */
/* Purpose:  This function specifies the start of one of the four gates      */
/*           (sweeps) relative to the delayed trigger point.                 */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_tGate_setOffsetTime (ViSession instrSession,
                                             ViInt32 channel, ViInt32 selectGate,
                                             ViReal64 offsetTime)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    if (rsnrp_invalidViInt32Range(selectGate, 1, 4))
        return VI_ERROR_PARAMETER3;
    
    sprintf (buffer,"SENS%%ld:TGAT%ld:OFFS:TIME", selectGate);
    
    return rsnrp_setViReal64Channel(instrSession,
                                    buffer,
                                    channel,
                                    offsetTime,
                                    0.0, 0.0,
                                    VI_ERROR_PARAMETER4);
}

/*===========================================================================*/
/* Function: Get Timegate Offset Time                                        */
/* Purpose:  This function queries the offset time set for specified gate.   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_tGate_getOffsetTime (ViSession instrSession,
                                             ViInt32 channel, ViInt32 selectGate,
                                             ViReal64 *offsetTime)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));

    if (rsnrp_invalidViInt32Range(selectGate, 1, 4))
        return VI_ERROR_PARAMETER3;

    sprintf (buffer,"SENS%%ld:TGAT%ld:OFFS:TIME?", selectGate);
    
    return rsnrp_getViReal64Channel(instrSession,
                                    buffer,
                                    channel,
                                    offsetTime,
                                    VI_ERROR_PARAMETER4);
}

/*===========================================================================*/
/* Function: Set Timegate Time                                               */
/* Purpose:  This function specifies the duration of one of the four gates   */
/*           (sweeps) relative to the delayed trigger point.                 */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_tGate_setTime (ViSession instrSession,
                                       ViInt32 channel, ViInt32 selectGate,
                                       ViReal64 time)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    if (rsnrp_invalidViInt32Range(selectGate, 1, 4))
        return VI_ERROR_PARAMETER3;

    sprintf (buffer,"SENS%%ld:TGAT%ld:TIME", selectGate);
    
    return rsnrp_setViReal64Channel(instrSession,
                                    buffer,
                                    channel,
                                    time,
                                    0.0, 0.0,
                                    VI_ERROR_PARAMETER4);
}

/*===========================================================================*/
/* Function: Get Timegate Time                                               */
/* Purpose:  This function reads the duration of one of the four gates       */
/*           (sweeps) relative to the delayed trigger point.                 */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_tGate_getTime (ViSession instrSession,
                                       ViInt32 channel, ViInt32 selectGate,
                                       ViReal64 *time)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    if (rsnrp_invalidViInt32Range(selectGate, 1, 4))
        return VI_ERROR_PARAMETER3;

    sprintf (buffer,"SENS%%ld:TGAT%ld:TIME?", selectGate);
    
    return rsnrp_getViReal64Channel(instrSession,
                                    buffer,
                                    channel,
                                    time,
                                    VI_ERROR_PARAMETER4);
}

/*===========================================================================*/
/* Function: Configure Timeslot                                              */
/* Purpose:  This function configures the parameters of Timeslot measurement */
/*           mode. Both exclude start and stop are set to 10% of timeslot    */
/*           width each.                                                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_tslot_configureTimeSlot (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViInt32 timeSlotCount,
                                                 ViReal64 width)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_tslot_setTimeSlotCount(instrSession, channel, timeSlotCount));
    
    rsnrp_status=rsnrp_tslot_setTimeSlotWidth(instrSession, channel, width);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        return VI_ERROR_PARAMETER4;
    
    CHECKERR(rsnrp_timing_setTimingExcludeStart(instrSession,channel,0.1*width));
    return rsnrp_timing_setTimingExcludeStop(instrSession,channel,0.1*width);
}

/*===========================================================================*/
/* Function: Set Time Slot Count                                             */
/* Purpose:  This function sets the number of simultaneously measured        */
/*           timeslots in the Timeslot mode.                                 */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_tslot_setTimeSlotCount (ViSession instrSession,
                                                ViInt32 channel,
                                                ViInt32 timeSlotCount)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViInt32Channel(instrSession,
                                   "SENS%ld:TSL:COUN",
                                   channel, 
                                   timeSlotCount, 1, 64, 
                                   VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Time Slot Count                                             */
/* Purpose:  This function reads the number of simultaneously measured       */
/*           timeslots in the Timeslot mode.                                 */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_tslot_getTimeSlotCount (ViSession instrSession,
                                                ViInt32 channel,
                                                ViInt32 *timeSlotCount)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViInt32Channel(instrSession,
                                   "SENS%ld:TSL:COUN?",
                                   channel,
                                   timeSlotCount,
                                   VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Time Slot Width                                             */
/* Purpose:  This function sets the length of the timeslot in the Timeslot   */
/*           mode.                                                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_tslot_setTimeSlotWidth (ViSession instrSession,
                                                ViInt32 channel, ViReal64 width)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                    "SENS%ld:TSL:WIDT",
                                    channel, 
                                    width, 
                                    0.0, 0.0, 
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Time Slot Width                                             */
/* Purpose:  This function reads the length of the timeslot in the Timeslot  */
/*           mode.                                                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_tslot_getTimeSlotWidth (ViSession instrSession,
                                                ViInt32 channel, ViReal64 *width)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                    "SENS%ld:TSL:WIDT?",
                                    channel, 
                                    width, 
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Configure Scope                                                 */
/* Purpose:  This function sets parameters of the Scope mode.                */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_scope_configureScope (ViSession instrSession,
                                              ViInt32 channel,
                                              ViInt32 scopePoints,
                                              ViReal64 scopeTime,
                                              ViReal64 offsetTime,
                                              ViBoolean realtime)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_scope_setAverageEnabled(instrSession, channel, VI_FALSE));
    CHECKERR(rsnrp_scope_setAverageTerminalControl (instrSession, channel,
                                                    RSNRP_TERMINAL_CONTROL_REPEAT));
    CHECKERR(rsnrp_scope_setPoints(instrSession,channel,scopePoints));

    rsnrp_status = rsnrp_scope_setTime(instrSession, channel, scopeTime);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        return VI_ERROR_PARAMETER4;

    if (!rsnrp_status)
    {
        rsnrp_status = rsnrp_scope_setOffsetTime(instrSession, channel, offsetTime);
        if (rsnrp_status == VI_ERROR_PARAMETER3)
            return VI_ERROR_PARAMETER5;
    }
    
    if (!rsnrp_status)
    {
        rsnrp_status = rsnrp_scope_setRealtimeEnabled(instrSession, channel, realtime);
        if (rsnrp_status == VI_ERROR_PARAMETER3)
            return VI_ERROR_PARAMETER6;
    }

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Scope Averaging State                                       */
/* Purpose:  This function switches the filter function of a sensor on or    */
/*           off.                                                            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_scope_setAverageEnabled (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViBoolean scopeAveraging)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession,
                                "SENS%ld:TRAC:AVER",
                                channel,
                                scopeAveraging, 
                                switchArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Scope Averaging State                                       */
/* Purpose:  This function reads the state of filter function of a sensor.   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_scope_getAverageEnabled (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViBoolean *scopeAveraging)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViBooleanChannel(instrSession,
                                "SENS%ld:TRAC:AVER?",
                                channel,
                                scopeAveraging, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Scope Average Count                                         */
/* Purpose:  This function sets the length of the filter for the Scope mode. */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_scope_setAverageCount (ViSession instrSession,
                                               ViInt32 channel, ViInt32 count)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViInt32Channel(instrSession,
                                   "SENS%ld:TRAC:AVER:COUN",
                                   channel, 
                                   count,
                                   0, 0, 
                                   VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Scope Average Count                                         */
/* Purpose:  This function returns the length of the filter                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_scope_getAverageCount (ViSession instrSession,
                                               ViInt32 channel, ViInt32 *count)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViInt32Channel(instrSession,
                                   "SENS%ld:TRAC:AVER:COUN?",
                                   channel, 
                                   count,
                                   VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Scope Terminal Control                                      */
/* Purpose:  This function determines whether a new result will be calculated*/
/*           immediately after a new measured value is available or only     */
/*           after an entire range of new values is available for the filter.*/
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_scope_setAverageTerminalControl (ViSession instrSession,
                                                         ViInt32 channel,
                                                         ViInt32 terminalControl)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession,
                                "SENS%ld:TRAC:AVER:TCON",
                                channel,
                                terminalControl, 
                                tControlArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Scope Terminal Control                                      */
/* Purpose:  This function returns selected terminal control type.           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_scope_getAverageTerminalControl (ViSession instrSession,
                                                         ViInt32 channel,
                                                         ViInt32 *terminalControl)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getEnumChannel(instrSession,
                                "SENS%ld:TRAC:AVER:TCON?",
                                channel,
                                terminalControl, 
                                tControlArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Scope Offset Time                                           */
/* Purpose:  This function determines the relative position of the trigger   */
/*           event in relation to the beginning of the Scope measurement     */
/*           sequence.                                                       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_scope_setOffsetTime (ViSession instrSession,
                                             ViInt32 channel,
                                             ViReal64 offsetTime)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                    "SENS%ld:TRAC:OFFS:TIME",
                                    channel, 
                                    offsetTime, 
                                    0.0, 0.0, 
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Scope Offset Time                                           */
/* Purpose:  This function reads the relative position of the trigger event  */
/*           in relation to the beginning of the Scope measurement sequence. */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_scope_getOffsetTime (ViSession instrSession,
                                             ViInt32 channel,
                                             ViReal64 *offsetTime)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                    "SENS%ld:TRAC:OFFS:TIME?",
                                    channel, 
                                    offsetTime, 
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Scope Points                                                */
/* Purpose:  This function sets the number of desired values per Scope       */
/*           sequence.                                                       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_scope_setPoints (ViSession instrSession,
                                         ViInt32 channel, ViInt32 scopePoints)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViInt32Channel(instrSession,
                                   "SENS%ld:TRAC:POIN",
                                   channel, 
                                   scopePoints,
                                   0, 0, 
                                   VI_ERROR_PARAMETER3);
    
}

/*===========================================================================*/
/* Function: Get Scope Points                                                */
/* Purpose:  This function reads the number of desired values per Scope      */
/*           sequence.                                                       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_scope_getPoints (ViSession instrSession,
                                         ViInt32 channel, ViInt32 *scopePoints)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViInt32Channel(instrSession,
                                   "SENS%ld:TRAC:POIN?",
                                   channel, 
                                   scopePoints,
                                   VI_ERROR_PARAMETER3);
    
}

/*===========================================================================*/
/* Function: Set Scope Realtime State                                        */
/* Purpose:  This function sets the state of real-time measurement setting.  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_scope_setRealtimeEnabled (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViBoolean realtime)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession,
                                "SENS%ld:TRAC:REAL",
                                channel,
                                realtime, 
                                switchArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Scope Realtime State                                        */
/* Purpose:  This function reads the state of real-time measurement setting. */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_scope_getRealtimeEnabled (ViSession instrSession,
                                                  ViInt32 channel,
                                                  ViBoolean *realtime)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViBooleanChannel(instrSession,
                                "SENS%ld:TRAC:REAL?",
                                channel,
                                realtime, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Scope Time                                                  */
/* Purpose:  This function sets the time to be covered by the Scope sequence.*/
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_scope_setTime (ViSession instrSession,
                                       ViInt32 channel, ViReal64 scopeTime)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                    "SENS%ld:TRAC:TIME",
                                    channel, 
                                    scopeTime, 
                                    0.0, 0.0, 
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Scope Time                                                  */
/* Purpose:  This function reads the value of the time to be covered by the  */
/*           Scope sequence.                                                 */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_scope_getTime (ViSession instrSession,
                                       ViInt32 channel, ViReal64 *scopeTime)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                    "SENS%ld:TRAC:TIME?",
                                    channel, 
                                    scopeTime, 
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Configure Internal                                              */
/* Purpose:  This function configures the parameters of internal trigger     */
/*           system.                                                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_configureInternal (ViSession instrSession,
                                                   ViInt32 channel,
                                                   ViReal64 triggerLevel,
                                                   ViInt32 triggerSlope)
{
    ViStatus    rsnrp_status;

    CHECKERR(rsnrp_trigger_setSource (instrSession, channel,
                                      RSNRP_TRIGGER_SOURCE_INTERNAL));
    CHECKERR(rsnrp_trigger_setAutoTriggerEnabled (instrSession, channel, VI_FALSE));
    CHECKERR(rsnrp_trigger_setAutoDelayEnabled (instrSession, channel, VI_TRUE));
    CHECKERR(rsnrp_trigger_setDelay (instrSession, channel, 0.0));
    CHECKERR(rsnrp_trigger_setCount (instrSession, channel, 1));
    CHECKERR(rsnrp_trigger_setHoldoff (instrSession, channel, 0.0));
    CHECKERR(rsnrp_trigger_setHysteresis (instrSession, channel, 3.0));
    CHECKERR(rsnrp_trigger_setLevel (instrSession, channel, triggerLevel));

    rsnrp_status = rsnrp_trigger_setSlope(instrSession, channel, triggerSlope);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        rsnrp_status = VI_ERROR_PARAMETER4;

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Configure External                                              */
/* Purpose:  This function configures the parameters of external trigger     */
/*           system.                                                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_configureExternal (ViSession instrSession,
                                                   ViInt32 channel,
                                                   ViReal64 triggerDelay)
{
    ViStatus    rsnrp_status;

    CHECKERR(rsnrp_trigger_setSource (instrSession, channel,
                                      RSNRP_TRIGGER_SOURCE_EXTERNAL));
    CHECKERR(rsnrp_trigger_setAutoTriggerEnabled (instrSession, channel, VI_FALSE));
    CHECKERR(rsnrp_trigger_setAutoDelayEnabled (instrSession, channel, VI_TRUE));
    CHECKERR(rsnrp_trigger_setDelay (instrSession, channel, triggerDelay));
    CHECKERR(rsnrp_trigger_setCount (instrSession, channel, 1));
    CHECKERR(rsnrp_trigger_setHoldoff (instrSession, channel, 0.0));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Immediate Trigger                                               */
/* Purpose:  This function performs triggering                               */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_immediate (ViSession instrSession,
                                           ViInt32 channel)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));

    return rsnrp_sendCmdChannel(instrSession,
                                "TRIG%ld",
                                channel);
}

/*===========================================================================*/
/* Function: Set Trigger Synchronize                                         */
/* Purpose:  Using this function one can assure, that measurements with      */
/*           externally triggered sensors start synchronously.               */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_setSynchronize (ViSession instrSession,
                                                ViBoolean state)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    CHECKERR(rsnrp_setEnum(instrSession,
                           "TRIG:ALL:SYNC",
                           state,
                           switchArr,
                           VI_ERROR_PARAMETER2));
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Trigger Synchronize                                         */
/* Purpose:  This function returns the state of trigger sinchronization.     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_getSynchronize (ViSession instrSession,
                                                ViBoolean *state)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    CHECKERR(rsnrp_getViBoolean(instrSession,
                                "TRIG:ALL:SYNC?",
                                state,
                                VI_ERROR_PARAMETER2));
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Trigger Auto Delay State                                    */
/* Purpose:  This function sets the setting of auto delay feature.           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_setAutoDelayEnabled (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViBoolean autoDelay)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession,
                                "TRIG%ld:DEL:AUTO",
                                channel,
                                autoDelay, 
                                switchArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Trigger Auto Delay State                                    */
/* Purpose:  This function reads the setting of auto delay feature.          */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_getAutoDelayEnabled (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViBoolean *autoDelay)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViBooleanChannel(instrSession,
                                "TRIG%ld:DEL:AUTO?",
                                channel,
                                autoDelay, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Auto Trigger State                                          */
/* Purpose:  This function turns On or Off the auto trigger feature.         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_setAutoTriggerEnabled (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViBoolean autoTrigger)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession,
                                "TRIG%ld:ATR",
                                channel,
                                autoTrigger, 
                                switchArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Auto Trigger State                                          */
/* Purpose:  This function reads the state of Auto Trigger.                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_getAutoTriggerEnabled (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViBoolean *autoTrigger)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViBooleanChannel(instrSession,
                                "TRIG%ld:ATR?",
                                channel,
                                autoTrigger, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Trigger Count                                               */
/* Purpose:  This function sets the number of measurement cycles to be       */
/*           performed when the measurement is started with INIT.            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_setCount (ViSession instrSession,
                                          ViInt32 channel, ViInt32 triggerCount)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViInt32Channel(instrSession, 
                                    "TRIG%ld:COUN",
                                    channel, 
                                    triggerCount, 
                                    0, 0,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Trigger Count                                               */
/* Purpose:  This function reads the number of measurement cycles to be      */
/*           performed when the measurement is started with INIT.            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_getCount (ViSession instrSession,
                                          ViInt32 channel,
                                          ViInt32 *triggerCount)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViInt32Channel(instrSession, 
                                    "TRIG%ld:COUN?",
                                    channel, 
                                    triggerCount, 
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Trigger Delay                                               */
/* Purpose:  This function defines the delay between the trigger event and   */
/*           the beginning of the actual measurement (integration).          */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_setDelay (ViSession instrSession,
                                          ViInt32 channel, ViReal64 triggerDelay)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                    "TRIG%ld:DEL",
                                    channel, 
                                    triggerDelay, 
                                    0.0, 0.0,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Trigger Delay                                               */
/* Purpose:  This function reads value of the delay (in seconds) between the */
/*           trigger event and the beginning of the actual measurement       */
/*           (integration).                                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_getDelay (ViSession instrSession,
                                          ViInt32 channel,
                                          ViReal64 *triggerDelay)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                    "TRIG%ld:DEL?",
                                    channel, 
                                    triggerDelay, 
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Trigger Holdoff                                             */
/* Purpose:  This function defines a period after a trigger event within     */
/*           which all further trigger events are ignored.                   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_setHoldoff (ViSession instrSession,
                                            ViInt32 channel,
                                            ViReal64 triggerHoldoff)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                    "TRIG%ld:HOLD",
                                    channel, 
                                    triggerHoldoff, 
                                    0.0, 0.0,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Trigger Holdoff                                             */
/* Purpose:  This function returns the value of a period after a trigger     */
/*           event within which all further trigger events are ignored.      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_getHoldoff (ViSession instrSession,
                                            ViInt32 channel,
                                            ViReal64 *triggerHoldoff)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                    "TRIG%ld:HOLD?",
                                    channel, 
                                    triggerHoldoff, 
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Trigger Hysteresis                                          */
/* Purpose:  This function is used to specify how far the signal level has   */
/*           to drop below the trigger level before a new signal edge can be */
/*           detected as a trigger event.                                    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_setHysteresis (ViSession instrSession,
                                               ViInt32 channel,
                                               ViReal64 triggerHysteresis)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                    "TRIG%ld:HYST",
                                    channel, 
                                    triggerHysteresis, 
                                    0.0, 0.0,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Trigger Hysteresis                                          */
/* Purpose:  This function reads the value of trigger hysteresis.            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_getHysteresis (ViSession instrSession,
                                               ViInt32 channel,
                                               ViReal64 *triggerHysteresis)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                    "TRIG%ld:HYST?",
                                    channel, 
                                    triggerHysteresis, 
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Trigger Level                                               */
/* Purpose:  This function determines the power a trigger signal must exceed */
/*           before a trigger event is detected. This setting is only used   */
/*           for internal trigger signal source.                             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_setLevel (ViSession instrSession,
                                          ViInt32 channel, ViReal64 triggerLevel)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setViReal64Channel(instrSession, 
                                    "TRIG%ld:LEV",
                                    channel, 
                                    triggerLevel, 
                                    0.0, 0.0,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Trigger Level                                               */
/* Purpose:  This function reads the power a trigger signal must exceed      */
/*           before a trigger event is detected.                             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_getLevel (ViSession instrSession,
                                          ViInt32 channel,
                                          ViReal64 *triggerLevel)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViReal64Channel(instrSession, 
                                    "TRIG%ld:LEV?",
                                    channel, 
                                    triggerLevel, 
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Trigger Slope                                               */
/* Purpose:  This function determines whether the rising (POSitive) or the   */
/*           falling (NEGative) edge of the signal is used for triggering.   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_setSlope (ViSession instrSession,
                                          ViInt32 channel, ViInt32 triggerSlope)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession,
                                "TRIG%ld:SLOP",
                                channel,
                                triggerSlope, 
                                slopeArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Trigger Slope                                               */
/* Purpose:  This function reads whether the rising (POSitive) or the falling*/
/*           (NEGative) edge of the signal is used for triggering.           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_getSlope (ViSession instrSession,
                                          ViInt32 channel, ViInt32 *triggerSlope)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getEnumChannel(instrSession,
                                "TRIG%ld:SLOP?",
                                channel,
                                triggerSlope, 
                                slopeArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Trigger Source                                              */
/* Purpose:  This function sets the trigger signal source for the            */
/*           WAIT_FOR_TRG state.                                             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_setSource (ViSession instrSession,
                                           ViInt32 channel,
                                           ViInt32 triggerSource)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession,
                                "TRIG%ld:SOUR",
                                channel,
                                triggerSource, 
                                trgSourceArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Trigger Source                                              */
/* Purpose:  This function gets the trigger signal source for the            */
/*           WAIT_FOR_TRG state.                                             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_trigger_getSource (ViSession instrSession,
                                           ViInt32 channel,
                                           ViInt32 *triggerSource)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getEnumChannel(instrSession,
                                "TRIG%ld:SOUR?",
                                channel,
                                triggerSource, 
                                trgSourceArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Sensor Info                                                     */
/* Purpose:  This function returns selected information on a sensor.         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_info (ViSession instrSession, ViInt32 channel,
                                   ViString infoType, ViInt32 arraySize,
                                   ViChar _VI_FAR info[])
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];

    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));

    if (!infoType)
        return VI_ERROR_PARAMETER3;

    sprintf(buffer,"SYST:SENS%ld:INFO? '%s'",channel, infoType);
    
    return rsnrp_getViString(instrSession,
                             buffer,
                             info,
                             arraySize,
                             VI_ERROR_PARAMETER4);
}

/*===========================================================================*/
/* Function: Sensor Info Headers                                             */
/* Purpose:  This function returns all available headers which can be        */
/*           retrieved from selected sensor.                                 */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_infoHeader (ViSession instrSession,
                                         ViInt32 channel,
                                         ViInt32 parameterNumber,
                                         ViInt32 arraySize, ViChar _VI_FAR header[])
{
    ViStatus    rsnrp_status;
    ViChar*     buffer = VI_NULL;
    ViChar*     p2buf;
    ViUInt32    count_local;
    ViInt32     i;
    ViChar      common_buffer[BUFFER_SIZE];
    ViBoolean   old_check;

    GETSESSIONDATA(instrSession);

    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    if (parameterNumber < 0)
        return VI_ERROR_PARAMETER3;

    if (!header)
        return VI_ERROR_PARAMETER4;

    /* Turn off error checking */
    old_check = instrPtr->errorChecking;
    CHECKERR(rsnrp_errorCheckState(instrSession,VI_FALSE));
    
    CHECKERR(rsnrp_sendCmdChannel(instrSession,
                                  "SYST:SENS%ld:INFO?",
                                  channel));
    CHECKERR(rsnrp_readDataUnknownLength(instrSession,&buffer, &count_local));
    
    p2buf = buffer+1;
    i = 0;
    
    while(p2buf && (parameterNumber > i))
    {
        p2buf = strstr(p2buf,"\",\"");
        if (p2buf)
        {
            p2buf+=3; /* move pointer behind the "," */
            i++;
        }
    }
    if (p2buf)
    {
        sscanf(p2buf,"%[^:]",common_buffer);
        strncpy(header,common_buffer,arraySize);
    }
    
    free(buffer);
    
    /* Set error checking to previous state */
    if(old_check)
            CHECKERR(rsnrp_checkStatus(instrSession));

    CHECKERR(rsnrp_errorCheckState(instrSession,old_check));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Sensor Info Count                                               */
/* Purpose:  This function returns the number of info headers for selected   */
/*           channel.                                                        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_infosCount (ViSession instrSession,
                                         ViInt32 channel, ViInt32 *count)
{
    ViStatus    rsnrp_status;
    ViChar*     buffer = VI_NULL;
    ViChar*     p2buf;
    ViUInt32    count_local, i;
    ViBoolean   old_check;

    GETSESSIONDATA(instrSession);
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    if (!count)
        return VI_ERROR_PARAMETER3;

    /* Turn off error checking */
    old_check = instrPtr->errorChecking;
    CHECKERR(rsnrp_errorCheckState(instrSession,VI_FALSE));

    CHECKERR(rsnrp_sendCmdChannel(instrSession,
                                  "SYST:SENS%ld:INFO?",
                                  channel));
    CHECKERR(rsnrp_readDataUnknownLength(instrSession,&buffer, &count_local));
    
    p2buf = buffer;
    i = 0;
    
    while(p2buf)
    {
        p2buf = strchr(p2buf,':');
        if (p2buf)
        {
            p2buf++; /* move pointer behind the : */
            i++;
        }
    }
    *count = i; /* the command returns pairs header/value, thus
                   the number of headers is equal to number of : */
    free(buffer);
    
    /* Set error checking to previous state */
    if(old_check)
            CHECKERR(rsnrp_checkStatus(instrSession));

    CHECKERR(rsnrp_errorCheckState(instrSession,old_check));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Abort                                                           */
/* Purpose:  This function immediately sets selected sensor to the IDLE      */
/*           state. Measurements in progress are interrupted.                */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_abort (ViSession instrSession, ViInt32 channel)
{
    ViStatus    rsnrp_status;

    GETSESSIONDATA(instrSession);
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    /* clear the measurement ready for selected channel */
    instrPtr->events.measuring &= ~(1<<channel);

    CHECKERR(rsnrp_sendCmdChannel(instrSession,
                                "ABOR%ld",
                                channel));
    /* read the value of measurement status event register and
       throw away the bit for selected channel */
    CHECKERR(rsnrp_updateCache(instrSession, RSNRP_STATCLASS_O_MEAS, (1<<channel), 0));
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Initiate                                                        */
/* Purpose:  This function starts a single-shot measurement on selected      */
/*           channel.                                                        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_initiate (ViSession instrSession,
                                       ViInt32 channel)
{
    ViStatus    rsnrp_status;

    GETSESSIONDATA(instrSession);
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    /* read the status to the cache to clear the measuring status register*/
    CHECKERR(rsnrp_updateCache(instrSession,RSNRP_STATCLASS_O_MEAS,(1<<channel),0));

    return rsnrp_sendCmdChannel(instrSession,
                                "INIT%ld",
                                channel);
}

/*===========================================================================*/
/* Function: Set Initiate Continuous State                                   */
/* Purpose:  This function selects either single-shot or continuous          */
/*           (free-running) measurement cycles.                              */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_setInitContinuousEnabled (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViBoolean continuousInitiate)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession,
                                "INIT%ld:CONT",
                                channel,
                                continuousInitiate, 
                                switchArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Initiate Continuous State                                   */
/* Purpose:  This function returns whether single-shot or continuous         */
/*           (free-running) measurement is selected.                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_getInitContinuousEnabled (ViSession instrSession,
                                                       ViInt32 channel,
                                                       ViBoolean *continuousInitiate)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViBooleanChannel(instrSession,
                                "INIT%ld:CONT?",
                                channel,
                                continuousInitiate, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Reset                                                           */
/* Purpose:  This function prompts the basic unit to send an *RST to the     */
/*           respective sensor. Measurements in progress are interrupted.    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_reset (ViSession instrSession, ViInt32 channel)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_sendCmdChannel(instrSession,
                                "SYST:SENS%ld:RES",
                                channel);
}

/*===========================================================================*/
/* Function: Set Sampling Frequency                                          */
/* Purpose:  This function selects the sampling frequency.                   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_setSamplingFrequency (ViSession instrSession,
                                                   ViInt32 channel,
                                                   ViInt32 samplingFrequency)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_setEnumChannel(instrSession,
                                "SENSe%ld:SAMP",
                                channel,
                                samplingFrequency, 
                                sampFreqArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Sampling Frequency                                          */
/* Purpose:  This function returns the selected sampling frequency.          */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_getSamplingFrequency (ViSession instrSession,
                                                   ViInt32 channel,
                                                   ViInt32 *samplingFrequency)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getEnumChannel(instrSession,
                                "SENSe%ld:SAMP?",
                                channel,
                                samplingFrequency, 
                                sampFreqArr, 
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Zero                                                            */
/* Purpose:  This function performs zeroing of selected sensor using the     */
/*           signal at the sensor input.                                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_zero (ViSession instrSession, ViInt32 channel)
{
    ViStatus    rsnrp_status;

    GETSESSIONDATA(instrSession);
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    /* read the status to the cache to clear the measuring status register and
       clear the measurement ready for selected channel */
    CHECKERR(rsnrp_updateCache(instrSession,RSNRP_STATCLASS_O_MEAS,(1<<channel),0));

    return rsnrp_sendCmdChannel(instrSession,
                                "CAL%ld:ZERO:AUTO ONCE",
                                channel);
}

/*===========================================================================*/
/* Function: Get Zeroing Complete State                                      */
/* Purpose:  This function returns the state of zeroing of the sensor.       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_isZeroComplete (ViSession instrSession,
                                             ViInt32 channel,
                                             ViBoolean *zeroingComplete)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    
    GETSESSIONDATA(instrSession);

    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));

    /* If there's 0 in the cache, read the state from instrument and fill
       the cache with new values */
    if (!(instrPtr->events.calibrating & (1<<channel)))
    {
        CHECKERR(rsnrp_updateCache(instrSession, RSNRP_STATCLASS_O_CAL, 0, 0));
    }

    /* Get the state of measurement from the cache and clear appropriate bit
       from the cache */
    if (instrPtr->events.calibrating & (1<<channel))
    {
        *zeroingComplete = VI_TRUE;
        instrPtr->events.calibrating &= ~(1<<channel);
    }
    else
        *zeroingComplete = VI_FALSE;

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Measurement Complete State                                  */
/* Purpose:  This function returns the state of the measurement.             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_isMeasurementComplete (ViSession instrSession,
                                                    ViInt32 channel,
                                                    ViBoolean *measurementComplete)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    
    GETSESSIONDATA(instrSession);

    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));

    /* If there's 0 in the cache, read the state from instrument and fill
       the cache with new values */
    if (!(instrPtr->events.measuring & (1<<channel)))
    {
        CHECKERR(rsnrp_updateCache(instrSession, RSNRP_STATCLASS_O_MEAS, 0, 0));
    }

    /* Get the state of measurement from the cache and clear appropriate bit
       from the cache */
    if (instrPtr->events.measuring & (1<<channel))
    {
        *measurementComplete = VI_TRUE;
        instrPtr->events.measuring &= ~(1<<channel);
    }
    else
        *measurementComplete = VI_FALSE;
    

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Sensor Self Test                                                */
/* Purpose:  This function performs a sensor test and returns a list of      */
/*           strings separated by commas. The contents of this test protocol */
/*           is sensor-specific.                                             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_chan_selfTest (ViSession instrSession,
                                       ViInt32 channel, ViChar _VI_FAR result[])
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       channel,
                                       VI_ERROR_PARAMETER2));
    
    return rsnrp_getViStringChannel(instrSession,
                                "SYST:SENS%ld:TEST?",
                                channel,
                                result,
                                BUFFER_SIZE,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Count                                                           */
/* Purpose:  This function returns the number of available spaces for offset */
/*           tables. Always returns 10.                                      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_offstbls_getCount (ViSession instrSession,
                                           ViInt32 *count)
{
    if (count)
    {
        *count = 10;
        return VI_SUCCESS;
    }
    else
        return VI_ERROR_PARAMETER2;
}

/*===========================================================================*/
/* Function: Define Name                                                     */
/* Purpose:  The numbers 0 to 9 are permanently assigned to the memory       */
/*           locations for device states. Names can be assigned to the       */
/*           numbers with this function.                                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_offstbl_defineName (ViSession instrSession,
                                            ViString tableName,
                                            ViInt32 tableNumber)
{
    ViChar      buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(tableNumber, 0, 9))
        return VI_ERROR_PARAMETER3;
    
    sprintf(buffer, "MEM:TABL:MAP %s,%ld",tableName, tableNumber);
    
    return rsnrp_sendCmd(instrSession,
                         buffer);
}

/*===========================================================================*/
/* Function: Read Name                                                       */
/* Purpose:  This function returns a name of correction table defined by     */
/*           table number.                                                   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_offstbl_readName (ViSession instrSession,
                                          ViInt32 tableNumber, ViInt32 arraySize,
                                          ViChar _VI_FAR name[])
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    ViChar*     p2buf;
    ViInt32     i = 0;

    if (rsnrp_invalidViInt32Range(tableNumber,0,9))
        return VI_ERROR_PARAMETER2;

    if (!name)
        return VI_ERROR_PARAMETER3;

    CHECKERR(rsnrp_getViString(instrSession,"MEM:CAT:TABL?",buffer,BUFFER_SIZE,
                               VI_ERROR_PARAMETER3));
    
    p2buf=strtok(buffer,"\"");
    p2buf=strtok(NULL,"\"");
    
    for (i=0;i<tableNumber;i++)
    {
        p2buf=strtok(NULL,"\"");
        p2buf=strtok(NULL,"\"");
    }

    p2buf=strtok(p2buf,",");
    
    if (!p2buf)
        rsnrp_status = RSNRP_ERROR_TABLE_NOT_FOUND;
    else
        strncpy(name,p2buf,arraySize);
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Define Elements                                                 */
/* Purpose:  This function defines the table points for selected table.      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_offstbl_defineElements (ViSession instrSession,
                                                ViInt32 tableNumber,
                                                ViInt32 count,
                                                ViReal64 frequencyArray[],
                                                ViReal64 magnitudeArray[])
{
    ViStatus    rsnrp_status;
    ViChar*     p2buf;
    ViInt32     i;
    ViChar*     dyn_buffer;

    if (count <= 0)
        return VI_ERROR_PARAMETER3;

    if (!frequencyArray)
        return VI_ERROR_PARAMETER4;
    
    if (!magnitudeArray)
        return VI_ERROR_PARAMETER5;
    
    CHECKERR(rsnrp_selectTable(instrSession, tableNumber));

    dyn_buffer = (ViChar*)malloc(count*16);
    
    p2buf = dyn_buffer; /* use p2buf only as pointer to already existing array */
    
    for (i=0; i < count; i++)
        p2buf += sprintf(p2buf,"%Lf,",frequencyArray[i]);
    
    /* remove last comma */
    *--p2buf = '\0';
    
    if ((rsnrp_status=rsnrp_setViString(instrSession,
                               "MEM:TABL:FREQ",
                               dyn_buffer,
                               VI_ERROR_PARAMETER3)) < VI_SUCCESS)
    {
        free(dyn_buffer);
        return rsnrp_status;
    }

    /* process array of gains */
    p2buf = dyn_buffer;
    for (i=0; i < count; i++)
        p2buf += sprintf(p2buf,"%Lf,",magnitudeArray[i]);
    
    /* remove last comma */
    *--p2buf = '\0';
    
    rsnrp_status=rsnrp_setViString(instrSession,
                               "MEM:TABL:GAIN",
                               dyn_buffer,
                               VI_ERROR_PARAMETER3);
    free(dyn_buffer);
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Read Elements                                                   */
/* Purpose:  This function reads all data in the selected table.             */
/*
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_offstbl_readElements (ViSession instrSession,
                                              ViInt32 tableNumber,
                                              ViInt32 *count,
                                              ViReal64 frequencyArray[],
                                              ViReal64 magnitudeArray[])
{
    ViStatus    rsnrp_status;
    ViChar*     p2buf;
    ViInt32     i, local_count;
    ViChar*     dyn_buffer;

    if (!count && !frequencyArray && !magnitudeArray)
        return VI_SUCCESS;
    
    CHECKERR(rsnrp_selectTable(instrSession, tableNumber));
    CHECKERR(rsnrp_getViInt32(instrSession,
                              "MEM:TABL:FREQ:POIN?",
                              &local_count,
                              VI_ERROR_PARAMETER3));
    if (count)
        *count = local_count;

    if (frequencyArray || magnitudeArray)
    {
        dyn_buffer = (ViChar*)malloc(local_count*16);
    
        if (frequencyArray)
        {
            if ((rsnrp_status=rsnrp_getViString(instrSession,
                                       "MEM:TABL:FREQ?",
                                       dyn_buffer,
                                       local_count*16,
                                       VI_ERROR_PARAMETER3)) < VI_SUCCESS)
            {
                free(dyn_buffer);
                return rsnrp_status;
            }

            p2buf = strtok(dyn_buffer,",");

            for (i=0; i < local_count; i++)
            {
                frequencyArray[i] = atof(p2buf);
                p2buf = strtok(NULL,",");
            }
        } /* if (frequencyArray) */

    
        if (magnitudeArray)
        {
            /* process array of gains */
            if ((rsnrp_status=rsnrp_getViString(instrSession,
                                       "MEM:TABL:GAIN?",
                                       dyn_buffer,
                                       local_count*16,
                                       VI_ERROR_PARAMETER3)) < VI_SUCCESS)
            {
                free(dyn_buffer);
                return rsnrp_status;
            }

            p2buf = strtok(dyn_buffer,",");

            for (i=0; i < local_count; i++)
            {
                magnitudeArray[i] = atof(p2buf);
                p2buf = strtok(NULL,",");
            }
        } /* if (magnitudeArray) */
        free(dyn_buffer);
    } /* if (frequencyArray || magnitudeArray) */
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Abort                                                           */
/* Purpose:  This function immediately sets the respective sensors to the    */
/*           IDLE state. Measurements in progress are interrupted.           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meass_abort (ViSession instrSession)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViInt32     i;
    
    for (i = 1; i<= MAX_MEASUREMENTS; i++)
    {
        CHECKERR(rsnrp_meas_abort(instrSession, i));
    }
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Count                                                           */
/* Purpose:  This function always returns number 8 which is the number of    */
/*           calculation blocks available.                                   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meass_getCount (ViSession instrSession,
                                        ViInt32 *count)
{
    if (count)
        *count = MAX_MEASUREMENTS;
    else
        return VI_ERROR_PARAMETER2;
        
    return VI_SUCCESS;
}

/*===========================================================================*/
/* Function: Initiate                                                        */
/* Purpose:  This function starts a single-shot measurement on all channels  */
/*           asociated with measurements.                                    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meass_initiate (ViSession instrSession)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViInt32     i;
    
    for (i = 1; i<= MAX_MEASUREMENTS; i++)
    {
        CHECKERR(rsnrp_meas_initiate(instrSession,i));
    }
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Send Software Trigger                                           */
/* Purpose:  Triggers a BUS event. If the sensor is in the WAIT_FOR_TRG      */
/*           state and the source for the trigger source is set to BUS,      */
/*           the sensor enters the MEASURING state. This function            */
/*           invalidates all current measuring results. A query of           */
/*           measurement data following this function will thus always return*/
/*           the measured value determined in response to this function.     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meass_sendSoftwareTrigger (ViSession instrSession)
{
    ViStatus    rsnrp_status;
    ViUInt16    meas_event_before, meas_event_after, meas_event_invalid;
    ViInt32     i;

    GETSESSIONDATA(instrSession);

    CHECKERR(rsnrp_getViBoolean(instrSession,
                               "STAT:OPER:MEAS?",
                               &meas_event_before,
                               VI_ERROR_PARAMETER3));

    CHECKERR(rsnrp_sendCmd(instrSession, "*TRG"));

    CHECKERR(rsnrp_getViBoolean(instrSession,
                               "STAT:OPER:MEAS?",
                               &meas_event_after,
                               VI_ERROR_PARAMETER3));

    /* clear all invalidated measurement bits */
    meas_event_invalid = 0;
    for (i = 1; i <= MAX_CHANNELS; i++)
    {
    /* if measurement was ready before *trg and is not ready after trg, it is
       invalidated. It is necessary to check both shadow and instrument's
       evnet status registers */
        if (
            (
             (meas_event_before & (1<<i)) || (instrPtr->events.measuring & (1<<i))
            ) 

             &&

             !(meas_event_after & (1<<i))
            
           )
            meas_event_invalid |= (1<<i);
    }
    
    /* put new states to the cache */
    instrPtr->events.measuring |= meas_event_after;
    /* invalidate invalid measurements in chache */
    instrPtr->events.measuring &= ~meas_event_invalid;
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Measurement Complete State                                  */
/* Purpose:  This function returns the summary status of all measurements.   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meass_isMeasurementComplete (ViSession instrSession,
                                                     ViBoolean *measurementCompleted)
{
    ViStatus    rsnrp_status;
    ViInt32     i;
    ViUInt32    the_mask = 0;
    
    GETSESSIONDATA(instrSession);
    
    /* create the mask */
    for (i = 1; i <= MAX_MEASUREMENTS; i++)
    {
        if (instrPtr->measurements[i].channel1)
            the_mask |= (1<<instrPtr->measurements[i].channel1);
        if (instrPtr->measurements[i].channel2)
            the_mask |= (1<<instrPtr->measurements[i].channel2);
    }

    /* try the fastest way - are all measurements already in cache? If not,
       read the state from instrument, if yes, clear and return */
    if ((instrPtr->events.measuring & the_mask) == the_mask)
    {
        instrPtr->events.measuring &= ~the_mask;
        *measurementCompleted = VI_TRUE;
        return rsnrp_status;
    }
    
    /* read and update the cache */
    CHECKERR(rsnrp_updateCache(instrSession, RSNRP_STATCLASS_O_MEAS, 0, 0));

    /* check the cache with new values */
    if ((instrPtr->events.measuring & the_mask) == the_mask)
    {
        instrPtr->events.measuring &= ~the_mask;
        *measurementCompleted = VI_TRUE;
    }
    else
        *measurementCompleted = VI_FALSE;

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Abort                                                           */
/* Purpose:  This function immediately sets the sensor(s) asociated with     */
/*           selected measurement to the IDLE state. Measurements in progress*/
/*           are interrupted.                                                */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_abort (ViSession instrSession,
                                    ViInt32 measurement)
{
    ViStatus    rsnrp_status;
    
    GETSESSIONDATA(instrSession);
    
    if (instrPtr->measurements[measurement].channel1)
        CHECKERR(rsnrp_chan_abort(instrSession,
                                  instrPtr->measurements[measurement].channel1));
    if (instrPtr->measurements[measurement].channel2)
        CHECKERR(rsnrp_chan_abort(instrSession,
                                  instrPtr->measurements[measurement].channel2));
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Initiate                                                        */
/* Purpose:  This function starts a single-shot measurement. The respective  */
/*           sensor goes to the INITIATED state.                             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_initiate (ViSession instrSession,
                                       ViInt32 measurement)
{
    ViStatus    rsnrp_status;
    
    GETSESSIONDATA(instrSession);
    
    if (instrPtr->measurements[measurement].channel1)
        CHECKERR(rsnrp_chan_initiate(instrSession,
                                  instrPtr->measurements[measurement].channel1));
    if (instrPtr->measurements[measurement].channel2)
        CHECKERR(rsnrp_chan_initiate(instrSession,
                                  instrPtr->measurements[measurement].channel2));
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Measurement Complete State                                  */
/* Purpose:  This function returns the status of selected measurement.       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_isMeasurementComplete (ViSession instrSession,
                                                    ViInt32 measurement,
                                                    ViBoolean *measurementComplete)
{
    ViStatus    rsnrp_status;
    ViBoolean   meas_completed_local;
    
    GETSESSIONDATA(instrSession);
    
    /* try the fastest way - are all measurements already in cache? If not,
       read the state from instrument, if yes, clear and return */

    meas_completed_local = 0;
    if (instrPtr->measurements[measurement].channel1)
        meas_completed_local = (ViBoolean)instrPtr->events.measuring & (1<<instrPtr->measurements[measurement].channel1);
    else
        if (instrPtr->measurements[measurement].channel2)
            meas_completed_local &= instrPtr->events.measuring & (1<<instrPtr->measurements[measurement].channel2);
        else
            return RSNRP_WARNING_NO_CHANNEL;
            
    if (!meas_completed_local)
    {
        /* read and update the cache */
        CHECKERR(rsnrp_updateCache(instrSession, RSNRP_STATCLASS_O_MEAS, 0, 0));

        /* check the cache with new values */
        if (instrPtr->measurements[measurement].channel1)
            meas_completed_local = (ViBoolean)instrPtr->events.measuring & (1<<instrPtr->measurements[measurement].channel1);

        if (instrPtr->measurements[measurement].channel2)
            meas_completed_local = (ViBoolean)meas_completed_local && (instrPtr->events.measuring & (1<<instrPtr->measurements[measurement].channel2));
    }
            
    if (meas_completed_local)
    {
        /* clear the bits in cache */
            instrPtr->events.measuring &= ~(1<<instrPtr->measurements[measurement].channel1);
            if (instrPtr->measurements[measurement].channel2)
                instrPtr->events.measuring &= ~(1<<instrPtr->measurements[measurement].channel2);
    }
            
    *measurementComplete = (meas_completed_local == 0) ? VI_FALSE : VI_TRUE;

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Configure ContAv                                                */
/* Purpose:  This function configures instrument to measure in ContAv mode.  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_configureContAv (ViSession instrSession,
                                              ViInt32 measurement,
                                              ViInt32 measuringFunction,
                                              ViInt32 sensor1, ViInt32 sensor2,
                                              ViInt32 resolution)
{
    ViStatus    rsnrp_status;
    ViChar      parameterList[BUFFER_SIZE];
    ViChar*     p2buf;
    GETSESSIONDATA(instrSession);
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       sensor1,
                                       VI_ERROR_PARAMETER4));

    if (rsnrp_invalidViInt32Range(resolution, 1, 4))
        return VI_ERROR_PARAMETER6;
    
    p2buf = parameterList;
    
    p2buf += sprintf(parameterList,"DEF,%ld,(@%ld)",
                     resolution,sensor1);
                     
    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
    {
        CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                           sensor2,
                                           VI_ERROR_PARAMETER4));
        sprintf (p2buf,",(@%ld)",sensor2);
    }
    
    CHECKERR(rsnrp_sendHighLevelCommand (instrSession, 
                                       "CONF%ld:%s",
                                       measurement,
                                       measuringFunction,
                                       parameterList,
                                       VI_NULL,
                                       0));

    instrPtr->measurements[measurement].channel1 = sensor1;

    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
        instrPtr->measurements[measurement].channel2 = sensor2;

    return rsnrp_status;    
}

/*===========================================================================*/
/* Function: Configure ContAv Buffered                                       */
/* Purpose:  This function configures instrument to measure in buffered      */
/*           ContAv mode.                                                    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_configureContAvBuffered (ViSession instrSession,
                                                      ViInt32 measurement,
                                                      ViInt32 measuringFunction,
                                                      ViInt32 sensor1,
                                                      ViInt32 sensor2,
                                                      ViInt32 bufferSize,
                                                      ViInt32 resolution)
{
    ViStatus    rsnrp_status;
    ViChar      parameterList[BUFFER_SIZE];
    ViChar*     p2buf;
    GETSESSIONDATA(instrSession);
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       sensor1,
                                       VI_ERROR_PARAMETER4));

    if (rsnrp_invalidViInt32Range(resolution, 1, 4))
        return VI_ERROR_PARAMETER7;
    
    p2buf = parameterList;
    
    p2buf += sprintf(parameterList,"(%ld),DEF,%ld,(@%ld)",
                     bufferSize,resolution,sensor1);
                     
    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
    {
        CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                           sensor2,
                                           VI_ERROR_PARAMETER5));
        sprintf (p2buf,",(@%ld)",sensor2);
    }
    
    CHECKERR(rsnrp_sendHighLevelCommand (instrSession, 
                                       "CONF%ld:ARR:%s",
                                       measurement,
                                       measuringFunction,
                                       parameterList,
                                       VI_NULL,
                                       0));
                                       
    instrPtr->measurements[measurement].channel1 = sensor1;

    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
        instrPtr->measurements[measurement].channel2 = sensor2;

    return rsnrp_status;    
}

/*===========================================================================*/
/* Function: Configure Burst                                                 */
/* Purpose:  This function configures instrument to measure in BurstAv mode. */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_configureBurst (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViInt32 measuringFunction,
                                             ViInt32 sensor1, ViInt32 sensor2,
                                             ViReal64 dropoutTolerance,
                                             ViReal64 excludeStart,
                                             ViReal64 excludeStop,
                                             ViInt32 resolution)
{
    ViStatus    rsnrp_status;
    ViChar      parameterList[BUFFER_SIZE];
    ViChar*     p2buf;
    GETSESSIONDATA(instrSession);
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       sensor1,
                                       VI_ERROR_PARAMETER4));

    if (rsnrp_invalidViInt32Range(resolution, 1, 4))
        return VI_ERROR_INSTR_PARAMETER9;
    
    p2buf = parameterList;
    
    p2buf += sprintf(parameterList,"%Le,%Le,%Le,DEF,%ld,(@%ld)",
                     dropoutTolerance,
                     excludeStart,
                     excludeStop,
                     resolution,
                     sensor1);
                     
    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
    {
        CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                           sensor2,
                                           VI_ERROR_PARAMETER5));
        sprintf (p2buf,",(@%ld)",sensor2);
    }
    
    CHECKERR(rsnrp_sendHighLevelCommand (instrSession, 
                                       "CONF%ld:BURS:%s",
                                       measurement,
                                       measuringFunction,
                                       parameterList,
                                       VI_NULL,
                                       0));
    instrPtr->measurements[measurement].channel1 = sensor1;

    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
        instrPtr->measurements[measurement].channel2 = sensor2;

    return rsnrp_status;    
}

/*===========================================================================*/
/* Function: Configure Timeslot                                              */
/* Purpose:  This function configures the parameters for Timeslot measuring  */
/*           mode of the instrument.                                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_configureTimeslot (ViSession instrSession,
                                                ViInt32 measurement,
                                                ViInt32 measuringFunction,
                                                ViInt32 sensor1, ViInt32 sensor2,
                                                ViReal64 timeslotWidth,
                                                ViReal64 numberOfTimeslots,
                                                ViReal64 excludeStart,
                                                ViReal64 excludeStop,
                                                ViInt32 resolution)
{
    ViStatus    rsnrp_status;
    ViChar      parameterList[BUFFER_SIZE];
    ViChar*     p2buf;
    GETSESSIONDATA(instrSession);
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       sensor1,
                                       VI_ERROR_PARAMETER4));

    if (rsnrp_invalidViInt32Range(resolution, 1, 4))
        return VI_ERROR_INSTR_PARAMETER10;
    
    p2buf = parameterList;
    
    p2buf += sprintf(parameterList,"%Le,%Le,%Le,%Le,DEF,%ld,(@%ld)",
                     timeslotWidth,
                     numberOfTimeslots,
                     excludeStart,
                     excludeStop,
                     resolution,
                     sensor1);
                     
    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
    {
        CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                           sensor2,
                                           VI_ERROR_PARAMETER5));
        sprintf (p2buf,",(@%ld)",sensor2);
    }
    
    CHECKERR(rsnrp_sendHighLevelCommand (instrSession, 
                                       "CONF%ld:TSL:%s",
                                       measurement,
                                       measuringFunction,
                                       parameterList,
                                       VI_NULL,
                                       0));

    instrPtr->measurements[measurement].channel1 = sensor1;

    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
        instrPtr->measurements[measurement].channel2 = sensor2;

    return rsnrp_status;    
}

/*===========================================================================*/
/* Function: Configure Scope                                                 */
/* Purpose:  This function defines parameters for Scope measuring mode of the*/
/*           instrument.                                                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_configureScope (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViInt32 sensor1,ViInt32 points,
                                             ViReal64 captureTime)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    ViChar*     p2buf;
    GETSESSIONDATA(instrSession);
    
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       sensor1,
                                       VI_ERROR_PARAMETER3));

    p2buf = buffer;
    
    p2buf += sprintf(buffer,"CONF%ld:XTIM (%ld),%Le,(@%ld)",
                     measurement,
                     points,
                     captureTime,
                     sensor1);
                     
    CHECKERR(rsnrp_sendCmd (instrSession, buffer));

    instrPtr->measurements[measurement].channel1 = sensor1;

    return rsnrp_status;    
}

/*===========================================================================*/
/* Function: Configure Math                                                  */
/* Purpose:  This function selects a measurement function that processes one */
/*           sensor or two together.                                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_configureMath (ViSession instrSession,
                                            ViInt32 measurement, ViInt32 sensor1,
                                            ViInt32 operation, ViInt32 sensor2)
{
    ViStatus    rsnrp_status;
    ViInt32     measurementMode;
    GETSESSIONDATA(instrSession);
    
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       sensor1,
                                       VI_ERROR_PARAMETER3));

    if (rsnrp_invalidViInt32Range(operation,0,6))
        return VI_ERROR_PARAMETER4;
        
    CHECKERR(rsnrp_getEnumChannel(instrSession, 
                                  "SENS%ld:FUNC?",
                                  sensor1,
                                  &measurementMode,
                                  measModeArr,
                                  VI_ERROR_PARAMETER3));

    if (measurementMode == RSNRP_SENSOR_MODE_SCOPE)
        return RSNRP_ERROR_INVALID_CONFIGURATION;
    
    if (operation != RSNRP_MATH_VALUE)
    {
        CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                           sensor2,
                                           VI_ERROR_PARAMETER5));

        CHECKERR(rsnrp_getEnumChannel(instrSession, 
                                      "SENS%ld:FUNC?",
                                      sensor2,
                                      &measurementMode,
                                      measModeArr,
                                      VI_ERROR_PARAMETER3));

        if (measurementMode == RSNRP_SENSOR_MODE_SCOPE)
            return RSNRP_ERROR_INVALID_CONFIGURATION;
    }
    
    CHECKERR(rsnrp_sendMathCommand(instrSession, measurement,
                                   sensor1, operation, sensor2));

    return rsnrp_status;    
}

/*===========================================================================*/
/* Function: Configure Math Ext                                              */
/* Purpose:  This function selects a measurement function that processes one */
/*           sensor or two together.                            .            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_configureMathExt (ViSession instrSession,
                                               ViInt32 measurement,
                                               ViInt32 sensor1, ViInt32 eval1,
                                               ViInt32 gate1, ViInt32 operation,
                                               ViInt32 sensor2, ViInt32 eval2,
                                               ViInt32 gate2)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViInt32     measurementMode1,
                measurementMode2;
    ViChar      buffer[BUFFER_SIZE];
    ViChar*     p2buf;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    /* Check parameters for primary channel */
    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       sensor1,
                                       VI_ERROR_PARAMETER3));

    CHECKERR(rsnrp_getEnumChannel(instrSession, 
                                  "SENS%ld:FUNC?",
                                  sensor1,
                                  &measurementMode1,
                                  measModeArr,
                                  VI_ERROR_PARAMETER3));

    if (rsnrp_invalidViInt32Range(eval1,0,3))
        return VI_ERROR_PARAMETER4;

    if (measurementMode1 != RSNRP_SENSOR_MODE_SCOPE && gate1 != 0)
        return VI_ERROR_PARAMETER5;

    if (measurementMode1 == RSNRP_SENSOR_MODE_SCOPE &&
        eval1 != RSNRP_CALCULATE_EVALUATE_SEQUENCE &&
        rsnrp_invalidViInt32Range(gate1,1,4))
        return VI_ERROR_PARAMETER5;
    
    if (rsnrp_invalidViInt32Range(operation,0,6))
        return VI_ERROR_PARAMETER6;
    
    /* construct primary CALCx:FEED1 command */
    p2buf = buffer;
    p2buf += sprintf(buffer, "CALC%ld:FEED1 ", measurement);
    
    if (measurementMode1 == RSNRP_SENSOR_MODE_SCOPE)
    {
        switch (eval1)
        {
        case RSNRP_CALCULATE_EVALUATE_SEQUENCE:
            p2buf += sprintf(p2buf, "\"POW:TRAC\"");
            break;
        
        case RSNRP_CALCULATE_EVALUATE_AVERAGE:
            p2buf += sprintf(p2buf, "\"POW:AVER ON SWE%ld\"", gate1);
            break;
        
        case RSNRP_CALCULATE_EVALUATE_PEAK_TO_AVERAGE:
            p2buf += sprintf(p2buf, "\"POW:PTAV ON SWE%ld\"", gate1);
            break;
        
        case RSNRP_CALCULATE_EVALUATE_PEAK:
            p2buf += sprintf(p2buf, "\"POW:PEAK ON SWE%ld\"", gate1);
            break;
        }
    }
    else
    {
        p2buf += sprintf(p2buf, "\"POW:AVER\"");
    }
    
    if (operation != RSNRP_MATH_VALUE)
    {
        /* Check parameters for secondary channel */
        CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                           sensor2,
                                           VI_ERROR_PARAMETER7));

        CHECKERR(rsnrp_getEnumChannel(instrSession, 
                                      "SENS%ld:FUNC?",
                                      sensor2,
                                      &measurementMode2,
                                      measModeArr,
                                      VI_ERROR_PARAMETER7));

        if (rsnrp_invalidViInt32Range(eval2,0,3))
            return VI_ERROR_PARAMETER8;

        if (measurementMode2 != RSNRP_SENSOR_MODE_SCOPE && gate2 != 0)
            return VI_ERROR_INSTR_PARAMETER9;
        
        if (measurementMode2 == RSNRP_SENSOR_MODE_SCOPE &&
            rsnrp_invalidViInt32Range(gate2,1,4))
            return VI_ERROR_INSTR_PARAMETER9;

        /* Send CALCx:FEED1 command */
        CHECKERR(rsnrp_sendCmd(instrSession, buffer));
        
        /* construct secondary CALCx:FEED2 command */
        p2buf = buffer;
        p2buf += sprintf(buffer, "CALC%ld:FEED2 ", measurement);
    
        if (measurementMode2 == RSNRP_SENSOR_MODE_SCOPE)
        {
            switch (eval2)
            {
            case RSNRP_CALCULATE_EVALUATE_SEQUENCE:
                p2buf += sprintf(p2buf, "\"POW:TRAC\"");
                break;
        
            case RSNRP_CALCULATE_EVALUATE_AVERAGE:
                p2buf += sprintf(p2buf, "\"POW:AVER ON SWE%ld\"", gate2);
                break;
        
            case RSNRP_CALCULATE_EVALUATE_PEAK_TO_AVERAGE:
                p2buf += sprintf(p2buf, "\"POW:PTAV ON SWE%ld\"", gate2);
                break;
        
            case RSNRP_CALCULATE_EVALUATE_PEAK:
                p2buf += sprintf(p2buf, "\"POW:PEAK ON SWE%ld\"", gate2);
                break;
            }
        }
        else
        {
            p2buf += sprintf(p2buf, "\"POW:AVER\"");
        }
    }
    
    /* Send CALCx:FEED2 command */
    CHECKERR(rsnrp_sendCmd(instrSession, buffer));

    /* Send measurement command */
    CHECKERR(rsnrp_sendMathCommand(instrSession, measurement,
                                   sensor1, operation, sensor2));
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Math Primary Channel                                        */
/* Purpose:  This function returns number of channel which is used as the    */
/*           first operand for configured mathematical operation.            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_getMathPrimaryChannel (ViSession instrSession,
                                                    ViInt32 measurement,
                                                    ViInt32 *sensor1)
{
    if (!sensor1)
        return VI_ERROR_PARAMETER3;
    
    return rsnrp_getMathExpression(instrSession,
                                   measurement,
                                   sensor1,
                                   VI_NULL,
                                   VI_NULL);
}

/*===========================================================================*/
/* Function: Get Math Secondary Channel                                      */
/* Purpose:  This function returns number of channel which is used as the    */
/*           second operand for configured mathematical operation.           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_getMatchSecondaryChannel (ViSession instrSession,
                                                       ViInt32 measurement,
                                                       ViInt32 *sensor2)
{
    if (!sensor2)
        return VI_ERROR_PARAMETER3;
    
    return rsnrp_getMathExpression(instrSession,
                                   measurement,
                                   VI_NULL,
                                   VI_NULL,
                                   sensor2);
}

/*===========================================================================*/
/* Function: Get Math Operator                                               */
/* Purpose:  This function returns the number corresponding to selected      */
/*           mathematical operation.                                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_getMathOperator (ViSession instrSession,
                                              ViInt32 measurement,
                                              ViInt32 *operation)
{
    if (!operation)
        return VI_ERROR_PARAMETER3;
    
    return rsnrp_getMathExpression(instrSession,
                                   measurement,
                                   VI_NULL,
                                   operation,
                                   VI_NULL);
}

/*===========================================================================*/
/* Function: Get Math Primary Eval                                           */
/* Purpose:  This function returns which of data is processed by             */
/*           the calculate block.                                            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_getMathPrimaryEval (ViSession instrSession,
                                                 ViInt32 measurement,
                                                 ViInt32 *eval1)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViInt32     sensor1,
                measurementMode;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    if (!eval1)
        return VI_ERROR_PARAMETER3;
    
    CHECKERR(rsnrp_meas_getMathPrimaryChannel(instrSession, measurement, &sensor1));
    
    CHECKERR(rsnrp_getEnumChannel(instrSession, 
                                  "SENS%ld:FUNC?",
                                  sensor1,
                                  &measurementMode,
                                  measModeArr,
                                  RSNRP_ERROR_INSTRUMENT_ERROR));

    if (measurementMode != RSNRP_SENSOR_MODE_SCOPE)
        *eval1 = RSNRP_CALCULATE_EVALUATE_AVERAGE;
    else
    {
        CHECKERR(rsnrp_getMathExExpression(instrSession, measurement,
                                           eval1, VI_NULL,
                                           VI_NULL, VI_NULL));
    }
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Math Secondary Eval                                         */
/* Purpose:  This function returns which of data is processed by             */
/*           the calculate block.                                            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_getMathSecondaryEval (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViInt32 *eval2)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViInt32     sensor2,
                measurementMode;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    if (!eval2)
        return VI_ERROR_PARAMETER3;
    
    CHECKERR(rsnrp_meas_getMathPrimaryChannel(instrSession, measurement, &sensor2));
    
    CHECKERR(rsnrp_getEnumChannel(instrSession, 
                                  "SENS%ld:FUNC?",
                                  sensor2,
                                  &measurementMode,
                                  measModeArr,
                                  RSNRP_ERROR_INSTRUMENT_ERROR));

    if (measurementMode != RSNRP_SENSOR_MODE_SCOPE)
        *eval2 = RSNRP_CALCULATE_EVALUATE_AVERAGE;
    else
    {
        CHECKERR(rsnrp_getMathExExpression(instrSession, measurement,
                                           VI_NULL, eval2,
                                           VI_NULL, VI_NULL));
    }
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Math Primary Gate                                           */
/* Purpose:  This function returns time gate used by the calculate block.    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_getMathPrimaryGate (ViSession instrSession,
                                                 ViInt32 measurement,
                                                 ViInt32 *gate1)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViInt32     sensor1,
                measurementMode;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    if (!gate1)
        return VI_ERROR_PARAMETER3;
    
    CHECKERR(rsnrp_meas_getMathPrimaryChannel(instrSession, measurement, &sensor1));
    
    CHECKERR(rsnrp_getEnumChannel(instrSession, 
                                  "SENS%ld:FUNC?",
                                  sensor1,
                                  &measurementMode,
                                  measModeArr,
                                  RSNRP_ERROR_INSTRUMENT_ERROR));

    if (measurementMode != RSNRP_SENSOR_MODE_SCOPE)
        *gate1 = 0;
    else
    {
        CHECKERR(rsnrp_getMathExExpression(instrSession, measurement,
                                           VI_NULL, VI_NULL,
                                           gate1, VI_NULL));
    }
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Math Secondary Gate                                         */
/* Purpose:  This function returns time gate used by the calculate block.    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_getMathSecondaryGate (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViInt32 *gate2)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViInt32     sensor2,
                measurementMode;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    if (!gate2)
        return VI_ERROR_PARAMETER3;
    
    CHECKERR(rsnrp_meas_getMathPrimaryChannel(instrSession, measurement, &sensor2));
    
    CHECKERR(rsnrp_getEnumChannel(instrSession, 
                                  "SENS%ld:FUNC?",
                                  sensor2,
                                  &measurementMode,
                                  measModeArr,
                                  RSNRP_ERROR_INSTRUMENT_ERROR));

    if (measurementMode != RSNRP_SENSOR_MODE_SCOPE)
        *gate2 = 0;
    else
    {
        CHECKERR(rsnrp_getMathExExpression(instrSession, measurement,
                                           VI_NULL, VI_NULL,
                                           VI_NULL, gate2));
    }
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Configure Limits                                                */
/* Purpose:  This function configures parameters of limit checking in        */
/*           absolute power.                                                 */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_configureLimits (ViSession instrSession,
                                               ViInt32 measurement,
                                               ViBoolean beep,
                                               ViBoolean lowerCheck,
                                               ViReal64 lowerLimitPower,
                                               ViBoolean upperCheck,
                                               ViReal64 upperLimitPower)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_limit_setAutoClearEnabled(instrSession, measurement, VI_FALSE));
    CHECKERR(rsnrp_limit_clearFailCount(instrSession,measurement));
    CHECKERR(rsnrp_limit_setBeep(instrSession,measurement,beep));
    
    rsnrp_status = rsnrp_limit_setLowerEnabled(instrSession,
                                               measurement,
                                               lowerCheck);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        return VI_ERROR_PARAMETER4;

    if (lowerCheck)
    {
        rsnrp_status = rsnrp_limit_setLowerPower(instrSession,
                                                 measurement,
                                                 lowerLimitPower);
        if (rsnrp_status == VI_ERROR_PARAMETER3)
            return VI_ERROR_PARAMETER5;
    }
    
    rsnrp_status = rsnrp_limit_setUpperEnabled(instrSession,
                                               measurement,
                                               upperCheck);

    if (rsnrp_status == VI_ERROR_PARAMETER3)
        return VI_ERROR_PARAMETER6;

    if (upperCheck)
    {
        rsnrp_status = rsnrp_limit_setUpperPower(instrSession,
                                                 measurement,
                                                 upperLimitPower);
        if (rsnrp_status == VI_ERROR_PARAMETER3)
            return VI_ERROR_PARAMETER7;
    }
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Configure Limits Ratio                                          *
/* Purpose:  This function configures parameters of ratio limit checking.    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_configureLimitsRatio (ViSession instrSession,
                                                    ViInt32 measurement,
                                                    ViBoolean beep,
                                                    ViBoolean lowerCheck,
                                                    ViReal64 lowerLimitRatio,
                                                    ViBoolean upperCheck,
                                                    ViReal64 upperLimitRatio)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_limit_setAutoClearEnabled(instrSession, measurement, VI_FALSE));
    CHECKERR(rsnrp_limit_clearFailCount(instrSession,measurement));
    CHECKERR(rsnrp_limit_setBeep(instrSession,measurement,beep));
    
    rsnrp_status = rsnrp_limit_setLowerEnabled(instrSession,
                                               measurement,
                                               lowerCheck);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        return VI_ERROR_PARAMETER4;

    if (lowerCheck)
    {
        rsnrp_status = rsnrp_limit_setLowerRatio(instrSession,
                                                 measurement,
                                                 lowerLimitRatio);
        if (rsnrp_status == VI_ERROR_PARAMETER3)
            return VI_ERROR_PARAMETER5;
    }
    
    rsnrp_status = rsnrp_limit_setUpperEnabled(instrSession,
                                               measurement,
                                               upperCheck);

    if (rsnrp_status == VI_ERROR_PARAMETER3)
        return VI_ERROR_PARAMETER6;

    if (upperCheck)
    {
        rsnrp_status = rsnrp_limit_setUpperRatio(instrSession,
                                                 measurement,
                                                 upperLimitRatio);
        if (rsnrp_status == VI_ERROR_PARAMETER3)
            return VI_ERROR_PARAMETER7;
    }
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Auto Clear State                                            */
/* Purpose:  This function automatically resets the limit monitoring state   */
/*           and the internal counter for limit violations.                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_setAutoClearEnabled (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViBoolean autoClear)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    return rsnrp_setEnumChannel(instrSession, 
                                "CALC%ld:LIM:CLE:AUTO",
                                measurement,
                                autoClear,
                                switchArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Auto Clear State                                            */
/* Purpose:  This function queries the setting of auto clear.                */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_getAutoClearEnabled (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViBoolean *autoClear)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    return rsnrp_getViBooleanChannel(instrSession, 
                                "CALC%ld:LIM:CLE:AUTO?",
                                measurement,
                                autoClear,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Beep State                                                  */
/* Purpose:  This function switches the acoustic warning for limit violations*/
/*           on or off.                                                      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_setBeep (ViSession instrSession,
                                       ViInt32 measurement, ViBoolean beep)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    return rsnrp_setEnumChannel(instrSession, 
                                "CALC%ld:LIM:BEEP",
                                measurement,
                                beep,
                                switchArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Beep State                                                  */
/* Purpose:  This function reads the state of acoustic warning for limit     */
/*           violations.                                                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_getBeep (ViSession instrSession,
                                       ViInt32 measurement, ViBoolean *beepState)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    return rsnrp_getViBooleanChannel(instrSession, 
                                "CALC%ld:LIM:BEEP?",
                                measurement,
                                beepState,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Fail State                                                  */
/* Purpose:  This function queries whether upper or lower limits have been   */
/*           exceeded.                                                       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_getFail (ViSession instrSession,
                                       ViInt32 measurement,
                                       ViBoolean *limitExceeded)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    return rsnrp_getViBooleanChannel(instrSession, 
                                "CALC%ld:LIM:BEEP?",
                                measurement,
                                limitExceeded,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Fail Count                                                  */
/* Purpose:  This function returns the number of limit violations that       */
/*           occurred.                                                       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_getFailCount (ViSession instrSession,
                                            ViInt32 measurement,
                                            ViInt32 *failCount)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    return rsnrp_getViInt32Channel(instrSession, 
                                "CALC%ld:LIM:FCO?",
                                measurement,
                                failCount,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Clear Fail Count                                                */
/* Purpose:  This function resets the limit monitoring state and the internal*/
/*           counter for limit violations.                                   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_clearFailCount (ViSession instrSession,
                                              ViInt32 measurement)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    return rsnrp_sendCmdChannel(instrSession,"CALC%ld:LIMit:CLE",measurement);
}

/*===========================================================================*/
/* Function: Set Lower Power                                                 */
/* Purpose:  This function sets a lower limit for the measured values.       */
/*           The R&S NRP can respond when values are below this limit.       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_setLowerPower (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 lowerLimitPower)
{
    ViStatus    rsnrp_status;
    ViInt32     units;
    
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_meas_getUnits(instrSession, measurement, &units));

    return rsnrp_setViReal64Channel(instrSession, "CALC%ld:LIM:LOW:POW",
                                    measurement,lowerLimitPower,
                                    lowLimitArray[units], highLimitArray[units],
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Lower Power                                                 */
/* Purpose:  This function reads a lower limit for the measured values.      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_getLowerPower (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 *lowerLimit)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getViReal64Channel(instrSession, "CALC%ld:LIM:LOW:POW?",
                                    measurement,lowerLimit,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Lower Ratio                                                 */
/* Purpose:  This function sets a lower limit for the measured power ratios. */
/*           The R&S NRP can respond when the ratios drop below this limit.  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_setLowerRatio (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 lowerLimitRatio)
{
    ViStatus    rsnrp_status;
    ViInt32     units;
    
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_meas_getUnitsRatio(instrSession, measurement, &units));

    return rsnrp_setViReal64Channel(instrSession, "CALC%ld:LIM:LOW:RAT",
                                    measurement,lowerLimitRatio,
                                    lowLimitArray[units], highLimitArray[units],
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Lower Ratio                                                 */
/* Purpose:  This function reads a lower limit for the measured power ratios.*/
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_getLowerRatio (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 *lowerLimitRatio)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getViReal64Channel(instrSession, "CALC%ld:LIM:LOW:RAT?",
                                    measurement,lowerLimitRatio,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Lower Check State                                           */
/* Purpose:  This function switches the monitoring function for the lower    */
/*           limit on or off.                                                */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_setLowerEnabled (ViSession instrSession,
                                               ViInt32 measurement,
                                               ViBoolean lowerCheck)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    return rsnrp_setEnumChannel(instrSession, 
                                "CALC%ld:LIM:LOW:STAT",
                                measurement,
                                lowerCheck,
                                switchArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Lower Check State                                           */
/* Purpose:  This function reads the state of lower limit check.             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_getLowerEnabled (ViSession instrSession,
                                               ViInt32 measurement,
                                               ViBoolean *lowerCheck)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getViBooleanChannel(instrSession, 
                                "CALC%ld:LIM:LOW:STAT?",
                                measurement,
                                lowerCheck,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Upper Power                                                 */
/* Purpose:  This function sets an upper limit for the measured values.      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_setUpperPower (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 upperLimitPower)
{
    ViStatus    rsnrp_status;
    ViInt32     units;
    
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_meas_getUnits(instrSession, measurement, &units));

    return rsnrp_setViReal64Channel(instrSession, "CALC%ld:LIM:UPP:POW",
                                    measurement,upperLimitPower,
                                    lowLimitArray[units], highLimitArray[units],
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Upper Power                                                 */
/* Purpose:  This function reads an upper limit for the measured values.     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_getUpperPower (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 *upperLimit)
{
    ViStatus    rsnrp_status;
    ViInt32     units;
    
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_meas_getUnits(instrSession, measurement, &units));

    return rsnrp_getViReal64Channel(instrSession, 
                                    "CALC%ld:LIM:UPP:POW?",
                                    measurement,
                                    upperLimit,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Upper Ratio                                                 */
/* Purpose:  This function sets an upper limit for the measured power ratios */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_setUpperRatio (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 upperLimitRatio)
{
    ViStatus    rsnrp_status;
    ViInt32     units;
    
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_meas_getUnitsRatio(instrSession, measurement, &units));

    return rsnrp_setViReal64Channel(instrSession, "CALC%ld:LIM:UPP:RAT",
                                    measurement,upperLimitRatio,
                                    lowLimitArray[units], highLimitArray[units],
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Upper Ratio                                                 */
/* Purpose:  This function reads an upper limit for the measured power ratios*/
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_getUpperRatio (ViSession instrSession,
                                             ViInt32 measurement,
                                             ViReal64 *upperLimitRatio)
{
    ViStatus    rsnrp_status;
    ViInt32     units;
    
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_meas_getUnitsRatio(instrSession, measurement, &units));

    return rsnrp_getViReal64Channel(instrSession, "CALC%ld:LIM:UPP:RAT?",
                                    measurement,upperLimitRatio,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Upper Check State                                           */
/* Purpose:  This function switches the monitoring function for the upper    */
/*           limit on or off.                                                */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_setUpperEnabled (ViSession instrSession,
                                               ViInt32 measurement,
                                               ViBoolean upperCheck)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    return rsnrp_setEnumChannel(instrSession, 
                                "CALC%ld:LIM:UPP:STAT",
                                measurement,
                                upperCheck,
                                switchArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Upper Check State                                           */
/* Purpose:  This function reads the state of the monitoring function for    */
/*           the upper limit on or off.                                      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_limit_getUpperEnabled (ViSession instrSession,
                                               ViInt32 measurement,
                                               ViBoolean *upperCheck)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getViBooleanChannel(instrSession, 
                                "CALC%ld:LIM:UPP:STAT?",
                                measurement,
                                upperCheck,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Configure Power                                                 */
/* Purpose:  This function determines a value that is used as a divisor      */
/*           (logarithmic subtraction) for all measured values of the        */
/*           calculate block.                                                */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_relative_configurePower (ViSession instrSession,
                                                 ViInt32 measurement,
                                                 ViReal64 referencePower)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_relative_setRefValueState(instrSession,
                                             measurement,
                                             VI_TRUE));

    return rsnrp_relative_setRefValuePower(instrSession,
                                           measurement,
                                           referencePower);
}

/*===========================================================================*/
/* Function: Configure Ratio                                                 */
/* Purpose:  This command determines a value that is used as a divisor       */
/*           (logarithmic subtraction) for all measured values of the        */
/*           calculate block.                                                */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_relative_configureRatio (ViSession instrSession,
                                                 ViInt32 measurement,
                                                 ViReal64 referenceRatio)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_relative_setRefValueState(instrSession,
                                             measurement,
                                             VI_TRUE));

    return rsnrp_relative_setRefValueRatio(instrSession,
                                           measurement,
                                           referenceRatio);
}

/*===========================================================================*/
/* Function: Set Ref Value Power                                             */
/* Purpose:  This function determines a value that is used as a divisor      */
/*           (logarithmic subtraction) for all measured values of the        */
/*           calculate block in the relative mode.                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_relative_setRefValuePower (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViReal64 referencePower)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    if (rsnrp_invalidViReal64Range(referencePower,-150.0,210.0))
        return VI_ERROR_PARAMETER3;

    sprintf (buffer,"CALC%ld:REL:POW %Le DBM", measurement,referencePower);
    
    return rsnrp_sendCmd(instrSession, buffer);
}

/*===========================================================================*/
/* Function: Get Ref Value Power                                             */
/* Purpose:  This function reads a value that is used as a divisor           */
/*           (logarithmic subtraction) for all measured values of the        */
/*           calculate block in the relative mode.                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_relative_getRefValuePower (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViReal64 *referencePower)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    return rsnrp_getViReal64Channel(instrSession,
                                    "CALC%ld:REL:POW?",
                                    measurement,
                                    referencePower,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Ref Value Ratio                                             */
/* Purpose:  This function determines a value that is used as a divisor      */
/*           (logarithmic subtraction) for all measured values of the        */
/*           calculate block in the relative mode.                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_relative_setRefValueRatio (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViReal64 referenceRatio)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    if (rsnrp_invalidViReal64Range(referenceRatio,-200.0,200.0))
        return VI_ERROR_PARAMETER3;

    sprintf (buffer,"CALC%ld:REL:RAT %Le DB", measurement,referenceRatio);
    
    return rsnrp_sendCmd(instrSession, buffer);
}

/*===========================================================================*/
/* Function: Get Ref Value Ratio                                             */
/* Purpose:  This function reads the value that is used as a divisor         */
/*           (logarithmic subtraction) for all measured values of the        */
/*           calculate block in the relative mode.                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_relative_getRefValueRatio (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViReal64 *referencePower)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    return rsnrp_getViReal64Channel(instrSession,
                                    "CALC%ld:REL:RAT?",
                                    measurement,
                                    referencePower,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Ref Value State                                             */
/* Purpose:  This function determines whether a reference value is used.     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_relative_setRefValueState (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViBoolean relative)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
        
    return rsnrp_setEnumChannel(instrSession,
                                "CALC%ld:REL:STAT",
                                measurement,
                                relative,
                                switchArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Ref Value State                                             */
/* Purpose:  This function reads whether a reference value is used.          */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_relative_getRefValueState (ViSession instrSession,
                                                   ViInt32 measurement,
                                                   ViBoolean *reference)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
        
    return rsnrp_getViBooleanChannel(instrSession,
                                "CALC%ld:REL:STAT?",
                                measurement,
                                reference,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Reset Ref Value                                                 */
/* Purpose:  This function sets the current measured value as the reference  */
/*           value.                                                          */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_relative_resetRefValue (ViSession instrSession,
                                                ViInt32 measurement)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    return rsnrp_sendCmdChannel(instrSession, "CALC%ld:REL:AUTO ONCE", measurement);
}

/*===========================================================================*/
/* Function: Set Units                                                       */
/* Purpose:  This function selects either a linear (W) or logarithmic (dB)   */
/*           scale for the display of absolute measured values.              */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_setUnits (ViSession instrSession,
                                       ViInt32 measurement, ViInt32 units)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    if (rsnrp_invalidViInt32Range(units,0,2))
        return VI_ERROR_PARAMETER3;

    return rsnrp_setEnumChannel(instrSession, 
                                "UNIT%ld:POW",
                                measurement,
                                units,
                                unitsArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Units                                                       */
/* Purpose:  This function reads the selected units for the display of       */
/*           absolute measured values.                                       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_getUnits (ViSession instrSession,
                                       ViInt32 measurement, ViInt32 *units)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getEnumChannel(instrSession, 
                                "UNIT%ld:POW?",
                                measurement,
                                units,
                                unitsArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Units Ratio                                                 */
/* Purpose:  This function selects either a linear (%) or logarithmic (dB)   */
/*           scale for the display of relative measured values.              */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_setUnitsRatio (ViSession instrSession,
                                            ViInt32 measurement, ViInt32 units)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    if (rsnrp_invalidViInt32Range(units,3,5))
        return VI_ERROR_PARAMETER3;

    return rsnrp_setEnumChannel(instrSession, 
                                "UNIT%ld:POW:RAT",
                                measurement,
                                units,
                                unitsArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Units Ratio                                                 */
/* Purpose:  This function reads the selected units for the display of       */
/*           relative measured values.                                       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_getUnitsRatio (ViSession instrSession,
                                            ViInt32 measurement, ViInt32 *units)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getEnumChannel(instrSession, 
                                "UNIT%ld:POW:RAT?",
                                measurement,
                                units,
                                unitsArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Measure ContAv                                                  */
/* Purpose:  This function configures instrument to measure in ContAv mode   */
/*           and performs a measurement.                                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_measureContAv (ViSession instrSession,
                                            ViInt32 measurement,
                                            ViInt32 measuringFunction,
                                            ViInt32 sensor1, ViInt32 sensor2,
                                            ViInt32 resolution,
                                            ViReal64 *measuredValue)
{
    ViStatus    rsnrp_status;
    ViChar      parameterList[BUFFER_SIZE];
    ViChar*     p2buf;

    GETSESSIONDATA(instrSession);
    
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       sensor1,
                                       VI_ERROR_PARAMETER4));

    if (rsnrp_invalidViInt32Range(resolution, 1, 4))
        return VI_ERROR_PARAMETER6;
    
    p2buf = parameterList;
    
    p2buf += sprintf(parameterList,"DEF,%ld,(@%ld)",
                     resolution,sensor1);
                     
    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
    {
        CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                           sensor2,
                                           VI_ERROR_PARAMETER4));
        sprintf (p2buf,",(@%ld)",sensor2);
    }
    
    CHECKERR(rsnrp_sendHighLevelCommand (instrSession, 
                                       "MEAS%ld:%s?",
                                       measurement,
                                       measuringFunction,
                                       parameterList,
                                       measuredValue,
                                       1));
    
    instrPtr->measurements[measurement].channel1 = sensor1;

    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
        instrPtr->measurements[measurement].channel2 = sensor2;

    return rsnrp_status;    
}

/*===========================================================================*/
/* Function: Measure ContAv Buffered                                         */
/* Purpose:  This function configures instrument to measure in buffered      */
/*           ContAv mode and performs measurement.                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_measureContAvBuffered (ViSession instrSession,
                                                    ViInt32 measurement,
                                                    ViInt32 measuringFunction,
                                                    ViInt32 sensor1,
                                                    ViInt32 sensor2,
                                                    ViInt32 bufferSize,
                                                    ViInt32 resolution,
                                                    ViReal64 measuredValues[])
{
    ViStatus    rsnrp_status;
    ViChar      parameterList[BUFFER_SIZE];
    ViChar*     p2buf;
    GETSESSIONDATA(instrSession);

    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       sensor1,
                                       VI_ERROR_PARAMETER4));

    if (rsnrp_invalidViInt32Range(resolution, 1, 4))
        return VI_ERROR_PARAMETER7;
    
    p2buf = parameterList;
    
    p2buf += sprintf(parameterList,"(%ld),DEF,%ld,(@%ld)",
                     bufferSize,resolution,sensor1);
                     
    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
    {
        CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                           sensor2,
                                           VI_ERROR_PARAMETER5));
        sprintf (p2buf,",(@%ld)",sensor2);
    }
    
    CHECKERR(rsnrp_sendHighLevelCommand (instrSession, 
                                       "MEAS%ld:ARR:%s?",
                                       measurement,
                                       measuringFunction,
                                       parameterList,
                                       measuredValues,
                                       bufferSize));

    instrPtr->measurements[measurement].channel1 = sensor1;

    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
        instrPtr->measurements[measurement].channel2 = sensor2;

    return rsnrp_status;    
}

/*===========================================================================*/
/* Function: Measure Burst                                                   */
/* Purpose:  This function configures instrument to measure in BurstAv mode  */
/*           and performs a measurement.                                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_measureBurst (ViSession instrSession,
                                           ViInt32 measurement,
                                           ViInt32 measuringFunction,
                                           ViInt32 sensor1, ViInt32 sensor2,
                                           ViReal64 dropoutTolerance,
                                           ViReal64 excludeStart,
                                           ViReal64 excludeStop,
                                           ViInt32 resolution,
                                           ViReal64 *measuredValue)
{
    ViStatus    rsnrp_status;
    ViChar      parameterList[BUFFER_SIZE];
    ViChar*     p2buf;
    GETSESSIONDATA(instrSession);
    
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       sensor1,
                                       VI_ERROR_PARAMETER4));

    if (rsnrp_invalidViInt32Range(resolution, 1, 4))
        return VI_ERROR_INSTR_PARAMETER9;
    
    p2buf = parameterList;
    
    p2buf += sprintf(parameterList,"%Le,%Le,%Le,DEF,%ld,(@%ld)",
                     dropoutTolerance,
                     excludeStart,
                     excludeStop,
                     resolution,
                     sensor1);
                     
    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
    {
        CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                           sensor2,
                                           VI_ERROR_PARAMETER5));
        sprintf (p2buf,",(@%ld)",sensor2);
    }
    
    CHECKERR(rsnrp_sendHighLevelCommand (instrSession, 
                                       "MEAS%ld:BURS:%s?",
                                       measurement,
                                       measuringFunction,
                                       parameterList,
                                       measuredValue,
                                       1));

    instrPtr->measurements[measurement].channel1 = sensor1;

    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
        instrPtr->measurements[measurement].channel2 = sensor2;

    return rsnrp_status;    
}

/*===========================================================================*/
/* Function: Measure Timeslot                                                */
/* Purpose:  This function configures the parameters for Timeslot measuring  */
/*           mode of the instrument and performs a measurement.              */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_measureTimeslot (ViSession instrSession,
                                              ViInt32 measurement,
                                              ViInt32 measuringFunction,
                                              ViInt32 sensor1, ViInt32 sensor2,
                                              ViReal64 timeslotWidth,
                                              ViReal64 numberOfTimeslots,
                                              ViReal64 excludeStart,
                                              ViReal64 excludeStop,
                                              ViInt32 resolution,
                                              ViReal64 *measuredValue)
{
    ViStatus    rsnrp_status;
    ViChar      parameterList[BUFFER_SIZE];
    ViChar*     p2buf;
    GETSESSIONDATA(instrSession);
    
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       sensor1,
                                       VI_ERROR_PARAMETER4));

    if (rsnrp_invalidViInt32Range(resolution, 1, 4))
        return VI_ERROR_INSTR_PARAMETER10;
    
    p2buf = parameterList;
    
    p2buf += sprintf(parameterList,"%Le,%Le,%Le,%Le,DEF,%ld,(@%ld)",
                     timeslotWidth,
                     numberOfTimeslots,
                     excludeStart,
                     excludeStop,
                     resolution,
                     sensor1);
                     
    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
    {
        CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                           sensor2,
                                           VI_ERROR_PARAMETER5));
        sprintf (p2buf,",(@%ld)",sensor2);
    }
    
    CHECKERR(rsnrp_sendHighLevelCommand (instrSession, 
                                       "MEAS%ld:TSL:%s?",
                                       measurement,
                                       measuringFunction,
                                       parameterList,
                                       measuredValue,
                                       1));

    instrPtr->measurements[measurement].channel1 = sensor1;

    if (measuringFunction > RSNRP_MEASUREMENT_SINGLE_REL)
        instrPtr->measurements[measurement].channel2 = sensor2;

    return rsnrp_status;    
}

/*===========================================================================*/
/* Function: Measure Scope                                                   */
/* Purpose:  This function defines parameters for Scope measuring mode of the*/
/*           instrument and performs a measurement.                          */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_measureScope (ViSession instrSession,
                                           ViInt32 measurement, ViInt32 sensor1,
                                           ViInt32 points, ViReal64 captureTime,
                                           ViReal64 *measuredValue)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    ViChar*     p2buf;
    GETSESSIONDATA(instrSession);
    
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_invalidChannelRange(instrSession,
                                       sensor1,
                                       VI_ERROR_PARAMETER3));

    p2buf = buffer;
    
    p2buf += sprintf(buffer,"MEAS%ld:XTIM? (%ld),%Le,(@%ld)",
                     measurement,
                     points,
                     captureTime,
                     sensor1);
                     
    CHECKERR(rsnrp_getViReal64 (instrSession, buffer, measuredValue, VI_ERROR_PARAMETER6));

    instrPtr->measurements[measurement].channel1 = sensor1;

    return rsnrp_status;    
}

/*===========================================================================*/
/* Function: Read                                                            */
/* Purpose:  A measurement is started without further configuration and the  */
/*           measurement result is returned.                                 */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_read (ViSession instrSession,
                                   ViInt32 measurement, ViReal64 *measuredValue)
{
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    return rsnrp_getViReal64Channel(instrSession,
                                    "READ%ld?",
                                    measurement,
                                    measuredValue,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Read Array                                                      */
/* Purpose:  A measurement is started without further configuration and the  */
/*           measurement result of the buffered mode measurement is returned.*/
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_readArray (ViSession instrSession,
                                        ViInt32 measurement,
                                        ViReal64 measuredValues[])
{
    ViStatus    rsnrp_status;
    ViInt32     array_size;
    ViChar      buffer[BUFFER_SIZE];
    
    GETSESSIONDATA(instrSession);
    
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_chan_getContAvBufferSize(instrSession,
                                            instrPtr->measurements[measurement].channel1,
                                            &array_size));

    sprintf(buffer,"READ%%ld:ARR? (%ld)",array_size);
    
    CHECKERR(rsnrp_sendCmd(instrSession, "FORM REAL"));

    CHECKERR(rsnrp_getViReal64BinArrayChannel(instrSession,
                                           buffer,
                                           measurement,
                                           measuredValues,
                                           array_size,
                                           VI_ERROR_PARAMETER3));

    return rsnrp_sendCmd(instrSession, "FORM ASC");
}

/*===========================================================================*/
/* Function: Read Timeslot                                                   */
/* Purpose:  A measurement is started without further configuration and the  */
/*           measurement result of the Timeslot mode measurement is returned.*/
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_readTimeslot (ViSession instrSession,
                                           ViInt32 measurement,
                                           ViInt32 arraySize,
                                           ViReal64 measuredValues[])
{
    ViStatus    rsnrp_status;
    
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_sendCmd(instrSession, "FORM REAL"));
    
    CHECKERR(rsnrp_getViReal64BinArrayChannel(instrSession,
                                           "READ%ld?",
                                           measurement,
                                           measuredValues,
                                           arraySize,
                                           VI_ERROR_PARAMETER3));

    return rsnrp_sendCmd(instrSession, "FORM ASC");
}

/*===========================================================================*/
/* Function: Read Scope                                                      */
/* Purpose:  A measurement is started without further configuration and the  */
/*           measurement result in the Scope mode is returned.               */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_readScope (ViSession instrSession,
                                        ViInt32 measurement,
                                        ViReal64 measuredValues[])
{
    ViStatus    rsnrp_status;
    ViInt32     array_size;
    ViChar      buffer[BUFFER_SIZE];
    ViReal64    scope_time;
    
    GETSESSIONDATA(instrSession);
    
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_scope_getPoints(instrSession,
                                   instrPtr->measurements[measurement].channel1,
                                   &array_size));

    CHECKERR(rsnrp_scope_getTime(instrSession,
                                 instrPtr->measurements[measurement].channel1,
                                 &scope_time));
                                 
    sprintf(buffer,"READ%%ld:XTIM? (%ld),%Le",array_size,scope_time);
    
    CHECKERR(rsnrp_sendCmd(instrSession, "FORM REAL"));

    CHECKERR(rsnrp_getViReal64BinArrayChannel(instrSession,
                                           buffer,
                                           measurement,
                                           measuredValues,
                                           array_size,
                                           VI_ERROR_PARAMETER3));

    return rsnrp_sendCmd(instrSession, "FORM ASC");
}

/*===========================================================================*/
/* Function: Fetch                                                           */
/* Purpose:  This function returns the last valid result.                    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_fetch (ViSession instrSession,
                                    ViInt32 measurement, ViReal64 *measuredValue)
{
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    return rsnrp_getViReal64Channel(instrSession,
                                    "FETC%ld?",
                                    measurement,
                                    measuredValue,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Fetch Array                                                     */
/* Purpose:  This function returns the last valid result array (for buffered */
/*           mode only).                                                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_fetchArray (ViSession instrSession,
                                         ViInt32 measurement,
                                         ViReal64 measuredValues[])
{
    ViStatus    rsnrp_status;
    ViInt32     array_size;
    ViChar      buffer[BUFFER_SIZE];
    
    GETSESSIONDATA(instrSession);
    
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_chan_getContAvBufferSize(instrSession,
                                            instrPtr->measurements[measurement].channel1,
                                            &array_size));
    sprintf(buffer,"FETC%%ld:ARR? (%ld)",array_size);

    CHECKERR(rsnrp_sendCmd(instrSession, "FORM REAL"));
    
    CHECKERR(rsnrp_getViReal64BinArrayChannel(instrSession,
                                           buffer,
                                           measurement,
                                           measuredValues,
                                           array_size,
                                           VI_ERROR_PARAMETER3));

    return rsnrp_sendCmd(instrSession, "FORM ASC");
}

/*===========================================================================*/
/* Function: Fetch Timeslot                                                  */
/* Purpose:  This function returns the last valid result in Timesot measuring*/
/*           mode.                                                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_fetchTimeslot (ViSession instrSession,
                                            ViInt32 measurement,
                                            ViInt32 arraySize,
                                            ViReal64 measuredValues[])
{
    ViStatus    rsnrp_status;
    
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_sendCmd(instrSession, "FORM REAL"));
    
    CHECKERR(rsnrp_getViReal64BinArrayChannel(instrSession,
                                           "FETC%ld?",
                                           measurement,
                                           measuredValues,
                                           arraySize,
                                           VI_ERROR_PARAMETER3));

    return rsnrp_sendCmd(instrSession, "FORM ASC");
}

/*===========================================================================*/
/* Function: Fetch Scope                                                     */
/* Purpose:  This function returns the last valid result in Scope measuring  */
/*           mode.                                                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_fetchScope (ViSession instrSession,
                                         ViInt32 measurement,
                                         ViReal64 measuredValues[])
{
    ViStatus    rsnrp_status;
    ViInt32     array_size;
    ViChar      buffer[BUFFER_SIZE];
    ViReal64    scope_time;
    
    GETSESSIONDATA(instrSession);
    
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_scope_getPoints(instrSession,
                                   instrPtr->measurements[measurement].channel1,
                                   &array_size));

    CHECKERR(rsnrp_scope_getTime(instrSession,
                                 instrPtr->measurements[measurement].channel1,
                                 &scope_time));
                                 
    sprintf(buffer,"FETC%%ld:XTIM? (%ld),%Le",array_size,scope_time);
    
    CHECKERR(rsnrp_sendCmd(instrSession, "FORM REAL"));

    CHECKERR(rsnrp_getViReal64BinArrayChannel(instrSession,
                                           buffer,
                                           measurement,
                                           measuredValues,
                                           array_size,
                                           VI_ERROR_PARAMETER3));

    return rsnrp_sendCmd(instrSession, "FORM ASC");
}

/*===========================================================================*/
/* Function: Fetch Secondary Value                                           */
/* Purpose:  This function queries one of the secondary value of the         */
/*           selected measurement block.                                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_fetchSecondaryValue (ViSession instrSession,
                                                  ViInt32 measurement,
                                                  ViInt32 valueType,
                                                  ViReal64 *secondaryValue)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    ViChar      rd_buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    if (rsnrp_invalidViInt32Range(valueType,0,2))
        return VI_ERROR_PARAMETER3;
    if (!secondaryValue)
        return VI_ERROR_PARAMETER4;

    sprintf(buffer,"CALC%ld:%s:DATA?", measurement, typeArr[valueType]);
    
    CHECKERR(rsnrp_getViString(instrSession,
                             buffer,
                             rd_buffer,
                             BUFFER_SIZE,
                             0));
    if (strstr(rd_buffer,"Invalid"))
        return RSNRP_ERROR_SECONDARY_VALUE_INVALID;
    
    *secondaryValue = atof(rd_buffer);
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Reset Extremes                                                  */
/* Purpose:  The R&S NRP stores the maximum and minimum values of each       */
/*           measurement for each calculate block. The current measured value*/
/*           is stored with call of this function.                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_meas_resetExtremes (ViSession instrSession,
                                            ViInt32 measurement)
{
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    return rsnrp_sendCmdChannel(instrSession,
                                "CALC%ld:EXTR:RES",
                                measurement);
                                
}

/*===========================================================================*/
/* Function: Set Reference Oscillator State                                  */
/* Purpose:  This function switches the optional RF generator (ON) or (OFF). */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_rfOsc_setEnabled (ViSession instrSession,
                                          ViBoolean referenceOscillator)
{
    ViStatus    rsnrp_status;
    
    GETSESSIONDATA(instrSession);
    CHECKOPT(RSNRP_OPT_B1);
    
    return rsnrp_setEnum(instrSession,
                         "OUTP:ROSC",
                         referenceOscillator,
                         switchArr,
                         VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Get Reference Oscillator State                                  */
/* Purpose:  This function reads the state of optional RF generator ON or OFF*/
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_rfOsc_getEnabled (ViSession instrSession,
                                          ViBoolean *referenceOscillator)
{
    ViStatus    rsnrp_status;
    
    GETSESSIONDATA(instrSession);
    CHECKOPT(RSNRP_OPT_B1);
    
    return rsnrp_getViBoolean(instrSession,
                         "OUTP:ROSC?",
                         referenceOscillator,
                         VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Count                                                           */
/* Purpose:  This function returns the number of available analog outputs.   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_outputs_getCount (ViSession instrSession,
                                          ViInt32 *analogOutputs)
{
    if (analogOutputs == VI_NULL)
        return VI_ERROR_PARAMETER2;

    *analogOutputs = MAX_OUTPUTS;
    return VI_SUCCESS;
}

/*===========================================================================*/
/* Function: Set Mode                                                        */
/* Purpose:  This function configures parameters of specified output.        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_setMode (ViSession instrSession,
                                        ViInt32 outputNumber,
                                        ViBoolean outputState,
                                        ViInt32 outputType, ViInt32 source)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    
    rsnrp_status = rsnrp_output_setState (instrSession,
                                    outputNumber,
                                    outputType,
                                    outputState);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        rsnrp_status = VI_ERROR_PARAMETER4;
    if (rsnrp_status == VI_ERROR_PARAMETER4)
        rsnrp_status = VI_ERROR_PARAMETER3;
    if (rsnrp_status != VI_SUCCESS)
        return rsnrp_status;

    rsnrp_status = rsnrp_output_setSource(instrSession,
                                   outputNumber,
                                   outputType,
                                   source);

    if (rsnrp_status == VI_ERROR_PARAMETER4)
        rsnrp_status = VI_ERROR_PARAMETER5;

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Mode                                                        */
/* Purpose:  This function reads the settings of selected output. TTL values */
/*           are valid only if output 1 is selected.                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_getMode (ViSession instrSession,
                                        ViInt32 outputNumber,
                                        ViBoolean *analogState,
                                        ViBoolean *TTLState,
                                        ViInt32 *analogSource,
                                        ViInt32 *TTLSource)
{
    ViStatus    rsnrp_status;
    
    rsnrp_status = rsnrp_output_getState (instrSession, outputNumber,
                                     RSNRP_OUTPUT_ANALOG, analogState);
    if (rsnrp_status == VI_ERROR_PARAMETER4)
        rsnrp_status = VI_ERROR_PARAMETER3;
    
    if (rsnrp_status != VI_SUCCESS)
        return rsnrp_status;

    rsnrp_status = rsnrp_output_getSource (instrSession, outputNumber,
                                     RSNRP_OUTPUT_ANALOG, analogSource);
    if (rsnrp_status == VI_ERROR_PARAMETER4)
        rsnrp_status = VI_ERROR_PARAMETER5;
    
    if (rsnrp_status != VI_SUCCESS)
        return rsnrp_status;

    if (outputNumber == 1)
    {
        CHECKERR(rsnrp_output_getState(instrSession,
                                       outputNumber,
                                       RSNRP_OUTPUT_TTL,
                                       TTLState));
    
        rsnrp_status = rsnrp_output_getSource (instrSession, outputNumber,
                                         RSNRP_OUTPUT_TTL, TTLSource);
        if (rsnrp_status == VI_ERROR_PARAMETER4)
            rsnrp_status = VI_ERROR_PARAMETER6;
    }

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Source                                                      */
/* Purpose:  This function sets the source of specified output.              */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_setSource (ViSession instrSession,
                                          ViInt32 outputNumber,
                                          ViInt32 outputType, ViInt32 source)
{
    ViChar      buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(outputNumber,1,MAX_OUTPUTS))
        return VI_ERROR_PARAMETER2;
    if (rsnrp_invalidViInt32Range(outputType,0,1))
        return VI_ERROR_PARAMETER3;
    if (outputType && (outputNumber != 1))
        return VI_ERROR_PARAMETER2;
    if (rsnrp_invalidViInt32Range(source,1,4))
        return VI_ERROR_PARAMETER4;

    sprintf(buffer, "OUTP:%s%ld:FEED \"CALC%ld%s\"", outpTypeArr[outputType],
                                                   outputNumber,
                                                   source,
                                                   outputType ? ":LIM" : "");
    return rsnrp_sendCmd(instrSession,buffer);
}

/*===========================================================================*/
/* Function: Get Source                                                      */
/* Purpose:  This function reads the source of specified output.             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_getSource (ViSession instrSession,
                                          ViInt32 outputNumber,
                                          ViInt32 outputType, ViInt32 *source)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    ViChar      rd_buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(outputNumber,1,MAX_OUTPUTS))
        return VI_ERROR_PARAMETER2;
    if (rsnrp_invalidViInt32Range(outputType,0,1))
        return VI_ERROR_PARAMETER3;
    if (outputType && (outputNumber != 1))
        return VI_ERROR_PARAMETER2;
    if (!source)
        return VI_ERROR_PARAMETER4;
    
    sprintf(buffer,"OUTP:%s%ld:FEED?",outpTypeArr[outputType],outputNumber);
    
    CHECKERR(rsnrp_getViString(instrSession,
                               buffer,
                               rd_buffer,
                               BUFFER_SIZE,
                               0));
    if (!sscanf(rd_buffer,"\"CALC%ld\"",source))
    {
        *source = 0;
        return VI_ERROR_INSTR_INTERPRETING_RESPONSE;
    }

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set State                                                       */
/* Purpose:  This function turns the specified output on or off.             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_setState (ViSession instrSession,
                                         ViInt32 outputNumber,
                                         ViInt32 outputType,
                                         ViBoolean outputState)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(outputNumber,1,MAX_OUTPUTS))
        return VI_ERROR_PARAMETER2;
    if (rsnrp_invalidViInt32Range(outputType,0,1))
        return VI_ERROR_PARAMETER3;
    if (outputType && (outputNumber != 1))
        return VI_ERROR_PARAMETER2;
    
    sprintf(buffer,"OUTP:%s%ld:STAT",outpTypeArr[outputType],outputNumber);
    return rsnrp_setEnum(instrSession,
                         buffer,
                         outputState,
                         switchArr,
                         VI_ERROR_PARAMETER4);
}

/*===========================================================================*/
/* Function: Get State                                                       */
/* Purpose:  This function reads the state of specified output.              */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_getState (ViSession instrSession,
                                         ViInt32 outputNumber,
                                         ViInt32 outputType,
                                         ViBoolean *outputState)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(outputNumber,1,MAX_OUTPUTS))
        return VI_ERROR_PARAMETER2;
    if (rsnrp_invalidViInt32Range(outputType,0,1))
        return VI_ERROR_PARAMETER3;
    if (outputType && (outputNumber != 1))
        return VI_ERROR_PARAMETER2;
    
    sprintf(buffer,"OUTP:%s%ld:STAT?",outpTypeArr[outputType], outputNumber);
    return rsnrp_getViBoolean(instrSession,
                              buffer,
                              outputState,
                              VI_ERROR_PARAMETER4);
}

/*===========================================================================*/
/* Function: Set Lower Power Equivalent                                      */
/* Purpose:  This function specifies the lower power limit of the            */
/*           characteristic for one of the two analog outputs if the         */
/*           associated calculate block returns a power as measured value.   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_setLowerPowerEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 lowerPower)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(outputNumber,1,MAX_OUTPUTS))
        return VI_ERROR_PARAMETER2;

    if (rsnrp_invalidViReal64Range(lowerPower,-120.0, 90.0))
        return VI_ERROR_PARAMETER3;
    
    sprintf(buffer,"OUTP:RECorder%ld:LIM:LOW:POW %Le DBM",outputNumber,
                   lowerPower);

    return rsnrp_sendCmd(instrSession, buffer);
}

/*===========================================================================*/
/* Function: Get Lower Power Equivalent                                      */
/* Purpose:  This function reads the lower power limit of the characteristic */
/*           for one of the two analog outputs if the associated calculate   */
/*           block returns a power as measured value.                        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_getLowerPowerEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 *lowerPower)
{
    if (rsnrp_invalidViInt32Range(outputNumber,1,MAX_OUTPUTS))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getViReal64Channel(instrSession,
                                    "OUTP:REC%ld:LIM:LOW:POW?",
                                    outputNumber,
                                    lowerPower,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Lower Ratio Equivalent                                      */
/* Purpose:  This function specifies the lower power limit of the            */
/*           characteristic for one of the two analog outputs if the         */
/*           associated calculate block returns a power ratio (unit DB) as   */
/*           the measured value.                                             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_setLowerRatioEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 lowerRatio)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(outputNumber,1,MAX_OUTPUTS))
        return VI_ERROR_PARAMETER2;

    if (rsnrp_invalidViReal64Range(lowerRatio,-200.0, 200.0))
        return VI_ERROR_PARAMETER3;
    
    sprintf(buffer,"OUTP:RECorder%ld:LIM:LOW:RAT %Le DB",outputNumber,
                   lowerRatio);

    return rsnrp_sendCmd(instrSession, buffer);
}

/*===========================================================================*/
/* Function: Get Lower Ratio Equivalent                                      */
/* Purpose:  This function reads the lower power ratio limit of the          */
/*           characteristic for one of the two analog outputs                */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_getLowerRatioEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 *lowerRatio)
{
    if (rsnrp_invalidViInt32Range(outputNumber,1,MAX_OUTPUTS))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getViReal64Channel(instrSession,
                                    "OUTP:REC%ld:LIM:LOW:RAT?",
                                    outputNumber,
                                    lowerRatio,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Upper Power Equivalent                                      */
/* Purpose:  This function specifies the upper power limit of the            */
/*           characteristic for one of the two analog outputs if the         */
/*           associated calculate block returns a power as measured value.   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_setUpperPowerEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 upperPower)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(outputNumber,1,MAX_OUTPUTS))
        return VI_ERROR_PARAMETER2;

    if (rsnrp_invalidViReal64Range(upperPower,-120.0, 90.0))
        return VI_ERROR_PARAMETER3;
    
    sprintf(buffer,"OUTP:RECorder%ld:LIM:UPP:POW %Le DBM",outputNumber,
                   upperPower);

    return rsnrp_sendCmd(instrSession, buffer);
}

/*===========================================================================*/
/* Function: Get Upper Power Equivalent                                      */
/* Purpose:  This function reads the upper power limit of the characteristic */
/*           for one of the two analog outputs if the associated calculate   */
/*           block returns a power as measured value.                        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_getUpperPowerEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 *upperPower)
{
    if (rsnrp_invalidViInt32Range(outputNumber,1,MAX_OUTPUTS))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getViReal64Channel(instrSession,
                                    "OUTP:REC%ld:LIM:UPP:POW?",
                                    outputNumber,
                                    upperPower,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Upper Ratio Equivalent                                      */
/* Purpose:  This function specifies the upper power limit of the            */
/*           characteristic for one of the two analog outputs if the         */
/*           associated calculate block returns a power ratio (unit DB) as   */
/*           the measured value.                                             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_setUpperRatioEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 upperRatio)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(outputNumber,1,MAX_OUTPUTS))
        return VI_ERROR_PARAMETER2;

    if (rsnrp_invalidViReal64Range(upperRatio,-200.0, 200.0))
        return VI_ERROR_PARAMETER3;
    
    sprintf(buffer,"OUTP:RECorder%ld:LIM:UPP:RAT %Le DB",outputNumber,
                   upperRatio);

    return rsnrp_sendCmd(instrSession, buffer);
}

/*===========================================================================*/
/* Function: Get Upper Ratio Equivalent                                      */
/* Purpose:  This function reads the upper power ratio limit of the          */
/*           characteristic for one of the two analog outputs                */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_getUpperRatioEquivalent (ViSession instrSession,
                                                        ViInt32 outputNumber,
                                                        ViReal64 *upperRatio)
{
    if (rsnrp_invalidViInt32Range(outputNumber,1,MAX_OUTPUTS))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getViReal64Channel(instrSession,
                                    "OUTP:REC%ld:LIM:UPP:RAT?",
                                    outputNumber,
                                    upperRatio,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Pass Voltage                                                */
/* Purpose:  This function sets the high voltage for the TTL output.         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_setPassVoltage (ViSession instrSession,
                                               ViReal64 passVoltage)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_sendCmd(instrSession,"OUTP:TTL:ACT HIGH"));
    
    return rsnrp_setViReal64(instrSession,
                             "OUTP:TTL:HVOL",
                             passVoltage,
                             0.0, 3.3,
                             VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Get Pass Voltage                                                */
/* Purpose:  This function reads the high voltage for the TTL output.        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_getPassVoltage (ViSession instrSession,
                                               ViReal64 *passVoltage)
{
    return rsnrp_getViReal64(instrSession,
                             "OUTP:TTL:HVOL?",
                             passVoltage,
                             VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Set Fail Voltage                                                */
/* Purpose:  This function sets the low voltage for the TTL output.          */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_setFailVoltage (ViSession instrSession,
                                               ViReal64 failVoltage)
{
    return rsnrp_setViReal64(instrSession,
                             "OUTP:TTL:LVOL",
                             failVoltage,
                             0.0, 3.3,
                             VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Get Fail Voltage                                                */
/* Purpose:  This function reads the low voltage for the TTL output.         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_output_getFailVoltage (ViSession instrSession,
                                               ViReal64 *failVoltage)
{
    return rsnrp_getViReal64(instrSession,
                             "OUTP:TTL:LVOL?",
                             failVoltage,
                             VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Count                                                           */
/* Purpose:  This function returns the number of storable device states.     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_states_getCount (ViSession instrSession,
                                         ViInt32 *count)
{
    return rsnrp_getViInt32(instrSession,"MEM:NST?",count,VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Define Name                                                     */
/* Purpose:  This function assigns name to memory location of the device     */
/*           states.                                                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_state_defineName (ViSession instrSession,
                                          ViString setupName,
                                          ViInt32 setupNumber)
{
    ViChar      buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(setupNumber, 1, 19))
        return VI_ERROR_PARAMETER3;
    
    sprintf(buffer, "MEM:STAT:DEF '%s',%ld",setupName, setupNumber);
    
    return rsnrp_sendCmd(instrSession,
                         buffer);
}

/*===========================================================================*/
/* Function: Read Name                                                       */
/* Purpose:  This function reads the name of setup stored on indicated       */
/*           position.                                                       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_state_readName (ViSession instrSession,
                                        ViInt32 setupNumber, ViChar _VI_FAR name[])
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    ViChar*     p2buf;
    ViInt32     i = 0;

    if (rsnrp_invalidViInt32Range(setupNumber,0,19))
        return VI_ERROR_PARAMETER2;

    if (!name)
        return VI_ERROR_PARAMETER3;

    CHECKERR(rsnrp_getViString(instrSession,"MEM:STAT:CAT?",buffer,BUFFER_SIZE,
                               VI_ERROR_PARAMETER3));
    
#if 0    
    /* move pointer to beginning of the data */
    p2buf = strstr(buffer,",\"");
    if (!p2buf)
        return RSNRP_ERROR_TABLE_NOT_FOUND;
    /* move pointer behind the ," */
    p2buf+=2;
    i = 0;
    
    while(p2buf && (setupNumber > i))
    {
        p2buf = strstr(p2buf,"\",\"");
        if (p2buf)
        {
            p2buf+=3; /* move pointer behind the "," */
            i++;
        }
    }
#else
    p2buf = strtok(buffer, ",");
    i = 0;
    
    while (p2buf && (setupNumber > i))
    {
        p2buf = strtok(VI_NULL, ",");
        i++;
    }
#endif

    if (p2buf)
#if 0
        sscanf(p2buf,"%[^,]",name);
#else
        sscanf(p2buf,"\"%[^\"]", name);
#endif
    else
        rsnrp_status = RSNRP_ERROR_TABLE_NOT_FOUND;
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Recall                                                          */
/* Purpose:  This function calls the device state which has been stored with */
/*           the *SAV command under the indicated number.                    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_state_recall (ViSession instrSession,
                                      ViInt32 setupNumber)
{
    return rsnrp_setViInt32(instrSession,
                            "*RCL",
                            setupNumber,
                            0, 19,
                            VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Save                                                            */
/* Purpose:  This function stores the current device state under the         */
/*           indicated number.                                               */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_state_save (ViSession instrSession,
                                    ViInt32 setupNumber)
{
    return rsnrp_setViInt32(instrSession,
                            "*SAV",
                            setupNumber,
                            1, 19,
                            VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Get Pixel Map                                                   */
/* Purpose:  This function reads the display content as binary block data.   */
/*           The content is sent line by line with one bit corresponding     */
/*           exactly to one pixel.                                           */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_display_getPIXMap (ViSession instrSession,
                                           ViChar _VI_FAR pixelMapData[])
{
    ViStatus    rsnrp_status;
    ViByte      buffer[9610];
    ViByte*     p2buf;
    ViByte*     p2dat;
    ViBoolean   term_char_old, old_check;
    ViInt32     read, i;
    ViByte      helper_buffer[40];
    ViByte bmp_header[] ={
        0x42, 0x4d, 0xbe, 0x25, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x28, 0x00,
        0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xf0, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x80, 0x25, 0x00, 0x00, 0xc4, 0x0e,
        0x00, 0x00, 0xc4, 0x0e, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
        0xff, 0x00, 0x00, 0x00, 0x00, 0x00
        };

    GETSESSIONDATA(instrSession);
    
    if (!pixelMapData)
        return VI_ERROR_PARAMETER2;

    CHECKERR(viGetAttribute (instrSession, VI_ATTR_TERMCHAR_EN, &term_char_old));
    CHECKERR(viSetAttribute (instrSession, VI_ATTR_TERMCHAR_EN, VI_FALSE));

    old_check = instrPtr->errorChecking;
    CHECKERR(rsnrp_errorCheckState(instrSession,VI_FALSE));
    CHECKERR(rsnrp_sendCmd(instrSession, "DISP:PIXM?"));
    CHECKERR(rsnrp_readInstrData (instrSession, 9606, (ViChar*)buffer, &read));
    if (read != 9606)
        return VI_ERROR_INSTR_INTERPRETING_RESPONSE;
    
    /* clear the buffer */
    CHECKERR(viScanf(instrSession,"%*[^\r\n]"));
    
    /* set back all attributes */
    CHECKERR(viSetAttribute (instrSession, VI_ATTR_TERMCHAR_EN, term_char_old));
    CHECKERR(rsnrp_errorCheckState(instrSession,old_check));
    if (old_check)
        CHECKERR(rsnrp_checkStatus(instrSession));
    
    p2dat = (ViPByte)pixelMapData;
    p2buf = (ViPByte)(buffer + 6);
    /* reverse the data to show correctly as bmp */
    for (i = 0; i < 120; i++)
    {
        memcpy(helper_buffer, p2buf+(i*40),40);
        memcpy(p2buf+(i*40),p2buf+9600-((i+1)*40),40);
        memcpy(p2buf+9600-((i+1)*40),helper_buffer,40);
    }
    
    memcpy(p2dat, bmp_header, 62);
    memcpy(p2dat+62,buffer+6,9600);

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Hide Message                                                    */
/* Purpose:  This function hides the message on display.                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_display_hideMessage (ViSession instrSession)
{
    return rsnrp_sendCmd(instrSession,"DISP:MESS OFF");
}

/*===========================================================================*/
/* Function: Hide Query                                                      */
/* Purpose:  This function hides the query message on display.               */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_display_hideQuery (ViSession instrSession)
{
    ViStatus    rsnrp_status;
    ViInt32     dummy;
    
    GETSESSIONDATA(instrSession);
    
    CHECKERR(rsnrp_sendCmd(instrSession,"DISP:MESS OFF"));
    
    if (!instrPtr->nonBlockingQuery)
    {
        /* restore ese */
        CHECKERR(rsnrp_setViInt32(instrSession,
                                    "*ESE",
                                    instrPtr->temporaryEse,
                                    0,255,
                                    0));
        /* clear ESR, throw awat its value */
        CHECKERR(rsnrp_getViInt32(instrSession,"*ESR?",&dummy,0));
        /* restore original esr value */
        instrPtr->events.esr = instrPtr->temporaryEsr;

        /* zero temporary variables */
        instrPtr->temporaryEsr = instrPtr->temporaryEse = instrPtr->nonBlockingQuery = 0;
    }
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Illumination State                                          */
/* Purpose:  This function switches the display (actually the backlighting)  */
/*           On or Off. Backlighting is switched on again in case of a reset.*/
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_display_setIlluminationEnabled (ViSession instrSession,
                                                        ViBoolean illumination)
{
    return rsnrp_setEnum(instrSession,"DISP:ILL", illumination,
                         switchArr, VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Get Illumination State                                          */
/* Purpose:  This function reads the state of display backlighting.          */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_display_getIlluminationEnabled (ViSession instrSession,
                                                        ViBoolean *illumination)
{
    return rsnrp_getViBoolean(instrSession,"DISP:ILL?", illumination,
                              VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Query Finished                                                  */
/* Purpose:  This function checks whether the query on instrument's display  */
/*           has been dismissed.                                             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_display_isQueryDone (ViSession instrSession,
                                             ViBoolean *queryDone)
{
    ViStatus    rsnrp_status;
    ViUInt16    stb;

    GETSESSIONDATA(instrSession);
    
    CHECKERR(viReadSTB(instrSession,&stb));
    if (((stb & 0x20) != 0)  && instrPtr->nonBlockingQuery)
    {
        *queryDone = VI_TRUE;
        /* discard the values in ESR */
        CHECKERR(rsnrp_getViBoolean(instrSession, "*ESR?", &stb, 0));
        /* restore ESR and ESE */
        CHECKERR(rsnrp_setViInt32(instrSession,"*ESE",instrPtr->temporaryEse,
                                  0,255,0));
        instrPtr->events.esr = instrPtr->temporaryEsr;
        
        /* zero temporary variables */
        instrPtr->temporaryEsr = instrPtr->temporaryEse = instrPtr->nonBlockingQuery = 0;
    }
    else
        *queryDone = VI_FALSE;
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Show Message                                                    */
/* Purpose:  This function shows text messages on instrument's display.      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_display_showMessage (ViSession instrSession,
                                             ViString message)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    
    sprintf(buffer,"'%s'",message);
    CHECKERR(rsnrp_sendCmd(instrSession, "DISP:MESS:TYPE MESS"));
    CHECKERR(rsnrp_setViString(instrSession, "DISP:MESS:TEXT",
                               buffer, VI_ERROR_PARAMETER2));
    return rsnrp_sendCmd(instrSession, "DISP:MESS ON");
}

/*===========================================================================*/
/* Function: Show Query                                                      */
/* Purpose:  This function shows text messages on instrument's display which */
/*           can be dismissed by pressing the Enter button on the instrument.*/
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_display_showQuery (ViSession instrSession,
                                           ViString message, ViBoolean blocking,
                                           ViUInt32 timeout)
{
    ViStatus    rsnrp_status;
    ViUInt16    ese, stb;
    ViChar      buffer[BUFFER_SIZE];
    ViBoolean   old_check;
    
    GETSESSIONDATA(instrSession);
    
    sprintf(buffer,"'%s'",message);
    
    if (rsnrp_invalidViBooleanRange(blocking))
        return VI_ERROR_PARAMETER3;

    instrPtr->nonBlockingQuery = !blocking;

    /* update the ESR value */
    CHECKERR(rsnrp_updateCache(instrSession,RSNRP_STATCLASS_ESR,0,0));

    CHECKERR(rsnrp_getViBoolean(instrSession,
                                "*ESE?",
                                &ese,
                                0));
    if (!blocking)
    {
        /* save the value of ESR */
        instrPtr->temporaryEsr = (ViUInt32)instrPtr->events.esr;
        /* save the value of ESE */
        instrPtr->temporaryEse = (ViUInt32)ese;
    }
    
    /* set ESE to generate only OPC event */
    CHECKERR(rsnrp_sendCmd(instrSession, "*ESE 1"));

    CHECKERR(rsnrp_sendCmd(instrSession, "DISP:MESS:TYPE QUER"));
    CHECKERR(rsnrp_setViString(instrSession, "DISP:MESS:TEXT",
                               buffer, VI_ERROR_PARAMETER2));

    if (blocking)
        CHECKERR(viEnableEvent (instrSession, VI_EVENT_SERVICE_REQ, VI_QUEUE, VI_NULL));

    old_check = instrPtr->errorChecking;
    CHECKERR(rsnrp_errorCheckState(instrSession,VI_FALSE));
    CHECKERR(rsnrp_sendCmd(instrSession, "DISP:MESS ON;*OPC"));
    CHECKERR(rsnrp_errorCheckState(instrSession, old_check));

    if (blocking)
    {
        /* wait for OPC */
        CHECKERR(viWaitOnEvent (instrSession, VI_EVENT_SERVICE_REQ, timeout, VI_NULL, VI_NULL));
        /* if the wait was successfull, clear the STB */
        CHECKERR(viReadSTB (instrSession, &stb));
        CHECKERR(viDisableEvent (instrSession, VI_EVENT_SERVICE_REQ, VI_ALL_MECH));
        CHECKERR(viDiscardEvents (instrSession, VI_ALL_ENABLED_EVENTS, VI_ALL_MECH));
        /* restore ese */
        CHECKERR(rsnrp_setViInt32(instrSession,
                                    "*ESE",
                                    ese,
                                    0,255,
                                    0));
        /* clear ESR, throw away its value */
        CHECKERR(rsnrp_getViBoolean(instrSession,"*ESR?",&ese,0));
        /* restore original esr value */
        instrPtr->events.esr = instrPtr->temporaryEsr;
        /* zero temporary variables */
        instrPtr->temporaryEsr = instrPtr->temporaryEse = 0;
    }

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Display Update                                              */
/* Purpose:  This function adjust the update frequency of the measured       */
/*           values in the display.                                          */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_display_setUpdate (ViSession instrSession,
                                           ViInt32 update)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    CHECKERR(rsnrp_setEnum(instrSession,
                           "DISP:UPD",
                           update,
                           displayUpdateArr,
                           VI_ERROR_PARAMETER2));
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Display Update                                              */
/* Purpose:  This function returns the update frequency of the measured      */
/*           values in the display.                                          */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_display_getUpdate (ViSession instrSession,
                                           ViInt32 *update)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    CHECKERR(rsnrp_getEnum(instrSession,
                           "DISP:UPD?",
                           update,
                           displayUpdateArr,
                           VI_ERROR_PARAMETER2));
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Display Show Error List                                     */
/* Purpose:  This function displays or removes a windows which contains the  */
/*           contents of the SCPI error queue.                               */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_display_setShowErrorlist (ViSession instrSession,
                                                  ViBoolean state)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    CHECKERR(rsnrp_setEnum(instrSession,
                           "DISP:ERR",
                           state,
                           switchArr,
                           VI_ERROR_PARAMETER2));
                           
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Display Show Error List                                     */
/* Purpose:  This function returns state of a windows which contains the     */
/*           contents of the SCPI error queue.                               */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_display_getShowErrorlist (ViSession instrSession,
                                                  ViBoolean *state)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    CHECKERR( rsnrp_getViBoolean(instrSession,
                                 "DISP:ERR?",
                                 state,
                                 VI_ERROR_PARAMETER2));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Battery Auto State                                          */
/* Purpose:                                                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_service_setBatteryAuto (ViSession instrSession,
                                                ViBoolean autoState)
{
    return rsnrp_setEnum(instrSession,
                         "SERV:BATT:AUTO",
                         autoState,
                         switchArr,
                         VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Get Battery Auto State                                          */
/* Purpose:                                                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_service_getBatteryAuto (ViSession instrSession,
                                                ViBoolean *autoState)
{
    return rsnrp_getViBoolean(instrSession,
                         "SERV:BATT:AUTO?",
                         autoState,
                         VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Get Detector Temperature                                        */
/* Purpose:  This function initiates a temperature measurement of the sensor */
/*           and reads the temperature value from the instrument.            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_service_getDetectorTemperature (ViSession instrSession,
                                                        ViInt32 channel,
                                                        ViReal64 *temperature)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));

    CHECKERR(rsnrp_sendCmdChannel(instrSession,"SERV%ld:UNL 1234",channel));
    CHECKERR(rsnrp_sendCmdChannel(instrSession,"SERV%ld:CAL:TEMP ONCE",channel));
    return rsnrp_getViReal64Channel(instrSession,"SERV%ld:CAL:TEMP:DATA?",
                                    channel,
                                    temperature,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Read SMB Word                                                   */
/* Purpose:                                                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_service_SMBReadWord (ViSession instrSession,
                                             ViInt32 i2CAddress,
                                             ViInt32 SMBusCommand,
                                             ViUInt16 *word)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    ViChar      rd_buffer[7];
    ViInt32     result;
    
    if (!word)
        return VI_ERROR_PARAMETER4;

    CHECKERR(rsnrp_sendCmd(instrSession,"SERV:UNL 1234"));
    sprintf(buffer,"SERV:SMB:RWOR? %ld,%ld",i2CAddress, SMBusCommand);
    
    CHECKERR(rsnrp_getViString(instrSession, 
                               buffer,
                               rd_buffer,
                               7,
                               0));
    sscanf(rd_buffer,"#H%X",&result);
    
    *word = (ViUInt16) result;
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Read SMB String                                                 */
/* Purpose:                                                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_service_SMBReadString (ViSession instrSession,
                                               ViInt32 i2CAddress,
                                               ViInt32 SMBusCommand,
                                               ViChar _VI_FAR string[])
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    
    CHECKERR(rsnrp_sendCmd(instrSession,"SERV:UNL 1234"));
    sprintf(buffer,"SERV:SMB:RBL? %ld,%ld",i2CAddress, SMBusCommand);
    
    return rsnrp_getViString(instrSession,
                             buffer,
                             string,
                             BUFFER_SIZE,
                             VI_ERROR_PARAMETER4);
}

/*===========================================================================*/
/* Function: Write SMB Word                                                  */
/* Purpose:                                                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_service_SMBWriteWord (ViSession instrSession,
                                              ViInt32 i2CAddress,
                                              ViInt32 SMBusCommand,
                                              ViUInt16 word)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    
    CHECKERR(rsnrp_sendCmd(instrSession,"SERV:UNL 1234"));
    sprintf(buffer,"SERV:SMB:WWOR %ld,%ld,%hu",i2CAddress, SMBusCommand, word);
    
    return rsnrp_sendCmd(instrSession, buffer);
}

/*===========================================================================*/
/* Function: Start Simulation                                                */
/* Purpose:  This function sets the number of simulation pairs count-value.  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_service_startSimulation (ViSession instrSession,
                                                 ViInt32 channel,
                                                 ViInt32 blockCount)
{
    ViStatus    rsnrp_status;

    GETSESSIONDATA(instrSession);
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));

    CHECKERR(rsnrp_sendCmdChannel(instrSession, "SERV%ld:UNL 1234", channel));
    
    instrPtr->simulationCount[channel] = blockCount;
    
    return rsnrp_setViInt32Channel(instrSession,
                                   "SERV%ld:RCO",
                                   channel,
                                   blockCount,
                                   0,0,
                                   VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Simulation Values                                           */
/* Purpose:  This function sets the values which will be simulated. Right    */
/*           before calling this function must be called function            */
/*           rsnrp_service_startSimulation!                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_service_setSimulationValues (ViSession instrSession,
                                                     ViInt32 channel,
                                                     ViInt32 valueCount[],
                                                     ViReal64 values[])
{
    ViStatus    rsnrp_status;
    ViInt32     i;

    GETSESSIONDATA(instrSession);
    
    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));

    if (!valueCount)
        return VI_ERROR_PARAMETER3;
    
    if (!values)
        return VI_ERROR_PARAMETER4;
    
    for (i = 0; i < instrPtr->simulationCount[channel]; i++)
    {
        CHECKERR(rsnrp_setViReal64Channel(instrSession,
                                   "SERV%ld:RES",
                                   channel,
                                   values[i],
                                   0.0,0.0,
                                   VI_ERROR_PARAMETER4));
        CHECKERR(rsnrp_setViInt32Channel(instrSession,
                                   "SERV%ld:RES",
                                   channel,
                                   valueCount[i],
                                   0,0,
                                   VI_ERROR_PARAMETER3));
    }
    return rsnrp_status;    
}

/*===========================================================================*/
/* Function: Stop Simulation                                                 */
/* Purpose:  This function stops the simulation by setting the count-value   */
/*           pairs to zero.                                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_service_stopSimulation (ViSession instrSession,
                                                ViInt32 channel)
{
    ViStatus    rsnrp_status;
    
    GETSESSIONDATA(instrSession);

    CHECKERR(rsnrp_invalidChannelRange(instrSession, 
                                       channel, 
                                       VI_ERROR_PARAMETER2));

    CHECKERR(rsnrp_sendCmdChannel(instrSession, "SERV%ld:UNL 1234", channel));
    
    instrPtr->simulationCount[channel] = 0;
    
    return rsnrp_sendCmdChannel(instrSession,
                                "SERV%ld:RCO 0",
                                channel);
}

/*===========================================================================*/
/* Function: Beep Immediate                                                  */
/* Purpose:  This function generates an audible signal via the built-in      */
/*           loudspeaker. The duration of the signal is set with the function*/
/*           rsnrp_system_beepTime(). Frequency and volume cannot be changed.*/
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_beepImmediate (ViSession instrSession)
{
    return rsnrp_sendCmd(instrSession, "SYST:BEEP");
}

/*===========================================================================*/
/* Function: Beep Stop                                                       */
/* Purpose:  This function stops the output of an audible signal.            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_beepStop (ViSession instrSession)
{
    return rsnrp_sendCmd(instrSession, "SYST:BEEP:STOP");
}

/*===========================================================================*/
/* Function: Set Beep Time                                                   */
/* Purpose:  This function sets the length in milliseconds of the audible    */
/*           signal output with SYS:BEEP.                                    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_setBeepTime (ViSession instrSession,
                                            ViReal64 beepTime)
{
    return rsnrp_setViReal64(instrSession,
                             "SYST:BEEP:TIME",
                             beepTime,
                             1.0, 60.0,
                             VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Get Beep Time                                                   */
/* Purpose:  This function reads the length in milliseconds of the audible   */
/*           signal output.                                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_getBeepTime (ViSession instrSession,
                                            ViReal64 *beepTime)
{
    return rsnrp_getViReal64(instrSession,
                             "SYST:BEEP:TIME?",
                             beepTime,
                             VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Set Beep On Overload                                            */
/* Purpose:  This function determines if the R&S NRP shall emit an acoustic  */
/*           signal when the power at a sensor exceeds the power limit.      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_setBeepOnOverload (ViSession instrSession,
                                                  ViBoolean state)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    CHECKERR(rsnrp_setEnum(instrSession,
                           "SYST:BEEP:NOT:OVER",
                           state,
                           switchArr,
                           VI_ERROR_PARAMETER2));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Beep On Overload                                            */
/* Purpose:  This function returns if the R&S NRP shall emit an acoustic     */
/*           signal when the power at a sensor exceeds the power limit.      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_getBeepOnOverload (ViSession instrSession,
                                                  ViBoolean *state)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    CHECKERR(rsnrp_getViBoolean(instrSession,
                                "SYST:BEEP:NOT:OVER?",
                                state,
                                VI_ERROR_PARAMETER2));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Keyclick                                                    */
/* Purpose:  This function enables or disables acoustic key clicks,          */
/*           respectively.                                                   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_setKeyclick (ViSession instrSession,
                                            ViBoolean state)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    CHECKERR(rsnrp_setEnum(instrSession,
                           "SYST:BEEP:NOT:KEY",
                           state,
                           switchArr,
                           VI_ERROR_PARAMETER2));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Keyclick                                                    */
/* Purpose:  This function returns the state of acoustic key clicks,         */
/*           respectively.                                                   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_getKeyclick (ViSession instrSession,
                                            ViBoolean *state)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    CHECKERR(rsnrp_getViBoolean(instrSession,
                                "SYST:BEEP:NOT:KEY?",
                                state,
                                VI_ERROR_PARAMETER2));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Clock Time                                                  */
/* Purpose:  This function sets the time. Since the R&S NRP has no           */
/*           battery-backed clock, the time has to be reset each time a new  */
/*           measurement is started if the clock was not set after the last  */
/*           power-on of the R&S NRP.                                        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_setClockTime (ViSession instrSession,
                                             ViInt32 hour, ViInt32 minute,
                                             ViInt32 second)
{
    ViChar  buffer[BUFFER_SIZE];

    if (rsnrp_invalidViInt32Range(hour,0,24))
        return VI_ERROR_PARAMETER2;
    
    if (rsnrp_invalidViInt32Range(minute,0,59))
        return VI_ERROR_PARAMETER3;

    if (rsnrp_invalidViInt32Range(second,0,59))
        return VI_ERROR_PARAMETER4;

    sprintf(buffer,"SYST:TIME %ld,%ld,%ld",hour,minute,second);
    
    return rsnrp_sendCmd(instrSession, buffer);
}

/*===========================================================================*/
/* Function: Get Clock Time                                                  */
/* Purpose:  This function reads the time.                                   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_getClockTime (ViSession instrSession,
                                             ViInt32 *hour, ViInt32 *minute,
                                             ViInt32 *second)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    
    if (!hour)
        return VI_ERROR_PARAMETER2;

    if (!minute)
        return VI_ERROR_PARAMETER3;

    if (!second)
        return VI_ERROR_PARAMETER4;

    CHECKERR(rsnrp_getViString(instrSession,"SYST:TIME?",buffer,BUFFER_SIZE,0));
    
    if (sscanf(buffer,"%ld,%ld,%ld",hour,minute,second) != 3)
        rsnrp_status = VI_ERROR_INSTR_INTERPRETING_RESPONSE;

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Date                                                        */
/* Purpose:  This function sets the date. Since the R&S NRP has no           */
/*           battery-backed clock, the date has to be reset each time a new  */
/*           measurement is started if the clock was not set after the last  */
/*           power-on of the R&S NRP.                                        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_setDate (ViSession instrSession, ViInt32 year,
                                        ViInt32 month, ViInt32 day)
{
    ViChar  buffer[BUFFER_SIZE];

    if (rsnrp_invalidViInt32Range(year,2000,2100))
        return VI_ERROR_PARAMETER2;
    
    if (rsnrp_invalidViInt32Range(month,1,12))
        return VI_ERROR_PARAMETER3;

    if (rsnrp_invalidViInt32Range(day,1,31))
        return VI_ERROR_PARAMETER4;

    sprintf(buffer,"SYST:DATE %ld,%ld,%ld",year,month,day);
    
    return rsnrp_sendCmd(instrSession, buffer);
}

/*===========================================================================*/
/* Function: Get Date                                                        */
/* Purpose:  This function reads the date.                                   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_getDate (ViSession instrSession,
                                        ViInt32 *year, ViInt32 *month,
                                        ViInt32 *day)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    
    if (!year)
        return VI_ERROR_PARAMETER2;

    if (!month)
        return VI_ERROR_PARAMETER3;

    if (!day)
        return VI_ERROR_PARAMETER4;

    CHECKERR(rsnrp_getViString(instrSession,"SYST:DATE?",buffer,BUFFER_SIZE,0));
    
    if (sscanf(buffer,"%ld,%ld,%ld",year,month,day) != 3)
        rsnrp_status = VI_ERROR_INSTR_INTERPRETING_RESPONSE;

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: System Info                                                     */
/* Purpose:  This function returns selected information on the instrument    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_info (ViSession instrSession,
                                     ViString infoType, ViInt32 arraySize,
                                     ViChar _VI_FAR info[])
{
    ViChar      buffer[BUFFER_SIZE];

    if (!infoType)
        return VI_ERROR_PARAMETER3;

    sprintf(buffer,"SYST:INFO? '%s'", infoType);
    
    return rsnrp_getViString(instrSession,
                             buffer,
                             info,
                             BUFFER_SIZE,
                             VI_ERROR_PARAMETER4);
}

/*===========================================================================*/
/* Function: System Info Headers                                             */
/* Purpose:  This function returns all available headers which can be        */
/*           retrieved from the instrument                                   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_infoHeader (ViSession instrSession,
                                           ViInt32 parameterNumber,
                                           ViInt32 arraySize,
                                           ViChar _VI_FAR header[])
{
    ViStatus    rsnrp_status;
    ViChar*     buffer = VI_NULL;
    ViChar*     p2buf;
    ViUInt32    count_local;
    ViInt32     i;
    ViChar      common_buffer[BUFFER_SIZE];
    ViBoolean   old_check;

    GETSESSIONDATA(instrSession);

    if (parameterNumber < 0)
        return VI_ERROR_PARAMETER3;

    if (!header)
        return VI_ERROR_PARAMETER4;

    /* Turn off error checking */
    old_check = instrPtr->errorChecking;
    CHECKERR(rsnrp_errorCheckState(instrSession,VI_FALSE));
    
    CHECKERR(rsnrp_sendCmd(instrSession,"SYST:INFO?"));
    CHECKERR(rsnrp_readDataUnknownLength(instrSession,&buffer, &count_local));
    
    p2buf = buffer+1;
    i = 0;
    
    while(p2buf && (parameterNumber > i))
    {
        p2buf = strstr(p2buf,"\",\"");
        if (p2buf)
        {
            p2buf+=3; /* move pointer behind the "," */
            i++;
        }
    }
    if (p2buf)
    {
        sscanf(p2buf,"%[^:]",common_buffer);
        strncpy(header,common_buffer,arraySize);
    }
    
    free(buffer);
    
    /* Set error checking to previous state */
    if(old_check)
            CHECKERR(rsnrp_checkStatus(instrSession));

    CHECKERR(rsnrp_errorCheckState(instrSession,old_check));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: System Info Count                                               */
/* Purpose:  This function returns the number of info headers for the        */
/*           instrument                                                      */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_infosCount (ViSession instrSession,
                                           ViInt32 *count)
{
    ViStatus    rsnrp_status;
    ViChar*     buffer = VI_NULL;
    ViChar*     p2buf;
    ViUInt32    count_local, i;
    ViBoolean   old_check;

    GETSESSIONDATA(instrSession);
    
    if (!count)
        return VI_ERROR_PARAMETER3;

    /* Turn off error checking */
    old_check = instrPtr->errorChecking;
    CHECKERR(rsnrp_errorCheckState(instrSession,VI_FALSE));

    CHECKERR(rsnrp_sendCmd(instrSession, "SYST:INFO?"));
    CHECKERR(rsnrp_readDataUnknownLength(instrSession,&buffer, &count_local));
    
    p2buf = buffer;
    i = 0;
    
    while(p2buf)
    {
        p2buf = strchr(p2buf,':');
        if (p2buf)
        {
            p2buf++; /* move pointer behind the : */
            i++;
        }
    }
    *count = i; /* the command returns pairs header/value, thus
                   the number of headers is equal to number of : */
    free(buffer);
    
    /* Set error checking to previous state */
    if(old_check)
            CHECKERR(rsnrp_checkStatus(instrSession));

    CHECKERR(rsnrp_errorCheckState(instrSession,old_check));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Key Char Value                                                  */
/* Purpose:  This function is used for testing purposes only and simulates   */
/*           a single stroke of an R&S NRP front-panel key.                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_keyCharValue (ViSession instrSession,
                                             ViInt32 simulatedKey)
{
    return rsnrp_setEnum(instrSession,
                         "SYST:KEY",
                         simulatedKey,
                         keyArr,
                         VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Key Numeric                                                     */
/* Purpose:  This function is used for testing purposes only and simulates   */
/*           a single stroke of an R&S NRP front-panel key.                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_keyNumeric (ViSession instrSession,
                                           ViInt32 simulatedKey)
{
    return rsnrp_setViInt32(instrSession,
                            "SYST:KEY",
                            simulatedKey,
                            1, 24,
                            VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Remote State                                                    */
/* Purpose:                                                                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_remoteState (ViSession instrSession,
                                            ViBoolean *remoteState)
{
    ViStatus    rsnrp_status;
    ViInt32     state;
    ViString    remoteArr[] = {"REM","LOC",VI_NULL};
    
    if (!remoteState)
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_getEnum(instrSession,
                   "SERV:REM:STAT?",
                   &state,
                   remoteArr,
                   0));

    *remoteState = (ViBoolean) state;
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Speed                                                       */
/* Purpose:  This function selects the data processing speed of the R&S NRP. */
/*           If Fast mode is selected, the display is switched off and       */
/*           the measured values are no longer displayed since the continuous*/
/*           update of the screen content requires computation time.         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_setSpeed (ViSession instrSession,
                                         ViInt32 speed)
{
    return rsnrp_setEnum(instrSession,
                         "SYST:SPE",
                         speed,
                         speedArr,
                         VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Get Speed                                                       */
/* Purpose:  This function reads the selected data processing speed of       */
/*           the R&S NRP.                                                    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_getSpeed (ViSession instrSession,
                                         ViInt32 *speed)
{
    return rsnrp_getEnum(instrSession,
                         "SYST:SPE?",
                         speed,
                         speedArr,
                         VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Set Error Extended Message                                      */
/* Purpose:  This function activates extended error messages.                */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_error_setExtendedMessage (ViSession instrSession,
                                                  ViBoolean state)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    CHECKERR(rsnrp_setEnum(instrSession,
                           "SYST:ERR:EXT",
                           state,
                           switchArr,
                           VI_ERROR_PARAMETER2));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get Error Extended Message                                      */
/* Purpose:  This function returns state of the extended error messages.     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_error_getExtendedMessage (ViSession instrSession,
                                                  ViBoolean *state)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    GETSESSIONDATA(instrSession);
    
    CHECKFWREVISION(3,01);
    
    CHECKERR(rsnrp_getViBoolean(instrSession,
                                "SYST:ERR:EXT?",
                                state,
                                VI_ERROR_PARAMETER2));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: SCPI Version                                                    */
/* Purpose:  This function returns a string that contains the current version*/
/*           number of the SCPI standard.                                    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_system_SCPIversion (ViSession instrSession,
                                            ViInt32 arraySize,
                                            ViChar _VI_FAR systemVersion[])
{
    return rsnrp_getViString(instrSession,
                             "SYST:VERS?",
                             systemVersion,
                             arraySize,
                             VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Count                                                           */
/* Purpose:  This function returns number of available windows (always 4)    */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_windows_getCount (ViSession instrSession,
                                          ViInt32 *count)
{
    if (!count)
        return VI_ERROR_PARAMETER2;
    
    *count = 4;
    return VI_SUCCESS;
}

/*===========================================================================*/
/* Function: Select                                                          */
/* Purpose:  This function selects a window.                                 */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_windows_setSelected (ViSession instrSession,
                                             ViInt32 window)
{
    return rsnrp_setViInt32(instrSession,
                            "DISP:WIND:SEL",
                            window,
                            1,4,
                            VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Selected                                                        */
/* Purpose:  This function returns the number of selected window.            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_windows_getSelected (ViSession instrSession,
                                             ViInt32 *selectedWindow)
{
    return rsnrp_getViInt32(instrSession,
                            "DISP:WIND:SEL?",
                            selectedWindow,
                            VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Set Size                                                        */
/* Purpose:  This function sets the window size.                             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_windows_setSize (ViSession instrSession,
                                         ViInt32 windowSize)
{
    return rsnrp_setEnum(instrSession,
                        "DISP:WIND:SIZE",
                        windowSize,
                        windowArr,
                        VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Get Size                                                        */
/* Purpose:  This function reads the window size.                            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_windows_getSize (ViSession instrSession,
                                         ViInt32 *windowSize)
{
    return rsnrp_getEnum(instrSession,
                        "DISP:WIND:SIZE?",
                        windowSize,
                        windowArr,
                        VI_ERROR_PARAMETER2);
}

/*===========================================================================*/
/* Function: Configure Window Analog                                         */
/* Purpose:  This function configures the scale of window measurement and    */
/*           selects the Analog display mode.                                */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_configureWindowAnalog (ViSession instrSession,
                                                      ViInt32 window,
                                                      ViInt32 resolution,
                                                      ViInt32 units,
                                                      ViReal64 lowerScale,
                                                      ViReal64 upperScale)
{
    ViStatus    rsnrp_status;
    
    if (rsnrp_invalidViInt32Range(units, 0, 5))
        return VI_ERROR_PARAMETER4;

    CHECKERR(rsnrp_window_setVisible(instrSession, window, VI_TRUE));
    CHECKERR(rsnrp_window_setFormat (instrSession, window, RSNRP_FORMAT_ANALOG));
    CHECKERR(rsnrp_window_setResolution (instrSession, window, resolution));
    if (units <= RSNRP_UNITS_DBUV)
        CHECKERR(rsnrp_meas_setUnits(instrSession,window,units));
    else
        CHECKERR(rsnrp_meas_setUnitsRatio(instrSession, window, units));

    
    CHECKERR(rsnrp_setViReal64Channel(instrSession,
                                      "DISP:WIND%ld:ANAL:LOW",
                                      window,
                                      lowerScale,
                                      lowLimitArray[units],
                                      highLimitArray[units],
                                      VI_ERROR_PARAMETER5));
    
    return rsnrp_setViReal64Channel(instrSession,
                                      "DISP:WIND%ld:ANAL:UPP",
                                      window,
                                      upperScale,
                                      lowLimitArray[units],
                                      highLimitArray[units],
                                      VI_ERROR_PARAMETER6);

}

/*===========================================================================*/
/* Function: Configure Window Digital                                        */
/* Purpose:  This function configures and selects the Digital display mode.  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_configureWindowDigital (ViSession instrSession,
                                                       ViInt32 window,
                                                       ViInt32 resolution,
                                                       ViInt32 auxiliaryValue)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_window_setVisible(instrSession, window, VI_TRUE));
    CHECKERR(rsnrp_window_setFormat (instrSession, window, RSNRP_FORMAT_DIGITAL));
    CHECKERR(rsnrp_window_setResolution (instrSession, window, resolution));
    rsnrp_status = rsnrp_window_setAuxiliaryValue(instrSession, window,
                                                  auxiliaryValue);
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        rsnrp_status = VI_ERROR_PARAMETER4;
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Configure Window Trace                                          */
/* Purpose:  This function configures the scale of window measurement and    */
/*           selects the Trace display mode.                                 */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_configureWindowTrace (ViSession instrSession,
                                                     ViInt32 window,
                                                     ViReal64 lowerScale,
                                                     ViReal64 upperScale)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_window_setTraceLowerPower(instrSession, window,lowerScale));
    rsnrp_status = rsnrp_window_setTraceUpperPower(instrSession, window,upperScale);
    
    if (rsnrp_status == VI_ERROR_PARAMETER3)
        rsnrp_status = VI_ERROR_PARAMETER4;

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Auxiliary Value                                             */
/* Purpose:  This function selects the auxiliary value to be dispalayed.     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_setAuxiliaryValue (ViSession instrSession,
                                                  ViInt32 window,
                                                  ViInt32 auxiliaryValue)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    if (rsnrp_invalidViInt32Range(auxiliaryValue,0,3))
        return VI_ERROR_PARAMETER3;

    return rsnrp_setEnumChannel(instrSession,
                                "DISP:WIND%ld:AVAL",
                                window,
                                auxiliaryValue,
                                auxArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Auxiliary Value                                             */
/* Purpose:  This function reads selected auxiliary value.                   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_getAuxiliaryValue (ViSession instrSession,
                                                  ViInt32 window,
                                                  ViInt32 *auxiliaryValue)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getEnumChannel(instrSession,
                                "DISP:WIND%ld:AVAL?",
                                window,
                                auxiliaryValue,
                                auxArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Format                                                      */
/* Purpose:  This function selects the display mode for measured data.       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_setFormat (ViSession instrSession,
                                          ViInt32 window, ViInt32 format)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    return rsnrp_setEnumChannel(instrSession,
                                "DISP:WIND%ld:FORM",
                                window,
                                format,
                                formatArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Format                                                      */
/* Purpose:  This function reads the display mode for measured data.         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_getFormat (ViSession instrSession,
                                          ViInt32 window, ViInt32 *format)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getEnumChannel(instrSession,
                                "DISP:WIND%ld:FORM?",
                                window,
                                format,
                                formatArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Meter Auto                                                      */
/* Purpose:  This function automatically determines scaling for the analog   */
/*           display.                                                        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_meterAuto (ViSession instrSession,
                                          ViInt32 window)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    return rsnrp_sendCmdChannel(instrSession,
                                "DISP:WIND%ld:ANAL:AUTO ONCE",
                                window);
}

/*===========================================================================*/
/* Function: Set Meter Lower Power                                           */
/* Purpose:  This function sets the lower limit value of the analog scale.   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_setMeterLowerPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 lowerPower)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    if (rsnrp_invalidViReal64Range(lowerPower,-150.0,210.0))
        return VI_ERROR_PARAMETER3;

    sprintf(buffer,"DISP:WIND%ld:ANAL:LOW:POW %LeDBM",window,lowerPower);
    
    return rsnrp_sendCmd(instrSession,buffer);
}

/*===========================================================================*/
/* Function: Get Meter Lower Power                                           */
/* Purpose:  This function sets the lower limit value of the analog scale.   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_getMeterLowerPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 *lowerPower)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getViReal64Channel(instrSession,
                                    "DISP:WIND%ld:ANAL:LOW:POW?",
                                    window,
                                    lowerPower,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Meter Lower Ratio                                           */
/* Purpose:  This function sets the lower limit value of the analog scale.   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_setMeterLowerRatio (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 lowerRatio)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    if (rsnrp_invalidViReal64Range(lowerRatio,-200.0,200.0))
        return VI_ERROR_PARAMETER3;

    sprintf(buffer,"DISP:WIND%ld:ANAL:LOW:RAT %LeDB",window,lowerRatio);

    return rsnrp_sendCmd(instrSession,buffer);
}

/*===========================================================================*/
/* Function: Get Meter Lower Ratio                                           */
/* Purpose:  This functin reads the lower limit value of the analog scale.   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_getMeterLowerRatio (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 *lowerRatio)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getViReal64Channel(instrSession,
                                    "DISP:WIND%ld:ANAL:LOW:RAT?",
                                    window,
                                    lowerRatio,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Meter Upper Power                                           */
/* Purpose:  This function sets the upper limit value of the analog scale.   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_setMeterUpperPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 upperPower)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    if (rsnrp_invalidViReal64Range(upperPower,-150.0,210.0))
        return VI_ERROR_PARAMETER3;

    sprintf(buffer,"DISP:WIND%ld:ANAL:UPP:POW %LeDBM",window,upperPower);
    
    return rsnrp_sendCmd(instrSession,buffer);
}

/*===========================================================================*/
/* Function: Get Meter Upper Power                                           */
/* Purpose:  This function reads the upper limit value of the analog scale.  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_getMeterUpperPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 *upperPower)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getViReal64Channel(instrSession,
                                    "DISP:WIND%ld:ANAL:UPP:POW?",
                                    window,
                                    upperPower,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Meter Upper Ratio                                           */
/* Purpose:  This function sets the upper limit value of the analog scale.   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_setMeterUpperRatio (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 upperRatio)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    if (rsnrp_invalidViReal64Range(upperRatio,-200.0,200.0))
        return VI_ERROR_PARAMETER3;

    sprintf(buffer,"DISP:WIND%ld:ANAL:UPP:RAT %LeDB",window,upperRatio);

    return rsnrp_sendCmd(instrSession,buffer);
}

/*===========================================================================*/
/* Function: Get Meter Upper Ratio                                           */
/* Purpose:  This function reads the upper limit value of the analog scale.  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_getMeterUpperRatio (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 *upperRatio)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getViReal64Channel(instrSession,
                                    "DISP:WIND%ld:ANAL:UPP:RAT?",
                                    window,
                                    upperRatio,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Resolution                                                  */
/* Purpose:  This function specifies the number of significant places of     */
/*           the mantissa when the measurement result is displayed on        */
/*           a linear scale.                                                 */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_setResolution (ViSession instrSession,
                                              ViInt32 window, ViInt32 resolution)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    return rsnrp_setViInt32Channel(instrSession,
                                   "DISP:WIND%ld:RES",
                                   window,
                                   resolution,
                                   1,4,
                                   VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Resolution                                                  */
/* Purpose:  This function reads the number of significant places of         */
/*           the mantissa when the measurement result is displayed on        */
/*           a linear scale.                                                 */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_getResolution (ViSession instrSession,
                                              ViInt32 window,
                                              ViInt32 *resolution)
{
    ViStatus    rsnrp_status;
    ViReal64    l_res;

    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    if (!resolution)
        return VI_ERROR_PARAMETER3;

    CHECKERR(rsnrp_getViReal64Channel(instrSession,
                                   "DISP:WIND%ld:RES?",
                                   window,
                                   &l_res,
                                   0));
    *resolution = (ViInt32) (fabs(log10(l_res)) + 1);
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Timeslot                                                    */
/* Purpose:  This function sets the displayed timeslot, if the primary or    */
/*           secondary channel is operated in the Timeslot mode.             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_setTimeslot (ViSession instrSession,
                                            ViInt32 window, ViInt32 timeslot)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    return rsnrp_setViInt32Channel(instrSession,
                                   "DISP:WIND%ld:TSL",
                                   window,
                                   timeslot,
                                   0,0,
                                   VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Timeslot                                                    */
/* Purpose:  This function reads the displayed timeslot.                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_getTimeslot (ViSession instrSession,
                                            ViInt32 window, ViInt32 *timeslot)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getViInt32Channel(instrSession,
                                   "DISP:WIND%ld:TSL?",
                                   window,
                                   timeslot,
                                   VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Title                                                       */
/* Purpose:  This function determines the string to be displayed in          */
/*           the respective window. The reset values depend on the window.   */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_setTitle (ViSession instrSession,
                                         ViInt32 window, ViString title)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    sprintf(buffer,"'%s'",title);
    
    return rsnrp_setViStringChannel(instrSession,
                                   "DISP:WIND%ld:NAME",
                                   window,
                                   buffer,
                                   VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Title                                                       */
/* Purpose:  This function reads the string displayed in the respective      */
/*           window.                                                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_getTitle (ViSession instrSession,
                                         ViInt32 window, ViInt32 arraySize,
                                         ViChar _VI_FAR title[])
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    ViChar*     p2buf;
    
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;
    if (arraySize <=0)
        return VI_ERROR_PARAMETER3;
    if (!title)
        return VI_ERROR_PARAMETER4;

    CHECKERR(rsnrp_getViStringChannel(instrSession,
                                   "DISP:WIND%ld:NAME?",
                                   window,
                                   buffer,
                                   BUFFER_SIZE,
                                   0));
    p2buf = strtok(buffer,"\"");
    if (p2buf)
        strncpy (title, p2buf, arraySize);
    else
        return VI_ERROR_INSTR_INTERPRETING_RESPONSE;
        
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Trace Lower Power                                           */
/* Purpose:  This function sets the lower limit for the power axis in the    */
/*           Scope mode.                                                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_setTraceLowerPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 lowerPower)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    if (rsnrp_invalidViReal64Range(lowerPower,-120.0,90.0))
        return VI_ERROR_PARAMETER3;

    sprintf(buffer,"DISP:WIND%ld:TRAC:LOW %LeDBM",window,lowerPower);
    
    return rsnrp_sendCmd(instrSession,buffer);
}

/*===========================================================================*/
/* Function: Get Trace Lower Power                                           */
/* Purpose:  This function reads the lower limit for the power axis in the   */
/*           Scope mode.                                                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_getTraceLowerPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 *lowerPower)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getViReal64Channel(instrSession,
                                    "DISP:WIND%ld:TRAC:LOW?",
                                    window,
                                    lowerPower,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Trace Upper Power                                           */
/* Purpose:  This function sets the upper limit for the power axis in the    */
/*           Scope mode.                                                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_setTraceUpperPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 upperPower)
{
    ViChar  buffer[BUFFER_SIZE];
    
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    if (rsnrp_invalidViReal64Range(upperPower,-120.0,90.0))
        return VI_ERROR_PARAMETER3;

    sprintf(buffer,"DISP:WIND%ld:TRAC:UPP %LeDBM",window,upperPower);
    
    return rsnrp_sendCmd(instrSession,buffer);
}

/*===========================================================================*/
/* Function: Get Trace Upper Power                                           */
/* Purpose:  This function reads the upper limit for the power axis in the   */
/*           Scope mode.                                                     */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_getTraceUpperPower (ViSession instrSession,
                                                   ViInt32 window,
                                                   ViReal64 *upperPower)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;

    return rsnrp_getViReal64Channel(instrSession,
                                    "DISP:WIND%ld:TRAC:UPP?",
                                    window,
                                    upperPower,
                                    VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Set Visible                                                     */
/* Purpose:  This function opens or closes a window.                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_setVisible (ViSession instrSession,
                                           ViInt32 window, ViBoolean visible)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;
   
    return rsnrp_setEnumChannel(instrSession,
                                "DISP:WIND%ld",
                                window,
                                visible,
                                switchArr,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Get Visible                                                     */
/* Purpose:  This function reads the state of specified window.              */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_window_getVisible (ViSession instrSession,
                                           ViInt32 window, ViBoolean *visible)
{
    if (rsnrp_invalidViInt32Range(window,1,4))
        return VI_ERROR_PARAMETER2;
   
    return rsnrp_getViBooleanChannel(instrSession,
                                "DISP:WIND%ld?",
                                window,
                                visible,
                                VI_ERROR_PARAMETER3);
}

/*===========================================================================*/
/* Function: Check Condition                                                 */
/* Purpose:  This function checks selected status register for bits defined  */
/*           in Bitmask and returns a logical OR of all defined bits.        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_status_checkCondition (ViSession instrSession,
                                               ViInt32 statusClass,
                                               ViUInt32 mask, ViBoolean *state)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    ViInt32     condition;
    
    if (rsnrp_invalidViInt32Range(statusClass,1,12))
        return VI_ERROR_PARAMETER2;
    if (!state)
        return VI_ERROR_PARAMETER4;
    if (rsnrp_invalidMask(mask, statusClass))
        return VI_ERROR_PARAMETER3;

    strcpy(buffer,gs_NrpRegs[statusClass].command);

    /* remove question mark */
    buffer[strlen(buffer) - 1] = ':';
    
    strcat(buffer,"COND?");
    CHECKERR(rsnrp_getViInt32(instrSession,
                              buffer,
                              &condition,
                              0));
    condition &= rsnrp_translateMask(mask,statusClass);
    *state = (condition != 0);

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Catch Event                                                     */
/* Purpose:  This function sets the PTransition and NTransition register of  */
/*           selected status register according to bitmask.                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_status_catchEvent (ViSession instrSession,
                                           ViInt32 statusClass, ViUInt32 mask,
                                           ViInt32 direction)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViChar      buffer[BUFFER_SIZE];
    ViChar*     p2buf;
    
    if (rsnrp_invalidViInt32Range(statusClass,1,12) || (statusClass == 3) || (statusClass == 4))
        return VI_ERROR_PARAMETER2;
    if (rsnrp_invalidMask(mask, statusClass))
        return VI_ERROR_PARAMETER3;
    if (rsnrp_invalidViInt32Range(direction,0,3))
        return VI_ERROR_PARAMETER4;

    strcpy(buffer,gs_NrpRegs[statusClass].command);

    /* remove question mark */
    buffer[strlen(buffer) - 1] = ':';

    switch(direction)
    {
        case RSNRP_DIRECTION_PTR:
        case RSNRP_DIRECTION_NTR:
            strcat(buffer,directionArr[direction]);
            CHECKERR(rsnrp_setViInt32(instrSession,
                                      buffer,
                                      rsnrp_translateMask(mask,statusClass),
                                      0,32767,
                                      VI_ERROR_PARAMETER3));
            break;
        case RSNRP_DIRECTION_BOTH:
            p2buf = buffer + strlen(buffer);
            strcat(buffer,directionArr[RSNRP_DIRECTION_PTR]);
            CHECKERR(rsnrp_setViInt32(instrSession,
                                      buffer,
                                      rsnrp_translateMask(mask,statusClass),
                                      0,32767,
                                      VI_ERROR_PARAMETER3));
            *p2buf = '\0';
            strcat(buffer,directionArr[RSNRP_DIRECTION_NTR]);
            CHECKERR(rsnrp_setViInt32(instrSession,
                                      buffer,
                                      rsnrp_translateMask(mask,statusClass),
                                      0,32767,
                                      VI_ERROR_PARAMETER3));
            break;
        case RSNRP_DIRECTION_NONE:
            p2buf = buffer + strlen(buffer);
            strcat(buffer,directionArr[RSNRP_DIRECTION_PTR]);
            CHECKERR(rsnrp_setViInt32(instrSession,
                                      buffer,
                                      0,
                                      0,32767,
                                      VI_ERROR_PARAMETER3));
            *p2buf = '\0';
            strcat(buffer,directionArr[RSNRP_DIRECTION_NTR]);
            CHECKERR(rsnrp_setViInt32(instrSession,
                                      buffer,
                                      0,
                                      0,32767,
                                      VI_ERROR_PARAMETER3));
            break;
    }

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Check Event                                                     */
/* Purpose:  This function checks the selected status register for events    */
/*           specified by bitmask and sets returns their states.             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_status_checkEvent (ViSession instrSession,
                                           ViInt32 statusClass, ViUInt32 mask,
                                           ViUInt32 resetMask, ViBoolean *events)
{
    ViStatus    rsnrp_status;
    rsnrp_tEventCache* pcache;
    ViUInt32*   pregister;
    
    GETSESSIONDATA(instrSession);

    if (rsnrp_invalidViInt32Range(statusClass,1,12) || (statusClass == 3) || (statusClass == 4))
        return VI_ERROR_PARAMETER2;
    if (rsnrp_invalidMask(mask, statusClass))
        return VI_ERROR_PARAMETER3;
    if (rsnrp_invalidMask(resetMask,statusClass))
        return VI_ERROR_PARAMETER4;
    if (!events)
        return VI_ERROR_PARAMETER5;
        
    CHECKERR(rsnrp_updateCache(instrSession,
                               statusClass,
                               0,
                               0));

    pcache = &(instrPtr->events);
    pregister = (ViUInt32*)(((ViByte*)pcache) + (statusClass*sizeof(ViUInt32)));
    
    *events = (*pregister & rsnrp_translateMask(mask, statusClass)) != 0;
    *pregister &= ~rsnrp_translateMask(resetMask,statusClass);
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Enable Event Notification                                       */
/* Purpose:  This function enables events defined by Bitmask in enable       */
/*           register respective to the selected status register.            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_status_enableEventNotification (ViSession instrSession,
                                                        ViInt32 statusClass,
                                                        ViUInt32 mask)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    ViChar*     p2buf;

    if (rsnrp_invalidViInt32Range(statusClass,1,12) || (statusClass == 3) || (statusClass == 4))
        return VI_ERROR_PARAMETER2;
    if (rsnrp_invalidMask(mask, statusClass))
        return VI_ERROR_PARAMETER3;

    strcpy(buffer,gs_NrpRegs[statusClass].command);

    /* remove question mark */
    buffer[strlen(buffer) - 1] = ':';
    strcat(buffer,"ENAB");
    
    CHECKERR(rsnrp_updateRegister(instrSession,
                                  buffer,
                                  0,
                                  rsnrp_translateMask(mask, statusClass)));

    if (gs_NrpRegs[statusClass].statusMask >=0)
    {
        /* remove :ENAB */
        p2buf = strrchr(buffer,':');
        *p2buf = '\0';
        /* remove :<reg> */
        p2buf = strrchr(buffer,':');
        *p2buf = '\0';
        /* add :ENAB */
        strcat(buffer,":ENAB");
        CHECKERR(rsnrp_updateRegister(instrSession,
                                      buffer,
                                      0,
                                      gs_NrpRegs[statusClass].statusMask));
    }

    if (gs_NrpRegs[statusClass].sreMask >=0)
    {
        CHECKERR(rsnrp_updateRegister(instrSession,
                                      "*ESE",
                                      0,
                                      gs_NrpRegs[statusClass].sreMask));
    }

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Disable Event Notification                                      */
/* Purpose:  This function disables events defined by Bitmask in enable      */
/*           register respective to the selected status register.            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_status_disableEventNotification (ViSession instrSession,
                                                         ViInt32 statusClass,
                                                         ViUInt32 mask)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    ViChar*     p2buf;

    if (rsnrp_invalidViInt32Range(statusClass,1,12) || (statusClass == 3) || (statusClass == 4))
        return VI_ERROR_PARAMETER2;
    if (rsnrp_invalidMask(mask, statusClass))
        return VI_ERROR_PARAMETER3;

    strcpy(buffer,gs_NrpRegs[statusClass].command);

    /* remove question mark */
    buffer[strlen(buffer) - 1] = ':';
    strcat(buffer,"ENAB");
    
    CHECKERR(rsnrp_updateRegister(instrSession,
                                  buffer,
                                  rsnrp_translateMask(mask, statusClass),
                                  0));

    if (gs_NrpRegs[statusClass].statusMask >=0)
    {
        /* remove :ENAB */
        p2buf = strrchr(buffer,':');
        *p2buf = '\0';
        /* remove :<reg> */
        p2buf = strrchr(buffer,':');
        *p2buf = '\0';
        /* add :ENAB */
        strcat(buffer,":ENAB");
        CHECKERR(rsnrp_updateRegister(instrSession,buffer,gs_NrpRegs[statusClass].statusMask,0));
    }

    if (gs_NrpRegs[statusClass].sreMask >=0)
    {
        CHECKERR(rsnrp_updateRegister(instrSession,"*ESE",gs_NrpRegs[statusClass].sreMask,0));
    }

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Preset                                                          */
/* Purpose:  This function resets the R&S NRP to default values.             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_status_preset (ViSession instrSession)
{
    ViStatus    rsnrp_status;
    
    CHECKERR(rsnrp_sendCmd(instrSession,"SYST:PRES"));

    CHECKERR (rsnrp_sendCmd(instrSession, "*ESE 255"));
    CHECKERR (rsnrp_sendCmd(instrSession, "*SRE 32"));
    /* Calibrating Status Register - set events to indicate calibration 
       completed */
    CHECKERR (rsnrp_sendCmd(instrSession, "STAT:OPER:CAL:PTR 0"));
    CHECKERR (rsnrp_sendCmd(instrSession, "STAT:OPER:CAL:NTR 30"));

    /* Measuring Status Register - set events to indicate measurement 
       completed */
    CHECKERR (rsnrp_sendCmd(instrSession, "STAT:OPER:MEAS:PTR 0"));
    CHECKERR (rsnrp_sendCmd(instrSession, "STAT:OPER:MEAS:NTR 30"));
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: State Checking                                                  */
/* Purpose:  This function switches ON/OFF state checking of the instrument  */
/*           (reading of the Standard Event Register and checking it for     */
/*           error). Nearly all driver function are using state checking.    */
/*           Switch this function to ON when debug your application. For     */
/*           better bus throughput and instruments performance switch it OFF.*/
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_errorCheckState (ViSession instrSession,
                                          ViBoolean state)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    rsnrp_instrRange instrPtr;

    CHECKERR (viGetAttribute (instrSession, VI_ATTR_USER_DATA, &instrPtr));

    instrPtr -> errorChecking = state;

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Write To Instrument                                             */
/* Purpose:  This function writes a command string to the instrument.        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_writeInstrData (ViSession instrSession, ViString writeBuffer)
{
    ViStatus rsnrp_status = VI_SUCCESS;
    
    CHECKERR (viPrintf (instrSession, "%s", writeBuffer));
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Read Instrument Buffer                                          */
/* Purpose:  This function reads the output buffer of the instrument.        */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_readInstrData (ViSession instrSession,
                                        ViInt32 numberBytesToRead,
                                        ViChar _VI_FAR readBuffer[],
                                        ViPInt32 numBytesRead)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViUInt32    tmpCnt;

    if (numBytesRead)
        *numBytesRead = 0L;
        
    CHECKERR (viRead (instrSession, (ViBuf)readBuffer, numberBytesToRead, &tmpCnt));
    
    if (numBytesRead)
        *numBytesRead = (ViInt32)tmpCnt;
    
    return rsnrp_status;
}


/*===========================================================================*/
/* Function: Query Instrument Buffer                                         */
/* Purpose:  This function queries data from the instrument's output buffer  */
/*           and returns it to the specified variable in memory.             */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_queryInstrData (ViSession instrSession,
                ViString writeBuffer, ViInt32 numberBytesToRead,
                ViChar _VI_FAR readBuffer[], ViPInt32 numBytesRead)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViUInt32    tmpCnt = 0;

    if (numBytesRead)
        *numBytesRead = 0L;

    if (writeBuffer)
        CHECKERR (viPrintf (instrSession, "%s", writeBuffer));

    if (readBuffer)
        CHECKERR (viRead (instrSession, (ViBuf)readBuffer, numberBytesToRead, &tmpCnt));

    if (numBytesRead)
        *numBytesRead = (ViInt32)tmpCnt;

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Reset                                                           */
/* Purpose:  This function resets the instrument.  If the reset function     */
/*           is not supported by the instrument, this function returns       */
/*           the warning VI_WARN_NSUP_RESET.                                 */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_reset (ViSession instrSession)
{
    ViStatus rsnrp_status = VI_SUCCESS;

    /*  Initialize the instrument to a known state.  */
    CHECKERR (viPrintf (instrSession, "*RST"));
    CHECKERR (rsnrp_defaultInstrSetup (instrSession));
       
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Self-Test                                                       */
/* Purpose:  This function executes the instrument self-test and returns     */
/*           the result. If the self test function is not supported by the   */
/*           instrument, this function returns the warning                   */
/*           VI_WARN_NSUP_SELF_TEST.                                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_self_test (ViSession instrSession,
                                    ViPInt16 testResult,
                                    ViChar _VI_FAR testMessage[])
{
    ViStatus rsnrp_status = VI_SUCCESS;

    CHECKERR (viPrintf (instrSession, "*TST?"));
    CHECKERR (viScanf (instrSession, "%ld", testResult));

    if (testMessage)
        sprintf(testMessage, "Self-Test %s.",
            (!(*testResult)) ? "Passed" : "Failed");

    CHECKERR (rsnrp_checkStatus (instrSession));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Error Query                                                     */
/* Purpose:  This function queries the instrument error queue, and returns   */
/*           the result. If the error query function is not supported by the */
/*           instrument, this function returns the warning                   */
/*           VI_WARN_NSUP_ERROR_QUERY.                                       */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_error_query (ViSession instrSession,
                                      ViPInt32 errorCode,
                                      ViChar _VI_FAR errorMessage[])
{
    ViStatus rsnrp_status = VI_SUCCESS;

    CHECKERR (viPrintf (instrSession, ":SYST:ERR?"));
    CHECKERR (viScanf (instrSession, "%ld,\"%[^\"]", errorCode, errorMessage));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Error Message                                                   */
/* Purpose:  This function translates the error return value from the        */
/*           instrument driver into a user-readable string.                  */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_error_message (ViSession instrSession,
                                        ViStatus statusCode,
                                        ViChar _VI_FAR errMessage[])
{
    ViStatus rsnrp_status = VI_SUCCESS;
    ViInt16 i;

    static rsnrp_tStringValPair statusDescArray[] = {
        {VI_WARN_NSUP_ID_QUERY,     "WARNING: ID Query not supported"},
        {VI_WARN_NSUP_RESET,        "WARNING: Reset not supported"},
        {VI_WARN_NSUP_SELF_TEST,    "WARNING: Self-test not supported"},
        {VI_WARN_NSUP_ERROR_QUERY,  "WARNING: Error Query not supported"},     
        {VI_WARN_NSUP_REV_QUERY,    "WARNING: Revision Query not supported"},
        {VI_ERROR_PARAMETER1,   "ERROR: Parameter 1 out of range"},
        {VI_ERROR_PARAMETER2,   "ERROR: Parameter 2 out of range"},
        {VI_ERROR_PARAMETER3,   "ERROR: Parameter 3 out of range"},
        {VI_ERROR_PARAMETER4,   "ERROR: Parameter 4 out of range"},
        {VI_ERROR_PARAMETER5,   "ERROR: Parameter 5 out of range"},
        {VI_ERROR_PARAMETER6,   "ERROR: Parameter 6 out of range"},
        {VI_ERROR_PARAMETER7,   "ERROR: Parameter 7 out of range"},
        {VI_ERROR_PARAMETER8,   "ERROR: Parameter 8 out of range"},
        {VI_ERROR_FAIL_ID_QUERY,"ERROR: Identification query failed"},
        {VI_ERROR_INV_RESPONSE, "ERROR: Interpreting instrument response"},
        {VI_ERROR_INSTR_FILE_OPEN,    "ERROR: Opening the specified file"},
        {VI_ERROR_INSTR_FILE_WRITE,   "ERROR: Writing to the specified file"},
        {VI_ERROR_INSTR_INTERPRETING_RESPONSE, "ERROR: Interpreting the instrument's response"},
        {VI_ERROR_INSTR_PARAMETER9 ,  "ERROR: Parameter 9 out of range"},
        {VI_ERROR_INSTR_PARAMETER10,  "ERROR: Parameter 10 out of range"},
        {VI_ERROR_INSTR_PARAMETER11,  "ERROR: Parameter 11 out of range"},
        {VI_ERROR_INSTR_PARAMETER12,  "ERROR: Parameter 12 out of range"},
        {VI_ERROR_INSTR_PARAMETER13,  "ERROR: Parameter 13 out of range"},
        {VI_ERROR_INSTR_PARAMETER14,  "ERROR: Parameter 14 out of range"},
        {VI_ERROR_INSTR_PARAMETER15,  "ERROR: Parameter 15 out of range"},
        {RSNRP_ERROR_INSTRUMENT_ERROR,         "ERROR: Instrument status error"},
        {RSNRP_ERROR_INVALID_CONFIGURATION,    "ERROR: Instrument configuration error"},
        {RSNRP_ERROR_INSTRUMENT_OPTION,        "ERROR: Option not installed"},
        {RSNRP_ERROR_INSTRUMENT_NSUP_MODEL,    "ERROR: Required Instrument model is not available."},
        {RSNRP_ERROR_SETTINGS_CONFLICT,        "ERROR: Instrument settings conflict."},
        {RSNRP_ERROR_TABLE_NOT_FOUND,          "ERROR: Specified offset table is not available."},
        {RSNRP_ERROR_SECONDARY_VALUE_INVALID,  "ERROR: Specified secondary value is invalid."},
        {RSNRP_ERROR_INVALID_FIRMWARE_VERSION, "ERROR: Instrument firmware version is invalid."},
        {VI_NULL, VI_NULL}
    };

    rsnrp_status = viStatusDesc (instrSession, statusCode, errMessage);
    if (rsnrp_status == VI_WARN_UNKNOWN_STATUS)
        {
        for (i = 0; statusDescArray[i].stringName; i++)
            {
            if (statusDescArray[i].stringVal == statusCode)
                {
                sprintf (errMessage, "%s", statusDescArray[i].stringName);
                return (VI_SUCCESS);
                }
            }
        sprintf (errMessage, "Unknown Error 0x%X", statusCode);
        return (VI_WARN_UNKNOWN_STATUS);
        }

    rsnrp_status = VI_SUCCESS;
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Revision Query                                                  */
/* Purpose:  This function returns the driver and instrument revisions.      */
/*           If the revision query function is not supported by the          */ 
/*           instrument, this function returns the warning                   */
/*           VI_WARN_NSUP_REV_QUERY.                                         */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_revision_query (ViSession instrSession,
                    ViChar _VI_FAR driverRev[], ViChar _VI_FAR instrRev[])
{
    ViStatus rsnrp_status = VI_SUCCESS;

    CHECKERR (viPrintf (instrSession, "*IDN?"));
    CHECKERR (viScanf (instrSession, "%*[^,],%*[^,],%*[^,],%[^\r\n]", instrRev));

    sprintf (driverRev, "%s", RSNRP_REVISION);
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Close                                                           */
/* Purpose:  This function closes the instrument.                            */
/*===========================================================================*/
ViStatus _VI_FUNC rsnrp_close (ViSession instrSession)
{
    rsnrp_instrRange instrPtr;
    ViSession rmSession;
    ViStatus rsnrp_status = VI_SUCCESS;

    CHECKERR( viGetAttribute (instrSession, VI_ATTR_RM_SESSION, &rmSession));
    CHECKERR( viGetAttribute (instrSession, VI_ATTR_USER_DATA, &instrPtr));

    if (instrPtr != NULL) 
        free (instrPtr);

    rsnrp_status = viClose (instrSession);
    viClose (rmSession);

    return rsnrp_status;
}

/*****************************************************************************/
/*= UTILITY ROUTINES (Non-Exportable Functions) =============================*/
/*****************************************************************************/

/*===========================================================================*/
/* Function: Send Command                                                    */
/* Purpose:  This function sends command to the instrument with error check. */
/*===========================================================================*/
ViStatus rsnrp_sendCmd (ViSession instrSession, ViString cmd)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViChar      wrt_buf[BUFFER_SIZE]="";

    sprintf(wrt_buf,"%s",cmd);

    CHECKERR(viWrite (instrSession, (ViPBuf)wrt_buf, strlen(wrt_buf),VI_NULL));
    CHECKERR(rsnrp_checkStatus(instrSession));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Send Command With Channel                                       */
/* Purpose:  This function sends command to the instrument with error check. */
/*===========================================================================*/
ViStatus rsnrp_sendCmdChannel (ViSession instrSession, ViString cmd, 
                               ViInt32 channel)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViChar      wrt_buf[BUFFER_SIZE]="";

    sprintf(wrt_buf,"%s",cmd);

    CHECKERR(viPrintf (instrSession, wrt_buf, channel));
    CHECKERR(rsnrp_checkStatus(instrSession));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set ViInt32                                                     */
/* Purpose:  This function sets command with integer parameter.              */
/*===========================================================================*/
ViStatus rsnrp_setViInt32 (ViSession instrSession,
                           ViString cmd,
                           ViInt32 value,
                           ViInt32 min,
                           ViInt32 max,
                           ViStatus parameterErrorCode)
{
    ViChar      wrt_buf[BUFFER_SIZE];

    if ((max - min) > 0)
        if (rsnrp_invalidViInt32Range (value, min, max))
            return parameterErrorCode;

    sprintf (wrt_buf, "%s %ld", cmd, value);

    return rsnrp_sendCmd(instrSession, wrt_buf);
}

/*===========================================================================*/
/* Function: Set ViInt32 With Channel                                        */
/* Purpose:  This function sets command with integer parameter.              */
/*===========================================================================*/
ViStatus rsnrp_setViInt32Channel (ViSession instrSession,
                           ViString cmd,
                           ViInt32 channel,
                           ViInt32 value,
                           ViInt32 min,
                           ViInt32 max,
                           ViStatus parameterErrorCode)
{
    ViChar      wrt_buf[BUFFER_SIZE];

    sprintf(wrt_buf,cmd, channel);

    return rsnrp_setViInt32(instrSession,
                             wrt_buf,
                             value,
                             min,
                             max,
                             parameterErrorCode);
}

/*===========================================================================*/
/* Function: Get ViInt32                                                     */
/* Purpose:  This function gets command with integer parameter.              */
/*===========================================================================*/
ViStatus rsnrp_getViInt32 (ViSession instrSession,
                           ViString cmd,
                           ViInt32 *value,
                           ViStatus parameterErrorCode)
{
    ViStatus    rsnrp_status = VI_SUCCESS;

    if (value == VI_NULL)
        return parameterErrorCode;

    CHECKERR(viPrintf(instrSession,"%s",cmd));
    CHECKERR(viScanf(instrSession, "%ld", value));
    CHECKERR(rsnrp_checkStatus (instrSession));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get ViInt32 With Channel                                        */
/* Purpose:  This function gets command with integer parameter.              */
/*===========================================================================*/
ViStatus rsnrp_getViInt32Channel (ViSession instrSession,
                           ViString cmd,
                           ViInt32 channel,
                           ViInt32 *value,
                           ViStatus parameterErrorCode)
{
    ViChar      wrt_buf[BUFFER_SIZE]="";

    sprintf(wrt_buf,cmd,channel);
    
    return rsnrp_getViInt32(instrSession,
                            wrt_buf,
                            value,
                            parameterErrorCode);
}

/*===========================================================================*/
/* Function: Get ViBoolean                                                   */
/* Purpose:  This function gets command with boolean parameter.              */
/*===========================================================================*/
ViStatus rsnrp_getViBoolean (ViSession instrSession,
                           ViString cmd,
                           ViBoolean *value,
                           ViStatus parameterErrorCode)
{
    ViStatus    rsnrp_status = VI_SUCCESS;

    if (value == VI_NULL)
        return parameterErrorCode;

    CHECKERR(viPrintf(instrSession,"%s",cmd));
    CHECKERR(viScanf(instrSession, "%hu",value));
    CHECKERR(rsnrp_checkStatus (instrSession));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get ViBoolean With Channel                                      */
/* Purpose:  This function gets command with boolean parameter.              */
/*===========================================================================*/
ViStatus rsnrp_getViBooleanChannel (ViSession instrSession,
                           ViString cmd,
                           ViInt32 channel,
                           ViBoolean *value,
                           ViStatus parameterErrorCode)
{
    ViChar      wrt_buf[BUFFER_SIZE]="";

    sprintf(wrt_buf,cmd,channel);

    return rsnrp_getViBoolean(instrSession,
                              wrt_buf,
                              value,
                              parameterErrorCode);
}

/*===========================================================================*/
/* Function: Set ViReal64                                                    */
/* Purpose:  This function sets command with double parameter.               */
/*===========================================================================*/
ViStatus rsnrp_setViReal64 (ViSession vi,
                            ViString cmd,
                            ViReal64 value,
                            ViReal64 min,
                            ViReal64 max,
                            ViStatus parameterErrorCode)
{
    ViChar      buffer[BUFFER_SIZE];

    /* do range checking only if there's difference between max and min */
    if ((max - min) > 1.0e-14)
        if (rsnrp_invalidViReal64Range (value, min, max))
            return parameterErrorCode;

    sprintf(buffer,"%s %Le",cmd, value);

    return rsnrp_sendCmd(vi, buffer);
}

/*===========================================================================*/
/* Function: Set ViReal64 With Channel                                       */
/* Purpose:  This function sets command with double parameter.               */
/*===========================================================================*/
ViStatus rsnrp_setViReal64Channel (ViSession instrSession,
                            ViString cmd,
                            ViInt32  channel,
                            ViReal64 value,
                            ViReal64 min,
                            ViReal64 max,
                            ViStatus parameterErrorCode)
{
    ViChar      wrt_buf[BUFFER_SIZE];

    sprintf(wrt_buf,cmd, channel);

    return rsnrp_setViReal64(instrSession,
                             wrt_buf,
                             value,
                             min,
                             max,
                             parameterErrorCode);
}

/*===========================================================================*/
/* Function: Get ViReal64                                                    */
/* Purpose:  This function gets command with double parameter.               */
/*===========================================================================*/
ViStatus rsnrp_getViReal64 (ViSession instrSession,
                            ViString cmd,
                            ViReal64 *value,
                            ViStatus parameterErrorCode)
{
    ViStatus    rsnrp_status = VI_SUCCESS;

    if (value == VI_NULL)
        return parameterErrorCode;

    CHECKERR(viPrintf(instrSession, "%s",cmd));
    CHECKERR(viScanf (instrSession, "%Le",value));
    CHECKERR(rsnrp_checkStatus (instrSession));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get ViReal64 With Channel                                       */
/* Purpose:  This function gets command with double parameter.               */
/*===========================================================================*/
ViStatus rsnrp_getViReal64Channel (ViSession instrSession,
                            ViString cmd,
                            ViInt32 channel,
                            ViReal64 *value,
                            ViStatus parameterErrorCode)
{
    ViChar      wrt_buf[BUFFER_SIZE];

    sprintf(wrt_buf,cmd, channel);

    return rsnrp_getViReal64(instrSession,
                             wrt_buf,
                             value,
                             parameterErrorCode);
}

/*===========================================================================*/
/* Function: Get ViReal64 Array                                              */
/* Purpose:  This function sends the command and reads the double array      */
/*===========================================================================*/
ViStatus rsnrp_getViReal64Array (ViSession instrSession,
                            ViString cmd,
                            ViReal64 *value,
                            ViInt32  count,
                            ViStatus parameterErrorCode)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViChar*     pbuffer = VI_NULL;
    ViChar*     p2buf;
    ViUInt32    local_count;
    ViInt32     i = 0;

    if (value == VI_NULL)
        return parameterErrorCode;

    CHECKERR(viPrintf(instrSession, "%s",cmd));
    CHECKERR(rsnrp_readDataUnknownLength(instrSession, &pbuffer, &local_count));
    p2buf = pbuffer;
    
    p2buf = strtok(pbuffer,",");
    while(p2buf)
    {
        value[i++] = atof(p2buf);
        p2buf = strtok(NULL,",");
    };

    free(pbuffer);
    CHECKERR(rsnrp_checkStatus (instrSession));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get ViReal64 Array With Channel                                 */
/* Purpose:  This function sends the command and reads the double array      */
/*===========================================================================*/
ViStatus rsnrp_getViReal64ArrayChannel (ViSession instrSession,
                                        ViString cmd,
                                        ViInt32 channel,
                                        ViReal64 *value,
                                        ViInt32  count,
                                        ViStatus parameterErrorCode)
{
    ViChar      wrt_buf[BUFFER_SIZE];

    sprintf(wrt_buf,cmd, channel);

    return rsnrp_getViReal64Array(instrSession,
                             wrt_buf,
                             value,
                             count,
                             parameterErrorCode);
}

/*===========================================================================*/
/* Function: Get ViReal64 Binary Array With Channel                          */
/* Purpose:  This function sends the command and reads the double array      */
/*===========================================================================*/
ViStatus rsnrp_getViReal64BinArrayChannel (ViSession instrSession,
                                        ViString cmd,
                                        ViInt32 channel,
                                        ViReal64 *value,
                                        ViInt32  count,
                                        ViStatus parameterErrorCode)
{
    ViStatus    rsnrp_status;
    ViInt32     length;
    ViBuf       pbuffer;
    ViChar      buffer[BUFFER_SIZE];
    ViBoolean   old_term;

    if (!value)
        return parameterErrorCode;
        
    CHECKERR(viPrintf(instrSession,cmd, channel));

    CHECKERR(viGetAttribute(instrSession, VI_ATTR_TERMCHAR_EN, &old_term));
    CHECKERR(viSetAttribute(instrSession, VI_ATTR_TERMCHAR_EN, VI_FALSE));

    /* Read header */
    CHECKERR(viRead(instrSession,(ViPBuf)buffer, 2,VI_NULL));
    sscanf(buffer,"#%ld",&length);
    
    CHECKERR(viRead(instrSession, (ViPBuf)buffer, length, VI_NULL));
    
    length = atol(buffer);
    
    pbuffer = (ViBuf)malloc(length);
    rsnrp_status = viRead(instrSession,pbuffer,length,VI_NULL);

    if ((rsnrp_status == VI_SUCCESS) || (rsnrp_status == VI_SUCCESS_MAX_CNT))
    {
        memcpy(value,
               pbuffer,
               (count < (length/8)) ? count * 8: length);

        /* Discard remaining data */
        while(viRead(instrSession, (ViPBuf)buffer, BUFFER_SIZE, VI_NULL) == VI_SUCCESS_MAX_CNT);
    }

    free (pbuffer);

    CHECKERR(viSetAttribute(instrSession, VI_ATTR_TERMCHAR_EN, old_term));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set Enum                                                        */
/* Purpose:  This function sets command with enumerated parameter.           */
/*===========================================================================*/
ViStatus rsnrp_setEnum (ViSession instrSession,
                        ViString cmd,
                        ViInt32 value,
                        ViString _VI_FAR list[],
                        ViStatus parameterErrorCode)
{
    ViStatus rsnrp_status = VI_SUCCESS;

    CHECKERR (rsnrp_checkIndexInList (instrSession, value, list, parameterErrorCode));

    return rsnrp_setViString (instrSession, cmd, list[value], parameterErrorCode);
}

/*===========================================================================*/
/* Function: Set Enum With Channel                                           */
/* Purpose:  This function sets command with enumerated parameter.           */
/*===========================================================================*/
ViStatus rsnrp_setEnumChannel (ViSession instrSession,
                        ViString cmd,
                        ViInt32 channel,
                        ViInt32 value,
                        ViString _VI_FAR list[],
                        ViStatus parameterErrorCode)
{
    ViStatus rsnrp_status = VI_SUCCESS;

    CHECKERR (rsnrp_checkIndexInList (instrSession, value, list, parameterErrorCode));

    return rsnrp_setViStringChannel (instrSession, cmd, channel, list[value], parameterErrorCode);
}

/*===========================================================================*/
/* Function: Get Enum                                                        */
/* Purpose:  This function sets command with enumerated parameter.           */
/*===========================================================================*/
ViStatus rsnrp_getEnum (ViSession instrSession,
                        ViString cmd,
                        ViInt32 *value,
                        ViString _VI_FAR list[],
                        ViStatus parameterErrorCode)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViChar      buffer[BUFFER_SIZE];


    CHECKERR(rsnrp_getViString(instrSession, cmd, buffer, BUFFER_SIZE, parameterErrorCode));

    CHECKERR(rsnrp_getIndexFromString (instrSession, buffer, list, value));

    return rsnrp_status;

}

/*===========================================================================*/
/* Function: Get Enum Channel                                                */
/* Purpose:  This function sets command with enumerated parameter.           */
/*===========================================================================*/
ViStatus rsnrp_getEnumChannel (ViSession instrSession,
                        ViString cmd,
                        ViInt32 channel,
                        ViInt32 *value,
                        ViString _VI_FAR list[],
                        ViStatus parameterErrorCode)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViChar      buffer[BUFFER_SIZE];


    CHECKERR(rsnrp_getViStringChannel(instrSession, cmd, channel, buffer, BUFFER_SIZE, parameterErrorCode));

    CHECKERR(rsnrp_getIndexFromString (instrSession, buffer, list, value));

    return rsnrp_status;

}

/*===========================================================================*/
/* Function: Set String                                                      */
/* Purpose:  This function sets command with string parameter.               */
/*===========================================================================*/
ViStatus rsnrp_setViString (ViSession instrSession,
                            ViString cmd,
                            ViString value,
                            ViStatus parameterErrorValue)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViChar*     wrt_buf;
    ViChar*     p2buf;

    if (value == VI_NULL)
        return parameterErrorValue;

    wrt_buf = (ViChar*)malloc(strlen(cmd)+strlen(value)+3);
    p2buf = wrt_buf;
    p2buf += sprintf(wrt_buf,"%s %s",cmd, value);

    CHECKERR(viWrite (instrSession, (ViBuf)wrt_buf, p2buf - wrt_buf, VI_NULL));

    free(wrt_buf);

    CHECKERR(rsnrp_checkStatus(instrSession));
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Set String With Channel                                         */
/* Purpose:  This function sets command with string parameter.               */
/*===========================================================================*/
ViStatus rsnrp_setViStringChannel (ViSession instrSession,
                            ViString cmd,
                            ViInt32 channel,
                            ViString value,
                            ViStatus parameterErrorValue)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViChar*     wrt_buf;
    ViChar*     p2buf;

    if (value == VI_NULL)
        return parameterErrorValue;

    wrt_buf = (ViChar*)malloc(strlen(cmd)+strlen(value)+10);
    p2buf = wrt_buf;
    p2buf += sprintf(wrt_buf, cmd, channel);
    p2buf += sprintf(p2buf," %s", value);

    rsnrp_status = viWrite (instrSession, (ViBuf)wrt_buf, p2buf - wrt_buf, VI_NULL);
    free(wrt_buf);

    CHECKERR(rsnrp_status);

    CHECKERR(rsnrp_checkStatus(instrSession));
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get String                                                      */
/* Purpose:  This function gets command with string parameter.               */
/*===========================================================================*/
ViStatus rsnrp_getViString (ViSession instrSession,
                            ViString _VI_FAR cmd,
                            ViString _VI_FAR value,
                            ViInt32 buffer_size,
                            ViStatus parameterErrorCode)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViChar      buffer[BUFFER_SIZE];

    if (value == VI_NULL)
        return parameterErrorCode;

    CHECKERR(viPrintf(instrSession,"%s", cmd));
    CHECKERR(viScanf(instrSession,"%[^\r\n]",buffer));
    
    strncpy(value,buffer,buffer_size);

    CHECKERR(rsnrp_checkStatus (instrSession));
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Get String with Channel                                         */
/* Purpose:  This function gets command with string parameter.               */
/*===========================================================================*/
ViStatus rsnrp_getViStringChannel (ViSession instrSession,
                                ViString _VI_FAR cmd,
                                ViInt32 channel,
                                ViString _VI_FAR value,
                                ViInt32 buffer_size,
                                ViStatus parameterErrorCode)
{
    ViChar      wrt_buf[BUFFER_SIZE];

    sprintf(wrt_buf,cmd, channel);

    return rsnrp_getViString(instrSession,
                             wrt_buf,
                             value,
                             buffer_size,
                             parameterErrorCode);
}

/*===========================================================================*/
/* Function: Check Index List                                                */
/* Purpose:  This function checks if the passed index is valid for given list*/
/*===========================================================================*/
ViStatus rsnrp_checkResolution (ViReal64 resolution, ViStatus statusIfError)
{
    ViReal64    list[] = {1,0.1,0.01,0.001,-1};
    ViInt32     i;

    if (resolution < 0)
        return statusIfError;

    for(i=0; list[i] != -1; i++);

    if (list[i] == -1)
        return statusIfError;
    else
        return VI_SUCCESS;
}

/*===========================================================================*/
/* Function: Check Index List                                                */
/* Purpose:  This function checks if the passed index is valid for given list*/
/*===========================================================================*/
ViStatus rsnrp_checkIndexInList (ViSession instrSession,
                                 ViInt32 index,
                                 ViString list[],
                                 ViStatus statusIfError)

{
    ViInt32     i=0;

    if (index < 0)
        return statusIfError;

    while (list[i++] != VI_NULL);
    if (index >= i)
        return statusIfError;

    return VI_SUCCESS;
}

/*===========================================================================*/
/* Function: Get Index From String                                           */
/* Purpose:  This function returns index for string.                         */
/*===========================================================================*/
ViStatus rsnrp_getIndexFromString (ViSession instrSession,
                                   ViString string,
                                   ViString *list,
                                   ViInt32 *index)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViUInt32    i,
                len;

    for (i = 0; list[i]; i++)
        if ((len = strlen (string)) == strlen (list[i])) {
            if (strncmp(string, list[i], strlen (string)) == 0) break;
        }

    *index = i;

    return (list[i]) ? rsnrp_status : VI_ERROR_INSTR_INTERPRETING_RESPONSE;
}

/*===========================================================================*/
/* Function: Delay
/* Purpose : With a call to rsnrp_delay, the current program is suspended
/*           from execution for the number of seconds specified by the
/*           argument seconds. rsnrp_delay is accurate to a millisecond.
/*           This function is platform independent.
/*===========================================================================*/
void rsnrp_delay (ViReal64 numberOfSeconds)
{
  clock_t StartTime = clock();
  
  while ((((double)(clock() - StartTime))/CLOCKS_PER_SEC) < (numberOfSeconds));

  return;
}

/*===========================================================================*/
/* Function: Array Item to Const                                             */
/* Purpose:  Search for item in array and return index. Last element of array*/
/*           must be NULL pointer.                                           */
/*===========================================================================*/
ViStatus rsnrp_itemToConst (ViChar **array, ViString buffer, ViPInt32 itemNo)
{
    for ( *itemNo = 0; array[*itemNo]; (*itemNo)++ )
        if (!strcmp (buffer, array[*itemNo]))
            break;

    return (!array[*itemNo]) ? VI_ERROR_INSTR_INTERPRETING_RESPONSE : VI_SUCCESS;
}

/*===========================================================================*/
/* Function: Register Mask Range                                             */
/* Purpose:  This function checks a mask whether it can be applied on the    */
/*           selected register.                                              */
/*===========================================================================*/
ViBoolean rsnrp_invalidMask (ViUInt32 mask, ViInt32 class)
{
    ViInt32 min,max;

    switch(class)
    {
        case RSNRP_STATCLASS_O_LOWER:
        case RSNRP_STATCLASS_O_UPPER:
        case RSNRP_STATCLASS_Q_WINDOW:
            min = RSNRP_WINDOW_01;
            max = RSNRP_ALL_WINDOWS;
            break;
        default:
            min = RSNRP_SENSOR_01;
            max = RSNRP_ALL_SENSORS;
    }
    return rsnrp_invalidViInt32Range(mask,min,max);
}


/*===========================================================================*/
/* Function: Register Mask Range                                             */
/* Purpose:  This function checks a mask whether it can be applied on the    */
/*           selected register.                                              */
/*===========================================================================*/
ViUInt32 rsnrp_translateMask (ViUInt32 mask, ViInt32 class)
{
    ViUInt32    local_mask;

    switch(class)
    {
        case RSNRP_STATCLASS_D_CONN:
            local_mask = mask & 6;
            local_mask |= ((mask & 24) >> 4);
            break;
        case RSNRP_STATCLASS_D_ERR:
            local_mask = mask & 6;
            local_mask |= ((mask & 24) >> 4);
            local_mask = local_mask >> 2;
            break;
        case RSNRP_STATCLASS_Q_ZER:
            local_mask = (mask >> 4);
            break;
        default:
            local_mask = mask;
    }
    return local_mask;
}

/*===========================================================================*/
/* Function: Channel Value Out Of Range                                      */
/* Purpose:  This function checks a channel value  if it is in the range.    */
/*           If the value is out of range, the return value is               */
/*           VI_TRUE, otherwise the return value is VI_FALSE.                */
/*===========================================================================*/
ViStatus rsnrp_invalidChannelRange (ViSession instrSession, ViInt32 channel,
                                    ViStatus errorCode)
{
    ViStatus    rsnrp_status;
    GETSESSIONDATA(instrSession);
    
    rsnrp_status = ((channel < 1) ||
                    !(instrPtr->channelMask & (1 << (channel - 1))))
                   ? errorCode : VI_SUCCESS;
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Boolean Value Out Of Range - ViBoolean                          */
/* Purpose:  This function checks a Boolean to see if it is equal to VI_TRUE */
/*           or VI_FALSE. If the value is out of range, the return value is  */
/*           VI_TRUE, otherwise the return value is VI_FALSE.                */
/*===========================================================================*/
ViBoolean rsnrp_invalidViBooleanRange (ViBoolean val)
{
    return (ViBoolean)((val != VI_FALSE && val != VI_TRUE) ? VI_TRUE : VI_FALSE);
}

/*===========================================================================*/
/* Function: Long Signed Integer Value Out Of Range - ViInt32                */
/* Purpose:  This function checks a long signed integer value to see if it   */  
/*           lies between a minimum and maximum value.  If the value is out  */
/*           of range, the return value is VI_TRUE, otherwise the return     */
/*           value is VI_FALSE.                                              */
/*===========================================================================*/
ViBoolean rsnrp_invalidViInt32Range (ViInt32 val, ViInt32 min, ViInt32 max)
{
    return (ViBoolean)((val < min || val > max) ? VI_TRUE : VI_FALSE);
}

/*===========================================================================*/
/* Function: Real (Double) Value Out Of Range - ViReal64                     */
/* Purpose:  This function checks a real (double) value to see if it lies    */  
/*           between a minimum and maximum value.  If the value is out of    */
/*           range, the return value is VI_TRUE, otherwise the return value  */
/*           is VI_FALSE.                                                    */
/*===========================================================================*/
ViBoolean rsnrp_invalidViReal64Range (ViReal64 val, ViReal64 min, ViReal64 max)
{
    return (ViBoolean)((val < min || val > max) ? VI_TRUE : VI_FALSE);
}

/*===========================================================================*/
/* Function: Check String                                                    */
/* Purpose:  This function checks a string value to see if it is             */
/*           of a defined length and initialized.                            */
/*===========================================================================*/
ViBoolean rsnrp_invalidViString (ViString val, ViUInt32 maxLength)
{
    if (val)
        return (ViBoolean)((strlen (val) > maxLength) ? VI_TRUE : VI_FALSE);

    return VI_TRUE;
}

/*================================================================================*/
/* Function: Wait On VISA Event                                                   */
/* Purpose:  This function waits for the occurrence of an enabled VISA Event. The */
/*           event type must be enabled before entering this function. Any        */
/*           programmatic commands, actions, or conditions, necessary to generate */
/*           the specified event type, must be satisified before entering this    */
/*           function. The function will wait for the specified timeout and then  */
/*           return. If the specified event is received then the function will    */
/*           return VI_SUCCESS (0), otherwise the status code for the generated   */
/*           error will be returned. In either case the function will disable the */
/*           event type and deallocate the event handle passed from viWaitOnEvent */
/*           before returning. WARNING: If VI_TMO_INFINITE is passed in as the    */
/*           timeout this function WILL NOT return until the specified event is   */
/*           received, if the event is not received the function will not return  */
/*           and it will be necessary to terminate CVI in order to regain control.*/
/*================================================================================*/
ViStatus rsnrp_waitOnVisaEvent (ViSession instrSession,
                                 ViEventType eventTypeIn, 
                                 ViUInt32 timeoutIn, ViPUInt16 STB)
{            
    ViStatus    rsnrp_status = VI_SUCCESS,
                tempStatus = VI_SUCCESS;
    ViEventType eventTypeOut = 0;
    ViEvent     eventHandle = 0;

    /* For debug purposes we want to be able to see the status returned by        */
    /* viDisableEvent and viClose if one of the internal functions fails but do   */
    /* not want to return that value as that is not where the initial error       */
    /* occurs in the function, so we assign it to tempStatus.                     */
    
    if ((rsnrp_status = viWaitOnEvent (instrSession, eventTypeIn, timeoutIn, 
                                        &eventTypeOut, &eventHandle)) < 0)
        {
        tempStatus = viDisableEvent (instrSession, eventTypeIn, VI_QUEUE);
        return rsnrp_status;
        } 
    
    if (eventTypeIn == VI_EVENT_SERVICE_REQ)
        if ((rsnrp_status = viReadSTB (instrSession, STB)) < 0)
            {
            tempStatus = viClose (eventHandle);
            tempStatus = viDisableEvent (instrSession, eventTypeIn, VI_QUEUE);
            return rsnrp_status;
            }   
        
    if ((rsnrp_status = viClose (eventHandle)) < 0)
        {
        tempStatus = viDisableEvent (instrSession, eventTypeIn, VI_QUEUE);
        return rsnrp_status;                 
        }
    
    if ((rsnrp_status = viDisableEvent (instrSession, eventTypeIn, VI_QUEUE)) < 0)
        return rsnrp_status;                                                   

    return rsnrp_status;                                    
}    

/*===========================================================================*/
/* Function: Initialize Clean Up                                             */
/* Purpose:  This function is used only by the rsnrp_init function.  When   */
/*           an error is detected this function is called to close the       */
/*           open resource manager and instrument object sessions and to     */
/*           set the instrSession that is returned from rsnrp_init to       */
/*           VI_NULL.                                                        */
/*===========================================================================*/
ViStatus rsnrp_initCleanUp (ViSession openRMSession,
                             ViPSession openInstrSession,
                             ViStatus currentStatus)
{
    rsnrp_instrRange instrPtr;
    
    if (viGetAttribute (*openInstrSession, VI_ATTR_USER_DATA, &instrPtr) >= 0)
        if (instrPtr != NULL) 
            free (instrPtr);

    viClose (*openInstrSession);
    viClose (openRMSession);
    *openInstrSession = VI_NULL;
    
    return currentStatus;
}

/*===========================================================================*/
/* Function: Read To File From Instrument                                    */
/* Purpose:  This function is used to read data from the instrument and      */
/*           write it to a user specified file.                              */
/*===========================================================================*/
ViStatus rsnrp_readToFile (ViSession instrSession, ViString filename,
                            ViUInt32 readBytes, ViPUInt32 retCount)
{
    ViStatus  rsnrp_status = VI_SUCCESS;
    ViByte    buffer[BUFFER_SIZE];
    ViUInt32  bytesReadInstr = 0, bytesWrittenFile = 0;
    FILE     *targetFile;

    *retCount = 0L;
    if ((targetFile = fopen (filename, "wb")) == VI_NULL)
        return VI_ERROR_INSTR_FILE_OPEN; /* not defined by VTL */

    for (;;)
        {
        if (readBytes > BUFFER_SIZE)
            rsnrp_status = viRead (instrSession, buffer, BUFFER_SIZE, &bytesReadInstr);
        else
            rsnrp_status = viRead (instrSession, buffer, readBytes, &bytesReadInstr);

        bytesWrittenFile = fwrite (buffer, sizeof (ViByte), (size_t)bytesReadInstr, targetFile);
        *retCount += bytesWrittenFile;
        if (bytesWrittenFile < bytesReadInstr)
            rsnrp_status = VI_ERROR_INSTR_FILE_WRITE; /* not defined by VTL */

        if ((readBytes <= BUFFER_SIZE) || (rsnrp_status <= 0) || (rsnrp_status == VI_SUCCESS_TERM_CHAR))
            break;

        readBytes -= BUFFER_SIZE;
        }

    fclose (targetFile);
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Write From File To Instrument                                   */
/* Purpose:  This function is used to read data from a user specified file   */
/*           and write it to the instrument.                                 */
/*===========================================================================*/
ViStatus rsnrp_writeFromFile (ViSession instrSession, ViString filename,
                               ViUInt32 writeBytes, ViPUInt32 retCount)
{
    ViStatus  rsnrp_status = VI_SUCCESS;
    ViByte    buffer[BUFFER_SIZE];
    ViUInt32  bytesRead = 0, bytesWritten = 0;
    FILE     *sourceFile;
    ViBoolean sendEnd = VI_FALSE;

    *retCount = 0L;
    if ((sourceFile = fopen (filename, "rb")) == VI_NULL)
        return VI_ERROR_INSTR_FILE_OPEN; /* not defined by VTL */

    while (!feof (sourceFile))
        {
        bytesRead = (ViUInt32)fread (buffer, sizeof (ViByte), BUFFER_SIZE, sourceFile);
        if ((writeBytes > BUFFER_SIZE) && (bytesRead == BUFFER_SIZE))
            {
            viGetAttribute (instrSession, VI_ATTR_SEND_END_EN, &sendEnd);
            viSetAttribute (instrSession, VI_ATTR_SEND_END_EN, VI_FALSE);
            rsnrp_status = viWrite (instrSession, buffer, BUFFER_SIZE, &bytesWritten);
            viSetAttribute (instrSession, VI_ATTR_SEND_END_EN, sendEnd);
            writeBytes -= BUFFER_SIZE;
            *retCount += bytesWritten;
            if (rsnrp_status < 0)
                break;
            }
        else
            {
            rsnrp_status = viWrite (instrSession, buffer, ((bytesRead < writeBytes) ? bytesRead : writeBytes), &bytesWritten);
            *retCount += bytesWritten;
            break;
            }
        }

    fclose (sourceFile);
    return rsnrp_status;
}

/*****************************************************************************/
/*----------- INSERT INSTRUMENT-DEPENDENT UTILITY ROUTINES HERE -------------*/
/*****************************************************************************/

/*===========================================================================*/
/* Function: Default Instrument Setup                                        */
/* Purpose:  This function sends a default setup to the instrument.  This    */
/*           function is called by the rsnrp_reset operation and by the     */
/*           rsnrp_init function if the reset option has not been           */
/*           selected.  This function is useful for configuring any          */
/*           instrument settings that are required by the rest of the        */
/*           instrument driver functions such as turning headers ON or OFF   */
/*           or using the long or short form for commands, queries, and data.*/                                    
/*===========================================================================*/
ViStatus rsnrp_defaultInstrSetup (ViSession instrSession)
{
    ViInt32             connected;
    ViStatus            rsnrp_status = VI_SUCCESS;
    ViChar              buffer[BUFFER_SIZE];
    ViInt32             idx = 0;
    rsnrp_instrRange    instrPtr;

    /* Determine if the structure has been initialized for the current VISA  */
    /* Session and malloc if it has not.                                     */
    CHECKERR (viGetAttribute (instrSession, VI_ATTR_USER_DATA, &instrPtr));

    /* reset all cached states */
    memset (instrPtr, 0, sizeof(struct rsnrp_statusDataRanges));

    /* Get firmware revision of instrument */
    CHECKERR( viQueryf (instrSession, "*IDN?", "%*[^,],%*[^,],%*[^,],%s", buffer));
    
    instrPtr->firmwareVersion = (ViUInt32) floor(atof(buffer) * 100);
    
    /*
        Following two ViInt32 variables are used to store information about
        currently connected instrument and its options. Each bit represents
        one instrument model or option.
    */
    instrPtr -> instrOption = 0x0;
    instrPtr -> errorChecking = VI_TRUE;

    /*--- Recognize Instrument options (use *OPT? command) --- */
    {
        static ViString optionsList[] =
            {
                "NRP-B1",
                "NRP-B2",
                "NRP-B3",
                "NRP-B4",
                "NRP-B5",
                "NRP-B6",

                VI_NULL
            };

        CHECKERR (viQueryf (instrSession, "*OPT?", "%[^\r\n]", buffer));

        for ( idx = 0; optionsList[idx]; idx++ )
            if (strstr (buffer, optionsList[idx]))
                instrPtr -> instrOption |= (1 << idx);
    }

    /* set the number of available channels in the session data */
    CHECKERR(viQueryf (instrSession, ":STAT:DEV:COND?", "%ld", &connected));
    connected &= 0x186;
    connected >>= 1;
    instrPtr->channelMask = 0;
    
    if (connected & 1)
    {
        instrPtr->channels++;
        instrPtr->channelMask |= 1 << 0;    /* Sensor A */
    }
    
    if (connected & 2)
    {
        instrPtr->channels++;
        instrPtr->channelMask |= 1 << 1;    /* Sensor B */
    }

    if (connected & 0x40)
    {
        instrPtr->channels++;
        instrPtr->channelMask |= 1 << 2;    /* Sensor C */
    }

    if (connected & 0x80)
    {
        instrPtr->channels++;
        instrPtr->channelMask |= 1 << 3;    /* Sensor D */
    }

#if 0   
    if (instrPtr->instrOption & 0x10) /* 0x10  Four channel interface */

        instrPtr->channels = 4;
    else
        if (instrPtr->instrOption & 0x2) /*0x2   Second channel interface */
            instrPtr->channels = 2;
        else
            instrPtr->channels = 1;
#endif

    /* swap bytes during the binary transmission */
    CHECKERR (rsnrp_sendCmd(instrSession, "FORM:BORD SWAP"));
    
    /* Set the status system to reflect driver's need */
    
    CHECKERR (rsnrp_sendCmd(instrSession, "*ESE 255"));
    CHECKERR (rsnrp_sendCmd(instrSession, "*SRE 32"));
    /* Calibrating Status Register - set events to indicate calibration 
       completed */
    CHECKERR (rsnrp_sendCmd(instrSession, "STAT:OPER:CAL:PTR 0"));
    CHECKERR (rsnrp_sendCmd(instrSession, "STAT:OPER:CAL:NTR 30"));

    /* Measuring Status Register - set events to indicate measurement 
       completed */
    CHECKERR (rsnrp_sendCmd(instrSession, "STAT:OPER:MEAS:PTR 0"));
    CHECKERR (rsnrp_sendCmd(instrSession, "STAT:OPER:MEAS:NTR 30"));

    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Select the table by number                                      */
/* Purpose:  This function selects the table stored in instrument's memory   */
/*           by assigned number.                                             */
/*===========================================================================*/
ViStatus rsnrp_selectTable (ViSession instrSession, ViInt32 tableNumber)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    ViChar*     p2buf;
    ViInt32     i;

    if (rsnrp_invalidViInt32Range(tableNumber,0,9))
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_getViString(instrSession,"MEM:CAT:TABL?",buffer,BUFFER_SIZE,
                               VI_ERROR_PARAMETER3));
    p2buf=strtok(buffer,"\"");
    p2buf=strtok(NULL,"\"");
    
    for (i=0;i<tableNumber;i++)
    {
        p2buf=strtok(NULL,"\"");
        p2buf=strtok(NULL,"\"");
    }
    p2buf=strtok(p2buf,",");

    if (!p2buf)
        return RSNRP_ERROR_TABLE_NOT_FOUND;
    else
        return rsnrp_setViString(instrSession,
                                   "MEM:TABL:SEL",
                                   p2buf,
                                   VI_ERROR_PARAMETER3);

}

/*===========================================================================*/
/* Function: Read and update cache                                           */
/* Purpose:  This function reads the measuring event register from instrument*/
/*           and updates the local cache                                     */
/*===========================================================================*/
ViStatus rsnrp_updateCache (ViSession instrSession, ViInt32 reg, ViUInt32 resetMask,
                            ViUInt32 setMask)
{
    ViStatus    rsnrp_status;
    ViUInt16    measuring_event_reg;
    rsnrp_tEventCache* pcache;
    ViUInt32*   pregister;
    
    GETSESSIONDATA(instrSession);
    
    CHECKERR(viPrintf(instrSession,"%s",gs_NrpRegs[reg].command));
    CHECKERR(viScanf(instrSession,"%hu",&measuring_event_reg));

    pcache = &(instrPtr->events);
    pregister = (ViUInt32*)(((ViByte*)pcache) + (reg*sizeof(ViUInt32)));
    
    *pregister |= (ViUInt32)measuring_event_reg;
    /* set values defined by mask */
    *pregister |= setMask;
    /* reset values defined by mask */
    *pregister &= ~resetMask;

    /* update also duplicite registers */
    switch(reg)
    {
        case RSNRP_STATCLASS_D_CONN:
            pcache->device_error |= measuring_event_reg;
            break;
        case RSNRP_STATCLASS_D_ERR:
            pcache->device_connected |= measuring_event_reg;
            break;
        case RSNRP_STATCLASS_Q_ZER:
            pcache->q_power |= measuring_event_reg;
            break;
        case RSNRP_STATCLASS_Q_POWER:
            pcache->q_zero |= measuring_event_reg;
            break;
    }
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Read and update register                                        */
/* Purpose:  This function reads the register value and sets the bits defined*/
/*           by both mask to appropriate states                              */
/*===========================================================================*/
ViStatus rsnrp_updateRegister (ViSession instrSession, ViString reg, ViUInt32 resetMask,
                            ViUInt32 setMask)
{
    ViStatus    rsnrp_status;
    ViInt32     current_value;
    ViChar      buffer[BUFFER_SIZE];

    strcpy(buffer,reg);
    strcat(buffer,"?");
    
    if (rsnrp_invalidViInt32Range(resetMask,0,32767))
        return VI_ERROR_PARAMETER3;
    if (rsnrp_invalidViInt32Range(setMask,0,32767))
        return VI_ERROR_PARAMETER4;
    if (!reg)
        return VI_ERROR_PARAMETER2;

    CHECKERR(rsnrp_getViInt32(instrSession,
                              buffer,
                              &current_value,
                              0));
    
    current_value |= setMask;
    current_value &= ~resetMask;

    return rsnrp_setViInt32(instrSession,
                            reg,
                            current_value,
                            0,0,0);
}

/*===========================================================================*/
/* Function: Send High Level Command                                         */
/* Purpose:  This function sends the high level command with parameters to   */
/*           instrument                                                      */
/*===========================================================================*/
ViStatus rsnrp_sendHighLevelCommand (ViSession  instrSession, 
                                     ViString   cmd,
                                     ViInt32    measurement,
                                     ViInt32    measuringFunction,
                                     ViString   parameterList,
                                     ViReal64   *result,
                                     ViInt32    count)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    ViChar*     p2buf;
    
    p2buf=buffer;
    
    if (rsnrp_invalidViInt32Range(measurement,1,MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
        
    if (rsnrp_invalidViInt32Range(measuringFunction,0,10))
        return VI_ERROR_PARAMETER3;

    p2buf += sprintf(buffer, cmd, measurement, measFuncArr[measuringFunction]);
    
    p2buf += sprintf(p2buf,
                     " %s",
                     parameterList);
   
    if (!result)
        rsnrp_status = rsnrp_sendCmd(instrSession, buffer);
    else
        if (count == 1)
            rsnrp_status = rsnrp_getViReal64(instrSession,buffer,result,VI_ERROR_PARAMETER6);
        else
            rsnrp_status = rsnrp_getViReal64Array(instrSession,buffer,result,count,VI_ERROR_PARAMETER6);
    
    return rsnrp_status;
}

/*===========================================================================*/
/* Function: Read Data With Unknown length                                   */
/* Purpose:  This function allocates buffer and reads data from the          */
/*           instrument                                                      */
/* WARNING:  You must deallocate buffer, if any pointer has been returned.   */
/*           This function does not work correctly, when you read bin data   */
/*           via serial interface                                            */
/*===========================================================================*/
ViStatus rsnrp_readDataUnknownLength(ViSession instrSession,
                                     ViChar **outBuffer,
                                     ViUInt32 *count)
{
    ViStatus rsnrp_status = VI_SUCCESS;
    ViUInt32 actSize, retCnt = 0;
    ViChar   *buffer = VI_NULL, *p2buffer;

    *outBuffer = VI_NULL;
    if (count) *count = 0;

    /* Allocate Buffer */
    actSize = BUFFER_SIZE;
    buffer = (ViChar *)calloc(actSize, sizeof(ViChar));
    if (buffer == VI_NULL) return VI_ERROR_ALLOC;

    p2buffer = buffer;
    do
        {
        if ((rsnrp_status = viRead (instrSession, (ViPBuf)p2buffer, BUFFER_SIZE, &retCnt)) < 0)
            {
            free(buffer);
            *outBuffer = VI_NULL;
            return rsnrp_status;
            }
        if (retCnt == BUFFER_SIZE) /* Buffer was small, reallocate it */
        {
            actSize += BUFFER_SIZE;
            buffer = (ViChar *)realloc(buffer, actSize);
            if (buffer == VI_NULL) return VI_ERROR_ALLOC;
            p2buffer = buffer + (actSize - BUFFER_SIZE); /* Set pointer to end of data in rallocated buffer */
        }
        else
        {
            *(p2buffer+retCnt) = '\0'; /* For sure set end of string on the end of data */
        }
       } while (rsnrp_status == 0x3FFF0006); /* if buffer was small, continue with next read */

    *outBuffer = buffer;
    if (count) *count = (actSize - BUFFER_SIZE) + retCnt;

    return rsnrp_status;
}

/*=========================================================================*/
/* Function: Read Math Expression                                          */
/* Purpose:  This function reads the math expression and returns both      */
/*           operands and operation number                                 */
/*=========================================================================*/
ViStatus rsnrp_getMathExpression(ViSession instrSession, ViInt32 measurement,
                                 ViInt32* operand1, ViInt32* operation, 
                                 ViInt32* operand2)
{
    ViStatus    rsnrp_status;
    ViChar      buffer[BUFFER_SIZE];
    ViChar*     p2buf;
    ViString    operation_array[]={"-","+","/","SWR","REFL","RLOS",VI_NULL};
    ViInt32     i = 0;
    
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    CHECKERR(rsnrp_getViStringChannel(instrSession,
                                      "CALC%ld:MATH?",
                                      measurement,
                                      buffer,
                                      BUFFER_SIZE,
                                      0));
    
    if (operation)
    {
        while(operation_array[i])
        {
            if (strstr(buffer,operation_array[i]))
                break;
            i++;
        }
        *operation = operation_array[i] ? i+1 : 0;
    }
    
    if (operand1)
    {
        p2buf = strtok(buffer,"(");
        p2buf = strtok(NULL,"(");
        if (sscanf(p2buf,"SENS%1ld",operand1) != 1)
        {
            *operand1 = 0;
            return VI_ERROR_INSTR_INTERPRETING_RESPONSE;
        }
    }

    if (operand2)
    {
        p2buf = strtok(buffer,"+-/,");
        p2buf = strtok(NULL,"+-/,");
        if (!p2buf)
        {
            *operand2 = 0;
            return VI_ERROR_INSTR_INTERPRETING_RESPONSE;
        }

        if (sscanf(p2buf,"SENS%1ld",operand2) != 1)
        {
            *operand2 = 0;
            return VI_ERROR_INSTR_INTERPRETING_RESPONSE;
        }
    }
    return rsnrp_status;
}

/*=========================================================================*/
/* Function: Read Math Ex Expression                                       */
/* Purpose:  This function reads the feed expression and returns evaluate  */
/*           modes and time gates.                                         */
/*=========================================================================*/
ViStatus rsnrp_getMathExExpression(ViSession instrSession, ViInt32 measurement,
                                   ViInt32* eval1, ViInt32* eval2, 
                                   ViInt32* gate1, ViInt32* gate2)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViChar      buffer[BUFFER_SIZE];
    ViString    eval_array[]={":TRAC",":AVER",":PTAV",":PEAK",VI_NULL};
    ViInt32     i = 0;
    
    if (rsnrp_invalidViInt32Range(measurement, 1, MAX_MEASUREMENTS))
        return VI_ERROR_PARAMETER2;
    
    CHECKERR(rsnrp_getViStringChannel(instrSession,
                                      "CALC%ld:FEED1?",
                                      measurement,
                                      buffer,
                                      BUFFER_SIZE,
                                      0));

    if (eval1)
    {
        /* Get evaluate type */
        while(eval_array[i])
        {
            if (strstr(buffer,eval_array[i]))
                break;
            i++;
        }
        *eval1 = eval_array[i] ? i : 0;
        
        if (eval_array[i] == VI_NULL)
            return VI_ERROR_INSTR_INTERPRETING_RESPONSE;
    }
    
    if (gate1)
    {
        /* parse gate number from string */
        ViChar* p2buf = strstr(buffer, "SWE");
        
        if (p2buf == VI_NULL)
            *gate1 = 0;
        else
        {
            /* skip first 3 characters (SWE) and find number value */
            p2buf += 3;
            
            while (p2buf)
            {
                if (isdigit(*p2buf))
                    break;
                
                p2buf++;
            }
            
            if (p2buf == VI_NULL)
                *gate1 = 1;
            else
                *gate1 = atoi(p2buf);
         }
    }

    CHECKERR(rsnrp_getViStringChannel(instrSession,
                                      "CALC%ld:FEED2?",
                                      measurement,
                                      buffer,
                                      BUFFER_SIZE,
                                      0));

    i = 0;

    if (eval2)
    {
        /* Get evaluate type */
        while(eval_array[i])
        {
            if (strstr(buffer,eval_array[i]))
                break;
            i++;
        }
        *eval2 = eval_array[i] ? i : 0;
        
        if (eval_array[i] == VI_NULL)
            return VI_ERROR_INSTR_INTERPRETING_RESPONSE;
    }
    
    if (gate2)
    {
        /* parse gate number from string */
        ViChar* p2buf = strstr(buffer, "SWE");
        
        if (p2buf == VI_NULL)
            *gate2 = 0;
        else
        {
            /* skip first 3 characters (SWE) and find number value */
            p2buf += 3;
            
            while (p2buf)
            {
                if (isdigit(*p2buf))
                    break;
                
                p2buf++;
            }
            
            if (p2buf == VI_NULL)
                *gate2 = 1;
            else
                *gate2 = atoi(p2buf);
         }
    }

    return rsnrp_status;
}

/*=========================================================================*/
/* Function: Check Status                                                  */
/* Purpose:  This function reads Event Status Register to check for        */
/*           possible errors.                                              */
/*=========================================================================*/
ViStatus rsnrp_checkStatus (ViSession instrSession)
{
    ViStatus            rsnrp_status = VI_SUCCESS;
    
    GETSESSIONDATA(instrSession);

    /* Error Checking Enabled? */
    if (instrPtr -> errorChecking)
        {
        /*--------------- Read the Standard Event Register ------------------*/
        CHECKERR(rsnrp_updateCache (instrSession,RSNRP_STATCLASS_ESR,0,0));

        /*---------------- Check if any error or message bit was asserted ---*/
        rsnrp_status = (instrPtr->events.esr & IEEE_ERROR_BITS) ? RSNRP_ERROR_INSTRUMENT_ERROR : VI_SUCCESS;

        /* clear read values */
        instrPtr->events.esr &= ~IEEE_ERROR_BITS;
        }

    return rsnrp_status;
}

/*=========================================================================*/
/* Function: Send Math Command                                             */
/* Purpose:  This function create and send math command.                   */
/*=========================================================================*/
ViStatus rsnrp_sendMathCommand(ViSession instrSession, ViInt32 measurement,
                               ViInt32 sensor1, ViInt32 operation,
                               ViInt32 sensor2)
{
    ViStatus    rsnrp_status = VI_SUCCESS;
    ViChar      buffer[BUFFER_SIZE];
    ViChar*     p2buf;
    GETSESSIONDATA(instrSession);
    
    p2buf = buffer;
    p2buf += sprintf(buffer, "CALC%ld:MATH ",measurement);
    
    switch(operation)
    {
        case RSNRP_MATH_VALUE:
            sprintf(p2buf,"\"(SENS%ld)\"", sensor1);
            break;
        case RSNRP_MATH_MINUS:
            sprintf(p2buf,"\"(SENS%ld-SENS%ld)\"", sensor1, sensor2);
            break;
        case RSNRP_MATH_PLUS:
            sprintf(p2buf,"\"(SENS%ld+SENS%ld)\"", sensor1, sensor2);
            break;
        case RSNRP_MATH_QUOTIENT:
            sprintf(p2buf,"\"(SENS%ld/SENS%ld)\"", sensor1, sensor2);
            break;
        case RSNRP_MATH_SWR:
            sprintf(p2buf,"\"SWR(SENS%ld,SENS%ld)\"", sensor1, sensor2);
            break;
        case RSNRP_MATH_REFL:
            sprintf(p2buf,"\"REFL(SENS%ld,SENS%ld)\"", sensor1, sensor2);
            break;
        case RSNRP_MATH_RLOSS:
            sprintf(p2buf,"\"RLOS(SENS%ld,SENS%ld)\"", sensor1, sensor2);
            break;
    }
    
    CHECKERR(rsnrp_sendCmd(instrSession, buffer));

    instrPtr->measurements[measurement].channel1 = sensor1;

    if (operation > RSNRP_MATH_VALUE)
        instrPtr->measurements[measurement].channel2 = sensor2;
        
    return rsnrp_status;
}

/*****************************************************************************/
/*=== END INSTRUMENT DRIVER SOURCE CODE =====================================*/
/*****************************************************************************/
