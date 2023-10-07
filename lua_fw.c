#include "fw_define.h"
#include "fw_upg.h"
#include "lfk/fw0/fw0_define.h"
#include "lfk/fw0/fw0_upgrade.h"
#include <stdlib.h>
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
#include "libserialport/libserialport.h"

#define FW0_PTR "FW0_PTR"

//###################### fw0 ###################################//
LUA_API fwu ** check_userdata(lua_State* L,const char * name)
{
	void* ud = luaL_checkudata(L, 1, name);
	luaL_argcheck(L, ud != NULL, 1," expected");
	return ud;
}

//#################### common func #######################################//
//1. fwu ptr 2.str
LUA_API int fw_print_info(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) == 2, 1, "arg error");
	fwu* fwu_ = lua_touserdata(L, 1);
	const char * str=lua_tostring(L, 2);
	q_print(NULL, NULL, fwu_, Q_MSG_INFO, str);
}
LUA_API int fw_print_debug(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) == 2, 1, "arg error");
	fwu* fwu_ = lua_touserdata(L, 1);
	const char* str = lua_tostring(L, 2);
	q_print(NULL, NULL, fwu_, Q_MSG_DEBUG, str);
}
LUA_API int fw_print_warning(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) == 2, 1, "arg error");
	fwu* fwu_ = lua_touserdata(L, 1);
	const char* str = lua_tostring(L, 2);
	q_print(NULL, NULL, fwu_, Q_MSG_WARNING, str);
}
LUA_API int fw_print_error(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) == 2, 1, "arg error");
	fwu* fwu_ = lua_touserdata(L, 1);
	const char* str = lua_tostring(L, 2);
	q_print(NULL, NULL, fwu_, Q_MSG_ERROR, str);
}

///step 1. fw0_init_fw 2. fw_set_serialport 3.op 4.fw0_free_fw
LUA_API int fw0_init_fw(lua_State* L)
{
	fwu_0* fwu_0_=malloc(sizeof(fwu_0));
	init_fw0(fwu_0_);
	init_fw(&(fwu_0_->fwu_));
	fwu_0** fwu0_user = lua_newuserdata(L, sizeof(fwu_0));
	*fwu0_user = fwu_0_;
	luaL_getmetatable(L, FW0_PTR);
	lua_setmetatable(L, -2);
	return 1;//return userdata  ** fwu0_user
}
LUA_API int fw0_free_fw(lua_State* L)
{
	fwu_0** fwu0temp = (fwu_0**)check_userdata(L, FW0_PTR);
	free(*fwu0temp);
	return 0;
}
//1. fwu 2.serialport 
LUA_API int fw_register_serialport(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) == 2, 1, "arg error");
	fwu* fwu_ = lua_touserdata(L, 1);
	struct sp_port* sp_port_ = (struct sp_port*)lua_touserdata(L, 2);
	fwu_->uart_t_.io_object_.userdata = sp_port_;
	return 0;
}
//######################## metatable FW0_PTR  funcs ######################//

