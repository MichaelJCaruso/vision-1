/***** Vbe_XFed_TopTask_Implementation  *****/

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

#include "Vbe_XFed_TopTask.h"

/************************
 *****  Supporting  *****
 ************************/


/********************************
 ********************************
 *****                      *****
 *****  Vbe::XFed::TopTask  *****
 *****                      *****
 ********************************
 ********************************/

/***************************
 ***************************
 *****  Run Time Type  *****
 ***************************
 ***************************/

DEFINE_CONCRETE_RTT (Vbe::XFed::TopTask);

/************************
 ************************
 *****  Meta Maker  *****
 ************************
 ************************/

void Vbe::XFed::TopTask::MetaMaker () {
}


/**************************
 **************************
 *****  Construction  *****
 **************************
 **************************/

Vbe::XFed::TopTask::TopTask (
    CallAgent *pCallAgent
) : BaseClass (0,0), m_pCallAgent (pCallAgent), m_pContinuation (&ThisClass::InvokeMessage) {
}

/*************************
 *************************
 *****  Destruction  *****
 *************************
 *************************/

Vbe::XFed::TopTask::~TopTask () {
}

/*******************
 *******************
 *****  Query  *****
 *******************
 *******************/

#if 0
bool Vbe::XFed::TopTask::datumAvailable_() const {
    return false;
}
#endif


/***************************
 ***************************
 *****  Continuations  *****
 ***************************
 ***************************/

void Vbe::XFed::TopTask::InvokeMessage () {
    m_pContinuation = &ThisClass::ReturnResults;
    m_pCallAgent->buildCall (this);
}

void Vbe::XFed::TopTask::ReturnResults () {
}


/***********************
 ***********************
 *****  Execution  *****
 ***********************
 ***********************/

/*****************
 *****  Run  *****
 *****************/

void Vbe::XFed::TopTask::run () {
    switch (m_xStage) {
    case Stage_Unrun:
	if (pausedOnEntry ())
	    return;
	/*****  NO BREAK  *****/

    case Stage_Entry:
	m_xStage = Stage_Running;
	/*****  NO BREAK  *****/

    case Stage_Running:
	while (runnable ()) {
	    if (m_pContinuation) {
		Continuation pContinuation = m_pContinuation;
		m_pContinuation = NilOf (Continuation);

		(this->*pContinuation) ();
	    }
	    else if (pausedOnExit ())
		return;
	    else
		exit ();
	}
	break;

    default:
	exit ();
	break;
    }
}

/**********************************
 **********************************
 *****  Call Builder Helpers  *****
 **********************************
 **********************************/

void Vbe::XFed::TopTask::commitCall (bool bIntensional) {
    commitCall (bIntensional ? Return_Intension : Return_Value);
}


/*************************************
 *************************************
 *****  Display and Description  *****
 *************************************
 *************************************/

void Vbe::XFed::TopTask::reportInfo (unsigned int xLevel, VCall const* pContext) const {
    reportInfoHeader (xLevel);

    report ("%s\n", description ());
}

void Vbe::XFed::TopTask::reportTrace () const  {
    reportTraceHeader ("T");

    report ("%s\n", description ());
}

char const* Vbe::XFed::TopTask::description () const {
    return "<---Vbe::XFed::TopTask--->";
}
