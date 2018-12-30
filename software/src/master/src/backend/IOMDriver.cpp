/*****  IOMDriver Implementation  *****/

/************************
 ************************
 *****  Interfaces  *****
 ************************
 ************************/

/********************
 *****  System  *****
 ********************/

#include "Vk.h"

/******************
 *****  Self  *****
 ******************/

#include "IOMDriver.h"

/************************
 *****  Supporting  *****
 ************************/

#include "batchvision.h"

#include "IOMHandle.h"

#include "RTclosure.h"
#include "RTstring.h"

#include "VChannelController.h"
#include "VComputationScheduler.h"

#include "Vca_VBS.h"
#include "Vca_VDeviceFactory.h"


/********************************
 ********************************
 *****                      *****
 *****  IOMDriver::Options  *****
 *****                      *****
 ********************************
 ********************************/

/**************************
 **************************
 *****  Construction  *****
 **************************
 **************************/

IOMDriver::Options::Options (Options const& rSource)
: m_fAutoMutex		(rSource.m_fAutoMutex)
, m_xPromptFormat	(rSource.m_xPromptFormat)
, m_xBinaryFormat	(rSource.m_xBinaryFormat)
, m_pEvaluator		(rSource.m_pEvaluator)
{
}

IOMDriver::Options::Options ()
: m_fAutoMutex		(true)
, m_xPromptFormat	(IOMPromptFormat_Normal)
, m_xBinaryFormat	(IOMBinaryFormat_Untranslated)
{
}

/*************************
 *************************
 *****  Destruction  *****
 *************************
 *************************/

IOMDriver::Options::~Options () {
}


/*******************
 *******************
 *****  Query  *****
 *******************
 *******************/

unsigned int IOMDriver::Options::optionValue (
    IOMHandle const* pHandle, IOMDriver const* Unused(pDriver)
) const {
    switch (pHandle->optionIndex ()) {
    case IOMOption_AutoMutex:
	return m_fAutoMutex;
    case IOMOption_BinaryFormat:
	return m_xBinaryFormat;
    case IOMOption_PromptFormat:
	return m_xPromptFormat;

    case IOMOption_TrimFormat:
	return pHandle->trimFormat ();
    case IOMOption_SeekOffset:
	return pHandle->seekOffset ();

    case IOMOption_TCPNODELAY: {
	    int optionValue;
	    pHandle->getTcpNodelay (&optionValue);
	    return optionValue;
	}
    default:
	return 0;
    }
}


/********************
 ********************
 *****  Update  *****
 ********************
 ********************/

void IOMDriver::Options::SetEvaluatorTo (VEvaluatorPump *pEvaluator) {
    m_pEvaluator.setTo (pEvaluator);
}

void IOMDriver::Options::SetOptionValue (
    IOMHandle* pHandle, IOMDriver *Unused(pDriver), unsigned int optionValue
)
{
    switch (pHandle->optionIndex ()) {
    case IOMOption_AutoMutex:
	m_fAutoMutex = optionValue;
	break;
    case IOMOption_BinaryFormat:
	m_xBinaryFormat = optionValue;
	break;
    case IOMOption_PromptFormat:
	m_xPromptFormat = optionValue;
	break;

    case IOMOption_TrimFormat:
	pHandle->setTrimFormat ((IOMTrimFormat)optionValue);
	break;
    case IOMOption_SeekOffset:
	pHandle->setSeekOffset (optionValue);
	break;
    case IOMOption_TCPNODELAY:
	pHandle->setTcpNodelay (optionValue);
	break;
    default:
	break;
    }
}


/***********************
 ***********************
 *****             *****
 *****  IOMDriver  *****
 *****             *****
 ***********************
 ***********************/

/***************************
 ***************************
 *****  Run Time Type  *****
 ***************************
 ***************************/

DEFINE_ABSTRACT_RTT (IOMDriver);

/**************************
 **************************
 *****  Construction  *****
 **************************
 **************************/

IOMDriver::IOMDriver (Options const& rOptions)
: m_iOptions		(rOptions)
, m_xState		(IOMState_Ready)
, m_iUseCount		(0)
, m_fHandlerWired	(true)
, m_fTracingEnabled	(false)
, m_fCheckpointsEnabled	(false)
{
    AttachToDriverTable ();
    retain ();
}

