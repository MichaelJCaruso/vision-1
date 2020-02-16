/*****  VTask Implementation  *****/

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

#include "VTask.h"

/************************
 *****  Supporting  *****
 ************************/

#include "verrdef.h"
#include "vfac.h"
#include "vfault.h"

#include "RTcharuv.h"
#include "RTdoubleuv.h"
#include "RTfloatuv.h"
#include "RTintuv.h"
#include "RTvstore.h"

#include "IOMDriver.h"

#include "VfGuardTool.h"

#include "VFragment.h"
#include "VNumericBinary.h"
#include "VOutputGenerator.h"
#include "V_VString.h"

#include "VBoundCall.h"
#include "VEvaluationCall.h"
#include "VPrimaryCall.h"

#include "VSymbol.h"
#include "VSymbolImplementation.h"

#include "Vxa_ICollection.h"
#include "Vxa_IVSNFTaskImplementation.h"


/***********************************
 ***********************************
 *****                         *****
 *****  VTaskConstructionData  *****
 *****                         *****
 ***********************************
 ***********************************/

/**************************
 **************************
 *****  Construction  *****
 **************************
 **************************/

VTaskConstructionData::VTaskConstructionData (
    VCall			*pCall,
    rtLINK_CType		*pSubset,
    M_CPD			*pReordering,
    rtCONTEXT_Handle		*pContext,
    unsigned int		iAttentionMask
) : m_pCall (pCall), m_pSubset (pSubset), m_pReordering (pReordering), m_pContext (pContext), m_iAttentionMask (iAttentionMask) {
}


/*******************
 *******************
 *****         *****
 *****  VTask  *****
 *****         *****
 *******************
 *******************/

/***************************
 ***************************
 *****  Run Time Type  *****
 ***************************
 ***************************/

DEFINE_ABSTRACT_RTT (VTask);

/************************
 ************************
 *****  Meta Maker  *****
 ************************
 ************************/

template class Vsi_cipath_c<VTask, VCall*>;
template class Vsi_ciref_c<VTask, VDescriptor>;
template class Vsi_uiref_c<VTask, DSC_Descriptor&>;
template class Vsi_uiref_c<VTask, VTaskDomain*>;
template class Vsi_uiref<VTask, DSC_Descriptor&>;
template class Vsi_f0_c<VTask, char const*>;
template class Vsi_b0_c<VTask>;

void VTask::MetaMaker () {
    static Vsi_cipath_c<VTask, VCall*> const
	si_call			(&VTask::getPathToCall);

    static Vsi_ciref_c<VTask, VDescriptor> const
	si_self			(&VTask::cgetSelf),
	si_current		(&VTask::cgetCurrent),
	si_my			(&VTask::cgetMy);

    static Vsi_uiref_c<VTask, DSC_Descriptor&> const
	si_date			(&VTask::getDate);

    static Vsi_uiref_c<VTask, VTaskDomain*> const
	si_domain		(&VTask::domain);

    static Vsi_uiref<VTask, DSC_Descriptor&> const
	si_local		(&VTask::getLocal);

    static Vsi_f0_c<VTask, char const*> const
	si_description		(&VTask::description);

    static Vsi_b0_c<VTask> const
	si_myAvailable		(&VTask::myAvailable);

    CSym ("isATask"		)->set (RTT, &g_siTrue);
    CSym ("isABlockTask"	)->set (RTT, &g_siFalse);
    CSym ("isAPrimitiveTask"	)->set (RTT, &g_siFalse);
    CSym ("isASNFTask"		)->set (RTT, &g_siFalse);
    CSym ("isATopTask"		)->set (RTT, &g_siFalse);
    CSym ("isAUtilityTask"	)->set (RTT, &g_siFalse);

    CSym ("myAvailable"		)->set (RTT, &si_myAvailable);

    CSym ("recipient"		)->set (RTT, &si_self);

    CSym ("self"		)->set (RTT, &si_self);
    CSym ("current"		)->set (RTT, &si_current);
    CSym ("my"			)->set (RTT, &si_my);
    CSym ("date"		)->set (RTT, &si_date);
    CSym ("local"		)->set (RTT, &si_local);

    CSym ("call"		)->set (RTT, &si_call);
    CSym ("domain"		)->set (RTT, &si_domain);

    CSym ("description"		)->set (RTT, &si_description);
}


/*********************
 *********************
 *****  Globals  *****
 *********************
 *********************/

/**********************
 *****  Counters  *****
 **********************/

unsigned int VTask::BlockTaskCount	= 0;
unsigned int VTask::PrimitiveTaskCount	= 0;
unsigned int VTask::LargeTaskCount	= 0;

unsigned int VTask::MaxTaskSize		= 0;

/************************
 *****  Parameters  *****
 ************************/

/**********************
 *****  Switches  *****
 **********************/

bool VTask::TracingExecution	= false;

/***********************
 *****  Constants  *****
 ***********************/

VSelector const VTask::g_iTopSelector;


/**************************
 **************************
 *****  Construction  *****
 **************************
 **************************/

/*---------------------------------------------------------------------------*
 *  Arguments:
 *	pCallerReordering	- an optional ('Nil' if absent) standard CPD
 *				  for a reference u-vector specifying the
 *				  reordering which must be applied to the
 *				  domain of the call that invoked this task.
 *				  Absence of this field indicates that no
 *				  reordering is necessary.  The reference
 *				  count of any supplied reordering will be
 *				  incremented as appropriate.
 *---------------------------------------------------------------------------*/
VTask::VTask (ConstructionData const &rTCData)
: BaseClass		(rTCData.call (), rTCData.caller (), rTCData.attentionMask ())
, m_pDomain		(callerDomain()->childDomain (rTCData.subset (), rTCData.reordering ()))
, m_pBlockContext	(rTCData.context ())
, m_pTemporalContext	(rTCData.subtaskTemporalContext ())
, m_pOutputBuffer	(m_pCaller->m_pOutputBuffer)
, m_xNextParameter	(rTCData.parameterCount ())
, m_pCodSpace		(0)
, m_cSegmentsExpected	(0)
, m_cSegmentsReceived	(0)
{
//  Look for task size outliers, ...
    unsigned int taskSize = ptoken ()->cardinality ();
    if (taskSize > largeTaskSize ())
        LargeTaskCount++;
    MaxTaskSize = V_Max (MaxTaskSize, taskSize);
}

VTask::VTask (
    Context*			pContext,
    VDescriptor&		rDatum,
    IOMDriver*			pChannel,
    VComputationScheduler*	pScheduler,
    VTask*			pSpawningTask
)
: VComputationUnit	(pContext, rDatum, pScheduler)
, m_pDomain		(pSpawningTask ? pSpawningTask->domain () : VTaskDomain::groundDomain ())
, m_pLocalContext (
    pSpawningTask ? pSpawningTask->getLocalContext () : new VReferenceableMonotype (
	ENVIR_KDsc_TheTmpTLE
    )
)
, m_pTemporalContext (
    pSpawningTask ? pSpawningTask->temporalContext () : ENVIR_KTemporalContext_Today
)
, m_pOutputBuffer	(new VOutputBuffer (m_pDomain, pChannel))
, m_xNextParameter	(0)
, m_cSegmentsExpected	(0)
, m_cSegmentsReceived	(0)
, m_pCodSpace		(0)
{
    m_pTemporalContext->retain ();
}

M_ASD *VTask::getCodSpace () {
    return m_pCodSpace = getCurrent ().codSpace ();
}


/*************************
 *************************
 *****  Destruction  *****
 *************************
 *************************/

VTask::~VTask () {
    m_pTemporalContext->release ();
}


