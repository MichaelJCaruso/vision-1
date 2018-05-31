/*****  Vsa_VPrompt Implementation  *****/

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

#include "Vsa_VPrompt.h"

/************************
 *****  Supporting  *****
 ************************/

#include "VReceiver.h"

#include "Vca_IDirectory.h"

#include "Vca_VBSConsumer.h"
#include "Vca_VBSProducer.h"

#include "Vca_VInterfaceEKG.h"
#include "Vca_VOutputSequencer.h"
#include "Vca_VTrigger.h"

#include "Vsa_IEvaluationResult.h"
#include "Vsa_IPoolEvaluation.h"

#include "Vsa_VEvaluatorClient.h"
#include "VTransientServices.h"

#include "cam.h"

#include "Vca_VNotifier.h"

extern     Vca::VGoferInterface<Vca::IInfoServer>::Reference g_pInfoServerGofer;


/***********************************
 ***********************************
 *****                         *****
 *****  Vsa::XPrompt::Tracker  *****
 *****                         *****
 ***********************************
 ***********************************/

#include "Vxa_VCollectable.h"
#include "Vxa_VCollectableObject.h"

/*****************************************************************
 *  Sample Vision Usage:

# Run once to prime batchvision session:
!tracker <- 999;  # <- run once
?g

# Run as many times as necessary:
JS jsObject do: [^self getQueryID print; ^self setTrackID: ^my :tracker increment. print; ^self getTrackID printNL];
?r 5

 *
 *****************************************************************/
namespace Vsa {
    namespace XPrompt {
        typedef int id_t;

        class Tracker : public Vxa::VCollectableObject {
            DECLARE_CONCRETE_RTTLITE (Tracker, Vxa::VCollectableObject);

        //  Class Builder
        public:
            class ClassBuilder : public Vxa::VCollectableObject::ClassBuilder {
            protected:
                ClassBuilder (Vxa::VClass *pClass);
            };
            friend ClassBuilder;

        //  ID Generator
        private:
            static id_t NewID ();

        //  Construction
        public:
            Tracker ();
        private:
        //  Destruction
            ~Tracker ();

        //  Methods
        protected:
            void GetQueryID (Vxa::VResultBuilder &rResult);
            void GetTrackID (Vxa::VResultBuilder &rResult);
            void SetTrackID (Vxa::VResultBuilder &rResult, id_t xTrackId);

        //  State
        private:
            id_t const m_xQueryID;
            id_t       m_xTrackID;
        }; // Tracker

    /******************/
        id_t Tracker::NewID () {
            static id_t xNextID = 0;
            return xNextID++;
        }

        Tracker::Tracker () : m_xQueryID (NewID ()), m_xTrackID (-1) {
        }
        Tracker::~Tracker () {
        }

        void Tracker::GetQueryID (Vxa::VResultBuilder &rResult) {
            rResult = m_xQueryID;
        }
        void Tracker::GetTrackID (Vxa::VResultBuilder &rResult) {
            rResult = m_xTrackID;
        }
        void Tracker::SetTrackID (Vxa::VResultBuilder &rResult, id_t xTrackID) {
            rResult = m_xTrackID;
            m_xTrackID = xTrackID;
        }

    /******************/
        Tracker::ClassBuilder::ClassBuilder (Vxa::VClass *pClass) : BaseClass::ClassBuilder (pClass) {
            defineMethod ("getQueryID" , &Tracker::GetQueryID);
            defineMethod ("getTrackID" , &Tracker::GetTrackID);
            defineMethod ("setTrackID:", &Tracker::SetTrackID);
        }
    }
}
namespace {
    Vxa::VCollectable<Vsa::XPrompt::Tracker> g_iMeta;
}
DEFINE_VXA_COLLECTABLE(Vsa::XPrompt::Tracker);


/***************************************
 ***************************************
 *****                             *****
 *****  Vsa::VPrompt::QueryOutput  *****
 *****                             *****
 ***************************************
 ***************************************/

namespace Vsa {
    class VPrompt::QueryOutput : public Vca::VOutputSequencer::Output, public Vsa::VEvaluatorClient {
	DECLARE_CONCRETE_RTTLITE (QueryOutput, Vca::VOutputSequencer::Output);

    //  Aliases
    public:
        typedef Vca::IRoleProvider2 IRoleProvider2;
        typedef Vca::ITrigger ITrigger;

	typedef Reference QueryOutputReference;
	typedef Vca::VInterfaceEKG<ThisClass,IVUnknown> interface_monitor_t;

