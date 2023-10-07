#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"

int main() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L); // 要装载系统库，比如math、table等。
    luaL_dofile(L, "test.lua");
    lua_pcall(L, 0, LUA_MULTRET, 0);

    lua_close(L);
}