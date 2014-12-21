#include <stdio.h>
#include <iostream>
#include <utility>
#include <string>
#include <type_traits>
#include <map>
#include <unordered_map>
#include "integer_sequence.h"
#include "lua.hpp"

// means lua templates

namespace lt {

    // =============================================================================

    template <typename _T> struct _LuaPusher
    {
        static inline int _invoke(lua_State *L, const _T &t)
        {
            static_assert(0, "unsupported data type");
            return 0;
        };
    };

#define _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_)               \
    {                                                               \
        static inline int _invoke(lua_State *L, _type_ t)           \
        {                                                           \
            _func_(L, t);                                           \
            return 1;                                               \
        }                                                           \
    }

#define _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(_type_, _func_) \
    template <> struct _LuaPusher<_type_>                           \
    _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_);                  \
    template <> struct _LuaPusher<const _type_>                     \
    _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_);                  \
    template <> struct _LuaPusher<const _type_ &>                   \
    _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_);                  \
    template <> struct _LuaPusher<_type_ &&>                        \
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

    template <> struct _LuaPusher<char *>
    {
        static inline int _invoke(lua_State *L, char *t) { lua_pushstring(L, t); return 1; }
    };
    template <> struct _LuaPusher<const char *>
    {
        static inline int _invoke(lua_State *L, const char *t) { lua_pushstring(L, t); return 1; }
    };

    template <> struct _LuaPusher<std::string>
    {
        static inline int _invoke(lua_State *L, const std::string &t) { lua_pushstring(L, t.c_str()); return 1; }
    };
    template <> struct _LuaPusher<const std::string>
    {
        static inline int _invoke(lua_State *L, const std::string &t) { lua_pushstring(L, t.c_str()); return 1; }
    };
    template <> struct _LuaPusher<const std::string &>
    {
        static inline int _invoke(lua_State *L, const std::string &t) { lua_pushstring(L, t.c_str()); return 1; }
    };
    template <> struct _LuaPusher<std::string &&>
    {
        static inline int _invoke(lua_State *L, std::string &&t) { lua_pushstring(L, t.c_str()); return 1; }
    };
    // =============================================================================

    template <typename _T> struct _LuaReader
    {
        typedef _T _ReturnType;
        static inline _ReturnType _invoke(lua_State *L, int idx)
        {
            static_assert(0, "unsupported data type");
            return _T();
        }
    };

#define _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_)               \
    {                                                               \
        typedef _type_ _ReturnType;                                 \
        static inline _ReturnType _invoke(lua_State *L, int idx)    \
        {                                                           \
            return (_ReturnType)_func_(L, idx);                     \
        }                                                           \
    }