    //  Starter
    public:
	class Starter : public Vca::VRolePlayer {
	    DECLARE_CONCRETE_RTTLITE (Starter, VRolePlayer);

	//  Construction
	public:
	    Starter (QueryOutput* pQueryOutput);

	//  Destruction
	private:
	    ~Starter () {
	    }

	//  Access
	public:
	    EvaluatorGofer* evaluatorGofer () const {
		return m_pQueryOutput->evaluatorGofer();
	    }

	//  Callbacks
	private:
	    void OnEvaluator (IEvaluator* pEvaluator);
	    virtual void OnError_(IError* pInterface, VString const& rMessage) OVERRIDE;

	//  State
	private:
	    QueryOutputReference const m_pQueryOutput;
	};
	friend class Starter;

    //  Timing Receiver
    public:
	typedef VReceiver<ThisClass,VString const&> StringReceiver;

    //  Timing Output
    public:
	class TimingOutput : public Vca::VOutputSequencer::Output {
	    DECLARE_CONCRETE_RTTLITE (TimingOutput, Vca::VOutputSequencer::Output);

	    friend class QueryOutput;

	//  Construction
	public:
	    TimingOutput (QueryOutput *pQuery, ThisClass *pSuccessor);

	//  Destruction
	private:
	    ~TimingOutput () {
	    }

	//  Use
	private:
	    virtual void load (Fifo &rOutputFifo) OVERRIDE;

	//  State
	private:
	    QueryOutputReference const m_pQuery;
	    Reference m_pSuccessor;
	};

    //  Construction
    public:
	QueryOutput (VPrompt *pPrompt, VString const &rPath, VString const &rQuery);

    //  Destruction
    private:
	~QueryOutput ();

    //  Access/Query
    protected:
	virtual void getDescription_(VString &rResult) const OVERRIDE;
    public:
	IEvaluator* evaluator () const {
	    return m_pPrompt->evaluator ();
	}
	EvaluatorGofer* evaluatorGofer () const {
	    return m_pPrompt->evaluatorGofer ();
	}

    //  VEvaluatorClient Callbacks
    private:
        void CheckIt (VTypeInfo *pInterfaceType, VString const &rWhere) const;
        void ShowIt  (VTypeInfo *pInterfaceType, VString const &rWhere) const;
        void Gotcha  (VTypeInfo *pInterfaceType, VString const &rWhere) const;
    public:
        virtual void QueryInterface (
            IVUnknown *pRole, VTypeInfo *pInterfaceType, IObjectSink *pInterfaceSink
        ) OVERRIDE;
        virtual void QueryRole2 (
            Vca::IRoleProvider2 *pRole, IStatusSink *pStatusSink, IObjectSink *pInterfaceSink, VTypeInfo *pInterfaceType, IVUnknown *pAggregator
        ) OVERRIDE;
        virtual void QueryRole (
            Vca::IRoleProvider *pRole, ITrigger *pStatusSink, IObjectSink *pInterfaceSink, VTypeInfo *pInterfaceType, IVUnknown *pAggregator
        ) OVERRIDE;

    public:
	virtual void OnAccept (IEvaluatorClient *pRole, IEvaluation *pEvaluation, Vca::U32 xQueuePosition) OVERRIDE {
	    OnAccept_(pEvaluation, xQueuePosition);
	}
	virtual void OnAccept_(IEvaluation *pEvaluation, Vca::U32 xQueuePosition) OVERRIDE;
	virtual void OnResult_(IEvaluationResult *pResult, VString const &rOutput) OVERRIDE;

    //  Value Callbacks
    protected:
	virtual void OnError_(IError* pInterface, VString const& rMessage) OVERRIDE;
	void OnResultError (IError* pInterface, VString const& rMessage);
    private:
	void OnEvaluator (IEvaluator* pEvaluator);

    //  Timing
    public:
	void outputTiming ();

	void loadTimingReport (V::VFifoLite &rOutputFifo);

	Status timingStatus () const {
	    return m_bTimingValid ? Status_Available : Status_Blocked;
	}

    private:
	void requestTimingReport ();

	void onTimingReport (StringReceiver *pRole, VString const &rTimingReport);
	void onTimingReportError (StringReceiver *pRole, IError* pInterface, VString const& rMessage);
	void onTimingReportEnd (StringReceiver *pRole);

	void satisfyTimingOutput ();

