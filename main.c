#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"

int main() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L); // Ҫװ��ϵͳ�⣬����math��table�ȡ�
    luaL_dofile(L, "test.lua");
    lua_pcall(L, 0, LUA_MULTRET, 0);

    lua_close(L);
}