/*******************
 *******************
 *****  Query  *****
 *******************
 *******************/

bool VTask::atOrAbove (VCall const *pCall) const {
    while (pCall) {
	VTask const*pCallCaller = pCall->caller ();
	if (this == pCallCaller)
	    return true;

	pCall = pCallCaller->call ();
    }

    return false;
}

bool VTask::atOrAbove (VTask const *pTask) const {
    while (pTask) {
	if (this == pTask)
	    return true;

	pTask = pTask->caller ();
    }

    return false;
}

bool VTask::atOrAbove (VComputationUnit const *pUnit) const {
    return pUnit->atOrBelow (this);
}

bool VTask::atOrBelow (VCall const *pCall) const {
    return pCall->atOrAbove (this);
}

bool VTask::atOrBelow (VTask const *pTask) const {
    return pTask->atOrAbove (this);
}

bool VTask::atOrBelow (VComputationUnit const *pUnit) const {
    return pUnit->atOrAbove (this);
}

bool VTask::datumAvailable_ () const {
    return m_xStage == Stage_Exit;
}


/********************
 ********************
 *****  Access  *****
 ********************
 ********************/

/*********************
 *****  General  *****
 *********************/

IOMDriver* VTask::channel_ () const {
    return channel ();
}

VComputationUnit* VTask::consumer_ () const {
    return consumer ();
}

VSelector const &VTask::selector_ () const {
    return selector ();
}


/***********************
 *****  Parameter  *****
 ***********************/

bool VTask::getParameter (unsigned int xParameter, VDescriptor& rDatum) {
    VDescriptor iDatum;
    if (!call ()->getParameter (xParameter, iDatum))
	return false;

    rtLINK_CType *pCallerSubset; M_CPD *pCallerReordering;
    getCallerSubsetAndReordering (pCallerSubset, pCallerReordering);
    rDatum.setToReorderedSubset (
	pCallerReordering, call ()->subtaskSubset (pCallerSubset), iDatum
    );

    return true;
}


/**************************
 *****  Relationship  *****
 **************************/

bool VTask::getPathToCall (
    M_CPD *&rpReordering, rtLINK_CType *&rpRestriction, VCall *&rpCall
) const {
    rpCall = call();
    if (rpCall) {
	getCallerSubsetAndReordering (rpRestriction, rpReordering);
	rpRestriction = rpCall->subtaskSubset (rpRestriction);
	return true;
    }
    return false;
}

bool VTask::getPathToCaller (
    M_CPD *&rpReordering, rtLINK_CType *&rpRestriction, VTask *&rpCaller
) const {
    rpCaller = m_pCaller;
    if (rpCaller) {
	getCallerSubsetAndReordering (rpRestriction, rpReordering);
	return true;
    }
    return false;
}

bool VTask::getPathToConsumer (
    M_CPD *&rpReordering, rtLINK_CType *&rpRestriction, VComputationUnit *&rpConsumer
) const {
    rpConsumer = consumer ();
    if (rpConsumer) {
	getCallerSubsetAndReordering (rpRestriction, rpReordering);
	return true;
    }
    return false;
}

bool VTask::getPathToCreator (
    M_CPD *&rpReordering, rtLINK_CType *&rpRestriction, VComputationUnit *&rpCreator
) const {
    VCall* pCreator;
    if (getPathToCall (rpReordering, rpRestriction, pCreator)) {
	rpCreator = pCreator;
	return true;
    }
    return false;
}


/***********************
 ***********************
 *****  Execution  *****
 ***********************
 ***********************/

/******************
 *****  Exit  *****
 ******************/

void VTask::exit () {
//  Return the right result, ...
    m_pDuc = &m_rDatum;

    switch (returnCase ()) {
    case Return_Current:
	if (m_pBlockContext)
	    loadDucWithCurrent ();
	break;
    default:
	break;
    }

//  Get this task's relationship to its caller, ...
    rtLINK_CType *pCallerSubset; M_CPD *pCallerReordering;
    getCallerSubsetAndReordering (pCallerSubset, pCallerReordering);

//  ... redistribute the task's result, ...
    if (pCallerReordering)
	m_pDuc->distribute (pCallerReordering);

//  and, if this task has a caller, ...
    if (m_pCaller) {
    //  ... return this task's output, ...
	if (outputBufferDomainIsValid ()) m_pOutputBuffer->moveOutputTo (
	    m_pCaller->m_pOutputBuffer, pCallerSubset, pCallerReordering
	);

    //  ... and resume the consumer of this task's data:
	call()->resumeConsumer ();
    }
    else
    //  otherwise, just return this task's output to stdout, ...
	m_pOutputBuffer->moveOutputToChannel ();

//  ... and wave bye-bye:
    //  bye
    //	bye
    terminate ();
}


/******************
 *****  Fail  *****
 ******************/

void VTask::fail () {
    m_pCuc.clear ();
    if (terminatedNot ()) {
	terminate ();

	reportTrace ();
	if (m_pCaller)
	    call()->failConsumer ();
    }
}


/*******************************
 *******************************
 *****  Call Construction  *****
 *******************************
 *******************************/

/*******************
 *****  Basic  *****
 *******************/

void VTask::beginBoundCall (M_CPD *pBlock, unsigned int iParameterCount, rtLINK_CType *pSubset) {
    m_pCuc = new VBoundCall (this, pSubset, iParameterCount, pBlock);
}

void VTask::beginMessageCall (VByteCodeScanner const &rMessageSource, rtLINK_CType *pSubset) {
    m_pCuc = new VPrimaryCall (this, pSubset, rMessageSource);
}

void VTask::beginMessageCall (
    char const *pMessageName, unsigned int iParameterCount, rtLINK_CType *pSubset
) {
    m_pCuc = new VPrimaryCall (this, pSubset, pMessageName, iParameterCount);
}

void VTask::beginMessageCall (unsigned int xMessageName, rtLINK_CType *pSubset) {
    m_pCuc = new VPrimaryCall (this, pSubset, xMessageName);
}

void VTask::beginValueCall (unsigned int iParameterCount, rtLINK_CType *pSubset) {
    m_pCuc = new VEvaluationCall (this, pSubset, iParameterCount);
}

void VTask::commitParameters (unsigned int xFirstParameter) {
    rtLINK_CType *pCallerSubset; M_CPD *pCallerReordering;
    getCallerSubsetAndReordering (pCallerSubset, pCallerReordering);
    for (unsigned int xParameter = xFirstParameter; xParameter < m_xNextParameter; xParameter++) {
	call()->subtaskParameter (pCallerSubset, pCallerReordering, xParameter, *m_pDuc);
	commitParameter ();
    }

    if (m_xNextParameter > xFirstParameter)
	m_xNextParameter = xFirstParameter;
}

void VTask::commitCall (ReturnCase xReturnCase, rtINDEX_Key *pTemporalContext, unsigned int xCallerPC) {
//  Commit (and pop) the call-under-construction, ...
    m_pCuc.setTo (m_pCuc->commit (xReturnCase, pTemporalContext, xCallerPC));
    setStepPoint ();

//  ... and get a new duc (sigh):
    m_pDuc = m_pCuc.isntNil () ? &m_pCuc->duc () : &m_rDatum;
}


/*************************
 *****  Specialized  *****
 *************************/

void VTask::sendUnaryMessageToCurrent (char const *pMessageName) {
    beginMessageCall (pMessageName, 0);
    commitRecipient (VMagicWord_Current);
    commitCall ();
}

void VTask::sendUnaryMessageToSelf (char const *pMessageName) {
    beginMessageCall (pMessageName, 0);
    commitRecipient (VMagicWord_Self);
    commitCall ();
}