    //  Monitoring
    private:
	virtual void monitorInterface (IVUnknown* pInterface) OVERRIDE;
	void cancelInterfaceMonitor();
	void signalInterfaceMonitor ();
	void cancelResultMonitor();
	void signalResultMonitor ();

    //  Update
    private:
	virtual void load (Fifo &rOutputFifo) OVERRIDE;

    //  State
    private:
	VPrompt::Reference	const	m_pPrompt;
	VString			const	m_iPath;
	VString			const	m_iQuery;
	interface_monitor_t::Reference	m_pInterfaceMonitor;
	interface_monitor_t::Reference	m_pResultMonitor;
	IEvaluationResult::Reference	m_pResult;
	VString				m_iOutput;
	VString				m_iTiming;
	TimingOutput::Reference		m_pTimingOutput;
	bool				m_bTimingRequested;
	bool				m_bTimingValid;
        bool                            m_bEOL;
        CAM::Operation                  m_iCamOp;
    };
}


/**************************
 **************************
 *****  Construction  *****
 **************************
 **************************/

Vsa::VPrompt::QueryOutput::QueryOutput (VPrompt* pPrompt, VString const& rPath, VString const& rQuery)
    : BaseClass (pPrompt->outputSequencer (), Status_Blocked)
    , m_pPrompt		(pPrompt)
    , m_iPath		(rPath)
    , m_iQuery		(rQuery)
    , m_bTimingRequested(false)
    , m_bTimingValid	(false)
    , m_bEOL            (false)
{
    m_iCamOp = (StreamCapture() << "file" << __FILE__ << "line" << __LINE__ 
        << "label" << "Vsa::VPrompt::QueryOutput::QueryOutput"
    );
    m_iCamOp.detachFromCamStack();

//    aggregate (pPrompt->queryRoleGofer ());

    aggregate (Vxa::Export (new XPrompt::Tracker ()));

    retain (); {
	if (IEvaluator* const pEvaluator = pPrompt->evaluator ())
	    OnEvaluator (pEvaluator);
	else {
	    (new Starter (this))->discard ();
	}
	startThis ();
    } untain ();
}

/*************************
 *************************
 *****  Destruction  *****
 *************************
 *************************/

Vsa::VPrompt::QueryOutput::~QueryOutput () {
}

/************************
 ************************
 *****  Entrapment  *****
 ************************
 ************************/

void Vsa::VPrompt::QueryOutput::CheckIt (VTypeInfo *pInterfaceType, VString const &rWhere) const {
    return m_bEOL ? Gotcha (pInterfaceType, rWhere) : ShowIt (pInterfaceType, rWhere);
}

void Vsa::VPrompt::QueryOutput::ShowIt (VTypeInfo *pInterfaceType, VString const &rWhere) const {
    VString iTypeName;
    pInterfaceType->getName (iTypeName);
    std::cerr << this << ": " << iTypeName << " " << rWhere << std::endl;
}

void Vsa::VPrompt::QueryOutput::Gotcha (VTypeInfo *pInterfaceType, VString const &rWhere) const {
    VString iGotcha (rWhere);
    iGotcha << " GOTCHA !!!";
    ShowIt (pInterfaceType, iGotcha);
}

void Vsa::VPrompt::QueryOutput::QueryInterface (
    IVUnknown *pRole, VTypeInfo *pInterfaceType, IObjectSink *pInterfaceSink
) {
    CheckIt (pInterfaceType, "QueryInterface");
    BaseClass::QueryInterface (pRole, pInterfaceType, pInterfaceSink);
}

void Vsa::VPrompt::QueryOutput::QueryRole2 (
    IRoleProvider2 *pRole, IStatusSink *pStatusSink, IObjectSink *pInterfaceSink, VTypeInfo *pInterfaceType, IVUnknown *pAggregator
) {
    CheckIt (pInterfaceType, "QueryRole2");
    BaseClass::QueryRole2 (pRole, pStatusSink, pInterfaceSink, pInterfaceType, pAggregator);
}

void Vsa::VPrompt::QueryOutput::QueryRole (
    IRoleProvider *pRole, ITrigger *pStatusSink, IObjectSink *pInterfaceSink, VTypeInfo *pInterfaceType, IVUnknown *pAggregator
) {
    CheckIt (pInterfaceType, "QueryRole");
    BaseClass::QueryRole (pRole, pStatusSink, pInterfaceSink, pInterfaceType, pAggregator);
}

/********************
 ********************
 *****  Access  *****
 ********************
 ********************/

void Vsa::VPrompt::QueryOutput::getDescription_(VString &rResult) const {
    BaseClass::getDescription_(rResult);
    rResult << ": " << m_iQuery;
}


