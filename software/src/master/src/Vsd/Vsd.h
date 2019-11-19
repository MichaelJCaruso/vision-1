#ifndef Vsd_Interface
#define Vsd_Interface
 
/*************************
 *****  DLL Linkage  *****
 *************************/

#ifdef VSD_EXPORTS
#define Vsd_API DECLSPEC_DLLEXPORT

#else
#define Vsd_API DECLSPEC_DLLIMPORT

#ifdef _WIN32
#pragma comment(lib, "vsd.lib")
#endif

#endif

#define Vsd_NAMESPACE_ENTER namespace Vsd {
#define Vsd_NAMESPACE_EXIT }

#define DECLARE_VsdINTERFACE(ifName) DECLARE_API_VINTERFACE(Vsd,ifName)
#define VsdINTERFACE(ifName,ifBase) VINTERFACE_IN_API(ifName,ifBase,Vsd)


/**************************
 *****  Declarations  *****
 **************************/

/***********************
 *****  Namespace  *****
 ***********************/

namespace Vsd {
    typedef V::VString VString;
}


#endif