// 1.hankshake timeout 2.handshake read_timeout
LUA_API int fw0_handshake(lua_State* L)
{
	fwu_0** fwu0temp = (fwu_0**)check_userdata(L, FW0_PTR);
	fwu_0* fwu0 = *fwu0temp;
	fwu0->fwu_.stop = 0;
	luaL_argcheck(L, lua_gettop(L) == 3,1, "param 1.hankshake timeout 2.handshake read_timeout");
	int timeout = lua_tointeger(L, 2);
	int rtimeout = lua_tointeger(L, 3);

	int rt = fwu0->xHandshake(fwu0, timeout, rtimeout);
	lua_pushinteger(L, rt);
	return 1;
}
//1.wdt_register_add
LUA_API int fw0_disable_wdt(lua_State* L)
{
	fwu_0** fwu0temp = (fwu_0**)check_userdata(L, FW0_PTR);
	fwu_0* fwu0 = *fwu0temp;
	luaL_argcheck(L, lua_gettop(L) == 2, 1, "param 1.wdt_register");
	int wdt_register_add = lua_tointeger(L, 2);
	int rt = fwu0->xDisable_wdt(fwu0, wdt_register_add);
	lua_pushinteger(L, rt);
	return 1;
}
/// 1. da_buffer
LUA_API int fw0_send_da(lua_State* L)
{
	fwu_0** fwu0temp = (fwu_0**)check_userdata(L, FW0_PTR);
	fwu_0* fwu0 = *fwu0temp;
	luaL_argcheck(L, lua_gettop(L) == 2, 1, "param da_buffer");
	int size = 0;
	char* da_buffer = lua_tolstring(L, 2, &size);
	int rt = fwu0->xSend_da(fwu0, da_buffer, size);
	lua_pushinteger(L, rt);
	return 1;
}
//no param
LUA_API int fw0_jump_da(lua_State* L)
{
	fwu_0** fwu0temp = (fwu_0**)check_userdata(L, FW0_PTR);
	fwu_0* fwu0 = *fwu0temp;
	luaL_argcheck(L, lua_gettop(L) == 1, 1, "param less than 1");
	int rt = fwu0->xJump_da(fwu0);
	lua_pushinteger(L, rt);
	return 1;
}
//1. baudrate
LUA_API int fw0_sync_da(lua_State* L)
{
	fwu_0** fwu0temp = (fwu_0**)check_userdata(L, FW0_PTR);
	fwu_0* fwu0 = *fwu0temp;
	luaL_argcheck(L, lua_gettop(L) == 2, 1, "param 1.down baudrate");
	int baudrate = lua_tointeger(L, 2);
	int rt = fwu0->xSync_da(fwu0, baudrate);
	lua_pushinteger(L, rt);
	return 1;
}
//1. begin_address 2.format_length
LUA_API int fw0_format_flash(lua_State* L)
{
	fwu_0** fwu0temp = (fwu_0**)check_userdata(L, FW0_PTR);
	fwu_0* fwu0 = *fwu0temp;
	luaL_argcheck(L, lua_gettop(L) == 3, 1, "param 1. begin_address 2.format_length ");
	int begin_address = lua_tointeger(L, 2);
	int format_length = lua_tointeger(L, 3);
	int rt = fwu0->xFormat_flash(fwu0, begin_address, format_length);
	lua_pushinteger(L, rt);
	return 1;
}
//1. fw_buffer 2.begin_address 3.is_bootloader 4.name
LUA_API int fw0_send_fw(lua_State* L)
{
	fwu_0** fwu0temp = (fwu_0**)check_userdata(L, FW0_PTR);
	fwu_0* fwu0 = *fwu0temp;
	luaL_argcheck(L, lua_gettop(L) == 5, 1, "param 1. fw_buffer 2.begin_address 3.is_bootloader 4.name ");
	int size = 0;
	char* fw_buffer = lua_tolstring(L, 2, &size);
	int begin_address = lua_tointeger(L, 3);
	int is_bootloader = lua_toboolean(L, 4);
	const char* name = lua_tostring(L, 5);
	fwu_0_rom rom={ fw_buffer ,size ,name,begin_address ,is_bootloader };
	int rt = fwu0->xSend_fw(fwu0, &rom);
	lua_pushinteger(L, rt);
	return 1;
}

LUA_API int fw0_set_stop(lua_State* L)
{
	fwu_0** fwu0temp = (fwu_0**)check_userdata(L, FW0_PTR);
	fwu_0* fwu0 = *fwu0temp;
	fwu0->fwu_.stop = 1;
	return 0;
}

LUA_API int fw0_get_ptr(lua_State* L)
{
	fwu_0** fwu0temp = (fwu_0**)check_userdata(L, FW0_PTR);
	fwu_0* fwu0 = *fwu0temp;
	if (fwu0 != NULL)
	{
		lua_pushlightuserdata(L, &fwu0->fwu_);
		return 1;
	}
	return 0;
}

static luaL_Reg common_fc[] = 
{
	{"new_fw0",fw0_init_fw},
	{"register",fw_register_serialport},
//print
	{"fw_info",fw_print_info},
	{"fw_debug",fw_print_debug},
	{"fw_warning",fw_print_warning},
	{"fw_error",fw_print_error},
	{NULL,NULL}
};
static luaL_Reg fw0_fc[]  =
{
	{"handshake",fw0_handshake},
	{"disable_wdt",fw0_disable_wdt},
	{"send_da",fw0_send_da},
	{"jump_da",fw0_jump_da},
	{"sync_da",fw0_sync_da},
	{"format_flash",fw0_format_flash},
	{"send_fw",fw0_send_fw},
	{"get",fw0_get_ptr},
	{"set_stop",fw0_set_stop},
	{"__gc",fw0_free_fw},
	{NULL,NULL}
};
///  firmware_upgarade lib
EXPORT int luaopen_firmware_upgrade(lua_State *L)
{
	//FW0 metatable
	luaL_newmetatable(L, FW0_PTR);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, fw0_fc, 0);
	luaL_newlib(L, common_fc);
	return 1;
}


