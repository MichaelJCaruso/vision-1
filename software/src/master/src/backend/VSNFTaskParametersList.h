#ifndef VSNFTaskParametersList_Interface
#define VSNFTaskParametersList_Interface

/************************
 *****  Components  *****
 ************************/

#include "VReference.h"

/**************************
 *****  Declarations  *****
 **************************/

#include "VSNFTaskParameters.h"

/*************************
 *****  Definitions  *****
 *************************/

class VSNFTaskParametersList : public VSNFTaskParameters::Reference {
//  Construction
public:
    VSNFTaskParametersList (VSNFTaskParametersList const& rOther);
    VSNFTaskParametersList ();

//  Destruction
public:
    ~VSNFTaskParametersList ();

//  Access/Update
public:
    inline void pop (VSNFTaskParameters::Reference& rpHeadReturn);
};



#endif
