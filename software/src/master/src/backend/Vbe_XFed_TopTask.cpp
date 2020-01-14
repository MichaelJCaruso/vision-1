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

Vbe::XFed::TopTask::TopTask (CallAgent *pCallAgent) : BaseClass (0,0), m_pCallAgent (pCallAgent) {
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
bool Vbe::XFed::TopTask::datumAvailable_ () const {
    return false;
}
#endif


/***********************
 ***********************
 *****  Execution  *****
 ***********************
 ***********************/

/*****************
 *****  Run  *****
 *****************/

void Vbe::XFed::TopTask::run () {
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