/*---------------------------------------------------------------------------
 *****  Utility to send a magic parameter, known message to the Duc.
 *
 *  Arguments:
 *	pMessageName		- a pointer to a null terminated string
 *				  naming the message to be sent.
 *
 *  Returns:
 *	NOTHING - Executed for side effect only.
 *
 *  Notes:
 *	This routine will push the current task's value of '^self' as the
 *	single parameter of this message.
 *
 *****/
void VTask::sendBinaryConverseWithSelf (char const *pMessageName) {
    beginMessageCall (pMessageName, 1);
    commitRecipient ();

    loadDucWithSelf ();
    commitParameter ();

    commitCall ();
}

/*---------------------------------------------------------------------------
 *****  Utility to send a magic parameter, known message to the Duc.
 *
 *  Arguments:
 *	pMessageName		- a pointer to a null terminated string
 *				  naming the message to be sent.
 *
 *
 *  Returns:
 *	NOTHING - Executed for side effect only.
 *
 *  Notes:
 *	This routine will push the current task's value of '^current' as the
 *	single parameter of this message.
 *
 *****/
void VTask::sendBinaryConverseWithCurrent (char const *pMessageName) {
    beginMessageCall (pMessageName, 1);
    commitRecipient ();

    loadDucWithCurrent ();
    commitParameter ();

    commitCall ();
}


/*---------------------------------------------------------------------------
 *****  Utility to send a magic parameter, known message to the Duc.
 *
 *  Arguments:
 *	xMessageName		- the index of the known selector to be sent.
 *
 *  Returns:
 *	NOTHING - Executed for side effect only.
 *
 *  Notes:
 *	This routine will push the current task's value of '^self' as the
 *	single parameter of this message.
 *
 *****/
void VTask::sendBinaryConverseWithSelf (unsigned int xMessageName) {
    beginMessageCall (xMessageName);
    commitRecipient ();

    loadDucWithSelf ();
    commitParameter ();

    commitCall ();
}

/*---------------------------------------------------------------------------
 *****  Utility to send a magic parameter, known message to the Duc.
 *
 *  Arguments:
 *	xMessageName		- the index of the known selector to be sent.
 *
 *  Returns:
 *	NOTHING - Executed for side effect only.
 *
 *  Notes:
 *	This routine will push the current task's value of '^current' as the
 *	single parameter of this message.
 *
 *****/
void VTask::sendBinaryConverseWithCurrent (unsigned int xMessageName) {
    beginMessageCall (xMessageName);
    commitRecipient ();

    loadDucWithCurrent ();
    commitParameter ();

    commitCall ();
}


/********************************
 ********************************
 *****  Context Management  *****
 ********************************
 ********************************/

/********************************
 *****  ^self/^current/^my  *****
 ********************************/

bool VTask::myAvailable () const {
    return m_pBlockContext.isntNil () && getMy ().isntEmpty ();
}

bool VTask::recipientAvailable_ () const {
    return m_pBlockContext.isntNil ();
}

bool VTask::cgetSelf (VDescriptor &rResultHolder) const {
    if (m_pBlockContext) {
	rResultHolder.setToCopied (getSelf ());
	return true;
    }
    return false;
}

bool VTask::cgetCurrent (VDescriptor &rResultHolder) const {
    if (m_pBlockContext) {
	rResultHolder.setToCopied (getCurrent ());
	return true;
    }
    return false;
}

bool VTask::cgetMy (VDescriptor &rResultHolder) const {
    if (m_pBlockContext) {
	DSC_Descriptor& rMy = getMy ();
	if (rMy.isEmpty ())
	    return false;
	rResultHolder.setToCopied (rMy);
	return true;
    }
    return false;
}


/********************
 *****  ^local  *****
 ********************/

VReferenceableMonotype *VTask::getLocalContext () {
/*****  If '^local' is already known, short circuit everything and return it, ...  *****/
    if (m_pLocalContext.isntNil ())
	return m_pLocalContext;

/*****  Otherwise, find the nearest task with a ^local context, ... *****/
    VTask *pSourceTask = m_pCaller;
    while (pSourceTask->m_pLocalContext.isNil ())
	pSourceTask = pSourceTask->caller ();

/*****  ... determine the relationship of the two task domains, ...  *****/
    rtLINK_CType *pSubset;
    M_CPD *pSequence;
    VTaskDomain::PathType xPathType = m_pDomain->getPathMaps (
	pSourceTask->domain (), pSubset, pSequence
    );

/***** ... derive the local context for this task, ...  *****/
    switch (xPathType) {
    case VTaskDomain::PathType_Identity:
	m_pLocalContext.setTo (pSourceTask->m_pLocalContext);
	break;

    case VTaskDomain::PathType_Parent:
	m_pLocalContext.setTo (
	    pSequence && pSubset ? new VReferenceableMonotype (
		pSequence, pSubset, *pSourceTask->m_pLocalContext
	    ) : pSequence ? new VReferenceableMonotype (
		pSequence, *pSourceTask->m_pLocalContext
	    ) : pSubset ? new VReferenceableMonotype (
		pSubset, *pSourceTask->m_pLocalContext
	    ) : pSourceTask->m_pLocalContext.operator->()
	);
	break;

    case VTaskDomain::PathType_Sequence:
	m_pLocalContext.setTo (
	    new VReferenceableMonotype (pSequence, *pSourceTask->m_pLocalContext)
	);
	break;

    case VTaskDomain::PathType_Subset:
	m_pLocalContext.setTo (
	    new VReferenceableMonotype (pSubset, *pSourceTask->m_pLocalContext)
	);
	break;

    case VTaskDomain::PathType_SequencedSubset:
	m_pLocalContext.setTo (
	    new VReferenceableMonotype (pSequence, pSubset, *pSourceTask->m_pLocalContext)
	);
	break;
    }

/*****  ... free the path maps, ...  *****/
    m_pDomain->releasePathMaps (xPathType, pSubset, pSequence);

/*****  ... push the new ^local up the caller path for possible re-use, ...  *****/
    VTaskDomain *pSourceDomain = domain ();
    VTask *pDestinationTask = m_pCaller;
    while (
	pDestinationTask &&
	pDestinationTask->domain () == pSourceDomain &&
	pDestinationTask->m_pLocalContext.isNil ()
    ) {
	pDestinationTask->m_pLocalContext.setTo (m_pLocalContext);
	pDestinationTask = pDestinationTask->m_pCaller;
    }

/*****  ... and return the ^local context:  *****/
    return m_pLocalContext;
}


/************************
 ************************
 *****  Duc Update  *****
 ************************
 ************************/

/********************************************
 *****  loadDucWith... Self/Current/My  *****
 ********************************************/

void VTask::loadDucWithSelf () const {
    m_pDuc->setToCopied (getSelf ());
}

void VTask::loadDucWithCurrent () const {
    m_pDuc->setToCopied (getCurrent ());
}

void VTask::loadDucWithMy () const {
    m_pDuc->setToCopied (getMy ());
}


/**********************************
 *****  loadDucWith... Super  *****
 **********************************/

void VTask::loadDucWithSuper () const {
/*****  Load accumulator with 'self'...  *****/
    loadDucWithSelf ();

/*****  ... and get the inheritance:  *****/
    m_pDuc->contentAsMonotype ().getInheritance ();
}


/***************************************
 *****  loadDucWith... Date/Local  *****
 ***************************************/

void VTask::loadDucWithDate () const {
    m_pDuc->setToCopied (getDate ());
}

void VTask::loadDucWithLocal () {
    m_pDuc->setToCopied (getLocal ());
}


/********************************************
 *****  loadDucWith... Other Monotypes  *****
 ********************************************/

