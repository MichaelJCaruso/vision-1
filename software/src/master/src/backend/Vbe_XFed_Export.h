#ifndef Vbe_XFed_Export_Interface
#define Vbe_XFed_Export_Interface

/************************
 *****  Components  *****
 ************************/

#include "Vbe_XFed.h"

#include "Vca_VRolePlayer.h"
#include "Vxa_ICollection.h"

/**************************
 *****  Declarations  *****
 **************************/

#include "DSC_Descriptor.h"

/*************************
 *****  Definitions  *****
 *************************/

namespace Vbe {
    namespace XFed {

    /*************************************
     *----  class Vbe::XFed::Export  ----*
     *************************************/
        class Export : public Vca::VRolePlayer {
            DECLARE_CONCRETE_RTTLITE (Export, Vca::VRolePlayer);

        //  Aliases
        public:
            typedef Vxa::ICollection ICollection;

        //  Construction
        public:
            Export (DSC_Descriptor const &rDSC);
            Export (DSC_Store const &rStore);
            Export (Vdd::Store *pStore);

        //  Destruction
        private:
            ~Export ();

        //  Roles
        //  ...Vxa::ICollection Role
        private:
            Vca::VRole<ThisClass,ICollection> m_pICollection;
        public:
            void getRole (ICollection::Reference &rpRole) {
                m_pICollection.getRole (rpRole);
            }

        //  ...Vxa::ICollection Methods
        public:
            void Bind (
                ICollection *pRole, ICaller *pCaller, VString const &rMessageName, cardinality_t cParameters, cardinality_t cTask
            );
            void Invoke (
                ICollection *pRole, ICaller *pCaller, VString const &rMessageName, cardinality_t cParameters, cardinality_t cTask
            );
            void QueryCardinality (ICollection *pRole, IVReceiver<cardinality_t> *pResultReceiver);

        //  ...Vxa::ISingleton Methods
        public:
            void ExternalImplementation (
                ISingleton *pRole, IVSNFTaskHolder *pCaller, VString const &rMessageName, cardinality_t cParameters, cardinality_t cTask
            );

        //  Access
        public:
            cardinality_t cardinality () const;

        //  State
        private:
            Vdd::Store::Reference const m_pStore;
        };

    } // namespace XFed
} // namespace Vbe

#endif
