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

#include "V_VRTTI.h"


/***************************************
 ***************************************
 *****                             *****
 *****  Vbe::XFed::CallAgent::Arg  *****
 *****                             *****
 ***************************************
 ***************************************/

Vbe::XFed::CallAgent::Arg::Arg (Vdd::Store *pStore ) : m_pStore (pStore) {
}

Vbe::XFed::CallAgent::Arg::~Arg () {
}


/****************************************
 ****************************************
 *****                              *****
 *****  Vbe::XFed::CallAgent::Arg_  *****
 *****                              *****
 ****************************************
 ****************************************/

template <typename pointer_data_t> class Vbe::XFed::CallAgent::Arg_ : public Arg {
    DECLARE_CONCRETE_RTTLITE (Arg_<pointer_data_t>, Arg);

//  Construction
public:
    Arg_(
        Vdd::Store *pStore, pointer_data_t const &rPointerData
    ) : BaseClass (pStore), m_iPointerData (rPointerData) {
    }

//  Destruction
private:
    ~Arg_() {
    }

//  State
private:
    pointer_data_t m_iPointerData;
};


/************************************************
 ************************************************
 *****                                      *****
 *****  Vbe::XFed::CallAgent::SelfProvider  *****
 *****                                      *****
 ************************************************
 ************************************************/

class Vbe::XFed::CallAgent::SelfProvider : public Vca::VRolePlayer {
    DECLARE_CONCRETE_RTTLITE (SelfProvider, Vca::VRolePlayer);

//  Construction
public:
    SelfProvider (CallAgent *pCallAgent) : m_pCallAgent (pCallAgent) {
    }

//  Destruction
private:
    ~SelfProvider () {
    }

//  Roles

//  State
private:
    CallAgent::Reference const m_pCallAgent;
};


/**********************************
 **********************************
 *****                        *****
 *****  Vbe::XFed::CallAgent  *****
 *****                        *****
 **********************************
 **********************************/

/**************************
 **************************
 *****  Construction  *****
 **************************
 **************************/

Vbe::XFed::CallAgent::CallAgent (
    Vdd::Store *pCluster,
    ICaller *pCaller,
    cardinality_t cTask,
    cardinality_t cParameters,
    VString const &rMethodName,
    bool bIntensional
) : m_pCluster (pCluster), m_pCaller (pCaller), m_pDomain (
    new VTaskDomain (cTask)
), m_iSelector (rMethodName, cParameters), m_bIntensional (
    bIntensional
), m_bGoodToGo (true), m_cSuspensions (0), m_pICallImplementation (this) {
}

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
    raiseUnimplementedOperationException (typeid(*this),"SetToObjects");
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


/***********************
 ***********************
 *****  Execution  *****
 ***********************
 ***********************/

bool Vbe::XFed::CallAgent::startCall () {
    suspend ();
    resume ();
}

void Vbe::XFed::CallAgent::resume () {
    switch (m_cSuspensions) {
    case 0:
        break;
    case 1:
        m_cSuspensions = 0;
        onResume ();
        break;
    default:
        m_cSuspensions--;
        break;
    }
}

void Vbe::XFed::CallAgent::onResume () {
}


/****************************
 ****************************
 *****  Implementation  *****
 ****************************
 ****************************/

bool Vbe::XFed::CallAgent::raiseTypeException (
    std::type_info const &rOriginatorType, std::type_info const &rUnexpectedType, char const *pWhere
) const {
    V::VRTTI iOriginatorRTTI (rOriginatorType);
    V::VRTTI iUnexpectedRTTI (rUnexpectedType);

    VString iMessage (128);
    iMessage << iOriginatorRTTI.name () << ": Unsupported " << pWhere << " Type: " << iUnexpectedRTTI.name ();

    return returnError (iMessage);
}

bool Vbe::XFed::CallAgent::raiseParameterTypeException (
    std::type_info const &rOriginatorType, std::type_info const &rUnexpectedType
) const {
    return raiseTypeException (rOriginatorType, rUnexpectedType, "Parameter");
}

bool Vbe::XFed::CallAgent::raiseResultTypeException (
    std::type_info const &rOriginatorType, std::type_info const &rUnexpectedType
) const {
    return raiseTypeException (rOriginatorType, rUnexpectedType, "Result");
}

bool Vbe::XFed::CallAgent::raiseUnimplementedOperationException (
    std::type_info const &rOriginatorType, char const *pWhere
) const {
    V::VRTTI iOriginatorRTTI (rOriginatorType);
    VString iMessage (128);
    iMessage << iOriginatorRTTI.name () << "::" << pWhere << ": Not Implemented";

    return returnError (iMessage);
}

bool Vbe::XFed::CallAgent::returnError (VString const &rMessage) const {
    m_bGoodToGo = false;

    m_pCaller->ReturnError (rMessage);
    return false; // false -> call failed
}

