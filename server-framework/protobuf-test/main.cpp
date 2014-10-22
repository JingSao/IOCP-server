
#if (defined _DEBUG) || (defined DEBUG)
#   define CRTDBG_MAP_ALLOC
#   include <crtdbg.h>
#endif

#include "google/protobuf/stubs/common.h"

int main()
{
#if (defined _DEBUG) || (defined DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetBreakAlloc(1217);
#endif
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    google::protobuf::ShutdownProtobufLibrary();
    // 12 12 12 12 12 12 12 16 16 8 28
    return 0;
}
