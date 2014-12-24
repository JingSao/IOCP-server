#ifndef _LUA_TEMPLATES_H_
#define _LUA_TEMPLATES_H_

#include <stdio.h>
#include <utility>
#include <string>
#include <type_traits>
#include <map>
#include <unordered_map>
#include "lua.hpp"

#if (defined _DEBUG) || (defined DEBUG)
#define REPORT_ERROR(_fmt_, ...) printf(_fmt_, ##__VA_ARGS__)
#else
#define REPORT_ERROR(...) do { } while (0)
#endif
// means lua templates

namespace lt {

    // =============================================================================

    template <typename _T> struct LuaPusher
    {
        static inline int push(lua_State *L, const _T &t)
        {
            static_assert(0, "unsupported data type");
            return 0;
        };
    };

#define _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_)               \
    {                                                               \
        static inline int push(lua_State *L, _type_ t)              \
        {                                                           \
            _func_(L, t);                                           \
            return 1;                                               \
        }                                                           \
    }

#define _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(_type_, _func_) \
    template <> struct LuaPusher<_type_>                            \
    _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_);                  \
    template <> struct LuaPusher<const _type_>                      \
    _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_);                  \
    template <> struct LuaPusher<const _type_ &>                    \
    _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_);                  \
    template <> struct LuaPusher<_type_ &&>                         \
    _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_);                  \

    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(signed char, lua_pushinteger);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(unsigned char, lua_pushunsigned);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(signed short, lua_pushinteger);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(unsigned short, lua_pushunsigned);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(signed int, lua_pushinteger);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(unsigned int, lua_pushunsigned);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(signed long, lua_pushinteger);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(unsigned long, lua_pushunsigned);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(float, lua_pushnumber);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(double, lua_pushnumber);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(bool, lua_pushboolean);

#undef _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE
#undef _EXPLICIT_SPACIALIZATION_BODY

    template <> struct LuaPusher<char *>
    {
        static inline int push(lua_State *L, char *t) { lua_pushstring(L, t); return 1; }
    };
    template <> struct LuaPusher<const char *>
    {
        static inline int push(lua_State *L, const char *t) { lua_pushstring(L, t); return 1; }
    };

    template <> struct LuaPusher<std::string>
    {
        static inline int push(lua_State *L, const std::string &t) { lua_pushstring(L, t.c_str()); return 1; }
    };
    template <> struct LuaPusher<const std::string>
    {
        static inline int push(lua_State *L, const std::string &t) { lua_pushstring(L, t.c_str()); return 1; }
    };
    template <> struct LuaPusher<const std::string &>
    {
        static inline int push(lua_State *L, const std::string &t) { lua_pushstring(L, t.c_str()); return 1; }
    };
    template <> struct LuaPusher<std::string &&>
    {
        static inline int push(lua_State *L, std::string &&t) { lua_pushstring(L, t.c_str()); return 1; }
    };

    // =============================================================================

    template <typename _T> struct LuaReader
    {
        typedef _T _ReturnType;
        static inline _ReturnType read(lua_State *L, int Idx)
        {
            static_assert(0, "unsupported data type");
            return _T();
        }
    };

#define _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_)               \
    {                                                               \
        typedef _type_ _ReturnType;                                 \
        static inline _ReturnType read(lua_State *L, int Idx)       \
        {                                                           \
            return (_ReturnType)_func_(L, Idx);                     \
        }                                                           \
    }

#define _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(_type_, _func_) \
    template <> struct LuaReader<_type_>                            \
    _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_);                  \
    template <> struct LuaReader <const _type_>                     \
    _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_);                  \
    template <> struct LuaReader <const _type_ &>                   \
    _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_);                  \
    template <> struct LuaReader <_type_ &&>                        \
    _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_)

    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(signed char, lua_tointeger);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(unsigned char, lua_tounsigned);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(signed short, lua_tointeger);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(unsigned short, lua_tounsigned);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(signed int, lua_tointeger);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(unsigned int, lua_tounsigned);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(signed long, lua_tointeger);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(unsigned long, lua_tounsigned);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(float, lua_tonumber);
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(double, lua_tonumber);
#pragma warning(push)
#pragma warning(disable: 4800)
    _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(bool, lua_toboolean);
#pragma warning(pop)

