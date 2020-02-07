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

        class CallAgent;

    /**************************************
     *----  class Vbe::XFed::TopTask  ----*
     **************************************/
        class TopTask : public VTopTaskBase {
            DECLARE_CONCRETE_RTT (TopTask, VTopTaskBase);

        //  Friends
            friend class CallAgent;

        //  Aliases
        public:
            typedef void (ThisClass::*Continuation) ();

        //  Friends
            friend class CallAgent;

        //  Meta Maker
        protected:
            static void MetaMaker ();

        //  Construction
        public:
            TopTask (CallAgent *pAgent);

        //  Destruction
        private:
            ~TopTask ();

        //  Continuations
        private:
            void InvokeMessage ();
            void ReturnResults ();

        //  Execution
        protected:
            virtual void run () OVERRIDE;

        //  Call Builder Helpers
        public:
            using BaseClass::commitCall;
            void commitCall (bool bIntensional);

        //  Display and Description
        public:
            virtual void reportInfo (unsigned int xLevel, VCall const* pContext = 0) const OVERRIDE;;
            virtual void reportTrace () const OVERRIDE;

            virtual char const* description () const OVERRIDE;

        //  State
        private:
            CallAgent::Reference const m_pCallAgent;
            Continuation m_pContinuation;
        };

    } // namespace XFed
} // namespace Vbe

#endif
