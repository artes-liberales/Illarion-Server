/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LUA_SCRIPT_HPP_
#define _LUA_SCRIPT_HPP_

extern "C" {
#include <lua.h>
}

#include <stdexcept>
#include "globals.hpp"
#include "Item.hpp"
#include <luabind/object.hpp>
#include "fuse_ptr.hpp"
#include <map>

class Character;
class World;

class ScriptException : public std::runtime_error {
public:
    ScriptException(std::string s) throw() : std::runtime_error(s) {}
};

enum SouTarTypes {
    LUA_NONE = 0, /**< not a correct type (only for initialisation) */
    LUA_FIELD = 1,  /**< target was a field */
    LUA_ITEM = 2, /**< target was a item */
    LUA_CHARACTER = 3 /**< target was character*/
};

enum LtaStates {
    LTS_NOLTACTION = 0, /**< no longtime action in this script */
    LTS_ACTIONABORTED = 1, /**< long time action was aborted */
    LTS_ACTIONSUCCESSFULL = 2  /**< long time action was performed sucessfulle*/
};

struct SouTar {
    Character *character;  /**< Source or target is a character, this is the pointer to it, otherwise NULL */
    ScriptItem item; /**< Source or target is a Item, this holds the information of the item */
    SouTarTypes Type; /**< Source or Target Type (if its an character, field or item) */
    position pos; /**< aboslute position of the object */
    /**
    * constructor which intializes the values
    */
    SouTar() {
        Type = LUA_NONE;
        item.id = 0;
        item.wear = 0;
        item.number = 0;
        character = NULL;
    }
};

class LuaScript {
public:
    LuaScript();
    LuaScript(std::string filename) throw(ScriptException);

    virtual ~LuaScript() throw();

    std::string getFileName() {
        return _filename;
    }

    lua_State *getLuaState() {
        return _luaState;
    }

    static void shutdownLua();
    bool existsEntrypoint(const std::string &entrypoint);
    void addQuestScript(const std::string &entrypoint, boost::shared_ptr<LuaScript> script);

protected:
    static lua_State *_luaState;
    static bool initialized;

    template<typename... Args>
    void callEntrypoint(const std::string &entrypoint, const Args &... args) {
        setCurrentWorldScript();

        if (!callQuestEntrypoint(entrypoint, args...)) {
            safeCall(entrypoint, args...);
        }
    };
    template<typename T, typename... Args>
    T callEntrypoint(const std::string &entrypoint, const Args &... args) {
        setCurrentWorldScript();
        callQuestEntrypoint(entrypoint, args...);
        return safeCall<T>(entrypoint, args...);
    };

private:
    void initialize();
    void init_base_functions();
    static int add_backtrace(lua_State *L);
    void writeErrorMsg();
    void writeCastErrorMsg(const std::string &entryPoint, const luabind::cast_failed &e);
    static void writeDebugMsg(const std::string &msg);
    void setCurrentWorldScript();
    luabind::object buildEntrypoint(const std::string &entrypoint) throw(luabind::error);

    template<typename... Args>
    bool callQuestEntrypoint(const std::string &entrypoint, const Args &... args) {
        auto entrypointRange = questScripts.equal_range(entrypoint);
        bool foundQuest = false;

        for (auto it = entrypointRange.first; it != entrypointRange.second; ++it) {
            foundQuest = foundQuest || it->second->safeCall<bool>(entrypoint, args...);
        }

        return foundQuest;
    }

    template<typename... Args>
    void safeCall(const std::string &entrypoint, const Args &... args) {
        try {
            auto luaEntrypoint = buildEntrypoint(entrypoint);
            luaEntrypoint(args...);
        } catch (luabind::error &e) {
            writeErrorMsg();
        }
    };
    template<typename T, typename... Args>
    T safeCall(const std::string &entrypoint, const Args &... args) {
        try {
            auto luaEntrypoint = buildEntrypoint(entrypoint);
            auto result = luaEntrypoint(args...);
            return luabind::object_cast<T>(result);
        } catch (luabind::cast_failed &e) {
            writeCastErrorMsg(entrypoint, e);
        } catch (luabind::error &e) {
            writeErrorMsg();
        }

        return T();
    };

    LuaScript(const LuaScript &);
    LuaScript &operator=(const LuaScript &);

    std::string _filename;
    std::vector<std::string> vecPath;
    typedef std::multimap<const std::string, boost::shared_ptr<LuaScript> > QuestScripts;
    QuestScripts questScripts;
};

#endif