/********************
 ********************
 *****  Timing  *****
 ********************
 ********************/

void Vsa::VPrompt::QueryOutput::outputTiming () {
    m_pTimingOutput.setTo (new TimingOutput (this, m_pTimingOutput));

    if (m_bTimingValid)
	m_pTimingOutput.clear ();
    else if (m_pResult)
	requestTimingReport ();
}

void Vsa::VPrompt::QueryOutput::loadTimingReport (V::VFifoLite &rOutputFifo) {
    rOutputFifo.setToData (m_iTiming, m_iTiming.length ());
}

void Vsa::VPrompt::QueryOutput::requestTimingReport () {
    if (!m_bTimingRequested) {
	m_bTimingRequested = true;

	StringReceiver::Reference const pTimingReportReceiver (
	    new StringReceiver (this, &ThisClass::onTimingReport, &ThisClass::onTimingReportEnd, &ThisClass::onTimingReportError)
	);

	m_pResultMonitor.setTo (new interface_monitor_t (this, &ThisClass::signalResultMonitor, m_pResult));
	m_pResult->GetTimingReport (pTimingReportReceiver);
    }
}

void Vsa::VPrompt::QueryOutput::onTimingReport (
    StringReceiver *pRole, VString const &rTimingReport
) {
    cancelResultMonitor ();
    m_iTiming.setTo (rTimingReport);
    satisfyTimingOutput ();
}

void Vsa::VPrompt::QueryOutput::onTimingReportError (
    StringReceiver *pRole, IError* pInterface, VString const& rMessage
) {
    cancelResultMonitor ();
}

void Vsa::VPrompt::QueryOutput::onTimingReportEnd (StringReceiver *pRole) {
    cancelResultMonitor ();
}

void Vsa::VPrompt::QueryOutput::satisfyTimingOutput () {
    m_bTimingRequested = m_bTimingValid = true;
    while (m_pTimingOutput) {
	m_pTimingOutput->setStatusToAvailable ();
	m_pTimingOutput.claim (m_pTimingOutput->m_pSuccessor);
    }
}


/************************
 ************************
 *****  Monitoring  *****
 ************************
 ************************/

void Vsa::VPrompt::QueryOutput::monitorInterface (IVUnknown* pInterface) {
    m_pInterfaceMonitor.setTo (new interface_monitor_t (this, &ThisClass::signalInterfaceMonitor, pInterface));
}

void Vsa::VPrompt::QueryOutput::cancelInterfaceMonitor () {
    m_bEOL = true;

    interface_monitor_t::Reference pInterfaceMonitor;
    pInterfaceMonitor.claim (m_pInterfaceMonitor);
    if (pInterfaceMonitor)
	pInterfaceMonitor->cancel ();
}

void Vsa::VPrompt::QueryOutput::signalInterfaceMonitor () {
    notify (18, "#18: Vsa::VPrompt: pool/batchvision this vprompt was connected to is gone\n");
    OnError_(0, "Disconnected");
}

void Vsa::VPrompt::QueryOutput::cancelResultMonitor () {
    interface_monitor_t::Reference pResultMonitor;
    pResultMonitor.claim (m_pResultMonitor);
    if (pResultMonitor)
	pResultMonitor->cancel ();
}

void Vsa::VPrompt::QueryOutput::signalResultMonitor () {
    notify (18, "#18: Vsa::VPrompt: Evaluation result monitor is signaled.\n");
    OnResultError (0, "Result no longer available.");
}

/********************
 ********************
 *****  Update  *****
 ********************
 ********************/

void Vsa::VPrompt::QueryOutput::load (Fifo &rOutputFifo) {
    rOutputFifo.setToData (m_iOutput, m_iOutput.length ());
    setStatusToDone ();
}


/***********************
 ***********************
 *****  Callbacks  *****
 ***********************
 ***********************/

/*******************
 *****  Value  *****
 *******************/

void Vsa::VPrompt::QueryOutput::OnError_(
    IError *pInterface, VString const &rMessage
) {
    cancelInterfaceMonitor ();

    VEvaluatorClient::onFailure (pInterface, rMessage);

    display ("\nOnError: %s\n", rMessage.content ());

    m_iOutput.setTo (rMessage);
    satisfyTimingOutput ();
    if (m_pPrompt->exitOnError ()) {
	fprintf (stderr, "%s\n", rMessage.content ());
	fflush (stderr);
	Vca::Exit (V::ExitErrorCode ());
    }
    else
	setStatusToAvailable ();
}

