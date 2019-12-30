/***** Vbe_XFed_CallAgent_Implementation  *****/

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

#include "Vbe_XFed_CallAgent.h"

/************************
 *****  Supporting  *****
 ************************/


/**************************
 **************************
 *****  Construction  *****
 **************************
 **************************/

/*************************
 *************************
 *****  Destruction  *****
 *************************
 *************************/

Vbe::XFed::CallAgent::~CallAgent () {
}


/****************************
 ****************************
 *****  Role Callbacks  *****
 ****************************
 ****************************/

/*************************************
 *****  IVSNFTaskImplementation  *****
 *************************************/

void Vbe::XFed::CallAgent::SetToInteger (
    IVSNFTaskImplementation *, unsigned int xParameter, i32_array_t const &rValue
) {
#if 0
    factory_reference_t pParameterFactory;
    if (getParameterFactory (pParameterFactory, xParameter) && pParameterFactory->createFromIntegers (rValue, this)) {
	onParameterReceipt (m_pTask, xParameter);
    } else {
	VString iMsg ("Vxa Parameters Retrieving Error: ");
	iMsg << xParameter;
	onFailure (0, iMsg);
    }
#endif
}

void Vbe::XFed::CallAgent::SetToDouble (
    IVSNFTaskImplementation *, unsigned int xParameter, f64_array_t const &rValue
) {
#if 0
    factory_reference_t pParameterFactory;
    if (getParameterFactory (pParameterFactory, xParameter) && pParameterFactory->createFromDoubles (rValue, this)) {
	onParameterReceipt (m_pTask, xParameter);
    } else {
	VString iMsg ("Vxa Parameters Retrieving Error: ");
	iMsg << xParameter;
	onFailure (0, iMsg);
    }
#endif
}

void Vbe::XFed::CallAgent::SetToVString (
    IVSNFTaskImplementation *, unsigned int xParameter, str_array_t const &rValue
) {
#if 0
    factory_reference_t pParameterFactory;
    if (getParameterFactory (pParameterFactory, xParameter) && pParameterFactory->createFromStrings (rValue, this)) {
	onParameterReceipt (m_pTask, xParameter);
    } else {
	VString iMsg ("Vxa Parameters Retrieving Error: ");
	iMsg << xParameter;
	onFailure (0, iMsg);
    }
#endif
}

void Vbe::XFed::CallAgent::SetToObjects (
    IVSNFTaskImplementation *, unsigned int xParameter, obj_array_t const &rValue
) {
#if 0
    raiseUnimplementedOperationException (typeid(*this),"SetToObjects");
#endif
}

void Vbe::XFed::CallAgent::SetToS2Integers (
    IVSNFTaskImplementation2 *pRole, unsigned int xParameter, i32_s2array_t const &rValue
) {
#if 0
    SetToInteger (pRole, xParameter, rValue);
#endif
}

void Vbe::XFed::CallAgent::OnParameterError (
    IVSNFTaskImplementation3 *pRole, unsigned int xParameter, VString const &rMsg
) {
    OnParameterError (xParameter, rMsg);
}

void Vbe::XFed::CallAgent::OnParameterError (
    IVSNFTaskImplementation3NC *pRole, unsigned int xParameter, VString const &rMsg
) {
    OnParameterError (xParameter, rMsg);
}

void Vbe::XFed::CallAgent::OnParameterError (
    unsigned int xParameter, VString const &rMsg
) {
#if 0
    onParameterReceipt (m_pTask, xParameter);
    reportCompletion ();
    m_pTask->kill ();
    VString iMsg ("Vxa Parameters Retrieving Error: ");
    iMsg << xParameter;
    iMsg << "\n" << rMsg;
    onFailure (0, iMsg);
#endif
}
