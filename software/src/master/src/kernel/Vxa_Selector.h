#ifndef Vxa_Selector_Interface
#define Vxa_Selector_Interface

/************************
 *****  Components  *****
 ************************/

#include "Vxa.h"

#include "V_VString.h"

/**************************
 *****  Declarations  *****
 **************************/

/*************************
 *****  Definitions  *****
 *************************/

namespace Vxa {
    class Vxa_API Selector {
        DECLARE_NUCLEAR_FAMILY (Selector);

    //  Construction
    public:
        Selector (VString const &rName, cardinality_t cParameters);
        Selector (Selector const &rOther);

    //  Destruction
    public:
        ~Selector ();

    //  Access
    public:
        VString const &fullName () const {
            return m_iName;
        }

        VString const &component (cardinality_t xComponent) const;
        bool component (VString &rComponent, cardinality_t xComponent) const;

        cardinality_t parameterCount () const {
            return m_aComponents.elementCount ();
        }

    //  State
    private:
        VString                   const m_iName;
        VkDynamicArrayOf<VString> const m_aComponents;
    };
}


#endif
