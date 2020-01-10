/***** Vbe_XFed_Export_Implementation  *****/

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

#include "Vbe_XFed_Export.h"

/************************
 *****  Supporting  *****
 ************************/

#include "DSC_Store.h"

#include "Vxa_ICaller.h"
#include "Vxa_IVSNFTaskHolder1.h"


/**************************
 **************************
 *****  Construction  *****
 **************************
 **************************/

Vbe::XFed::Export::Export (DSC_Descriptor const &rStore) : Export (rStore.store ()) {
}

Vbe::XFed::Export::Export (DSC_Store const &rStore) : Export (static_cast<Vdd::Store*>(rStore)) {
}

Vbe::XFed::Export::Export (Vdd::Store *pStore) : m_pStore (pStore), m_pICollection (this) {
}

/*************************
 *************************
 *****  Destruction  *****
 *************************
 *************************/

Vbe::XFed::Export::~Export () {
}


/****************************
 ****************************
 *****  Role Callbacks  *****
 ****************************
 ****************************/

void Vbe::XFed::Export::Bind (
    ICollection *pRole, ICaller *pCaller, VString const &rMethodName, cardinality_t cParameters, cardinality_t cTask
) {
    if (pCaller) {
//        VCallType2 iCallHandle (this, rMethodName, cParameters, cTask, pCaller, true);
//        invokeCall (iCallHandle);
    }
}

void Vbe::XFed::Export::Invoke (
    ICollection *pRole, ICaller *pCaller, VString const &rMethodName, cardinality_t cParameters, cardinality_t cTask
) {
    if (pCaller) {
//        VCallType2 iCallHandle (this, rMethodName, cParameters, cTask, pCaller, false);
//        invokeCall (iCallHandle);
    }
}

void Vbe::XFed::Export::QueryCardinality (ICollection *pRole, IVReceiver<cardinality_t> *pResultReceiver) {
    if (pResultReceiver)
        pResultReceiver->OnData (cardinality ());
}

void Vbe::XFed::Export::ExternalImplementation (
    ISingleton *pRole, IVSNFTaskHolder *pCaller, VString const &rMethodName, cardinality_t cParameters, cardinality_t cTask
) {
/*================*
 *  Call Type 1 NOT SUPPORTED
 *================*/
    Vxa::IVSNFTaskHolder1::Pointer const pCaller1 (dynamic_cast<IVSNFTaskHolder1*>(pCaller));
    if (pCaller1) {
        pCaller1->ReturnError ("Vxa::ISingleton Call Type Not Supported");
    } else {
        pCaller->TurnBackSNFTask ();
    }
}


/********************
 ********************
 *****  Access  *****
 ********************
 ********************/

Vxa::cardinality_t Vbe::XFed::Export::cardinality () const {
    return m_pStore->getPToken ()->cardinality ();
}
