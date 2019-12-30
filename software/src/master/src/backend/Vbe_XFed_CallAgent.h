#ifndef Vbe_XFed_CallAgent_Interface
#define Vbe_XFed_CallAgent_Interface

/************************
 *****  Components  *****
 ************************/

#include "Vbe_XFed.h"

#include "Vca_VRolePlayer.h"
#include "Vxa_IVSNFTaskImplementation3NC.h"

/**************************
 *****  Declarations  *****
 **************************/

/*************************
 *****  Definitions  *****
 *************************/

namespace Vbe {
    namespace XFed {

    /****************************************
     *----  class Vbe::XFed::CallAgent  ----*
     ****************************************/
        class CallAgent : public Vca::VRolePlayer {
            DECLARE_CONCRETE_RTTLITE (CallAgent, Vca::VRolePlayer);

        //  Aliases
        public:
            typedef Vxa::IVSNFTaskImplementation3NC ICallImplementation;

        //  Construction
        public:

        //  Destruction
        private:
            ~CallAgent ();

        //  Roles
        //  ...Vxa::ICallImplementation Role
        private:
            Vca::VRole<ThisClass,ICallImplementation> m_pICallImplementation;
        public:
            void getRole (ICallImplementation::Reference &rpRole) {
                m_pICallImplementation.getRole (rpRole);
            }

        //  IVSNFTaskImplementation Methods
	public:
            void SetToInteger (
                IVSNFTaskImplementation *, unsigned int xParameter, i32_array_t const &rValue
            );
            void SetToDouble (
                IVSNFTaskImplementation *, unsigned int xParameter, f64_array_t const &rValue
            );
            void SetToVString (
                IVSNFTaskImplementation *, unsigned int xParameter, str_array_t const &rValue
            );
            void SetToObjects (
                IVSNFTaskImplementation *, unsigned int xParameter, obj_array_t const &rValue
            );

        //  once used as an interface member, but now a dummy member...
            void PopulateVariantCompleted (IVSNFTaskImplementation * pRole) {
            };

        //  IVSNFTaskImplementation2 Methods
        public:
            void SetToS2Integers (
                IVSNFTaskImplementation2*, unsigned int xParameter, i32_s2array_t const &rValue
            );

        //  IVSNFTaskImplementation3 Methods
        public:
            void OnParameterError (
                IVSNFTaskImplementation3*, unsigned int xParameter, VString const &rMsg
            );

        //  IVSNFTaskImplementation3NC Methods
        public:
            void OnParameterError (
                IVSNFTaskImplementation3NC*, unsigned int xParameter, VString const &rMsg
            );

        private:
            void OnParameterError (
                unsigned int xParameter, VString const &rMsg
            );

        //  Access
        public:

        //  State
        private:
        };

    } // namespace XFed
} // namespace Vbe

#endif
