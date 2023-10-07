package.cpath="./?.dll"

local firmware_upgrade =require "firmware_upgrade"

local serialport =require "G_luaSerialPort"
local mfw0=firmware_upgrade.new_fw0()

local msp=serialport.new()

msp:open("COM1",9600)
firmware_upgrade.register(mfw0:get(),msp:get())

local ret=mfw0:handshake(10000,2);
print(ret)

