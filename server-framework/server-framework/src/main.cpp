
#if (defined _DEBUG) || (defined DEBUG)
#   define CRTDBG_MAP_ALLOC
#   include <crtdbg.h>
#endif

#include "iocp/IOCPServerController.h"
#include "iocp/IOCPClientContext.h"

#include "google/protobuf/stubs/common.h"


#include <stdint.h>

#define MAKE_BODY_SIZE(a0, a1, a2, a3) ((((uint32_t)(uint8_t)(a0)) << 24) | ((uint32_t)(uint8_t)(a1) << 16) | ((uint32_t)(uint8_t)(a2) << 8) | ((uint32_t)(uint8_t)(a3)))
#define BODY_SIZE_GET0(s) (uint8_t)(((s) >> 24) & 0xFF)
#define BODY_SIZE_GET1(s) (uint8_t)(((s) >> 16) & 0xFF)
#define BODY_SIZE_GET2(s) (uint8_t)(((s) >> 8) & 0xFF)
#define BODY_SIZE_GET3(s) (uint8_t)((s) & 0xFF)
#define BODY_SIZE_GET(s, n) (uint8_t)(((s) >> (((uint32_t)(3 - (n))) << 3)) & 0xFF)

int main()
{
#if (defined _DEBUG) || (defined DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetBreakAlloc(1217);
#endif
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    iocp::ServerController::startup();

    iocp::ServerController server([](iocp::ClientContext *context, const char *buf, size_t len)->size_t {
        if (len < 4)
        {
            return 0;
        }

        size_t bodySize = MAKE_BODY_SIZE(buf[0], buf[1], buf[2], buf[3]);
        size_t packetLen = bodySize + 4;
        if (packetLen > len)
        {
            return 0;
        }

        //LOG_DEBUG("%16s:%5hu send %lu bytes\n", context->getIP(), context->getPort(), len);
        context->postSend(buf, len);

        return len;
    }, [](iocp::ClientContext *context) {
        LOG_DEBUG("%16s:%5hu disconnected\n", context->getIP(), context->getPort());
    });

    server.start(nullptr, 8899);
    scanf("%*d");
    server.end();

    iocp::ServerController::cleanup();

    google::protobuf::ShutdownProtobufLibrary();
    // 12 12 12 12 12 12 12 16 16 8 28
    return 0;
}
