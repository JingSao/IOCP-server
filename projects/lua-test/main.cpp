#include <stdio.h>
#include "lua.hpp"
#include "lua-templates.h"



namespace test {
    template <typename...> struct _Print
    {
        static void invoke() { }
    };

    template <typename T> struct _Print<T>
    {
        static void invoke(const T &t) { std::cout << t << std::endl; }
    };

    template <typename T, typename ...ARGS> struct _Print<T, ARGS...>
    {
        static void invoke(const T &t, const ARGS &...args)
        {
            std::cout << t << std::endl;
            _Print<ARGS...>::invoke(args...);
        }
    };



    template <typename...> struct _PrintType
    {
        static void invoke() { }
    };

    template <typename T> struct _PrintType<T>
    {
        static void invoke() { std::cout << typeid(T).name() << std::endl; }
    };

    template <typename T, typename ...ARGS> struct _PrintType<T, ARGS...>
    {
        static void invoke()
        {
            std::cout << typeid(T).name() << std::endl;
            _PrintType<ARGS...>::invoke();
        }
    };
}

void testExecuteLua()
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    luaL_dofile(L, "test_execute.lua");

    lua_close(L);
}

int luaCallFunc(lua_State *L)
{
    const char *str = luaL_checklstring(L, 1, nullptr);
    puts(str);

    lua_pushnumber(L, (lua_Number)5);
    return 1;
}

struct ABC
{
    int func(int a) { return printf("%d", a); }
};

int test2(int a, unsigned b, float c) {return 0; }

lua_State *testCallCFunc()
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    //lua_pushlightuserdata(L, (void*)100);
    //lua_pushcclosure(L, [](lua_State *L)->int {
    //    void *p = lua_touserdata(L, lua_upvalueindex(1));
    //    const char *str = lua_tostring(L, 1); //luaL_checklstring(L, 1, nullptr);
    //    puts(str);

    //    lua_pushnumber(L, (lua_Number)5);
    //    return 1;
    //}, 1);
    ////lua_pushcfunction(L, luaCallFunc);
    //lua_setglobal(L, "luaCallFunc");

    //lt::registerFunction(L, "luaCallFunc", static_cast<int(*)(const char*)>([](const char *str){
    //    puts(str);
    //    return 5;
    //}));
    //luaL_dofile(L, "test_callc.lua");

    //int a[100];
    //qsort(a, 100, sizeof(int), [](const void *a, const void *b) { return *(int *)a - *(int *)b; });
    //std::exception

    lt::registerCFunction(L, "test1",
        static_cast<const char *(*)(int, unsigned, float, const std::string&)>(
        [](int a, unsigned b, float c, const std::string &d) {
        printf("%d %u %f %s\n", a, b, c, d.c_str());
        return (const char *)"123";
    }));

    lt::registerMemberFunction(L, "ABC_func", &ABC::func);

    return L;
}

int _func(int, float)
{
    return 0;
}

void testCallLuaFunc()
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    //luaL_loadfilex(L, "test_calllua.lua", nullptr);
    luaL_loadstring(L, "function func(a, b, c, d) print(\"Hello World!\"..a..b..c..d) end");
    lua_resume(L, nullptr, 0);

    lt::callFunction(L, "func", 1, 2, 3, 4, 5, 6, 7);
    //lua_getglobal(L, "func");
    //int err = lua_resume(L, nullptr, 0);
    //if (err != 0)
    //{
    //    puts(lua_tolstring(L, -1, nullptr));
    //}
    //lua_pcall(L, 0, 0, 0);

    lua_close(L);
}

#include <utility>
#include <functional>
#include <iostream>
#include <string>

void func(float a, unsigned b)
{
    std::cout << "a = " << a << " b = " << b << std::endl;
    //return 0;
}

int main()
{
    //testExecuteLua();
    testCallLuaFunc();
    lua_State *L = testCallCFunc();

    luaL_dostring(L,
        "local n = test1(0xFFFFFFFF, 0xFFFFFFFF, 3.5, \"Hello World!\")"
        "print(n)"
        "n = ABC_func(nil, 9)"
        "print(test1(1))"
        "print(n)"
        );

    lua_close(L);

    std::placeholders::_9;

    return 0;
}
