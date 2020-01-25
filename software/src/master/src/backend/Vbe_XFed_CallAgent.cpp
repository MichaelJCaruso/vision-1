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

//  Aliases
public:
    typedef IVReceiver<object_reference_t> ISelfReferenceSink;
    typedef IVReceiver<object_reference_array_t const&> ISelfReferenceArraySink;

//  Construction
public:
    SelfProvider (
        CallAgent *pCallAgent
    ) : m_pCallAgent (pCallAgent), m_pISelfReferenceSink (this), m_pISelfReferenceArraySink (this) {
        retain (); {
            ISelfReferenceArraySink::Reference pSelfSink;
            getRole (pSelfSink);
            m_pCallAgent->caller ()->GetSelfReferences (pSelfSink);
        } untain ();
    }

//  Destruction
private:
    ~SelfProvider () {
    }

//  Roles
public:
    using BaseClass::getRole;

//  ISelfReferenceSink Role
private:
    Vca::VRole<ThisClass,ISelfReferenceSink> m_pISelfReferenceSink;
public:
    void getRole (ISelfReferenceSink::Reference &rpRole) {
        m_pISelfReferenceSink.getRole (rpRole);
    }

//  ISelfReferenceSink Methods
public:
    void OnData (ISelfReferenceSink *pRole, object_reference_t xSelfReference);

//  ISelfReferenceArraySink Role
private:
    Vca::VRole<ThisClass,ISelfReferenceArraySink> m_pISelfReferenceArraySink;
public:
    void getRole (ISelfReferenceArraySink::Reference &rpRole) {
        m_pISelfReferenceArraySink.getRole (rpRole);
    }

//  ISelfReferenceArraySink Methods
public:
    void OnData (ISelfReferenceArraySink *pRole, object_reference_array_t const &rSelfReferences);

//  IClient Methods
protected:
    void OnError_(Vca::IError *pInterface, VString const &rMessage);

//  State
private:
    CallAgent::Reference const m_pCallAgent;
};


/****************************
 ****************************
 *****  Implementation  *****
 ****************************
 ****************************/

void Vbe::XFed::CallAgent::SelfProvider::OnData (
    ISelfReferenceSink *pRole, object_reference_t xSelfReference
) {
    m_pCallAgent->onSelf (new Arg_<object_reference_t>(m_pCallAgent->cluster (), xSelfReference));
}

void Vbe::XFed::CallAgent::SelfProvider::OnData (
    ISelfReferenceArraySink *pRole, object_reference_array_t const &rSelfReferences
) {
    m_pCallAgent->onSelf (new Arg_<object_reference_array_t>(m_pCallAgent->cluster (), rSelfReferences));
}

void Vbe::XFed::CallAgent::SelfProvider::OnError_(Vca::IError *pInterface, VString const &rMessage) {
    m_pCallAgent->returnError (rMessage);
}


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
), m_bGoodToGo (true), m_cSuspensions (cParameters + 2), m_pICallImplementation (this) {
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
    IVSNFTaskImplementation *, unsigned int xArgument, i32_array_t const &rValue
) {
    onArgument (xArgument, new Arg_<i32_array_t>(0, rValue));
}

void Vbe::XFed::CallAgent::SetToDouble (
    IVSNFTaskImplementation *, unsigned int xArgument, f64_array_t const &rValue
) {
    onArgument (xArgument, new Arg_<f64_array_t>(0, rValue));
}

void Vbe::XFed::CallAgent::SetToVString (
    IVSNFTaskImplementation *, unsigned int xArgument, str_array_t const &rValue
) {
    onArgument (xArgument, new Arg_<str_array_t>(0, rValue));
}

void Vbe::XFed::CallAgent::SetToObjects (
    IVSNFTaskImplementation *, unsigned int xArgument, obj_array_t const &rValue
) {
    raiseUnimplementedOperationException (typeid(*this),"SetToObjects");
}

void Vbe::XFed::CallAgent::SetToS2Integers (
    IVSNFTaskImplementation2 *pRole, unsigned int xArgument, i32_s2array_t const &rValue
) {
    onArgument (xArgument, new Arg_<i32_s2array_t>(0, rValue));
}

void Vbe::XFed::CallAgent::OnParameterError (
    IVSNFTaskImplementation3 *pRole, unsigned int xArgument, VString const &rMsg
) {
    OnParameterError (xArgument, rMsg);
}

void Vbe::XFed::CallAgent::OnParameterError (
    IVSNFTaskImplementation3NC *pRole, unsigned int xArgument, VString const &rMsg
) {
    OnParameterError (xArgument, rMsg);
}

void Vbe::XFed::CallAgent::OnParameterError (unsigned int xArgument, VString const &rMsg) {
    VString iMsg ("Argument Retrieval Error: ");
    iMsg << xArgument << "\n" << rMsg.content ();

    returnError (iMsg);
}


/***********************
 ***********************
 *****  Execution  *****
 ***********************
 ***********************/

void Vbe::XFed::CallAgent::start () {
    requestSelf ();
    for (unsigned int xArgument = 0; xArgument < parameterCount (); xArgument++)
        requestArgument (xArgument);
    resume ();
}

void Vbe::XFed::CallAgent::onArgument (unsigned int xArgument, Arg *pArg) {
    if (xArgument < m_apArgs.cardinality () && m_apArgs[xArgument].setIfNil (pArg))
        resume ();
}

void Vbe::XFed::CallAgent::onSelf (Arg *pArg) {
    if (m_pSelf.setIfNil (pArg))
        resume ();
}

void Vbe::XFed::CallAgent::requestArgument (unsigned int xArgument) {
    caller ()->GetParameter (xArgument);
}

void Vbe::XFed::CallAgent::requestSelf () {
    (new SelfProvider (this))->release ();
}

void Vbe::XFed::CallAgent::resume () {
    if (m_cSuspensions > 0 && 0 == --m_cSuspensions)
        finish ();
}

void Vbe::XFed::CallAgent::finish () {
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

