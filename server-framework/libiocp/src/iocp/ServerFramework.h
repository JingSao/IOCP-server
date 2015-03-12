#ifndef _SERVER_FRAMEWORK_H_
#define _SERVER_FRAMEWORK_H_

#include "ServerFrameworkImpl.h"

namespace iocp {
    template <class _T = void> class ClientContext final : public _impl::_ClientContext
    {
    public:
        ClientContext() : _val(_T()) { }
        ~ClientContext() { }

        _T _val;

        const char *getIp() const { return _ip; }
        uint16_t getPort() const { return _port; }
    };

    template <> class ClientContext<void> : public _impl::_ClientContext
    {
    public:
        const char *getIp() const { return _ip; }
        uint16_t getPort() const { return _port; }
    };

    template <class _T = void> class ServerFramework final : public _impl::_ServerFramework
    {
    public:
        ServerFramework() { }
        ~ServerFramework() { }

        using _impl::_ServerFramework::initialize;
        using _impl::_ServerFramework::uninitialize;

        typedef std::function<size_t (ClientContext<_T> *ctx, const char *buf, size_t len)> ClientRecvCallback;
        typedef std::function<void (ClientContext<_T> *ctx)> ClientDisconnectCallback;

        bool startup(const char *ip, uint16_t port,
            const ClientRecvCallback &clientRecvCallback,
            const ClientDisconnectCallback &clientDisconnectCallback)
        {
            return _impl::_ServerFramework::startup(ip, port, [clientRecvCallback](_impl::_ClientContext *ctx, const char *buf, size_t len)->size_t {
                return clientRecvCallback((ClientContext<_T> *)ctx, buf, len);
            }, [clientDisconnectCallback](_impl::_ClientContext *ctx) {
                return clientDisconnectCallback((ClientContext<_T> *)ctx);
            });
        }

        using _impl::_ServerFramework::shutdown;

    private:
        ServerFramework(const ServerFramework &) = delete;
        ServerFramework(ServerFramework &&) = delete;
        ServerFramework &operator=(const ServerFramework &) = delete;
        ServerFramework &operator=(ServerFramework &&) = delete;
    };
}

#endif
