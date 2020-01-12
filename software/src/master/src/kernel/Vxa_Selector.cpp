/*****  Vxa_Selector Implementation  *****/

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

#include "Vxa_Selector.h"

/************************
 *****  Supporting  *****
 ************************/


/***************************
 ***************************
 *****                 *****
 *****  Vxa::Selector  *****
 *****                 *****
 ***************************
 ***************************/

/**************************
 **************************
 *****  Construction  *****
 **************************
 **************************/

Vxa::Selector::Selector (
    VString const &rName, cardinality_t cParameters
) : m_iName (rName), m_aComponents (cParameters) {
    VString iNameResidue (rName); VString iNextResidue;
    for (cardinality_t xComponent = 0;
         xComponent < cParameters && iNameResidue.getPrefix (
             ':', const_cast<VString&>(m_aComponents[xComponent]), iNextResidue
         );
         xComponent++
    ) {
        iNameResidue.setTo (iNextResidue);
    }
}

Vxa::Selector::Selector (
    Selector const &rOther
) : m_iName (rOther.m_iName), m_aComponents (rOther.m_aComponents) {
}

/*************************
 *************************
 *****  Destruction  *****
 *************************
 *************************/

Vxa::Selector::~Selector () {
}

/********************
 ********************
 *****  Access  *****
 ********************
 ********************/

V::VString const &Vxa::Selector::component (cardinality_t xComponent) const {
    return xComponent < parameterCount () ? m_aComponents[xComponent] : m_iName;
}

bool Vxa::Selector::component (VString &rComponent, unsigned int xComponent) const {
    if (xComponent < parameterCount ()) {
        rComponent.setTo (m_aComponents[xComponent]);
        return true;
    }
    return false;
}