void VTask::loadDucWithCoerced (rtINDEX_Key *pTemporalContext) const {
    pTemporalContext->RealizeSequence ();
    loadDucWithCoerced (rtINDEX_Key_Sequence (pTemporalContext));
}

void VTask::loadDucWithCoerced (DSC_Descriptor& rValue) const {
    m_pDuc->setToCopied (rValue);
    m_pDuc->contentAsMonotype ().coerce (ptoken ());
}

void VTask::loadDucWithRepresentative (VGroundStore *pGroundStore, unsigned int xRepresentative) const {
    if (IsNil (pGroundStore))
	loadDucWithNA ();
    else {
	rtVSTORE_Handle::Reference pSurrogate;
	pGroundStore->getSurrogate (pSurrogate);
	loadDucWithReference (pSurrogate, pGroundStore->ptoken_(), xRepresentative);
    }
}

void VTask::loadDucWith (VGroundStore *pGroundStore) const {
    rtVSTORE_Handle::Reference pSurrogate;
    pGroundStore->getSurrogate (pSurrogate);
    loadDucWithIdentity (pSurrogate, pGroundStore->ptoken_());
}

void VTask::loadDucWithIdentity (Store *pStore, rtPTOKEN_Handle *pPPT) const {
    m_pDuc->setToIdentity (pStore, pPPT);
}


/*---------------------------------------------------------------------------
 *****  Utility to load the accumulator with a descriptor for a new L-Store.
 *
 *  Arguments:
 *	pContentPrototype	- the address of a CPD for the new cluster's
 *				  content prototype (Nil for Vector).
 *
 *  Returns:
 *	NOTHING - Executed for side effect only.
 *
 *****/
void VTask::loadDucWithNewLStore (Store *pContentPrototype) const {
/*****  Create the L-store PToken...  *****/
    rtPTOKEN_Handle::Reference pInstancePToken (NewCodPToken ());

/*****  ... and create and return the new L-Store:  *****/
    loadDucWithListOrStringStore (
	new rtLSTORE_Handle (pInstancePToken, pContentPrototype)
    );
}


void VTask::loadDucWith (VRunTimeType *pRTT) const {
    loadDucWith (pRTT ? pRTT->name () : "NullRTT");
}

void VTask::loadDucWith (char const *pString) const {
    if (IsNil (pString))
	loadDucWithNA ();
    else {
	rtPTOKEN_Handle::Reference pPPT (NewCodPToken (strlen (pString) + 1));
	VCPDReference pCUV (0, NewCharUV (pPPT));

	strcpy (rtCHARUV_CPD_Array (pCUV), pString);

	rtLSTORE_Handle::Reference pLStore (rtLSTORE_NewStringStoreFromUV (pCUV));
	loadDucWithReference (pLStore, pLStore->getPToken (), 0);
    }
}

void VTask::loadDucWith (VString const &rString) const {
    loadDucWith ((char const*)rString);
}

void VTask::loadDucWith (VSelector const &rSelector) const {
    unsigned int xSelector;
    if (rSelector.getKSData (xSelector)) {
	loadDucWithSelector (xSelector);
	return;
    }

    rtBLOCK_Handle::Reference pBlock;
    if (rSelector.getBSData (pBlock, xSelector)) {
	loadDucWithSelector (pBlock, xSelector);
	return;
    }

    loadDucWith (rSelector.messageName ());
}


/********************************************
 *****  loadDucWith... Other Polytypes  *****
 ********************************************/

VFragmentation &VTask::loadDucWithFragmentation (rtPTOKEN_Handle *pPPT) const {
    return m_pDuc->setToFragmentation (pPPT);
}

VDescriptor *VTask::loadDucWithGuardedDatum (rtLINK_CType *pGuard) const {
//  If all elements are defined (no guard), the duc can hold the result directly:
    if (IsNil (pGuard))
	return m_pDuc;

//  If no elements are defined (|guard| == 0), release the guard and set the duc to NA:
    if (0 == pGuard->ElementCount ()) {
	pGuard->release ();
	loadDucWithNA ();
	return NilOf (VDescriptor*);
    }

//  Otherwise, construct a fragmentation to hold the guarded datum, ...
    VFragmentation &rFragmentation = loadDucWithFragmentation (pGuard->RPT ());

    VDescriptor *pGuardedDatum = &rFragmentation.createFragment (pGuard)->datum();

    rtLINK_CType *pUndefined = pGuard->Complement ();
    rFragmentation.createFragment (pUndefined)->datum().setToNA (
	pUndefined->PPT (), codKOT ()
    );

    return pGuardedDatum;
}


/*---------------------------------------------------------------------------
 *****  Routine to load the accumulator with a boolean result.
 *****  If linkc is not nil then some of the answers were false.
 *
 *  Arguments:
 *	pTrueSubset	- Optional (Nil if omitted), the address of a link
 *                        constructor specifying the true values. If this link
 *			  is supplied, its RPT must match 'this->ptoken()' Also,
 *			  if this link is supplied, ownership of the caller's
 *			  reference to this link will be transfered to the 'duc'.
 *
 *  Returns:
 *	NOTHING
 *
 *****/
void VTask::loadDucWithBoolean (rtLINK_CType *pTrueSubset) const {
/*****  If there is no link, then load a simple true ... *****/
    if (IsNil (pTrueSubset)) {
	loadDucWithTrue ();
	return;
    }

/*****  If the link is empty, return a simple false ... *****/
    if (pTrueSubset->ElementCount () == 0) {
	pTrueSubset->release ();
	loadDucWithFalse ();
	return;
    }

/***** Otherwise, construct a fragmentation ... *****/
    VFragmentation &rFragmentation = loadDucWithFragmentation ();
    M_KOT *pKOT = codKOT ();

    /***  with true values... ***/
    rFragmentation.createFragment (pTrueSubset)->datum ().setToTrue (
	pTrueSubset->PPT (), pKOT
    );

    /***  and false values... ***/
    rtLINK_CType *pFalseSubset = pTrueSubset->Complement ();
    rFragmentation.createFragment (pFalseSubset)->datum ().setToFalse (
	pFalseSubset->PPT (), pKOT
    );
}


/*---------------------------------------------------------------------------
 *****  Utility to load the accumulator with a partial correspondence.
 *
 *  Arguments:
 *	pStore			- a standard CPD for the string store to be
 *				  returned.
 *	pSubset			- Optional (Nil if omitted) address of a link
 *				  constructor specifying the 'ptoken ()'
 *				  subset to which 'pStore' applies.
 *
 *  Returns:
 *	NOTHING - Executed for side effect on the accumulator only.
 *
 *****/
void VTask::loadDucWithPartialCorrespondence (Vdd::Store *pStore, rtLINK_CType *pSubset) const {
    if (IsntNil (pSubset) && 0 == pSubset->ElementCount ()) {
	loadDucWithNA ();

	if (pStore)
	    pStore->discard ();

	return;
    }
    
    VDescriptor *pDSC;
    rtPTOKEN_Handle *ddPT;

    if (IsntNil (pSubset)) {
	VFragmentation &rFragmentation = loadDucWithFragmentation ();

	/*** Load the NA values into the fragmentation ... ***/
	rtLINK_CType *pNALink = pSubset->Complement ();
	rFragmentation.createFragment (pNALink)->datum ().setToNA (pNALink->PPT (), codKOT ());

	/*** Load the non-NA 'values' into the fragmentation ... ***/
	pSubset->retain ();
	pDSC = &rFragmentation.createFragment (pSubset)->datum ();
	ddPT = pSubset->PPT ();
    }
    else {
	pDSC = m_pDuc;
	ddPT = ptoken ();
    }

    pDSC->setToCorrespondence (ddPT, pStore);
}