/*************************
 *************************
 *****  Destruction  *****
 *************************
 *************************/

IOMDriver::~IOMDriver () {
    DetachFromDriverTable ();
}


/********************
 ********************
 *****  Access  *****
 ********************
 ********************/

char const* IOMDriver::consumedStartupExpression () {
    return 0;
}

int IOMDriver::getPeerNameString (char** Unused(ppString), size_t* Unused(psString)) {
    m_iStatus.MakeFailureStatus ();
    return -1;
}

int IOMDriver::getSocketNameString (char** Unused(ppString), size_t* Unused(psString)) {
    m_iStatus.MakeFailureStatus ();
    return -1;
}

/********************************
 ********************************
 *****  Auto-Mutex Support  *****
 ********************************
 ********************************/

bool IOMDriver::acquireAutoMutex (VMutexClaim& rClaim, VComputationUnit* pSupplicant) {
    return !autoMutex () || ENVIR_KDsc_TheTLE.store ()->AcquireMutex (rClaim, pSupplicant);
}


/*********************
 *********************
 *****  Control  *****
 *********************
 *********************/

unsigned int IOMDriver::IncrementUseCount () {
    if (0 == m_iUseCount++)
	retain ();
    return m_iUseCount;
}

unsigned int IOMDriver::DecrementUseCount () {
    if (m_iUseCount > 0 && 0 == --m_iUseCount) {
	Close ();
	release ();
    }
    return m_iUseCount;
}


/*-----------------------------------------------------------------------*
 *  The enable and disable handler methods only control the operation of
 *  the master controller of a driver.
 *-----------------------------------------------------------------------*/
int IOMDriver::EnableHandler (VTask* pSpawningTask) {
    switch (m_xState) {
    case IOMState_Ready:
	m_xState = IOMState_Starting;
	m_pController.setTo (new VChannelController (pSpawningTask, this));
	m_pController->start ();
	return true;

    case IOMState_Starting:
    case IOMState_Restarting:
    case IOMState_Running:
	return true;

    default:
	return false;
    }
}

int IOMDriver::DisableHandler () {
    switch (m_xState) {
    case IOMState_Ready:
	return true;

    case IOMState_Starting:
    case IOMState_Restarting:
    case IOMState_Running:
	m_xState = IOMState_Ready;
	triggerAll ();
	return true;

    default:
	return false;
    }
}


void IOMDriver::SetStateTo (IOMState xState) {
    if (xState == m_xState)
	return;

    switch (m_xState) {
    case IOMState_Ready:
    case IOMState_Starting:
    case IOMState_Restarting:
    case IOMState_Running:
	m_xState = xState;
	break;

    default:
	break;
    }

    switch (m_xState) {
    case IOMState_Starting:
    case IOMState_Restarting:
    case IOMState_Running:
	break;

    default:
	m_pController = 0;
	triggerAll ();
	break;
    }
}


/*****************************
 *****************************
 *****  Data Conversion  *****
 *****************************
 *****************************/

void IOMDriver::ConvertByteArray (void *pArray, size_t sArray) {
    switch (binaryFormat ()) {
#if defined(_WIN32)
    case IOMBinaryFormat_PARisc:
    case IOMBinaryFormat_Sparc:
#else
    case IOMBinaryFormat_Intel:
#endif
	Vk_ReverseArray (pArray, sArray);
	break;

    default:
	break;
    }
}


/*******************
 *******************
 *****  Input  *****
 *******************
 *******************/

/******************************
 *****  Virtual Defaults  *****
 ******************************/

IOMDriver *IOMDriver::GetConnection () {
    return NilOf (IOMDriver*);
}

VkStatusType IOMDriver::InputOperationStatus () const {
    return VkStatusType_Failed; 
}


/**************************
 *****  Binary Input  *****
 **************************/

bool IOMDriver::GetByte (IOMHandle const* pHandle, unsigned char *value) {
    if (sizeof (*value) != GetData (pHandle, value, sizeof (*value)))
	return false;

    return true;
}

