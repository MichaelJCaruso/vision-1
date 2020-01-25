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
#include "Vxa_IVSNFTaskImplementation3.h"
#include "Vxa_IVSNFTaskImplementation3NC.h"

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

        typedef Vxa::IVSNFTaskImplementation3NC IVSNFTaskImplementation3NC;
        typedef Vxa::IVSNFTaskImplementation3   IVSNFTaskImplementation3;
        typedef Vxa::IVSNFTaskImplementation2   IVSNFTaskImplementation2;
        typedef Vxa::IVSNFTaskImplementation    IVSNFTaskImplementation;

        typedef Vxa::i32_array_t i32_array_t;
        typedef Vxa::f64_array_t f64_array_t;
        typedef Vxa::str_array_t str_array_t;
        typedef Vxa::obj_array_t obj_array_t;

        typedef Vxa::i32_s2array_t i32_s2array_t;

        typedef Vxa::object_reference_t       object_reference_t;
        typedef Vxa::object_reference_array_t object_reference_array_t;

    } // namespace XFed
} // namespace Vbe

#endif