/*---------------------------------------------------------------------------
 *****  Routine to load the accumulator with a domain limited result.
 *****  If linkc is not nil then some of the answers were NA's.
 *
 *  Arguments:
 *	pLink		- Optional (Nil if omitted), the address of a link
 *                        constructor specifying the non-NA values from
 *	                  'uvector'.  If this link is specified, its RPT is
 *			  expected to be 'ptoken ()'.
 *	pUVector	- a standard CPD for a uvector holding the results
 *			  being returned.  'this->ptoken ()' is the expected
 *			  PPT of this u-vector.
 *
 *  Returns:
 *	NOTHING
 *
 *****/
void VTask::loadDucWithPartialFunction (rtLINK_CType *pLink, M_CPD *pUVector) const {
/*****  If there is no link, then load a simple descriptor ... *****/
    if (IsNil (pLink)) {
	loadDucWithMonotype (pUVector);
	return;
    }

/*****  ... else, if the link is empty, return a single NA ... *****/
    if (pLink->ElementCount () == 0) {
	loadDucWithNA ();
	return;
    }

/***** ... else, load a fragmentation ... *****/
    /***  extract the good values from the pUVector ... ***/
    M_CPD *pNewUV;
    RTYPE_PerformHandlerOperation (
	RTYPE_DoLCExtract, pUVector->RType (), &pNewUV, pUVector, pLink
    );

    /***  and load the good values into the accumulator ... ***/
    VFragmentation &rFragmentation = loadDucWithFragmentation ();

    pLink->retain ();
    rFragmentation.createFragment (pLink)->datum ().setToMonotype (pNewUV);
    pNewUV->release ();

    /*** now load the NA values into the fragmentation ... ***/
    rtLINK_CType *pNALink = pLink->Complement ();
    rFragmentation.createFragment (pNALink)->datum ().setToNA (pNALink->PPT (), codKOT ());
}

void VTask::loadDucWithPartialFunction (VfGuardTool &rGuard, M_CPD *pUVector) const {
    rtLINK_CType *pGuard = rGuard.commit ();
    loadDucWithPartialFunction (pGuard, pUVector);
    if (pGuard)
	pGuard->release ();
    pUVector->release ();
}


/*---------------------------------------------------------------------------
 *****  Utility to load the accumulator with the results of a
 *****  non-scalar predicate.
 *
 *  Arguments:
 *	pKOTM1			- the address of the KOTM associated with 'pLink1'.
 *	pLink1			- a link constructor specifying the elements of
 *				  this task with values of type *pKOTC1.  This
 *				  link constructor can be open.
 *	pKOTM2, pLink2		- see pKOTM1 and pLink1.
 *
 *  Returns:
 *	NOTHING - Executed for side effect on the accumulator only.
 *
 *****/
void VTask::loadDucWithPredicateResult (
    rtLINK_CType *pTrueSubset, rtLINK_CType *pFalseSubset
) const {
    M_KOTM pTrueKOTM  = &M_KOT::TheTrueClass;
    M_KOTM pFalseKOTM = &M_KOT::TheFalseClass;
    loadDucWithPredicateResult (pTrueSubset, pFalseSubset, pTrueKOTM, pFalseKOTM);
}

void VTask::loadDucWithPredicateResult (
    rtLINK_CType *pLink1, rtLINK_CType *pLink2, M_KOTM pKOTM1, M_KOTM pKOTM2
) const {
    rtPTOKEN_Handle::Reference fragmentPToken;
    M_KOT *pCodKOT = codKOT ();

    VFragmentation &rFragmentation = loadDucWithFragmentation (pLink1->RPT ());

/*****  Create a fragment for 'store/ptoken/link' set #1 ...  *****/
    size_t elementCount = pLink1->ElementCount ();
    if (elementCount > 0) {
	if (pLink1->IsntOpen ())
	    fragmentPToken.setTo (pLink1->PPT ());
	else {
	    fragmentPToken.setTo (NewDomPToken (elementCount));
	    pLink1->Close (fragmentPToken);
	}
	rFragmentation
	    .createFragment (pLink1)->datum ()
	    .setToConstant (fragmentPToken, pCodKOT->*pKOTM1);
    }
    else {
        pLink1->release ();
    }

/*****  Create a fragment for 'store/ptoken/link' set #2 ...  *****/
    if ((elementCount = pLink2->ElementCount ()) > 0) {
	if (pLink2->IsntOpen ())
	    fragmentPToken.setTo (pLink2->PPT ());
	else {
	    fragmentPToken.setTo (NewDomPToken (elementCount));
	    pLink2->Close (fragmentPToken);
	}
	rFragmentation
	    .createFragment (pLink2)->datum ()
	    .setToConstant (fragmentPToken, pCodKOT->*pKOTM2);
    }
    else {
        pLink2->release ();
    }
}


void VTask::loadDucWithNextParameter (VSelector const *pParameterSelector) {
    if (m_xNextParameter > 0) {
	rtLINK_CType *pCallerSubset; M_CPD *pCallerReordering;
	getCallerSubsetAndReordering (pCallerSubset, pCallerReordering);
	call()->subtaskParameter (pCallerSubset, pCallerReordering, --m_xNextParameter, *m_pDuc);
    }
    else {
	loadDucWithNA ();
	issueWarningMessage (
	    "Parameter", pParameterSelector, "Unavailable"
	);
    }
}

void VTask::loadDucWithParameter (unsigned int xParameter) const {
    if (xParameter >= parameterCount ()) raiseException (
	EC__UsageError,
	"%s: Parameter %u requested, only %u parameter%s available",
	description (),
	xParameter,
	parameterCount (),
	parameterCount () == 1 ? "" : "s"
    );

    rtLINK_CType *pCallerSubset; M_CPD *pCallerReordering;
    getCallerSubsetAndReordering (pCallerSubset, pCallerReordering);
    call()->subtaskParameter (pCallerSubset, pCallerReordering, xParameter, *m_pDuc);
}


/*********************************
 *********************************
 *****  Operator Processors  *****
 *********************************
 *********************************/

