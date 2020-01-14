#ifndef Vbe_XFed_TopTask_Interface
#define Vbe_XFed_TopTask_Interface

/************************
 *****  Components  *****
 ************************/

#include "VTopTaskBase.h"

#include "Vbe_XFed_CallAgent.h"

/**************************
 *****  Declarations  *****
 **************************/

/*************************
 *****  Definitions  *****
 *************************/

namespace Vbe {
    namespace XFed {

    /**************************************
     *----  class Vbe::XFed::TopTask  ----*
     **************************************/
        class TopTask : public VTopTaskBase {
            DECLARE_CONCRETE_RTT (TopTask, VTopTaskBase);

        //  Meta Maker
        protected:
            static void MetaMaker ();

        //  Construction
        public:
            TopTask (CallAgent *pAgent);

        //  Destruction
        private:
            ~TopTask ();

        //  Execution
        protected:
            virtual void run () OVERRIDE;

        //  Display and Description
        public:
            virtual void reportInfo (unsigned int xLevel, VCall const* pContext = 0) const OVERRIDE;;
            virtual void reportTrace () const OVERRIDE;

            virtual char const* description () const OVERRIDE;

        //  State
        private:
            CallAgent::Reference const m_pCallAgent;
        };

    } // namespace XFed
} // namespace Vbe

#endif
