#include <stdio.h>
#include "lua.hpp"

int main()
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    luaL_loadfilex(L, "test.lua", nullptr);
    lua_resume(L, nullptr, 0);

    lua_getglobal(L, "func");
    int err = lua_resume(L, nullptr, 0);
    if (err != 0)
    {
        puts(lua_tolstring(L, -1, nullptr));
    }

    lua_close(L);

    return 0;
}