#include <lua5.4/lua.h>
#include <lua5.4/lauxlib.h>
#include <lua5.4/lualib.h>
#include <string.h>

// Function to get a reference to a specific named table
static int getTableReference(lua_State *L, const char *tableName) {

    // If tableName is an empty string, return a reference to the global table
    if (tableName[0] == '\0') {
        lua_pushglobaltable(L);
        int globalTableReference = luaL_ref(L, LUA_REGISTRYINDEX);
        return globalTableReference;
    }

    // Push the global environment table onto the stack
    lua_getglobal(L, "_G");

    // Traverse the nested tables based on the table name
    char* table_name = strdup(tableName);
    const char *token = strtok(table_name, ".");
    printf("token: %s\n", token);

    while (token != NULL) {
        lua_pushstring(L, token);
        lua_gettable(L, -2); // Get the field from the table on top of the stack

        if (!lua_istable(L, -1)) {
            luaL_error(L, "Table '%s' not found or is not a valid table", tableName);
            return 0; // Return 0 values to indicate failure
        }

        lua_remove(L, -2); // Remove the previous table from the stack
        token = strtok(NULL, ".");
        printf("token: %s\n", token);
    }

    // Now, you have the reference to the specified nested table on top of the stack
    // You can use lua_ref to create a reference to the table and return the reference

    int tableReference = luaL_ref(L, LUA_REGISTRYINDEX);
    printf("Reference for '%s': %d\n\n", tableName, tableReference);

    return tableReference;
}

static void printTable(lua_State *L, int tableReference) {

  printf("Printing table ref %d\n", tableReference);
  lua_rawgeti(L, LUA_REGISTRYINDEX, tableReference);
  lua_pushnil(L);  // Push nil to start the traversal

    while (lua_next(L, -2) != 0) {
        // Key is at index -2 and value is at index -1
        if (lua_type(L, -2) == LUA_TSTRING) {
            const char *key = lua_tostring(L, -2);
            const char *value = lua_tostring(L, -1);

            printf("Key: %s, Value: %s\n", key, value);
        } else {
            // Handle other types if needed
        }

        // Remove the value, leaving the key for the next iteration
        lua_pop(L, 1);
    }
    printf("\n");
}

int main() {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    // Execute your Lua script here
    if(luaL_dofile(L, "./scripts/configtable.lua") != LUA_OK) {
      luaL_error(L, "Error: %d: %s\n", __LINE__, lua_tostring(L, -1));
      return -1;
    }
    
    // Example: Get a reference to a nested table
    int ref0 = getTableReference(L, ""); // global table reference
    int ref1 = getTableReference(L, "config_table.nested_table");
    int ref2 = getTableReference(L, "config_table");
    int ref3 = getTableReference(L, "personnel_records");

    if (ref1 != 0) {
        printTable(L, ref1);
        luaL_unref(L, LUA_REGISTRYINDEX, ref1); // Release the reference when you are done
    }
    if (ref2 != 0) {
        printTable(L, ref2);
        luaL_unref(L, LUA_REGISTRYINDEX, ref2); 
    }
    if (ref3 != 0) {
        printTable(L, ref3);
        luaL_unref(L, LUA_REGISTRYINDEX, ref3); 
    }

    if(ref0 != 0) {
       printTable(L, ref0);
       luaL_unref(L, LUA_REGISTRYINDEX, ref0); 
    }

    lua_close(L);

    return 0;
}

/*

Here's a way to set string keys for values in the registry. try this as well.

LUA_REGISTRYINDEX is a predefined index in the Lua C API that refers to the registry table. The 
registry table is a regular Lua table that can be used to store C data that needs to be accessed 
by multiple Lua states. You can use any Lua value except nil as a key to store data in the registry 
table. However, because all C libraries share the same registry, you must choose with care what 
values you use as keys, to avoid collisions 1.

Here is an example of using a string key with LUA_REGISTRYINDEX:

```
lua_pushstring(L, "my_key");
lua_pushinteger(L, 42);
lua_settable(L, LUA_REGISTRYINDEX);
```

In this example, we are storing an integer value of 42 in the registry table with a string key 
of "my_key". To retrieve this value later, we can use the following code:

```
lua_pushstring(L, "my_key");
lua_gettable(L, LUA_REGISTRYINDEX);
int my_value = lua_tointeger(L, -1);
```

This will retrieve the value stored with the key "my_key" from the registry table and convert 
it to an integer.

I hope this helps!
*/