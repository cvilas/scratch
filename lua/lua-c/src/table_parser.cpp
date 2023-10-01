#include <iostream>
#include <lua5.4/lua.hpp>

void parseTable(lua_State* L, const std::string& prefix = "");

void parseValue(lua_State* L, const std::string& key, const std::string& prefix = "") {
    switch (lua_type(L, -1)) {
        case LUA_TSTRING:
            std::cout << prefix + key << " = " << lua_tostring(L, -1) << std::endl;
            break;
        case LUA_TNUMBER:
            std::cout << prefix + key << " = " << lua_tonumber(L, -1) << std::endl;
            break;
        case LUA_TTABLE:
            parseTable(L, prefix + key + ".");
            break;
        default:
            // Handle other types as needed
            break;
    }
}

void parseTable(lua_State* L, const std::string& prefix) {
    lua_pushnil(L);  // Start table iteration
    while (lua_next(L, -2) != 0) {
        // Uses 'key' (at index -2) and 'value' (at index -1)
        if (lua_type(L, -2) == LUA_TSTRING) {
            std::string key = lua_tostring(L, -2);
            parseValue(L, key, prefix);
        }

        // Removes 'value'; keeps 'key' for the next iteration
        lua_pop(L, 1);
    }
}

int main() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    if (luaL_dofile(L, "./scripts/configtable.lua") == 0) {
        lua_getglobal(L, "config_table");  

        if (lua_istable(L, -1)) {
            parseTable(L, "");
        } else {
            std::cerr << "Error: 'config' is not a table." << std::endl;
        }
    } else {
        std::cerr << "Error loading table" << std::endl;
    }

    lua_close(L);
    return 0;
}