void Vsa::VPrompt::QueryOutput::OnResultError (
    IError *pInterface, VString const &rMessage
) {
    cancelResultMonitor ();

    VEvaluatorClient::onFailure (pInterface, rMessage);

    display ("\nOnResultError: %s\n", rMessage.content ());

    m_iOutput.setTo (rMessage);
    satisfyTimingOutput ();
    setStatusToAvailable ();
}

void Vsa::VPrompt::QueryOutput::OnEvaluator (IEvaluator* pEvaluator) {
    onQuery (pEvaluator, m_iPath, m_iQuery);
}

/******************
 *****  Role  *****
 ******************/

void Vsa::VPrompt::QueryOutput::OnAccept_(IEvaluation *pEvaluation, Vca::U32 xQueuePosition) {
    CAM_OPERATION(co) << "message" << 
        "Vsa::VPrompt::QueryOutput::OnAccept_ queue position %queuePosition" << xQueuePosition
    ;
    co.stitchParent(&m_iCamOp);

    IPoolEvaluation::Reference const pPoolEvaluation (dynamic_cast <IPoolEvaluation*> (pEvaluation));
    if (pPoolEvaluation && m_pPrompt->usingAnyData ()) {
	pPoolEvaluation->UseAnyGenerationWorker ();
    }
}

void Vsa::VPrompt::QueryOutput::OnResult_(IEvaluationResult *pResult, VString const &rOutput) {
    CAM_OPERATION(co) << "message" << "receiving query result of size %size" << rOutput.length();
    co.stitchParent(&m_iCamOp);

    cancelInterfaceMonitor ();

    VEvaluatorClient::onSuccess ();

    m_pResult.setTo (pResult);
    m_iOutput.setTo (rOutput);
    if (m_pResult.isNil ())
	satisfyTimingOutput ();
    else if (m_pTimingOutput)
	requestTimingReport ();
    setStatusToAvailable ();
}


/************************************************
 ************************************************
 *****                                      *****
 *****  Vsa::VPrompt::QueryOutput::Starter  *****
 *****                                      *****
 ************************************************
 ************************************************/

/**************************
 **************************
 *****  Construction  *****
 **************************
 **************************/

Vsa::VPrompt::QueryOutput::Starter::Starter (QueryOutput *pQueryOutput) : m_pQueryOutput (pQueryOutput) {
    retain (); {
	evaluatorGofer()->supplyMembers (this, &ThisClass::OnEvaluator, &ThisClass::OnError_);
    } untain ();
}

/***********************
 ***********************
 *****  Callbacks  *****
 ***********************
 ***********************/

void Vsa::VPrompt::QueryOutput::Starter::OnEvaluator (IEvaluator* pEvaluator) {
    m_pQueryOutput->OnEvaluator (pEvaluator);
}

void Vsa::VPrompt::QueryOutput::Starter::OnError_(IError* pInterface, VString const& rMessage) {
    m_pQueryOutput->OnError_(pInterface, rMessage);
}


/*****************************************************
 *****************************************************
 *****                                           *****
 *****  Vsa::VPrompt::QueryOutput::TimingOutput  *****
 *****                                           *****
 *****************************************************
 *****************************************************/

/**************************
 **************************
 *****  Construction  *****
 **************************
 **************************/

Vsa::VPrompt::QueryOutput::TimingOutput::TimingOutput (
    QueryOutput *pQuery, ThisClass *pSuccessor
) : BaseClass (pQuery->sequencer (), pQuery->timingStatus ()), m_pQuery (pQuery), m_pSuccessor (pSuccessor) {
    retain (); {
	startThis ();
    } untain ();
}

/*****************
 *****************
 *****  Use  *****
 *****************
 *****************/

void Vsa::VPrompt::QueryOutput::TimingOutput::load (Fifo &rOutputFifo) {
    m_pQuery->loadTimingReport (rOutputFifo);
    setStatusToDone ();
}


/**************************
 **************************
 *****                *****
 *****  Vsa::VPrompt  *****
 *****                *****
 **************************
 **************************/

/**************************
 **************************
 *****  Construction  *****
 **************************
 **************************/