bool IOMDriver::GetShort (IOMHandle const* pHandle, unsigned short *value) {
    if (sizeof (*value) != GetData (pHandle, value, sizeof (*value)))
	return false;

    ConvertShort (value);
    return true;
}

bool IOMDriver::GetLong (IOMHandle const* pHandle, unsigned int *value) {
    if (sizeof (*value) != GetData (pHandle, value, sizeof (*value)))
	return false;

    ConvertLong (value);
    return true;
}

bool IOMDriver::GetFloat (IOMHandle const* pHandle, float *value) {
    if (sizeof (*value) != GetData (pHandle, value, sizeof (*value)))
	return false;

    ConvertFloat (value);
    return true;
}

bool IOMDriver::GetDouble (IOMHandle const* pHandle, double *value) {
    if (sizeof (*value) != GetData (pHandle, value, sizeof (*value)))
	return false;

    ConvertDouble (value);
    return true;
}


/******************
 *****  Wait  *****
 ******************/

/******************************************************************************
 * Routine IOMDriver::Wait:
 *  Waiting is done on the global event set for infinite time. Returns on
 * getting an any event. When using this method for waiting on pending input/output 
 * events of this driver, the user needs to have this Wait () method inside 
 * a loop and check whether the event that was fired was the one that was 
 * desired by checking the completion condition of the desired operation. 
 * (As used in io.cpp's IO_pfgets () method for getting input) 
 ******************************************************************************/
void IOMDriver::Wait () {
    PutBufferedData ();

    Vca::VCohortManager iCM; bool rHandledEvents = false;
    while (
	iCM.doEvents (
	    Batchvision::BlockingWait (), rHandledEvents
	) && !rHandledEvents
    ) {
    }
}


/********************
 ********************
 *****  Output  *****
 ********************
 ********************/

/***********************
 *****  Diversion  *****
 ***********************/

void IOMDriver::OpenOutputFile (
    char const* Unused(pFileName), bool Unused(fOutputRedirected)
) {
}

void IOMDriver::CloseOutputFile ()
{
}


/********************
 *****  Binary  *****
 ********************/

size_t IOMDriver::PutByte (IOMHandle const* pHandle, unsigned char value) {
    return PutData (pHandle, &value, sizeof (value));
}

size_t IOMDriver::PutShort (IOMHandle const* pHandle, unsigned short value) {
    ConvertShort (&value);
    return PutData (pHandle, &value, sizeof (value));
}

size_t IOMDriver::PutLong (IOMHandle const* pHandle, unsigned int value) {
    ConvertLong (&value);
    return PutData (pHandle, &value, sizeof (value));
}

size_t IOMDriver::PutFloat (IOMHandle const* pHandle, float value) {
    ConvertFloat (&value);
    return PutData (pHandle, &value, sizeof (value));
}

size_t IOMDriver::PutDouble (IOMHandle const* pHandle, double value) {
    ConvertDouble (&value);
    return PutData (pHandle, &value, sizeof (value));
}


/***********************
 *****  Character  *****
 ***********************/

size_t __cdecl IOMDriver::Print (size_t sData, char const* fmt, ...) {
    V_VARGLIST (arglist, fmt);
    return VPrint (sData, fmt, arglist);
}

size_t __cdecl IOMDriver::Report (size_t sData, char const* fmt, ...) {
    V_VARGLIST (arglist, fmt);
    return VReport (sData, fmt, arglist);
}