void VTask::process (VNumericBinary& rOperator) {
    bool notHandled = false;

    if (loadDucWithNextParameterAsMonotype ()) {
	normalizeDuc ();
	DSC_Descriptor& rOperand = ducMonotype ();

	Vdd::Store *pOperandStore = rOperand.store ();
	RTYPE_Type xOperandRType = rOperand.pointerRType ();

	DSC_Descriptor& rCurrent = getCurrent ();
	RTYPE_Type xCurrentRType = rCurrent.pointerRType ();

	switch (xCurrentRType) {
	case RTYPE_C_DoubleUV:
	    switch (xOperandRType) {
	    case RTYPE_C_DoubleUV:
		if (pOperandStore->DoesntNameTheDoubleClass ())
		    notHandled = true;
		else if (isScalar ()) {
		    loadDucWithGuardedDouble  (
			rOperator.value (
			    DSC_Descriptor_Scalar_Double (rCurrent),
			    DSC_Descriptor_Scalar_Double (rOperand)
			)
		    );
		}
		else {
		    VGuardedDoubleGenerator iGenerator (this);
		    rOperator.computeDD (
			iGenerator,
			rtDOUBLEUV_CPD_Array (DSC_Descriptor_Value (rCurrent)),
			rtDOUBLEUV_CPD_Array (DSC_Descriptor_Value (rOperand))
		    );
		    iGenerator.commit ();
		}
		break;

	    case RTYPE_C_FloatUV:
		if (pOperandStore->DoesntNameTheFloatClass ())
		    notHandled = true;
		else if (isScalar ()) {
		    loadDucWithGuardedDouble  (
			rOperator.value (
			    DSC_Descriptor_Scalar_Double (rCurrent),
			    DSC_Descriptor_Scalar_Float  (rOperand)
			)
		    );
		}
		else {
		    VGuardedDoubleGenerator iGenerator (this);
		    rOperator.computeDF (
			iGenerator,
			rtDOUBLEUV_CPD_Array (DSC_Descriptor_Value (rCurrent)),
			rtFLOATUV_CPD_Array (DSC_Descriptor_Value (rOperand))
		    );
		    iGenerator.commit ();
		}
		break;

	    case RTYPE_C_IntUV:
		if(pOperandStore->DoesntNameTheIntegerClass ())
		    notHandled = true;
		else if (isScalar ()) {
		    loadDucWithGuardedDouble  (
			rOperator.value (
			    DSC_Descriptor_Scalar_Double (rCurrent),
			    DSC_Descriptor_Scalar_Int    (rOperand)
			)
		    );
		}
		else {
		    VGuardedDoubleGenerator iGenerator (this);
		    rOperator.computeDI (
			iGenerator,
			rtDOUBLEUV_CPD_Array (DSC_Descriptor_Value (rCurrent)),
			rtINTUV_CPD_Array (DSC_Descriptor_Value (rOperand))
		    );
		    iGenerator.commit ();
		}
		break;

	    default:
		notHandled = true;
		break;
	    }
	    break;

	case RTYPE_C_FloatUV:
	    switch (xOperandRType) {
	    case RTYPE_C_DoubleUV:
		if (pOperandStore->DoesntNameTheDoubleClass ())
		    notHandled = true;
		else if (isScalar ()) {
		    loadDucWithGuardedDouble  (
			rOperator.value (
			    DSC_Descriptor_Scalar_Float  (rCurrent),
			    DSC_Descriptor_Scalar_Double (rOperand)
			)
		    );
		}
		else {
		    VGuardedDoubleGenerator iGenerator (this);
		    rOperator.computeFD (
			iGenerator,
			rtFLOATUV_CPD_Array (DSC_Descriptor_Value (rCurrent)),
			rtDOUBLEUV_CPD_Array (DSC_Descriptor_Value (rOperand))
		    );
		    iGenerator.commit ();
		}
		break;

	    case RTYPE_C_FloatUV:
		if (pOperandStore->DoesntNameTheFloatClass ())
		    notHandled = true;
		else if (isScalar ()) {
		    loadDucWithGuardedDouble  (
			rOperator.value (
			    DSC_Descriptor_Scalar_Float  (rCurrent),
			    DSC_Descriptor_Scalar_Float  (rOperand)
			)
		    );
		}
		else {
		    VGuardedDoubleGenerator iGenerator (this);
		    rOperator.computeFF (
			iGenerator,
			rtFLOATUV_CPD_Array (DSC_Descriptor_Value (rCurrent)),
			rtFLOATUV_CPD_Array (DSC_Descriptor_Value (rOperand))
		    );
		    iGenerator.commit ();
		}
		break;

	    case RTYPE_C_IntUV:
		if(pOperandStore->DoesntNameTheIntegerClass ())
		    notHandled = true;
		else if (isScalar ()) {
		    loadDucWithGuardedDouble  (
			rOperator.value (
			    DSC_Descriptor_Scalar_Float  (rCurrent),
			    DSC_Descriptor_Scalar_Int    (rOperand)
			)
		    );
		}
		else {
		    VGuardedDoubleGenerator iGenerator (this);
		    rOperator.computeFI (
			iGenerator,
			rtFLOATUV_CPD_Array (DSC_Descriptor_Value (rCurrent)),
			rtINTUV_CPD_Array (DSC_Descriptor_Value (rOperand))
		    );
		    iGenerator.commit ();
		}
		break;

	    default:
		notHandled = true;
		break;
	    }
	    break;

	case RTYPE_C_IntUV:
	    switch (xOperandRType) {
	    case RTYPE_C_DoubleUV:
		if (pOperandStore->DoesntNameTheDoubleClass ())
		    notHandled = true;
		else if (isScalar ()) {
		    loadDucWithGuardedDouble  (
			rOperator.value (
			    DSC_Descriptor_Scalar_Int    (rCurrent),
			    DSC_Descriptor_Scalar_Double (rOperand)
			)
		    );
		}
		else {
		    VGuardedDoubleGenerator iGenerator (this);
		    rOperator.computeID (
			iGenerator,
			rtINTUV_CPD_Array (DSC_Descriptor_Value (rCurrent)),
			rtDOUBLEUV_CPD_Array (DSC_Descriptor_Value (rOperand))
		    );
		    iGenerator.commit ();
		}
		break;

	    case RTYPE_C_FloatUV:
		if (pOperandStore->DoesntNameTheFloatClass ())
		    notHandled = true;
		else if (isScalar ()) {
		    loadDucWithGuardedDouble  (
			rOperator.value (
			    DSC_Descriptor_Scalar_Int    (rCurrent),
			    DSC_Descriptor_Scalar_Float  (rOperand)
			)
		    );
		}
		else {
		    VGuardedDoubleGenerator iGenerator (this);
		    rOperator.computeIF (
			iGenerator,
			rtINTUV_CPD_Array (DSC_Descriptor_Value (rCurrent)),
			rtFLOATUV_CPD_Array (DSC_Descriptor_Value (rOperand))
		    );
		    iGenerator.commit ();
		}
		break;

	    case RTYPE_C_IntUV:
		if (pOperandStore->DoesntNameTheIntegerClass ())
		    notHandled = true;
		else if (isScalar ()) {
		    loadDucWithGuardedDouble  (
			rOperator.value (
			    DSC_Descriptor_Scalar_Int    (rCurrent),
			    DSC_Descriptor_Scalar_Int    (rOperand)
			)
		    );
		}
		else {
		    VGuardedDoubleGenerator iGenerator (this);
		    rOperator.computeII (
			iGenerator,
			rtINTUV_CPD_Array (DSC_Descriptor_Value (rCurrent)),
			rtINTUV_CPD_Array (DSC_Descriptor_Value (rOperand))
		    );
		    iGenerator.commit ();
		}
		break;

	    default:
		notHandled = true;
		break;
	    }
	    break;

	default:
	    raiseException (
		EC__UsageError,
		"VTask::process(VNumericBinary): '%s' Values Not Supported",
		RTYPE_TypeIdAsString (xCurrentRType)
	    );
	    break;
	}
    }
    else notHandled = true;

    if (notHandled)
	sendBinaryConverseWithCurrent (rOperator.selector ());
}


/*************************************
 *************************************
 *****  Display and Description  *****
 *************************************
 *************************************/

void VTask::reportInfoHeader (unsigned int xLevel) const {
    report ("\n----%4u:\n\n", xLevel);
}

void VTask::reportTraceHeader (char const *pWhatKindOfTaskAmI) const {
    report (
	"***** %4s%7d ", pWhatKindOfTaskAmI, ptoken ()->cardinality ()
    );
}

void __cdecl VTask::display (char const *pFormat, ...) const {
    V_VARGLIST (pArguments, pFormat);
    channel ()->VPrint (0, pFormat, pArguments);
}

void __cdecl VTask::report (char const *pFormat, ...) const {
    V_VARGLIST (pArguments, pFormat);
    channel ()->VReport (0, pFormat, pArguments);
}


void VTask::decompilation (VString &rString, VCall const *Unused (pContext)) const {
    rString.setTo (description ());
}

void VTask::decompilationPrefix	(VString &rString, VCall const *Unused (pContext)) const {
    rString.setTo (description ());
}