Vsa::VPrompt::VPrompt (
    VClientApplication* pApplication, EvaluatorGofer* pEvaluatorGofer, IEvaluator* pEvaluator, VBSConsumer* pStreamToPeer
) : BaseClass			(pApplication),
    m_iQueryRoleProviderName	(commandStringValue ("bridgeServer", "FASTBridgeServer")),
    m_pEvaluator		(pEvaluator),
    m_pEvaluatorGofer		(pEvaluatorGofer),
    m_pOutputSequencer		(new Sequencer (pStreamToPeer)),
    m_bUsingExtendedPrompts	(commandSwitchValue ("E")),
    m_bUsingAnyData		(commandSwitchValue ("anyData")),
    m_bExitOnError              (commandSwitchValue ("exitOnError", "VPromptExitOnError"))
{
    retain (); {
	m_pOutputSequencer->onEndTrigger (new CallbackTrigger(this, &ThisClass::onEndTrigger));

	outputPrompt ();
    } untain ();
}

/*************************
 *************************
 *****  Destruction  *****
 *************************
 *************************/

Vsa::VPrompt::~VPrompt () {
}

/********************
 ********************
 *****  Access  *****
 ********************
 ********************/

Vsa::VPrompt::RoleGofer* Vsa::VPrompt::queryRoleGofer() const {
    return m_iQueryRoleProviderName.isntEmpty () ? new Vca::Gofer::Named<IRoleProvider,Vca::IDirectory>(m_iQueryRoleProviderName) : 0;
}

/********************************
 ********************************
 *****  Event Subscription  *****
 ********************************
 ********************************/

void Vsa::VPrompt::onEndTrigger (CallbackTrigger* pTrigger) {
    onDone ();
}


/******************************
 ******************************
 *****  Input Processing  *****
 ******************************
 ******************************/

bool Vsa::VPrompt::onInputLine (char const *pLine, size_t sLine) {
    if (sLine >= 4 && strncmp (pLine, "QUIT", 4) == 0) {
	return false;
    }

    if (sLine < 1)
	m_iThisQuery << "\n";
    else if (pLine[0] != '?') {
	m_iThisQuery.append (pLine, sLine);
	m_iThisQuery << "\n";
    }
    else if (sLine > 1) {
        int cRepetitionsRequested = 0;
	switch (pLine[1]) {
	case 'f':
	    m_iThisQuery.clear ();
	    break;
        case 'r':
            cRepetitionsRequested = atoi (&pLine[2]) - 1;
	case 'g':
	case 'G':
	    outputQuery (m_iThisQuery);
	    m_iLastQuery.setTo (m_iThisQuery);
            while (cRepetitionsRequested-- > 0)
                outputQuery (m_iThisQuery);
	    m_iThisQuery.clear ();
	    break;
	case 'l':
	case 'L':
	    outputQuery (m_iLastQuery);
	    break;
	case 'o':
	    if (sLine > 3) {
		switch (pLine[2]) {
		case 'E':
		    switch (pLine[3]) {
		    case '+':
			setUsingExtendedPromptsTo (true);
			break;
		    case '-':
			setUsingExtendedPromptsTo (false);
			break;
		    default:
			break;
		    }
		    break;
		default:
		    break;
		}
	    }
	    break;
	case 's':
	    outputString (m_iThisQuery);
	    break;
	case 'S':
	    outputString (m_iLastQuery);
	    break;
	case 't':
	    outputQueryTiming ();
	    break;
	default:
	    break;
	}
    }

    outputPrompt ();

    return true;
}


/*******************************
 *******************************
 *****  Output Generation  *****
 *******************************
 *******************************/

void Vsa::VPrompt::outputEnd () const {
    m_pOutputSequencer->outputTerminator ();
}

void Vsa::VPrompt::outputPrompt () const {
    static VString iStandardPrompt ("V> ", false);
    static VString iExtendedPrompt ("V> \005\n", false);
    m_pOutputSequencer->outputString (m_bUsingExtendedPrompts ? iExtendedPrompt : iStandardPrompt);
}

void Vsa::VPrompt::outputQuery (VString const &rQuery) {
    CAM_OPERATION(co) << "message" << "running query: %query" << rQuery;
    m_pLastQuery.setTo (new QueryOutput (this, m_iPath, rQuery));
}

void Vsa::VPrompt::outputQueryTiming () const {
    if (m_pLastQuery)
	m_pLastQuery->outputTiming ();
}

void Vsa::VPrompt::outputString (VString const &rString) const {
    m_pOutputSequencer->outputString (rString);
}


/********************************************************
 ********************************************************
 ****  EXPLICITLY Mandated Template Instantiations  *****
 ********************************************************
 ********************************************************/

template class Vca::VLineGrabber_<Vsa::VPrompt>;
