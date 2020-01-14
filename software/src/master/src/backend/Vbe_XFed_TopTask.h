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
            DECLARE_CONCRETE_RTTLITE (TopTask, VTopTaskBase);

        //  Construction
        public:
            TopTask (CallAgent *pAgent);

        //  Destruction
        private:
            ~TopTask ();

        //  State
        private:
            CallAgent::Reference const m_pCallAgent;
        };

    } // namespace XFed
} // namespace Vbe

#endif