#define _EXPLICIT_SPACIALIZATION_FOR_BASE_DATA_TYPE(_type_, _func_) \
    template <> struct _LuaReader<_type_>                           \
    _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_);                  \
    template <> struct _LuaReader <const _type_>                    \
    _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_);                  \
    template <> struct _LuaReader <const _type_ &>                  \
    _EXPLICIT_SPACIALIZATION_BODY(_type_, _func_);                  \
    template <> struct _LuaReader <_type_ &&>                       \
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

    template <> struct _LuaReader<const char *>
    {
        typedef const char *_ReturnType;
        static inline _ReturnType _invoke(lua_State *L, int idx)
        {
            if (lua_isstring(L, idx)) return lua_tolstring(L, idx, nullptr);
            else return nullptr;
        }
    };

    template <> struct _LuaReader<std::string>
    {
        typedef std::string _ReturnType;
        static inline _ReturnType _invoke(lua_State *L, int idx)
        {
            const char *str = nullptr;
            if (lua_isstring(L, idx))  str = lua_tolstring(L, idx, nullptr);
            return str == nullptr ? std::string() : std::string(str);
        }
    };

    template <> struct _LuaReader<const std::string &>
    {
        typedef std::string _ReturnType;
        static inline _ReturnType _invoke(lua_State *L, int idx)
        {
            const char *str = nullptr;
            if (lua_isstring(L, idx))  str = lua_tolstring(L, idx, nullptr);
            return str == nullptr ? std::string() : std::string(str);
        }
    };

    template <typename _T, typename _ALLOC, template<typename, typename> class _VEC>
    struct _LuaReader<_VEC<_T, _ALLOC> >
    {
        typedef _VEC<_T, _ALLOC> _ReturnType;
        static _ReturnType _invoke(lua_State *L, int idx)
        {
            _ReturnType ret;
            if (lua_istable(L, idx))
            {
                lua_pushnil(L);
                int pos = (idx >= 0) ? idx : idx - 1;
                while (lua_next(L, pos) != 0)
                {
                    ret.push_back(_LuaReader<_T>::_invoke(L, -1));
                    lua_pop(L, 1);
                }
            }
            return ret;
        }
    };

    template <typename _KEY, typename _VAL, typename _CMP, typename _ALLOC>
    struct _LuaReader<std::map<_KEY, _VAL, _CMP, _ALLOC> >
    {
        typedef std::map<_KEY, _VAL, _CMP, _ALLOC> _ReturnType;
        static _ReturnType _invoke(lua_State *L, int idx)
        {
            _ReturnType ret;
            if (lua_istable(L, idx))
            {
                lua_pushnil(L);
                int pos = (idx >= 0) ? idx : idx - 1;
                while (lua_next(L, pos) != 0)
                {
                    _KEY key = _LuaReader<_KEY>::_invoke(L, -2);
                    _VAL val = _LuaReader<_VAL>::_invoke(L, -1);
                    ret.insert(typename _ReturnType::value_type(key, val));
                    lua_pop(L, 1);
                }
            }
            return ret;
        }
    };

    template <typename _KEY, typename _VAL, typename _HASHER, typename _CMP, typename _ALLOC>
    struct _LuaReader<std::unordered_map<_KEY, _VAL, _HASHER, _CMP, _ALLOC> >
    {
        typedef std::unordered_map<_KEY, _VAL, _HASHER, _CMP, _ALLOC> _ReturnType;
        static _ReturnType _invoke(lua_State *L, int idx)
        {
            _ReturnType ret;
            if (lua_istable(L, idx))
            {
                lua_pushnil(L);
                int pos = (idx >= 0) ? idx : idx - 1;
                while (lua_next(L, pos) != 0)
                {
                    _KEY key = _LuaReader<_KEY>::_invoke(L, -2);
                    _VAL val = _LuaReader<_VAL>::_invoke(L, -1);
                    ret.insert(typename _ReturnType::value_type(key, val));
                    lua_pop(L, 1);
                }
            }
            return ret;
        }
    };

    // =============================================================================

    template <typename _T> struct _LuaPoper
    {
        static inline _T _invoke(lua_State *L)
        {
            _T ret = _LuaReader<_T>::_invoke();
            lua_pop(L, 1);
            return ret;
        }
    };

    template <> struct _LuaPoper<void>
    {
        static inline void _invoke(lua_State *L) { lua_pop(L, 1); }
    };

    // =============================================================================

    template <typename... _T> struct _ParametersParser
    {
        static inline int _invoke(lua_State *)
        {
            static_assert(sizeof...(_T) == 0, "bad parameter parser");
            return 0;
        }
    };

    template <typename _T0> struct _ParametersParser<_T0>
    {
        static inline int _invoke(lua_State *L, _T0 &&arg0)
        {
            _LuaPusher<_T0>::_invoke(L, arg0);
            return 1;
        }
    };

    template <typename _T0, typename ..._Args> struct _ParametersParser<_T0, _Args...>
    {
        static int _invoke(lua_State *L, _T0 &&arg0, _Args &&...args)
        {
            _LuaPusher<_T0>::_invoke(L, arg0);
            return 1 + _ParametersParser<_Args...>::_invoke(std::forward<_Args>(args)...);
        }
    };

    // =============================================================================

    template <typename _Ret, typename ..._Args> struct _CFunctionCaster
    {
        typedef _Ret(*FuncType)(_Args...);
        template <int... _IDX>
        static void _invoke(lua_State *L, FuncType fn, const std::integer_sequence<int, _IDX...> &)
        {
            lua_pushlightuserdata(L, fn);
            lua_pushunsigned(L, 0);
            lua_pushcclosure(L, [](lua_State *L)->int {
                FuncType fn = (FuncType)lua_touserdata(L, lua_upvalueindex(1));
                lua_Unsigned argc = lua_tounsigned(L, lua_upvalueindex(2));
                _Ret ret = (*fn)(_LuaReader<_Args>::_invoke(L, _IDX + 1)...);
                return _LuaPusher<_Ret>::_invoke(L, ret);
            }, 2);
        }
    };

    template <typename ..._Args> struct _CFunctionCaster<void, _Args...>
    {
        typedef void(*FuncType)(_Args...);
        template <int... _IDX>
        static void _invoke(lua_State *L, FuncType fn, const std::integer_sequence<int, _IDX...> &)
        {
            lua_pushlightuserdata(L, fn);
            lua_pushunsigned(L, 0);
            lua_pushcclosure(L, [](lua_State *L)->int {
                FuncType fn = (FuncType)lua_touserdata(L, lua_upvalueindex(1));
                lua_Unsigned argc = lua_tounsigned(L, lua_upvalueindex(2));
                (*fn)(_LuaReader<_Args>::_invoke(L, _IDX + 1)...);
                return 0;
            }, 2);
        }
    };

    template <typename _Fn> struct _CFunctionRegister { };

    template <typename _Ret, typename ..._Args> struct _CFunctionRegister<_Ret(_Args...)>
    {
        static inline void _invoke(lua_State *L, _Ret(*fn)(_Args...))
        {
            _CFunctionCaster<_Ret, _Args...>::_invoke(L, fn, std::make_integer_sequence<int, sizeof...(_Args)>());
        }
    };

    // =============================================================================

    template <typename _Ret, typename ..._Args>
    inline void registerCFunction(lua_State *L, const char *name, _Ret(*fn)(_Args...))
    {
        _CFunctionRegister<_Ret(_Args...)>::_invoke(L, fn);
        lua_setglobal(L, name);
    }

    template <typename _Fn, typename ..._BindArgs>
    void registerFunctor(lua_State *L, const char *name, _Fn &&fn, _BindArgs &&...args)
    {

    }

    // =============================================================================

    template <typename _Ret, typename ..._Args>
    _Ret callFunction(lua_State *L, const char *name, _Args &&...args)
    {
        lua_getglobal(L, name);
        if (lua_isfunction(L, -1))
        {
            int argc = _ParametersParser<_Args...>::_invoke(L, std::forward<_Args>(args)...);
            lua_pcall(L, argc, 1, 0);
            return _LuaPoper<_Ret>::_invoke(L);
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
