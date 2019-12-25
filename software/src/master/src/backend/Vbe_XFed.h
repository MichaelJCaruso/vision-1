#ifndef Vbe_XFed_Interface
#define Vbe_XFed_Interface

/************************
 *****  Components  *****
 ************************/

#include "Vbe.h"

/**************************
 *****  Declarations  *****
 **************************/

#include "Vxa_ICaller.h"
#include "Vxa_ICollection.h"

/*************************
 *****  Definitions  *****
 *************************/

namespace Vbe {
    namespace XFed {

    /*********************
     *----  Aliases  ----*
     *********************/
        typedef Vxa::cardinality_t cardinality_t;

        typedef Vxa::ICaller          ICaller;
        typedef Vxa::IVSNFTaskHolder2 IVSNFTaskHolder2;
        typedef Vxa::IVSNFTaskHolder1 IVSNFTaskHolder1;
        typedef Vxa::IVSNFTaskHolder  IVSNFTaskHolder;

        typedef Vxa::ICollection ICollection;
        typedef Vxa::ISingleton  ISingleton;
    } // namespace XFed
} // namespace Vbe

#endif