/******************************************************************************
 * Routine VReport:
 * This VReport method has been modified to support asynchronous writing of data.
 * To ensure that normal data writtern to 'stdout' and error data written by this
 * routine to 'stderr' appear in the expected order, checkpoint synchronization
 * is used when 'stdout' and 'stderr' are managed by different drivers (note that
 * checkpointing is not required when 'stdout' and 'stderr' are managed by the
 * same driver and would, in fact, deadlock if attempted).
 *
 *        (DataStream)            (ErrorStream)
 *
 *         | Data  |              |            |
 *         |_______|   Releases   |____________|
 *         |       |------------->| CP2        |
 *         | CP1   |              |____________|
 *         |_______|<-+           | Error Data |
 *         |       |  | Releases  |____________|
 *         |       |  +-----------| CP3        |
 *
 * CheckPointing:
 * In the above scenario, "BLOCKING" checkpoints 'CP1' and 'CP2' are first inserted
 * into the data and error streams respectively.  Both drivers are required to output
 * whatever data they had in their buffers prior to the creation of those checkpoints
 * but are prohibited from writing any new data that arrives after those checkpoints
 * until the checkpoint is released.  With these protective checkpoints in place, the
 * error data produced by this routine is written to the error stream followed by
 * "NONBLOCKING" checkpoint 'CP3'.
 *
 * Operationally, when the data stream has drained its pre-'CP1' data, 'CP1' is
 * triggered.  The handler for that trigger explicitly releases 'CP2', allowing
 * the error stream to drain the newly written error data.  Once that data has been
 * flushed, 'CP3' is triggered, releasing 'CP1' and the data stream it is blocking.
 * 
 * The order in which these checkpoints are created is important.  If no buffered
 * data stream data is present when 'CP1' is created, 'CP1' will be triggered
 * immediately.  To ensure that that triggering causes the release of 'CP2', 'CP2'
 * must exist when 'CP1' is created.
 *
 ******************************************************************************/

size_t IOMDriver::VReport (size_t sData, char const* fmt, va_list ap) {

    IOMDriver *const pErrorDriver = m_pErrorOutputDriver ? m_pErrorOutputDriver.operator->() : this;
    size_t result = 0;

    if (pErrorDriver != this && pErrorDriver->checkpointsEnabled ()) {
	//  create triggers that release checkpoints .....
	Vca::VTrigger<IOMDriver>::Reference const pErrorDriverTrigger (
	    new Vca::VTrigger<IOMDriver> (
                pErrorDriver, &IOMDriver::releaseBlockingCheckPoint
            )
	);

	Vca::VTrigger<IOMDriver>::Reference const pTrigger (
	    new Vca::VTrigger<IOMDriver> (
                this, &IOMDriver::releaseBlockingCheckPoint
            )
	);

	pErrorDriver->createCheckPoint (true);			//CP2
	createCheckPoint (true, pErrorDriverTrigger);	        //CP1

	result = pErrorDriver->VPrint (sData, fmt, ap); 
	pErrorDriver->createCheckPoint (false, pTrigger);	//CP3

	PutBufferedData ();	
	pErrorDriver->PutBufferedData ();
    }
    else {  // no checkpointing is done if same stream..
	PutBufferedData ();
	result = pErrorDriver->VPrint (sData, fmt, ap);
	pErrorDriver->PutBufferedData ();
    }
    return result;
}


/*********************************************
 *********************************************
 *****  Event Handling and Notification  *****
 *********************************************
 *********************************************/

void IOMDriver::untilInputArrivesSuspend (VComputationUnit* pSuspendable) {
    switch (m_xState) {
    case IOMState_Ready:
    case IOMState_Starting:
    case IOMState_Restarting:
    case IOMState_Running:
	m_iTrigger.suspend (pSuspendable);
	break;

    default:
	break;
    }
}

/************************
 ************************
 *****  TriggerSet  *****
 ************************
 ************************/

bool IOMDriver::insertIntoTriggerSet (Vca::ITrigger *pTrigger) {
    return m_iTriggerSet.Insert (pTrigger);
}

bool IOMDriver::deleteFromTriggerSet (Vca::ITrigger *pTrigger) {
    return m_iTriggerSet.Delete (pTrigger);
}

/*********************
 *********************
 *****  Trigger  *****
 *********************
 *********************/

/******************************************************************************
 * Routine: triggerAll
 *  This method triggers the computation units suspended on this IOMDriver, both
 * directly and via TriggerSet mechanism.
 * Resumes all the computation unit suspened directly. Iterates through the 
 * TriggerSet and fires the triggers which could lead to resumption of Computation 
 * Units.
******************************************************************************/

void IOMDriver::triggerAll () {
    m_iTrigger.triggerAll ();
    m_iTriggerSet.trigger ();
}
