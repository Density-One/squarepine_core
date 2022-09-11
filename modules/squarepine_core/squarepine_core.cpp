#if ! JUCE_MAC && ! JUCE_IOS
    #undef JUCE_CORE_INCLUDE_NATIVE_HEADERS
    #define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1
#endif

#include "squarepine_core.h"

#if JUCE_WINDOWS
    #include <netlistmgr.h>
    #include <Wlanapi.h>
    #pragma comment (lib, "iphlpapi.lib")
    #pragma comment (lib, "wlanapi.lib")
#endif

//==============================================================================
// Just in case some idiotic library or system header is up to no good...
#undef GET
#undef HEAD
#undef POST
#undef PUT
#undef DELETE
#undef CONNECT
#undef OPTIONS
#undef TRACE

//==============================================================================
namespace sp
{
    #include "cryptography/CRC.cpp"
    //#include "cryptography/SHA1.cpp"
    #include "debugging/CrashStackTracer.cpp"
    #include "misc/ArrayIterationUnroller.cpp"
    #include "misc/CodeBeautifiers.cpp"
    #include "misc/CommandHelpers.cpp"
    #include "misc/FPUFlags.cpp"
    #include "networking/GoogleAnalyticsReporter.cpp"
    #include "networking/NetworkCache.cpp"

    //==============================================================================
    double getCurrentSystemRSSI();
    NetworkConnectivityChecker::NetworkType getCurrentSystemNetworkType();

    #include "networking/NetworkConnectivityCheckerPosix.cpp"
    #include "networking/NetworkConnectivityCheckerWindows.cpp"
    #include "networking/NetworkConnectivityChecker.cpp"

    //==============================================================================
    #include "networking/OAuth2.cpp"
    #include "networking/Product.cpp"
    #include "networking/User.cpp"
    #include "networking/WebServiceUtilities.cpp"
    #include "networking/WooCommerce.cpp"
    #include "rng/ISAAC.cpp"
    #include "rng/Xorshift.cpp"
    #include "text/LanguageCodes.cpp"
    #include "text/CountryCodes.cpp"
    #include "text/LanguageHandler.cpp"
    #include "valuetree/JSONToValueTree.cpp"

    #include "unittests/AllocatorUnitTests.cpp"
    #include "unittests/AngleUnitTests.cpp"
    #include "unittests/MathsUnitTests.cpp"
    #include "unittests/RNGUnitTests.cpp"
    #include "unittests/SquarePineCoreUnitTestGatherer.cpp"
}
