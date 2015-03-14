#ifndef _SERVER_FRAMEWORK_H_
#define _SERVER_FRAMEWORK_H_

#include "ServerFrameworkImpl.h"

namespace iocp {
    template <class _T = void> class ClientContext final : public _impl::_ClientContext
    {
    public:
        ClientContext() : _userData(_T()) { }
        ~ClientContext() { }

        _T getUserData() { return _userData; }
        const _T &getUserData() const { return _userData; }
        void setUserData(const _T &userData) { _userData = userData; }

    protected:
        _T _userData;
    };

    template <> class ClientContext<void> : public _impl::_ClientContext
    {
    };

    template <class _T = void> class ServerFramework final : public _impl::_ServerFramework
    {
    public:
        ServerFramework()
        {
            _allocateCtx = []() { return new (std::nothrow) ClientContext<_T>; };
            _deallocateCtx = [](_impl::_ClientContext *ctx) { delete (ClientContext<_T> *)ctx; };
        }

        ~ServerFramework() { }

        using _impl::_ServerFramework::initialize;
        using _impl::_ServerFramework::uninitialize;

        typedef std::function<size_t (ClientContext<_T> *ctx, const char *buf, size_t len)> RecvCallback;
        typedef std::function<void (ClientContext<_T> *ctx)> DisconnectCallback;

        bool startup(const char *ip, uint16_t port,
            const RecvCallback &onRecv,
            const DisconnectCallback &onDisconnect)
        {
            _onRecv = [onRecv](_impl::_ClientContext *ctx, const char *buf, size_t len)->size_t {
                return onRecv((ClientContext<_T> *)ctx, buf, len);
            };
            _onDisconnect = [onDisconnect](_impl::_ClientContext *ctx) {
                onDisconnect((ClientContext<_T> *)ctx);
            };
            return _impl::_ServerFramework::startup(ip, port);
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