void VTask::decompilationSuffix	(VString &rString, VCall const *Unused (pContext)) const {
    rString.setTo ("");
}

unsigned int VTask::decompilationIndent (VCall const *Unused (pContext)) const {
    return 0;
}

unsigned int VTask::decompilationOffset (VCall const *Unused (pContext)) const {
    return 0;
}

void VTask::dumpByteCodes (VCall const *Unused (pContext)) const {
}


/**********************************************
 **********************************************
 *****  Exception and Warning Generation  *****
 **********************************************
 **********************************************/

void VTask::issueWarningMessage (
    char const *pPrefixString, VSelector const *pSelector, char const *pSuffixString
) const
{
    static char const *const format = "%s '%s' %s";

    char const *pSelectorName = IsntNil (pSelector)
	? pSelector->messageName ()
	: description ();
    char *pBuffer = (char*)allocate (
	strlen (pSelectorName) +
	strlen (format) +
	strlen (pPrefixString) +
	strlen (pSuffixString)
    );

    STD_sprintf (pBuffer, format, pPrefixString, pSelectorName, pSuffixString);
    report ("\n>>> %s <<<\n", pBuffer);
    FAULT_RecordWarning (pBuffer);
    deallocate (pBuffer);
}

#if 1
/********************************************
 ********************************************
 *****  External Result Return Helpers  *****
 ********************************************
 ********************************************/

/*****************************************
 *----  Polytype Segment Management  ----*
 *****************************************/

VFragment *VTask::createSegment (object_reference_array_t const &rInjector) {
    rtLINK_CType *const pInjector = rtLINK_RefConstructor (ptoken ());
    unsigned int const sInjector = rInjector.cardinality ();
    if (sInjector > 0) {
	unsigned int xRange = rInjector[0];
	unsigned int sRange = 1;

	for (unsigned int xElement = 1; xElement < sInjector; xElement++) {
	    if (sRange + xRange == rInjector[xElement])
		sRange++;
	    else {
		pInjector->AppendRange (xRange, sRange);
		xRange = rInjector[xElement];
		sRange = 1;
	    }
	}
	if (sRange > 0)
	    pInjector->AppendRange (xRange,sRange);
    }
    pInjector->Close (NewDomPToken (sInjector));

    return (
	0 == m_cSegmentsReceived++ ? loadDucWithFragmentation () : duc().contentAsFragmentation ()
    ).createFragment (pInjector);
}

bool VTask::wrapupSegment () {
    return allSegmentsReceived () && wrapup ();
}

bool VTask::wrapup () {
    resume ();
    return true;
}

bool VTask::SetSegmentCountTo (unsigned int cSegments) {
    m_cSegmentsExpected = cSegments;
    return wrapupSegment ();
}

/******************************************************
 *----  VTask::ProcessArray<Source_T,Result_T> ----*
 ******************************************************/

template <typename Source_T, typename Result_T> void VTask::ProcessArray (
    VDescriptor &rResult, rtPTOKEN_Handle *pPPT, VkDynamicArrayOf<Source_T> const &rSourceArray, Result_T const *&rpResultArray
) const {
    unsigned int const count = rSourceArray.cardinality ();
    if(count==1) {
	rResult.setToConstant (pPPT, codKOT (), static_cast<Result_T>(rSourceArray[0]));
	rpResultArray = 0;
    } else {
	Result_T *pResultArray;
	M_CPD *const pResultUV = NewUV (pPPT, pResultArray);
	for (unsigned int xElement = 0; xElement<count; xElement++) {
	    pResultArray[xElement] = static_cast<Result_T>(rSourceArray[xElement]);
	}
	rResult.setToMonotype (pResultUV);
	pResultUV->release ();
	rpResultArray = pResultArray;
    }
}
template void VTask::ProcessArray<bool,int>           (VDescriptor&, rtPTOKEN_Handle*, VkDynamicArrayOf<bool> const&, int const*&) const;

template void VTask::ProcessArray<char,int>           (VDescriptor&, rtPTOKEN_Handle*, VkDynamicArrayOf<char> const&, int const*&) const;
template void VTask::ProcessArray<short,int>          (VDescriptor&, rtPTOKEN_Handle*, VkDynamicArrayOf<short> const&, int const*&) const;
template void VTask::ProcessArray<int,int>            (VDescriptor&, rtPTOKEN_Handle*, VkDynamicArrayOf<int> const&, int const*&) const;

template void VTask::ProcessArray<unsigned char,int>  (VDescriptor&, rtPTOKEN_Handle*, VkDynamicArrayOf<unsigned char> const&, int const*&) const;
template void VTask::ProcessArray<unsigned short,int> (VDescriptor&, rtPTOKEN_Handle*, VkDynamicArrayOf<unsigned short> const&, int const*&) const;
template void VTask::ProcessArray<unsigned int,int>   (VDescriptor&, rtPTOKEN_Handle*, VkDynamicArrayOf<unsigned int> const&, int const*&) const;

template void VTask::ProcessArray<float,float>        (VDescriptor&, rtPTOKEN_Handle*, VkDynamicArrayOf<float> const&, float const*&) const;
template void VTask::ProcessArray<double,double>      (VDescriptor&, rtPTOKEN_Handle*, VkDynamicArrayOf<double> const&, double const*&) const;


static char const * DynamicArrayStringAccessFn(bool reset, va_list pArgs) {
    V::VArgList iArgList (pArgs);

    static VkDynamicArrayOf<V::VString> const  *pStrings;
    static unsigned int xString;
    if (reset) {
        pStrings = iArgList.arg<VkDynamicArrayOf<V::VString> const*>();
        xString = 0;
    }
    else if (xString < pStrings->cardinality())
        return (*pStrings)[xString++];
    return NilOf (char const*);
}

template<> void VTask::ProcessArray<V::VString,V::VString> (
    VDescriptor &rResult, rtPTOKEN_Handle *pPPT, VkDynamicArrayOf<VString> const &rSourceArray, VString const *&rpResultArray
) const {
    rResult.setToCorrespondence (
	pPPT, rtLSTORE_NewStringStore (
            codScratchPad (), DynamicArrayStringAccessFn, &rSourceArray
        )
    );
    rpResultArray = 0;
}

template<> void VTask::ProcessArray<Vxa::ISingleton::Reference,Vxa::ISingleton::Reference> (
    VDescriptor &rResult, rtPTOKEN_Handle *pPPT, VkDynamicArrayOf<Vxa::ISingleton::Reference> const &rSourceArray, Vxa::ISingleton::Reference const *&rpResultArray
) const {
    unsigned int const count = rSourceArray.cardinality ();
    if (count==1) {
//	loadDucWithRepresentative (new VExternalGroundStore (rSourceArray[0]));
	VExternalGroundStore::Reference const pXGS (new VExternalGroundStore (rSourceArray[0]));
	rtVSTORE_Handle::Reference pSurrogate;
	pXGS->getSurrogate (pSurrogate);

	// Return reference to object
	rResult.setToReferenceConstant (pPPT, pSurrogate, pXGS->ptoken_(), 0);
    } else {
	VFragmentation &rFragmentation = loadDucWithFragmentation ();
	for (unsigned int xElement=0; xElement<count; xElement++) {
            // Create a task subset of "1"
	    rtLINK_CType *const pSubset = rtLINK_RefConstructor (pPPT);
	    pSubset->AppendRange(xElement,1);
	    rtPTOKEN_Handle::Reference pSubsetDomain (NewDomPToken (1));
	    pSubset->Close (pSubsetDomain);

            // Create the external ground store
	    VExternalGroundStore::Reference const pXGS (new VExternalGroundStore (rSourceArray[xElement]));
	    rtVSTORE_Handle::Reference pSurrogate;
	    pXGS->getSurrogate (pSurrogate);

            // Return reference to object
	    rFragmentation.createFragment (pSubset)->datum ().setToReferenceConstant (
		pSubsetDomain, pSurrogate, pXGS->ptoken_(), 0
	    );
        }
    }
    rpResultArray = 0;
}


