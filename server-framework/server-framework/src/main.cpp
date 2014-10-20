#define _CRT_SECURE_NO_WARNINGS

#if (defined _DEBUG) || (defined DEBUG)
#   define CRTDBG_MAP_ALLOC
#   include <crtdbg.h>
#endif

#include "iocp/IOCPServerModule.h"

#include "google/protobuf/stubs/common.h"

int main()
{
#if (defined _DEBUG) || (defined DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(198);
#endif
    /*
    iocp::ServerModule server;
    server.start(nullptr, 8899);
    scanf("%*d");
    server.end();
*/
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