#undef _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE
#undef _EXPLICIT_SPACIALIZATION_BODY

    template <> struct LuaReader<const char *>
    {
        typedef const char *_ReturnType;
        static inline _ReturnType read(lua_State *L, int Idx)
        {
            if (lua_isstring(L, Idx)) return lua_tolstring(L, Idx, nullptr);
            else return nullptr;
        }
    };

    template <> struct LuaReader<std::string>
    {
        typedef std::string _ReturnType;
        static inline _ReturnType read(lua_State *L, int Idx)
        {
            const char *str = nullptr;
            if (lua_isstring(L, Idx))  str = lua_tolstring(L, Idx, nullptr);
            return str == nullptr ? std::string() : std::string(str);
        }
    };

    template <> struct LuaReader<const std::string &>
    {
        typedef std::string _ReturnType;
        static inline _ReturnType read(lua_State *L, int Idx)
        {
            const char *str = nullptr;
            if (lua_isstring(L, Idx))  str = lua_tolstring(L, Idx, nullptr);
            return str == nullptr ? std::string() : std::string(str);
        }
    };

    template <typename _T, typename _Alloc, template<typename, typename> class _Vec>
    struct LuaReader<_Vec<_T, _Alloc> >
    {
        typedef _Vec<_T, _Alloc> _ReturnType;
        static _ReturnType read(lua_State *L, int Idx)
        {
            _ReturnType ret;
            if (lua_istable(L, Idx))
            {
                lua_pushnil(L);
                int pos = (Idx >= 0) ? Idx : Idx - 1;
                while (lua_next(L, pos) != 0)
                {
                    ret.push_back(LuaReader<_T>::read(L, -1));
                    lua_pop(L, 1);
                }
            }
            return ret;
        }
    };

    template <typename _Key, typename _Val, typename _Cmp, typename _Alloc>
    struct LuaReader<std::map<_Key, _Val, _Cmp, _Alloc> >
    {
        typedef std::map<_Key, _Val, _Cmp, _Alloc> _ReturnType;
        static _ReturnType read(lua_State *L, int Idx)
        {
            _ReturnType ret;
            if (lua_istable(L, Idx))
            {
                lua_pushnil(L);
                int pos = (Idx >= 0) ? Idx : Idx - 1;
                while (lua_next(L, pos) != 0)
                {
                    _Key key = LuaReader<_Key>::read(L, -2);
                    _Val val = LuaReader<_Val>::read(L, -1);
                    ret.insert(typename _ReturnType::value_type(key, val));
                    lua_pop(L, 1);
                }
            }
            return ret;
        }
    };

    template <typename _Key, typename _Val, typename _Hasher, typename _Cmp, typename _Alloc>
    struct LuaReader<std::unordered_map<_Key, _Val, _Hasher, _Cmp, _Alloc> >
    {
        typedef std::unordered_map<_Key, _Val, _Hasher, _Cmp, _Alloc> _ReturnType;
        static _ReturnType read(lua_State *L, int Idx)
        {
            _ReturnType ret;
            if (lua_istable(L, Idx))
            {
                lua_pushnil(L);
                int pos = (Idx >= 0) ? Idx : Idx - 1;
                while (lua_next(L, pos) != 0)
                {
                    _Key key = LuaReader<_Key>::read(L, -2);
                    _Val val = LuaReader<_Val>::read(L, -1);
                    ret.insert(typename _ReturnType::value_type(key, val));
                    lua_pop(L, 1);
                }
            }
            return ret;
        }
    };

    // =============================================================================

    template <typename _T> struct LuaPoper
    {
        static inline _T pop(lua_State *L)
        {
            _T ret = LuaReader<_T>::read();
            lua_pop(L, 1);
            return ret;
        }
    };

    template <> struct LuaPoper<void>
    {
        static inline void pop(lua_State *L) { lua_pop(L, 1); }
    };

    // =============================================================================
    template <size_t ...> struct _ArgIdx { };
    template <typename _ArgIdxType, typename ...> struct _MakeArgIdxWrap
    {
        typedef _ArgIdxType type;
    };

    template <size_t ..._Idx, typename _T0, typename ..._TN>
    struct _MakeArgIdxWrap<_ArgIdx<_Idx...>, _T0, _TN...>
        : _MakeArgIdxWrap<_ArgIdx<sizeof...(_TN), _Idx...>, _TN...>
    {
    };

    template <typename ..._TN> struct _MakeArgIdx : _MakeArgIdxWrap<_ArgIdx<>, _TN...>
    {
    };

    // =============================================================================
    template <typename _Ret, typename ..._Args> struct _DoCallCFunc
    {
        static inline int _call(lua_State *L, _Ret(*fn)(_Args...), _Args &&...args)
        {
            _Ret ret = (*fn)(std::forward<_Args>(args)...);
            return LuaPusher<_Ret>::push(L, ret);
        }
    };

    template <typename ..._Args> struct _DoCallCFunc<void, _Args...>
    {
        static inline int _call(lua_State *L, void(*fn)(_Args...), _Args &&...args)
        {
            (*fn)(std::forward<_Args>(args)...);
            return 0;
        }
    };

    template <typename _Ret, typename ..._Args> struct _CFuncRegister
    {
        typedef _Ret(*FuncType)(_Args...);
        template <size_t ..._Idx>
        static void _register(lua_State *L, FuncType fn, _ArgIdx<_Idx...> &&)
        {
            void *data = lua_newuserdata(L, sizeof(FuncType *));
            *(FuncType *)data = fn;
            lua_pushunsigned(L, 0);
            lua_pushcclosure(L, [](lua_State *L)->int {
                int n = lua_gettop(L);
                if (n != sizeof...(_Idx))
                {
                    REPORT_ERROR("wrong number of arguments: %d, was expecting %d\n", n, sizeof...(_Idx));
                }
                FuncType *fn = (FuncType *)lua_touserdata(L, lua_upvalueindex(1));
                lua_Unsigned argc = lua_tounsigned(L, lua_upvalueindex(2));
                return _DoCallCFunc<_Ret, _Args...>::_call(
                    L, *fn, LuaReader<_Args>::read(L, _Idx + 1)...);
            }, 2);
        }
    };

    // =============================================================================
    template <typename _Ret, typename _Obj, typename ..._Args> struct _DoCallMemFunc
    {
        static inline int _call(lua_State *L, _Obj *obj, _Ret(_Obj::*fn)(_Args...), _Args &&...args)
        {
            _Ret ret = (obj->*fn)(std::forward<_Args>(args)...);
            return LuaPusher<_Ret>::push(L, ret);
        }
    };

    template <typename _Obj, typename ..._Args> struct _DoCallMemFunc<void, _Obj, _Args...>
    {
        static inline int _call(lua_State *L, _Obj *obj, void(_Obj::*fn)(_Args...), _Args &&...args)
        {
            (obj->*fn)(std::forward<_Args>(args)...);
            return 0;
        }
    };

    template <typename _Ret, typename _Obj, typename ..._Args> struct _MemFuncRegister
    {
        typedef _Ret(_Obj::*FuncType)(_Args...);
        template <int... _Idx>
        static void _register(lua_State *L, FuncType fn, _ArgIdx<_Idx...> &&)
        {
            void *data = lua_newuserdata(L, sizeof(FuncType *));
            *(FuncType *)data = fn;
            lua_pushunsigned(L, 0);
            lua_pushcclosure(L, [](lua_State *L)->int {
                int n = lua_gettop(L);
                if (n != sizeof...(_Idx))
                {
                    REPORT_ERROR("wrong number of arguments: %d, was expecting %d\n", n, sizeof...(_Idx));
                }
                FuncType *fn = (FuncType *)lua_touserdata(L, lua_upvalueindex(1));
                lua_Unsigned argc = lua_tounsigned(L, lua_upvalueindex(2));
                _Obj *obj = (_Obj *)lua_topointer(L, 1);
                return _DoCallMemFunc<_Ret, _Obj, _Args...>::_call(
                    L, obj, *fn, LuaReader<_Args>::read(L, _Idx + 2)...);
            }, 2);
        }
    };

    // =============================================================================

    template <typename _Ret, typename ..._Args>
    inline void registerCFunction(lua_State *L, const char *name, _Ret(*fn)(_Args...))
    {
        _CFuncRegister<_Ret, _Args...>::_register(L, fn, typename _MakeArgIdx<_Args...>::type());
        lua_setglobal(L, name);
    }

    template <typename _Ret, typename _Obj, typename ..._Args>
    inline void registerMemberFunction(lua_State *L, const char *name, _Ret(_Obj::*fn)(_Args...))
    {
        _MemFuncRegister<_Ret, _Obj, _Args...>::_register(L, fn, typename _MakeArgIdx<_Args...>::type());
        lua_setglobal(L, name);
    }

    // =============================================================================

    template <typename ..._T> struct _ParametersPusher
    {
        static inline int push(lua_State *, _T &&...)
        {
            static_assert(sizeof...(_T) == 0, "bad parameter parser");
            return 0;
        }
    };

    template <typename _T0> struct _ParametersPusher<_T0>
    {
        static inline int push(lua_State *L, _T0 &&arg0)
        {
            LuaPusher<_T0>::push(L, std::forward<_T0>(arg0));
            return 1;
        }
    };

    template <typename _T0, typename ..._Args> struct _ParametersPusher<_T0, _Args...>
    {
        static int push(lua_State *L, _T0 &&arg0, _Args &&...args)
        {
            LuaPusher<_T0>::push(L, std::forward<_T0>(arg0));
            return 1 + _ParametersPusher<_Args...>::push(L, std::forward<_Args>(args)...);
        }
    };

    // =============================================================================

    template <typename _Ret, typename ..._Args>
    _Ret callFunction(lua_State *L, const char *name, _Args &&...args)
    {
        lua_getglobal(L, name);
        if (lua_isfunction(L, -1))
        {
            int argc = _ParametersPusher<_Args...>::push(L, std::forward<_Args>(args)...);
            lua_pcall(L, argc, 1, 0);
            return LuaPoper<_Ret>::pop(L);
        }
        return _Ret();
    }

    // =============================================================================

    template <typename ..._Args>
    void callFunction(lua_State *L, const char *name, _Args &&...args)
    {
        callFunction<void>(L, name, std::forward<_Args>(args)...);
    }
};

#endif