/*****************************************************
 *----  VTask::ReturnArray<Source_T,Result_T> ----*
 *****************************************************/

template <typename Source_T, typename Result_T> void VTask::ReturnArray (
    VkDynamicArrayOf<Source_T> const &rSourceArray, Result_T const *&rpResultArray
) {
    ProcessArray (duc (), ptoken (), rSourceArray, rpResultArray);
    wrapup ();
}
template void VTask::ReturnArray<bool,int>          (VkDynamicArrayOf<bool> const&, int const*&);

template void VTask::ReturnArray<char,int>          (VkDynamicArrayOf<char> const&, int const*&);
template void VTask::ReturnArray<short,int>         (VkDynamicArrayOf<short> const&, int const*&);
template void VTask::ReturnArray<int,int>           (VkDynamicArrayOf<int> const&,int const*&);

template void VTask::ReturnArray<unsigned char,int> (VkDynamicArrayOf<unsigned char> const&, int const*&);
template void VTask::ReturnArray<unsigned short,int>(VkDynamicArrayOf<unsigned short> const&, int const*&);
template void VTask::ReturnArray<unsigned int,int>  (VkDynamicArrayOf<unsigned int> const&, int const*&);

template void VTask::ReturnArray<float,float>       (VkDynamicArrayOf<float> const&,float const*&);
template void VTask::ReturnArray<double,double>     (VkDynamicArrayOf<double> const&,double const*&);

template void VTask::ReturnArray<V::VString,V::VString> (VkDynamicArrayOf<VString> const&,VString const*&);

template void VTask::ReturnArray<Vxa::ISingleton::Reference,Vxa::ISingleton::Reference>(
    VkDynamicArrayOf<Vxa::ISingleton::Reference> const&,Vxa::ISingleton::Reference const*&
);


/*******************************************************
 *----  VTask::ReturnSegment<Source_T,Result_T> ----*
 *******************************************************/

template <typename Source_T, typename Result_T> bool VTask::ReturnSegment (
    object_reference_array_t const &rInjector, VkDynamicArrayOf<Source_T> const &rSourceArray, Result_T const *&rpResultArray
) {
    VFragment *const pFragment = createSegment (rInjector);
    ProcessArray (pFragment->datum (), pFragment->subset ()->PPT (), rSourceArray, rpResultArray);
    return wrapupSegment ();
}
template bool VTask::ReturnSegment<bool,int>           (object_reference_array_t const &rInjector, VkDynamicArrayOf<bool> const&, int const*&);

template bool VTask::ReturnSegment<char,int>           (object_reference_array_t const &rInjector, VkDynamicArrayOf<char> const&, int const*&);
template bool VTask::ReturnSegment<short,int>          (object_reference_array_t const &rInjector, VkDynamicArrayOf<short> const&, int const*&);
template bool VTask::ReturnSegment<int,int>            (object_reference_array_t const &rInjector, VkDynamicArrayOf<int> const&, int const*&);

template bool VTask::ReturnSegment<unsigned char,int>  (object_reference_array_t const &rInjector, VkDynamicArrayOf<unsigned char> const&, int const*&);
template bool VTask::ReturnSegment<unsigned short,int> (object_reference_array_t const &rInjector, VkDynamicArrayOf<unsigned short> const&, int const*&);
template bool VTask::ReturnSegment<unsigned int,int>   (object_reference_array_t const &rInjector, VkDynamicArrayOf<unsigned int> const&, int const*&);

template bool VTask::ReturnSegment<double,double>      (object_reference_array_t const &rInjector, VkDynamicArrayOf<double> const&, double const*&);
template bool VTask::ReturnSegment<float,float>        (object_reference_array_t const &rInjector, VkDynamicArrayOf<float> const&, float const*&);

template bool VTask::ReturnSegment<V::VString,V::VString>    (object_reference_array_t const &rInjector, VkDynamicArrayOf<VString> const&, VString const*&);


/************************************************
 *----  VTask::ReturnSingleton<Source_T> ----*
 ************************************************/

template <typename Source_T> void VTask::ReturnSingleton (Source_T iSingleton) {
    loadDucWith (iSingleton);
    wrapup ();
}
template void VTask::ReturnSingleton<int>         (int);
template void VTask::ReturnSingleton<double>      (double);
template void VTask::ReturnSingleton<float>       (float);
template void VTask::ReturnSingleton<char const*> (char const*);


/***************************************
 *----  VTask::ReturnNASegment  ----*
 ***************************************/

bool VTask::ReturnNASegment (object_reference_array_t const &rInjector) {
    VFragment *const pFragment = createSegment (rInjector);
    pFragment->datum ().setToNA (pFragment->subset ()->PPT (), codKOT ());
    return wrapupSegment ();
}

/***************************************
 *----  VTask::ReturnObject(s)  ----*
 ***************************************/

void VTask::ProcessObjects (
    VDescriptor &rResult, rtPTOKEN_Handle *pPPT, ICollection *pCluster, object_reference_t sCluster, object_reference_array_t const &rxObjects
) const {
    VExternalGroundStore::Reference const pXGS (new VExternalGroundStore (codSpace (), pCluster, sCluster));
    rtVSTORE_Handle::Reference pSurrogate;
    pXGS->getSurrogate (pSurrogate);

    unsigned int const count = rxObjects.cardinality ();
    if (count == 1) {
//	loadDucWithRepresentative (pXGS, rxObjects[0]);
	rResult.setToReferenceConstant (pPPT, pSurrogate, pXGS->ptoken_(), rxObjects[0]);
    } else {
	M_CPD *const pResultUV = rtREFUV_New (pPPT, pXGS->ptoken_());
	rtREFUV_ElementType *const pResultArray = rtREFUV_CPD_Array (pResultUV);
	for (unsigned int xElement = 0; xElement<count; xElement++) {
	    pResultArray[xElement] = rxObjects[xElement];
	}
	rResult.setToMonotype (pSurrogate, pResultUV);
	pResultUV->release ();
    }
}

bool VTask::ReturnObjects (
    object_reference_array_t const &rInjector, ICollection *pCluster, object_reference_t sCluster, object_reference_array_t const &rxObjects
) {
    VFragment *const pFragment = createSegment (rInjector);
    ProcessObjects (pFragment->datum (), pFragment->subset ()->PPT (), pCluster, sCluster, rxObjects);
    return wrapupSegment ();
}

void VTask::ReturnObjects (ICollection *pCluster, object_reference_t sCluster, object_reference_array_t const &rxObjects) {
    ProcessObjects (duc (), ptoken (), pCluster, sCluster, rxObjects);
    wrapup ();
}

void VTask::ReturnObject (ICollection *pCluster, object_reference_t sCluster, object_reference_t xObject) {
    loadDucWithRepresentative (new VExternalGroundStore (codSpace (), pCluster, sCluster), xObject);
    wrapup ();
}

/*******************
 *----  Other  ----*
 *******************/

void VTask::SetOutput (VkDynamicArrayOf<VString> const & rArray){
    VOutputGenerator iOutputGenerator (this);
    for(unsigned int xElement=0; xElement< rArray.cardinality (); xElement++) {
        iOutputGenerator.putString (rArray[xElement]);
        iOutputGenerator.advance ();
    }
}
#endif
