//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _CLIENT_COMMANDS_HPP_
#define _CLIENT_COMMANDS_HPP_

#include "Player.hpp"
#include "World.hpp"
#include "Character.hpp"
#include <string>
#include "types.hpp"
#include "Logger.hpp"
#include "data/WeaponObjectTable.hpp"
#include "data/CommonObjectTable.hpp"
#include "data/MonsterTable.hpp"
#include "data/TilesTable.hpp"
#include "dialog/InputDialog.hpp"
#include <boost/shared_ptr.hpp>
#include "script/LuaNPCScript.hpp"
#include "script/LuaScript.hpp"
#include "script/LuaItemScript.hpp"
#include "script/LuaMagicScript.hpp"
#include "script/LuaMonsterScript.hpp"
#include "script/LuaTileScript.hpp"
#include "script/LuaLookAtPlayerScript.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/BasicClientCommand.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include <list>

extern WeaponObjectTable *WeaponItems;
extern MonsterTable *MonsterDescriptions;
extern CommonObjectTable *CommonItem;
extern TilesTable *Tiles;
extern boost::shared_ptr<LuaLookAtPlayerScript>lookAtPlayerScript;

enum clientcommands {
    C_LOGIN_TS = 0x0D, /*<login*/
    C_SCREENSIZE_TS = 0xA0,
    C_CHARMOVE_TS = 0x10,
    C_IMOVERSTART_TS = 0x20,
    C_IMOVEREND_TS = 0x2A,
    C_PSPINRSTART_TS = 0x40,
    C_PSPINREND_TS = 0x49,
    C_LOOKATMAPITEM_TS = 0xFF,
    C_LOOKATCHARACTER_TS = 0x18,
    C_USE_TS = 0xFE,
    C_CAST_TS = 0xFD,
    C_ATTACKPLAYER_TS = 0xFA,
    C_INTRODUCE_TS = 0xF6,
    C_SAY_TS = 0xF5,
    C_SHOUT_TS = 0xF4,
    C_WHISPER_TS = 0xF3,
    C_REFRESH_TS = 0xF2,
    C_LOGOUT_TS = 0xF1,
    C_LOOKINTOCONTAINERONFIELD_TS = 0xEC,
    C_LOOKINTOINVENTORY_TS = 0xEB,
    C_LOOKINTOSHOWCASECONTAINER_TS = 0xEA,
    C_CLOSECONTAINERINSHOWCASE_TS = 0xE9,
    C_DROPITEMFROMSHOWCASEONMAP_TS = 0xE8,
    C_MOVEITEMBETWEENSHOWCASES_TS = 0xE7,
    C_MOVEITEMFROMMAPINTOSHOWCASE_TS = 0xE6,
    C_MOVEITEMFROMMAPTOPLAYER_TS = 0xE5,
    C_DROPITEMFROMPLAYERONMAP_TS = 0xE4,
    C_MOVEITEMINSIDEINVENTORY_TS = 0xE3,
    C_MOVEITEMFROMSHOWCASETOPLAYER_TS = 0xE2,
    C_MOVEITEMFROMPLAYERTOSHOWCASE_TS = 0xE1,
    C_LOOKATSHOWCASEITEM_TS = 0xE0,
    C_LOOKATINVENTORYITEM_TS = 0xDF,
    C_ATTACKSTOP_TS = 0xDE,
    C_REQUESTSKILLS_TS = 0xDD,
    C_LOOKATMENUITEM_TS = 0xDC,
    C_KEEPALIVE_TS = 0xD8,
    C_REQUESTAPPEARANCE_TS = 0x0E,
    C_INPUTDIALOG_TS = 0x50
};

/**
*simple class for storing data in a fifo implemented stack for the use commands
*/
class ByteStack {
public:
    ByteStack() : ok(true) {};
    ~ByteStack() {
        stack.clear();
        paramstack.clear();
    };

    void push(unsigned char v) {
        stack.push_back(v);
    }

    unsigned char pop() {
        if (!stack.empty()) {
            unsigned char ret = stack.front();
            stack.pop_front();
            return ret;
        }

        ok = false;
        return 0;
    }

    void push(short int v) {
        paramstack.push_back(v);
    }

    short int popparam() {
        if (!paramstack.empty()) {
            short int ret = paramstack.front();
            paramstack.pop_front();
            return ret;
        }

        ok = false;
        return 0;
    }

    bool ok; //<will be false if more data wants to be red than inside the stack

private:
    std::list<unsigned char>stack;
    std::list<short int>paramstack;
};

class InputDialogTS : public BasicClientCommand {
private:
    unsigned int dialogId;
    bool success;
    std::string input;

public:
    InputDialogTS() : BasicClientCommand(C_INPUTDIALOG_TS) {
    }

    virtual ~InputDialogTS() {};

    virtual void decodeData() {
        dialogId = getIntFromBuffer();
        success = getUnsignedCharFromBuffer() > 0;
        input = getStringFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->executeInputDialog(dialogId, success, input);
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new InputDialogTS());
        return cmd;
    }
};

class RequestAppearanceTS : public BasicClientCommand {
public:
    RequestAppearanceTS() : BasicClientCommand(C_REQUESTAPPEARANCE_TS) {
    }

    virtual ~RequestAppearanceTS() {};

    virtual void decodeData() {
        id = getIntFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        Character *ch = World::get()->Players.findID(id);

        if (ch == NULL) {
            ch = World::get()->Monsters.findID(id);

            if (ch == NULL) {
                ch = World::get()->Npc.findID(id);
            }
        }

        //check if we found a character befor
        if (ch != NULL) {
            ch->updateAppearanceForPlayer(player, true);
        }

    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new RequestAppearanceTS());
        return cmd;
    }

    TYPE_OF_CHARACTER_ID id;
};

/**
*@ingroup Clientcommands
*player looks at a character
*/
class LookAtCharacterTS : public BasicClientCommand {
public:
    /**
    *a player attacked another char
    */
    LookAtCharacterTS() : BasicClientCommand(C_LOOKATCHARACTER_TS) {
        id = 0;
        mode = 0;
    }

    virtual ~LookAtCharacterTS() {

    }

    virtual void decodeData() {
        id = getIntFromBuffer();
        mode = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));

        if (id < MONSTER_BASE) {
            Player *pl = World::get()->Players.findID(id);

            if (pl) {
                if (lookAtPlayerScript) {
                    lookAtPlayerScript->lookAtPlayer(player, pl, mode);
                }
            }

            //code for player handling
        } else if (id >= MONSTER_BASE && id < NPC_BASE) {
            Monster *monster = World::get()->Monsters.findID(id);

            if (monster) {
                MonsterStruct mon;

                if (MonsterDescriptions->find(monster->getType(), mon)) {
                    if (mon.script && mon.script->existsEntrypoint("lookAtMonster")) {
                        mon.script->lookAtMonster(player, monster, mode);
                        return;
                    }

                    std::string outtext;

                    switch (player->getPlayerLanguage()) {
                    case Language::german:
                        outtext = "Das ist: " + mon.name ;
                        break;
                    case Language::english:
                        outtext =  "This is: " + mon.name ;
                        break;
                    default:
                        outtext +=  "unknown mother tongue" ;
                        break;
                    }

                    boost::shared_ptr<BasicServerCommand>cmd(new CharDescription(id, outtext));
                    //player->inform( outtext );
                    player->Connection->addCommand(cmd);
                }
            }
        } else {
            NPC *npc = World::get()->Npc.findID(id);

            if (npc) {
                if (npc->getScript() && npc->getScript()->existsEntrypoint("lookAtNpc")) {
                    npc->getScript()->lookAtNpc(player, mode);
                    return;
                }

                std::string outtext;

                switch (player->getPlayerLanguage()) {
                case Language::german:
                    outtext = "Das ist: " + npc->name ;
                    break;
                case Language::english:
                    outtext =  "This is: " + npc->name ;
                    break;
                default:
                    outtext +=  "unknown mother tongue" ;
                    break;
                }

                boost::shared_ptr<BasicServerCommand>cmd(new CharDescription(id, outtext));
                //player->inform( outtext );
                player->Connection->addCommand(cmd);

            }
        }


    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new LookAtCharacterTS());
        return cmd;
    }

    TYPE_OF_CHARACTER_ID id;
    uint8_t mode;
};

/**
*@ingroup Clientcommands
*character casts something
*/
class CastTS : public BasicClientCommand {
public:
    CastTS() : BasicClientCommand(C_CAST_TS) {
        cid = 0;
        xc = 0;
        yc = 0;
        zc = 0;
        pos = 0;
        paramtemp = 0;
        counter = 0;
        showcase = 0;

    }

    virtual ~CastTS() {
    };

    virtual void decodeData() {
        spellId = static_cast<unsigned long int>(getIntFromBuffer());
        cid = getUnsignedCharFromBuffer();

        switch (cid) {
        case UID_KOORD:
            xc = static_cast<short int>(getShortIntFromBuffer());
            yc = static_cast<short int>(getShortIntFromBuffer());
            zc = static_cast<short int>(getShortIntFromBuffer());
            break;

        case UID_SHOWC:
            showcase = getUnsignedCharFromBuffer();
            pos = getUnsignedCharFromBuffer();
            break;
        case UID_INV:
            pos = getUnsignedCharFromBuffer();
            break;
        case UID_VAR:
            paramtemp = getShortIntFromBuffer();
            break;
        case UID_MAGICWAND:
            break;
        }

        //counter = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug", player->name + " is casting!");
        Logger::writeMessage("Casting", player->name + " is casting!");

        bool paramOK = true;
        //CScript* skript = NULL;

        boost::shared_ptr<LuaMagicScript> LuaMageScript;

        // berprfen, ob der Spieler die Runen beherrscht
        if ((spellId & player->magic.flags[ player->magic.type ]) == spellId) {
            SpellStruct CastedSpell;

            if (Spells->find(spellId,player->magic.type, CastedSpell)) {
                LuaMageScript = CastedSpell.script;
            }
        }

        Logger::writeMessage("Casting", player->name + " can't cast the spell: " + Logger::toString(spellId) + " , flags: " + Logger::toString(player->magic.flags[ player->magic.type ]),false);

        //Source des Castens zuweisen
        SouTar Source, Target;
        Source.character = dynamic_cast<Character *>(player);
        Source.pos = player->pos;
        Source.Type = LUA_CHARACTER;

        switch (cid) {
        case UID_KOORD:

            Field *temp;
            Logger::writeMessage("Casting",player->name + " trys to cast on a coordinate pos(" + Logger::toString(xc) + "," + Logger::toString(yc) + "," + Logger::toString(zc)+")",false);

            if (LuaMageScript) {
                if (!World::get()->GetPToCFieldAt(temp, xc, yc, zc)) {
                    Logger::writeError("World_Debug", "cant find field for casting at pos(" + Logger::toString(xc) + "," + Logger::toString(yc) + "," + Logger::toString(zc) + ") !");
                    paramOK = false;
                } else {
                    // Feld gefunden
                    if (temp->IsPlayerOnField() || temp->IsMonsterOnField() || temp->IsNPCOnField()) {
                        Character *tmpCharacter = World::get()->findCharacterOnField(xc, yc, zc);

                        if (tmpCharacter != NULL) {
                            //Nothing to do here
                        } else {
                            Logger::writeMessage("Casting","Character found at target field!",false);
                        }

                        // Character auf Feld ist ein Spieler
                        if ((tmpCharacter->character == Character::player) && (LuaMageScript)) {
                            Logger::writeMessage("Casting","Target Character: player",false);
                            //Lua Script zuweisung
                            Target.character = tmpCharacter;
                            Target.pos = tmpCharacter->pos;
                            Target.Type = LUA_CHARACTER;
                        }
                        // Character auf Feld ist ein NPC
                        else if ((tmpCharacter->character == Character::npc) && (LuaMageScript)) {
                            Logger::writeMessage("Casting","Target Character: NPC",false);
                            //Lua Script zuweisung
                            Target.character = tmpCharacter;
                            Target.pos = tmpCharacter->pos;
                            Target.Type = LUA_CHARACTER;
                        } else if ((tmpCharacter->character == Character::monster) && (LuaMageScript)) {
                            Logger::writeMessage("Casting","Target Character: monster",false);
                            //Lua Script zuweisung
                            Target.character = tmpCharacter;
                            Target.pos = tmpCharacter->pos;
                            Target.Type = LUA_CHARACTER;
                        }
                    } else {
                        Item it;

                        if (temp->ViewTopItem(it)) {
                            Logger::writeMessage("Casting","Item found at target field!",false);

                            if (LuaMageScript) {
                                //Lua Script zuweisung
                                Target.pos = position(xc, yc, zc); //Position des TargetItems
                                Target.Type = LUA_ITEM;
                                Target.item = it;
                                Target.item.type = ScriptItem::it_field;
                                Target.item.pos = position(xc, yc, zc); //Position des TargetItems
                                Target.item.owner = player;
                            }
                        } else {
                            Logger::writeMessage("Casting","empty field!",false);

                            if (LuaMageScript) {
                                //Lua Script zuweisung
                                Target.pos = position(xc, yc, zc); //Position des TargetItems
                                Target.Type = LUA_FIELD;
                            }
                        } // Item oder leer ?
                    } // character ?
                } // Feld vorhanden ?
            } // skript != NULL ?
            else {
                std::cerr<<"LuaMageScript false, paramOK = false!"<<std::endl;
                paramOK = false;
            }

            break;

        case UID_SHOWC:

#ifdef World_DEBUG
            std::cout << "showcase: " << (int) showcase << " pos: " << (int) pos << std::endl;
#endif

            Logger::writeMessage("Casting", player->name + " is casting in showcas: "+Logger::toString(showcase)+" pos "+Logger::toString(pos));

            if (LuaMageScript) {
                if (showcase < MAXSHOWCASES) {
                    Container *ps = player->showcases[ showcase ].top();

                    if (ps != NULL) {
#ifdef World_DEBUG
                        std::cout << "Container gefunden" << std::endl;
#endif
                        ScriptItem tempi;
                        Container *tempc;

                        if (ps->viewItemNr(pos, tempi, tempc)) {
#ifdef World_DEBUG
                            std::cout << "pos gefunden" << std::endl;
#endif

                            if (LuaMageScript) {
                                Target.Type = LUA_ITEM;
                                ps->viewItemNr(pos, Target.item, tempc);
                                Target.item.pos = position(xc, yc, zc);

                                if (showcase == 0) {
                                    Target.item.type = ScriptItem::it_showcase1;
                                } else {
                                    Target.item.type = ScriptItem::it_showcase2;
                                }

                                Target.item.itempos = pos;
                                Target.item.owner = player;
                                Target.pos = position(xc, yc, zc);
                            }
                        } else {
                            std::cerr<<"ps->viewItemNr false, paramOK = false!"<<std::endl;
                            paramOK = false;
                        }
                    } else {
                        std::cerr<<"ps!=Null false, paramOK = false!"<<std::endl;
                        paramOK = false;
                    }
                } else {
                    std::cerr << "showcase < MAXSHOWCASE false, paramOK = false!"<<std::endl;
                    paramOK = false;
                }
            } // LuaMageScript == NULL ?

            break;
        case UID_MAGICWAND:

            //UID_MAGICWAND wird immer gesandt wenn kein Ziel gewaehlt wird.
#ifdef World_DEBUG
            std::cout << "UID_MAGICWAND" << std::endl;
#endif
            Logger::writeMessage("Casting","Cast with Wand",false);

            if ((player->attackmode) && (player->enemyid != 0) && (LuaMageScript)) {
                bool zauberstab=false;

                if ((player->characterItems[ LEFT_TOOL ].id != 0) && (player->characterItems[ LEFT_TOOL ].id != BLOCKEDITEM)) {
                    if (WeaponItems->find(player->characterItems[ LEFT_TOOL ].id, tempWeapon)) {
                        if (tempWeapon.WeaponType == 13) {
                            zauberstab = true;
                            std::cout << "Zauberstab in der Hand -> OK" << std::endl;
                        }
                    }
                }

                if ((player->characterItems[ RIGHT_TOOL ].id != 0) && (player->characterItems[ RIGHT_TOOL ].id != BLOCKEDITEM)) {
                    if (WeaponItems->find(player->characterItems[ RIGHT_TOOL ].id, tempWeapon)) {
                        if (tempWeapon.WeaponType == 13) {
                            zauberstab = true;
                            std::cout << "Zauberstab in der Hand -> OK" << std::endl;
                        }
                    }
                }


                if (zauberstab) {
                    switch (player->enemytype) {

                        //Muss spaeter angepasst werden wenn es nur noch einen Charactervektor gibt.

                    case Character::player:
#ifdef World_DEBUG
                        std::cout << "Gegner ist ein Spieler" << std::endl;
#endif

                        if (LuaMageScript) {
                            Target.Type = LUA_CHARACTER;
                            Target.character = World::get()->findCharacter(player->enemyid);

                            if (Target.character != NULL) {
                                Target.pos = Target.character->pos;
                            } else {
                                paramOK = false;
                                std::cerr << "Kein geeignetes Ziel fr Zauberstab gefunden (Target.Character == NULL)!" << std::endl;
                            }
                        }

                        break;

                    case Character::npc:
#ifdef World_DEBUG
                        std::cout << "Gegner ist ein NPC" << std::endl;
#endif

                        if (LuaMageScript) {
                            Target.Type = LUA_CHARACTER;
                            Target.character = World::get()->findCharacter(player->enemyid);

                            if (Target.character != NULL) {
                                Target.pos = Target.character->pos;
                            } else {
                                paramOK = false;
                                std::cerr << "Kein geeignetes Ziel fr Zauberstab gefunden (Target.Character == NULL)!" << std::endl;
                            }
                        }

                        break;

                    case Character::monster:
#ifdef World_DEBUG
                        std::cout << "Gegner ist ein Monster" << std::endl;
#endif

                        if (LuaMageScript) {
                            Target.Type = LUA_CHARACTER;
                            Target.character = World::get()->findCharacter(player->enemyid);

                            if (Target.character != NULL) {
                                Target.pos = Target.character->pos;
                            } else {
                                paramOK = false;
                                std::cerr << "Kein geeignetes Ziel fr Zauberstab gefunden (Target.Character == NULL)!" << std::endl;
                            }
                        }

                        break;
                    } // switch
                } // zauberstab
                else {
                    std::cout<<"Zauberstab = false, paramOK = false!"<<std::endl;
                    paramOK = false;
                }

            } // enemyid != 0
            else {

                if (!LuaMageScript) {
                    std::cout<<"LuaMageScript nicht gesetzt, paramOK = false!"<<std::endl;
                    paramOK = false;
                }
            }

            break;

        case UID_INV:

#ifdef World_DEBUG
            std::cout << "cast mit Inv" << std::endl;
#endif
            Logger::writeMessage("Casting","cast in inventory",false);

            if (LuaMageScript) {
                if (pos < (MAX_BELT_SLOTS + MAX_BODY_ITEMS)) {
#ifdef World_DEBUG
                    std::cout << "gltiger Wert" << std::endl;
#endif

                    if (player->characterItems[ pos ].id != 0) {
#ifdef World_DEBUG
                        std::cout << "Position " << (int) pos << " am Koerper ist besetzt" << std::endl;
#endif

                        if (LuaMageScript) {
                            Target.Type = LUA_ITEM;
                            Target.item = (ScriptItem)player->characterItems[ pos ];
                            Target.item.pos = player->pos;

                            if (pos < MAX_BODY_ITEMS) {
                                Target.item.type = ScriptItem::it_inventory;
                            } else {
                                Target.item.type = ScriptItem::it_belt;
                            }

                            Target.item.itempos = pos;
                            Target.item.owner = player;
                            Target.pos = player->pos;
                        }
                    } else {
                        std::cerr<<"cp->characterItems[pos].id != 0 false, paramOK = false!"<<std::endl;
                        paramOK = false;
                    }
                } else {
                    std::cerr<<"pos < (MAX_BELT_SLOTS + MAX_BODY_ITEMS), paramOK = false!"<<std::endl;
                    paramOK = false;
                }
            } // skript != NULL ?

            break;

        } // end of switch ID

        if (LuaMageScript) {
            Logger::writeMessage("Casting","try to call magic script",false);
            player->ltAction->setLastAction(LuaMageScript, Source, Target, counter , paramtemp, LongTimeAction::AT_MAGIC);
            std::string msg;
#ifdef World_DEBUG
            std::cout<<"Try to call LuaMageScript with paramtemp: "<<paramtemp<<std::endl;
            std::cout<<"paramOK: "<<paramOK<<std::endl;
#endif
#ifdef DO_UNCONSCIOUS

            if ((paramOK) && player->IsAlive() && player->IsConscious() && (player->GetStatus() < 10))
#else
            if ((paramOK) && player->IsAlive() && (player->GetStatus() < 10)) {
#endif

                switch (Target.Type) {
                case LUA_NONE:
                    LuaMageScript->CastMagic(player,counter,paramtemp,static_cast<unsigned char>(LTS_NOLTACTION));
                    msg = "Casted spell: " + Logger::toString(spellId);
                    break;
                case LUA_FIELD:
                    LuaMageScript->CastMagicOnField(player,Target.pos,counter,paramtemp,static_cast<unsigned char>(LTS_NOLTACTION));
                    msg = "Casted spell: " + Logger::toString(spellId) + " on field at pos(" + Logger::toString(Target.pos.x) + "," + Logger::toString(Target.pos.y) + "," + Logger::toString(Target.pos.z) + ")";
                    break;
                case LUA_CHARACTER:
                    LuaMageScript->CastMagicOnCharacter(player,Target.character,counter,paramtemp,static_cast<unsigned char>(LTS_NOLTACTION));

                    if (Target.character->character == Character::monster) {
                        MonsterStruct monStruct;
                        Monster *temp = dynamic_cast<Monster *>(Target.character);

                        if (MonsterDescriptions->find(temp->getType(), monStruct)) {
                            if (monStruct.script) {
                                monStruct.script->onCasted(temp,player);
                            }
                        } else {
                            std::cerr<<"Didn't finde Monster Description for: "<< temp->getType() << " can't call onCasted!"<<std::endl;
                        }

                    }

                    msg = "Casted spell: " + Logger::toString(spellId) + " on character: " + Target.character->name + "(" + Logger::toString(Target.character->id) + ")";
                    break;
                case LUA_ITEM:
                    LuaMageScript->CastMagicOnItem(player,Target.item,counter,paramtemp,static_cast<unsigned char>(LTS_NOLTACTION));
                    msg = "Casted spell: " + Logger::toString(spellId) + " on item: " + Logger::toString(Target.item.id);
                    break;
                default:
                    LuaMageScript->CastMagic(player,counter,paramtemp,static_cast<unsigned char>(LTS_NOLTACTION));
                    msg = "Casted spell: " + Logger::toString(spellId) + " on item: " + Logger::toString(Target.item.id);
                    break;
                } //Ende Switch

            //monitoringClientList->sendCommand( new SendActionTS( player->id, player->name, 2, msg));
        } //ENde if player->IsAlive

        Logger::writeMessage("Casting","all succeeded",false);
    }
}

boost::shared_ptr<BasicClientCommand> clone() {
    boost::shared_ptr<BasicClientCommand>cmd(new CastTS());
    return cmd;

}

unsigned char showcase;
unsigned char pos;
short int xc,yc,zc;
short int paramtemp;
unsigned char cid;
unsigned char counter;
unsigned long int spellId;
};


/**
*@ingroup Clientcommands
*character uses something
*/
class UseTS : public BasicClientCommand {
public:
    UseTS() : BasicClientCommand(C_USE_TS) {
    }

    virtual ~UseTS() {
    };

    virtual void decodeData() {
        unsigned char count = getUnsignedCharFromBuffer();
        bs.push(count);

        if (count > 3) {
            Logger::writeMessage("World_Debug ", "count:" + Logger::toString(count) + " to large, set to 3!");
            count = 3;
        }

        for (; count > 0 ; --count) {
            unsigned char uid = getUnsignedCharFromBuffer();
            bs.push(uid);

            switch (uid) {
            case UID_SKILL:
                break;

            case UID_KOORD:
                bs.push(getShortIntFromBuffer());
                bs.push(getShortIntFromBuffer());
                bs.push(getShortIntFromBuffer());
                break;

            case UID_SHOWC:
                bs.push(getUnsignedCharFromBuffer());
                bs.push(getUnsignedCharFromBuffer());
                break;

            case UID_INV:
                bs.push(getUnsignedCharFromBuffer());
                break;
            case UID_VAR:
                bs.push(getShortIntFromBuffer());
                break;
            }
        }

        counter = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        std::cout << "USE_PERFORM_START" << std::endl;
        time(&(player->lastaction));
        player->ltAction->abortAction();
        unsigned char count = bs.pop();

        if (count > 3) {
            Logger::writeMessage("World_Debug ", "count:" + Logger::toString(count) + " to large, set to 3!");
            count = 3;
        }

        Logger::writeMessage("Use", player->name + " uses something, count: " + Logger::toString(static_cast<int>(count)));
        bool first = true;
        bool paramOK = true;
        //CScript* skript = NULL;

        //Parameter fr Lua Item Scripte
        boost::shared_ptr<LuaItemScript> LuaScript;
        boost::shared_ptr<LuaNPCScript> LuaNPCScript;
        boost::shared_ptr<LuaMonsterScript> LuaMonsterScript;
        boost::shared_ptr<LuaTileScript> LuaTileScript;
        SouTar Source, Target;
        CommonStruct com;
        TilesStruct Tile;

        unsigned char showcase = 0;
        unsigned char pos = 0;
        short int xc = 0;
        short int yc = 0;
        short int zc = 0;
        short int paramtemp = 0;

        for (; count > 0; --count) {
            unsigned char uid = bs.pop();
            Logger::writeMessage("Use", "use ID :" + Logger::toString(static_cast<int>(uid)), false);

            switch (uid) {
            case UID_SKILL:
                break;

            case UID_KOORD:

                Field *temp;
                xc = static_cast<short int>(bs.popparam());
                yc = static_cast<short int>(bs.popparam());
                zc = static_cast<short int>(bs.popparam());

                Logger::writeMessage("Use", "UID_KOORD",false);
                Logger::writeMessage("Use","xc: " +Logger::toString(static_cast<int>(xc)) + " yc: " + Logger::toString(static_cast<int>(yc)) + " zc: " + Logger::toString(static_cast<int>(zc)), false);

                if (!World::get()->GetPToCFieldAt(temp, xc, yc, zc)) {
                    Logger::writeError("World_Debug","Use UID_KOORD field not found!");
                    Logger::writeMessage("Use","Use UID_KOORD field not found at pos ( " + Logger::toString(static_cast<int>(xc)) + "," + Logger::toString(static_cast<int>(yc)) + "," + Logger::toString(static_cast<int>(zc)) + ")");
                    paramOK = false;
                } else {
                    // Feld gefunden
                    //Prfen ob sich irgendeine art Char auf dem Feld befindet (Spaeter nur noch IsCharOnField vorerst noch alle Arten pruefen
                    if (temp->IsPlayerOnField() || temp->IsNPCOnField() || temp->IsMonsterOnField()) {
                        Logger::writeMessage("Use", "Character on field found!", false);
                        Character *tmpCharacter = World::get()->findCharacterOnField(xc, yc, zc);

                        if (tmpCharacter != NULL) {
                            //Zuweisen des Targets fr LuaScripte wenn ParamOK true ist, ein LuaScriptGeladen wurden und es nicht der erste Durchgang ist
                            if (!first && paramOK && (LuaScript || LuaMonsterScript || LuaNPCScript || LuaTileScript)) {
                                Target.pos = position(xc, yc, zc);
                                Target.character = tmpCharacter;
                                Target.Type = LUA_CHARACTER;
                            } //Wenn kein Luascript geladen ist oder es der erste durchgang ist.
                            else if (tmpCharacter->character == Character::player) {
                                Logger::writeMessage("Use","Character is a player!",false);

                                if (first) {
                                    //TODO Add Playerrace scripts
                                }
                            }// end tmpCharacter->type == Character::player
                            else if (tmpCharacter->character == Character::npc) {
                                Logger::writeMessage("Use","Character is a NPC!", false);

                                if (first) {
                                    NPC *scriptNPC = dynamic_cast<NPC *>(tmpCharacter);
                                    LuaNPCScript = scriptNPC->getScript();

                                    if (LuaNPCScript) {
                                        Source.pos = scriptNPC->pos;
                                        Source.character = scriptNPC;
                                        Source.Type = LUA_CHARACTER;
                                    }
                                }
                            }// end else if ( tmpCharacter->type == Character::NPC )
                            else if (tmpCharacter->character == Character::monster) {
                                Logger::writeMessage("Use","Character is a monster!",false);

                                if (first) {
                                    Monster *scriptMonster = dynamic_cast<Monster *>(tmpCharacter);
                                    MonsterStruct monStruct;

                                    if (MonsterDescriptions->find(scriptMonster->getType(),monStruct)) {
                                        LuaMonsterScript = monStruct.script;
                                    } else {
                                        Logger::writeError("World_Debug","try to use Monster but id: " + Logger::toString(scriptMonster->getType()) +" not found in database!");
                                        Logger::writeMessage("Use","try to use Monster but id: " + Logger::toString(scriptMonster->getType()) +" not found in database!", false);
                                    }

                                    if (LuaMonsterScript) {
                                        Source.pos = scriptMonster->pos;
                                        Source.character = scriptMonster;
                                        Source.Type = LUA_CHARACTER;
                                    }
                                }
                            }//end if ( tmpCharacter->type == Character::Monster )
                        } else {
                            Logger::writeError("World_Debug", "Character on field (" + Logger::toString(xc) + "," + Logger::toString(yc) + "," + Logger::toString(zc) + ") not found!");
                            Logger::writeMessage("Use", "Character on field (" + Logger::toString(xc) + "," + Logger::toString(yc) + "," + Logger::toString(zc) + ") not found!", false);
                        }
                    } //end  temp->IsPlayerOnField() || temp->IsNPCOnField() || temp->IsMonsterOnField()
                    else {
                        Logger::writeMessage("Use","no character on field!", false);
                        Item it;

                        if (temp->ViewTopItem(it)) {
                            Logger::writeMessage("Use","Item on field", false);

                            if (first) {
                                LuaScript = CommonItems->findScript(it.id);

                                if (LuaScript) {
                                    Source.Type = LUA_ITEM;
                                    temp->ViewTopItem(Source.item);
                                    Source.item.pos = position(xc, yc, zc); //Position des SourceItems
                                    Source.item.type = ScriptItem::it_field; //Position des SourceItems
                                    Source.item.owner = player; //Owner des Items
                                    Source.pos = position(xc, yc, zc);
                                    first = false;
                                }
                            } else if (LuaScript || LuaMonsterScript || LuaNPCScript || LuaTileScript) {
                                Target.Type = LUA_ITEM;
                                temp->ViewTopItem(Target.item);
                                Target.item.pos = position(xc, yc, zc);
                                Target.item.type = ScriptItem::it_field;
                                Target.item.owner = player;
                                Target.pos = position(xc, yc, zc);
                            }
                        } else {
                            Logger::writeMessage("Use","empty field!",false);

                            if (first) {

                                if (Tiles->find(temp->getTileId(),Tile)) {
                                    LuaTileScript = Tile.script;
                                }

                                if (LuaTileScript) {
                                    Source.Type = LUA_FIELD;
                                    Source.pos = position(xc, yc, zc);
                                    first = false;
                                }
                            } else if (LuaScript || LuaMonsterScript || LuaNPCScript || LuaTileScript) {
                                Target.Type = LUA_FIELD;
                                Target.pos = position(xc, yc, zc);
                            }
                        }
                    }
                }

                break;

            case UID_SHOWC:

                showcase = bs.pop();
                pos = bs.pop();
                Logger::writeMessage("Use", "showcase: " + Logger::toString(static_cast<int>(showcase)) + " pos: " + Logger::toString(static_cast<int>(pos)),false);

                if (showcase < MAXSHOWCASES) {
                    Container *ps = player->showcases[ showcase ].top();

                    if (ps != NULL) {
                        Logger::writeMessage("Use", "Container gefunden!", false);
                        ScriptItem tempi;
                        Container *tempc;

                        if (ps->viewItemNr(pos, tempi, tempc)) {
                            Logger::writeMessage("Use", "pos found item id: " + Logger::toString(tempi.id),false);

                            if (first) {
                                LuaScript = CommonItems->findScript(tempi.id);

                                if (LuaScript) {
                                    Source.Type = LUA_ITEM;
                                    ps->viewItemNr(pos, Source.item, tempc);
                                    Source.item.pos = position(xc, yc, zc);

                                    if (showcase == 0) {
                                        Source.item.type = ScriptItem::it_showcase1;
                                    } else {
                                        Source.item.type = ScriptItem::it_showcase2;
                                    }

                                    Source.item.itempos = pos;
                                    Source.item.owner = player;
                                    Source.pos = position(xc, yc, zc);
                                    first = false;
                                }
                            } else if (LuaScript || LuaMonsterScript || LuaNPCScript || LuaTileScript) {
                                Target.Type = LUA_ITEM;
                                ps->viewItemNr(pos, Target.item, tempc);
                                Target.pos = player->pos;
                                Target.item.pos = position(xc, yc, zc);

                                if (showcase == 0) {
                                    Target.item.type = ScriptItem::it_showcase1;
                                } else {
                                    Target.item.type = ScriptItem::it_showcase2;
                                }

                                Target.item.itempos = pos;
                                Target.item.owner = player;
                            }
                        } else {
                            paramOK = false;
                        }
                    } else {
                        paramOK = false;
                    }
                } else {
                    paramOK = false;
                }

                break;

            case UID_INV:

                pos = bs.pop();

                if (pos < (MAX_BELT_SLOTS + MAX_BODY_ITEMS)) {
                    Logger::writeMessage("Use", "position approved!",false);

                    if (player->characterItems[ pos ].id != 0) {
                        Logger::writeMessage("Use","at position " + Logger::toString(static_cast<int>(pos)) + " on body, is an item with id: " + Logger::toString(player->characterItems[ pos ].id),false);

                        if (first) {

                            LuaScript = CommonItems->findScript(player->characterItems[ pos ].id) ;

                            if (LuaScript) {
                                Source.Type = LUA_ITEM;
                                Source.item = (ScriptItem)player->characterItems[ pos ];
                                Source.item.pos = player->pos;

                                if (pos < MAX_BODY_ITEMS) {
                                    Source.item.type = ScriptItem::it_inventory;
                                } else {
                                    Source.item.type = ScriptItem::it_belt;
                                }

                                Source.item.itempos = pos;
                                Source.item.owner = player;
                                Source.pos = player->pos;

                                first = false;
                            }
                        } else if (LuaScript || LuaMonsterScript || LuaNPCScript || LuaTileScript) {
                            Target.Type = LUA_ITEM;
                            Target.item = (ScriptItem)player->characterItems[ pos ];
                            Target.item.pos = player->pos;

                            if (pos < MAX_BODY_ITEMS) {
                                Target.item.type = ScriptItem::it_inventory;
                            } else {
                                Target.item.type = ScriptItem::it_belt;
                            }

                            Target.item.itempos = pos;
                            Target.item.owner = player;
                            Target.pos = player->pos;
                            first = false;
                        }
                    } else {
                        paramOK = false;
                    }
                } else {
                    paramOK = false;
                }

                break;

            case UID_VAR:
                paramtemp = bs.popparam();
                Logger::writeMessage("Use","received menu event " + Logger::toString(static_cast<int>(paramtemp)),false);
                break;

            default:

                paramOK = false;
                break;
            }
        }

        if (!bs.ok) {
            std::cout<<"error while popping for use events!"<<std::endl;
            paramOK = false;
        }

        Logger::writeMessage("Use", "received counter parameter: " + Logger::toString(static_cast<int>(counter)),false) ;

        Logger::writeMessage("Use_Scripts",""); //Insert time only
        Logger::writeMessage("Use_Scripts","=========Use Script Start=============",false);
        Logger::writeMessage("Use_Scripts","Source pos (" + Logger::toString(Source.pos.x) + "," + Logger::toString(Source.pos.y) + "," + Logger::toString(Source.pos.z) + ")",false);
        Logger::writeMessage("Use_Scripts","Source type: " + Logger::toString(Source.Type), false);
        Logger::writeMessage("Use_Scripts", "Source Character: " + Logger::toString(Source.character),false);
        Logger::writeMessage("Use_Scripts","",false); //Insert blank line
        Logger::writeMessage("Use_Scripts","Target pos (" + Logger::toString(Target.pos.x) + "," + Logger::toString(Target.pos.y) + "," + Logger::toString(Target.pos.z) + ")",false);
        Logger::writeMessage("Use_Scripts","Target Type: " + Logger::toString(Target.Type),false);
        Logger::writeMessage("Use_Scripts","Target Character: " + Logger::toString(Target.character),false);
        Logger::writeMessage("Use_Scripts","==========Use Script End=============",false);
        Logger::writeMessage("Use_Scripts","",false); //Insert blank line

        std::string msg;

        if (Source.Type == LUA_ITEM && Source.item.quality < 100) {
            if (player->getPlayerLanguage() == 0) {
                player->inform("Du kannst keine unfertigen Gegenst�nde benutzen!");
            } else {
                player->inform("You can't use unfinished items!");
            }
        } else if (LuaScript) {
            player->ltAction->setLastAction(LuaScript, Source, Target, counter , paramtemp, LongTimeAction::AT_USE);
#ifdef DO_UNCONSCIOUS

            if ((paramOK) && player->IsAlive() && player->IsConscious())
#else
            if ((paramOK) && player->IsAlive())
#endif
            {

                if (Source.Type == LUA_ITEM && (Target.Type == LUA_ITEM || Target.Type == LUA_NONE)) {
                    LuaScript->UseItem(player, Source.item, Target.item, counter, static_cast<TYPE_OF_ITEM_ID>(paramtemp), static_cast<unsigned char>(LTS_NOLTACTION));
                    msg = "Used Item: " + Logger::toString(Source.item.id) + " with item: " + Logger::toString(Target.item.id);
                }
            }
        } else if (LuaNPCScript) {
            player->ltAction->setLastAction(LuaNPCScript, Source, Target, counter , paramtemp, LongTimeAction::AT_USE);
#ifdef DO_UNCONSCIOUS

            if ((paramOK) && player->IsAlive() && cp->IsConscious())
#else
            if ((paramOK) && player->IsAlive())
#endif
            {
                if (Source.Type == LUA_CHARACTER && (Target.Type == LUA_NONE)) {
                    LuaNPCScript->useNPC(player, counter, static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                    msg = "Used NPC: " + Source.character->name + "(" + Logger::toString(Source.character->id) + ")";
                } else if (Source.Type == LUA_CHARACTER && Target.Type == LUA_FIELD) {
                    LuaNPCScript->useNPCWithField(player, Target.pos, counter, static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                    msg = "Used NPC: " + Source.character->name + "(" + Logger::toString(Source.character->id) + ") with empty field at pos(" + Logger::toString(Target.pos.x) + "," + Logger::toString(Target.pos.y) + "," + Logger::toString(Target.pos.z) + ")";
                } else if (Source.Type == LUA_CHARACTER && Target.Type == LUA_ITEM) {
                    LuaNPCScript->useNPCWithItem(player, Target.item, counter, static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                    msg = "Used NPC: " + Source.character->name + "(" + Logger::toString(Source.character->id) + ") with Item: " + Logger::toString(Target.item.id);
                } else if (Source.Type == LUA_CHARACTER && Target.Type == LUA_CHARACTER) {
                    LuaNPCScript->useNPCWithCharacter(player, Target.character, counter, static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                    msg = "Used NPC: " + Source.character->name + "(" + Logger::toString(Source.character->id) + ") with character: " + Target.character->name + "(" + Logger::toString(Target.character->id) + ")";
                }
            }

        } else if (LuaMonsterScript) {
            player->ltAction->setLastAction(LuaMonsterScript, Source, Target, counter , paramtemp, LongTimeAction::AT_USE);
#ifdef DO_UNCONSCIOUS

            if ((paramOK) && player->IsAlive() && player->IsConscious())
#else
            if ((paramOK) && player->IsAlive())
#endif
            {
                if (Source.Type == LUA_CHARACTER && (Target.Type == LUA_NONE)) {
                    LuaMonsterScript->useMonster(Source.character,player,counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                    msg = "Used Monster: " + Source.character->name + "(" + Logger::toString(Source.character->id) + ")";
                } else if (Source.Type == LUA_CHARACTER && Target.Type == LUA_FIELD) {
                    LuaMonsterScript->useMonsterWithField(Source.character,player,Target.pos,counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                    msg = "Used Monster: " + Source.character->name + "(" + Logger::toString(Source.character->id) + ") with empty field at pos(" + Logger::toString(Target.pos.x) + "," + Logger::toString(Target.pos.y) + "," + Logger::toString(Target.pos.z) + ")";
                } else if (Source.Type == LUA_CHARACTER && Target.Type == LUA_ITEM) {
                    LuaMonsterScript->useMonsterWithItem(Source.character,player,Target.item,counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                    msg = "Used Monster: " + Source.character->name + "(" + Logger::toString(Source.character->id) + ") with Item: " + Logger::toString(Target.item.id);
                } else if (Source.Type == LUA_CHARACTER && Target.Type == LUA_CHARACTER) {
                    LuaMonsterScript->useMonsterWithCharacter(Source.character,player,Target.character,counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                    msg = "Used Monster: " + Source.character->name + "(" + Logger::toString(Source.character->id) + ") with character: " + Target.character->name + "(" + Logger::toString(Target.character->id) + ")";
                }
            }
        } else if (LuaTileScript) {
            player->ltAction->setLastAction(LuaTileScript, Source, Target, counter , paramtemp, LongTimeAction::AT_USE);
#ifdef DO_UNCONSCIOUS

            if ((paramOK) && player->IsAlive() && player->IsConscious())
#else
            if ((paramOK) && player->IsAlive())
#endif
            {
                if (Source.Type == LUA_FIELD && Target.Type == LUA_NONE) {
                    LuaTileScript->useTile(player,Source.pos,counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                } else if (Source.Type == LUA_FIELD && Target.Type == LUA_FIELD) {
                    LuaTileScript->useTileWithField(player,Source.pos,Target.pos,counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                } else if (Source.Type == LUA_FIELD && Target.Type == LUA_ITEM) {
                    LuaTileScript->useTileWithItem(player,Source.pos,Target.item, counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                } else if (Source.Type == LUA_FIELD && Target.Type == LUA_CHARACTER) {
                    LuaTileScript->useTileWithCharacter(player,Source.pos,Target.character, counter,static_cast<TYPE_OF_ITEM_ID>(paramtemp),static_cast<unsigned char>(LTS_NOLTACTION));
                }
            }
        }

        boost::shared_ptr<BasicServerCommand>cmd(new BBSendActionTC(player->id, player->name, 3,msg));
        World::get()->monitoringClientList->sendCommand(cmd);

    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new UseTS());
        return cmd;
    }

    unsigned char counter;
    ByteStack bs;
};

/**
*@ingroup Clientcommands
*character looks at an menu item
*/
class LookAtMenuItemTS : public BasicClientCommand {
public:
    LookAtMenuItemTS() : BasicClientCommand(C_LOOKATMENUITEM_TS) {
    }

    virtual ~LookAtMenuItemTS() {};

    virtual void decodeData() {
        pos = getUnsignedCharFromBuffer();
        id = getShortIntFromBuffer();
    }

    void performAction(Player *player) {
        Logger::writeMessage("World_Debug",player->name + "looks at an item from a menu.");
        time(&(player->lastaction));
#ifdef DO_UNCONSCIOUS

        if (player->IsAlive() && player->IsConscious())
#else
        if (player->IsAlive())
#endif
        {
            World::get()->lookAtMenueItem(player, pos, id);
            player->actionPoints -= P_LOOK_COST;
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new LookAtMenuItemTS());
        return cmd;
    }
    TYPE_OF_ITEM_ID id;
    unsigned char pos;
};

/**
*@ingroup Clientcommands
*client gives signal that he is ok
*/
class KeepAliveTS : public BasicClientCommand {
public:
    KeepAliveTS() : BasicClientCommand(C_KEEPALIVE_TS) {
    }

    virtual ~KeepAliveTS() {};

    virtual void decodeData() {
    }

    void performAction(Player *player) {
        Logger::writeMessage("World_Debug", "KEEPALIVE_TS von Spieler " + player->name);

        // Release Jailed Players if sentance has expired
        if (player->GetStatus() == JAILEDFORTIME) {
            time_t tempkeepalive;
            time(&tempkeepalive);
            int temptime = tempkeepalive - player->lastkeepalive;

            if ((player->GetStatusTime() - temptime) <= 0) {
                player->SetStatus(0);
                player->SetStatusTime(0);

                position warpto;
                std::stringstream ssx(configOptions["release_x"]);
                ssx >> warpto.x;
                std::stringstream ssy(configOptions["release_y"]);
                ssy >> warpto.y;
                std::stringstream ssz(configOptions["release_z"]);
                ssz >> warpto.z;

                player->Warp(warpto);
                //CWorld::get()->warpPlayer( player, warpto );
            } else {
                player->SetStatusTime(player->GetStatusTime() - temptime);
            }
        }

        time(&(player->lastkeepalive));
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new KeepAliveTS());
        return cmd;
    }
};

/**
*@ingroup Clientcommands
*client asked to send his skills
*/
class RequestSkillsTS : public BasicClientCommand {
public:
    RequestSkillsTS() : BasicClientCommand(C_REQUESTSKILLS_TS) {
    }

    virtual ~RequestSkillsTS() {};

    virtual void decodeData() {
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->sendAllSkills();
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new RequestSkillsTS());
        return cmd;
    }
};

/**
*@ingroup Clientcommands
*char stopps an attack
*/
class AttackStopTS : public BasicClientCommand {
public:
    AttackStopTS() : BasicClientCommand(C_ATTACKSTOP_TS) {
    }

    virtual ~AttackStopTS() {};

    virtual void decodeData() {
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        player->attackmode = false;
        //hast to be changed to a command
        boost::shared_ptr<BasicServerCommand>cmd(new TargetLostTC());
        player->Connection->addCommand(cmd);
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new AttackStopTS());
        return cmd;
    }
};

/**
*@ingroup Clientcommands
*char looks at an item in his inventory
*/
class LookAtInventoryItemTS : public BasicClientCommand {
public:
    LookAtInventoryItemTS() : BasicClientCommand(C_LOOKATINVENTORYITEM_TS) {
    }

    virtual ~LookAtInventoryItemTS() {};

    virtual void decodeData() {
        pos = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        Logger::writeMessage("World_Debug",player->name + " looks at an item in the inventory.");
        time(&(player->lastaction));
#ifdef DO_UNCONSCIOUS

        if (player->IsAlive() && player->IsConscious())
#else
        if (player->IsAlive())
#endif
        {
            World::get()->lookAtInventoryItem(player, pos);
            player->actionPoints -= P_LOOK_COST;
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new LookAtInventoryItemTS());
        return cmd;
    }
    unsigned char pos;
};

/**
*@ingroup Clientcommands
*char looks at an item in his showcase
*/
class LookAtShowCaseItemTS : public BasicClientCommand {
public:
    LookAtShowCaseItemTS() : BasicClientCommand(C_LOOKATSHOWCASEITEM_TS) {
    }

    virtual ~LookAtShowCaseItemTS() {};

    virtual void decodeData() {
        showcase = getUnsignedCharFromBuffer();
        pos = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        Logger::writeMessage("World_Debug", player->name + " looks at an item in a container.");
        time(&(player->lastaction));
#ifdef DO_UNCONSCIOUS

        if (player->IsAlive() && player->IsConscious())
#else
        if (player->IsAlive())
#endif
        {
            World::get()->lookAtShowcaseItem(player, showcase, pos);
            player->actionPoints -= P_LOOK_COST;
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new LookAtShowCaseItemTS());
        return cmd;
    }
    unsigned char showcase;
    unsigned char pos;
};

/**
*@ingroup Clientcommands
*char moves item from inventory to a showcase
*/
class MoveItemFromPlayerToShowCaseTS : public BasicClientCommand {
public:
    MoveItemFromPlayerToShowCaseTS() : BasicClientCommand(C_MOVEITEMFROMPLAYERTOSHOWCASE_TS) {
    }

    virtual ~MoveItemFromPlayerToShowCaseTS() {};

    virtual void decodeData() {
        cpos = getUnsignedCharFromBuffer();
        showcase = getUnsignedCharFromBuffer();
        pos = getUnsignedCharFromBuffer();
        count = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug", player->name + "moves an item from the inventory to showcase!");
#ifdef DO_UNCONSCIOUS

        if (player->IsAlive() && player->IsConscious())
#else
        if (player->IsAlive())
#endif
        {
            World::get()->moveItemFromPlayerIntoShowcase(player, cpos, showcase, pos, count);
            player->actionPoints -= P_ITEMMOVE_COST;
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new MoveItemFromPlayerToShowCaseTS());
        return cmd;
    }
    unsigned char showcase;
    unsigned char pos;
    unsigned char cpos;
    unsigned char count;
};

/**
*@ingroup Clientcommands
*char moves item from showcase to his inventory
*/
class MoveItemFromShowCaseToPlayerTS : public BasicClientCommand {
public:
    MoveItemFromShowCaseToPlayerTS() : BasicClientCommand(C_MOVEITEMFROMSHOWCASETOPLAYER_TS) {
    }

    virtual ~MoveItemFromShowCaseToPlayerTS() {};

    virtual void decodeData() {
        showcase = getUnsignedCharFromBuffer();
        pos = getUnsignedCharFromBuffer();
        cpos = getUnsignedCharFromBuffer();
        count = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug", player->name + " moves an item from the shocase to the inventory!");
#ifdef DO_UNCONSCIOUS

        if (player->IsAlive() && player->IsConscious())
#else
        if (player->IsAlive())
#endif
        {
            World::get()->moveItemFromShowcaseToPlayer(player, showcase, pos, cpos, count);
            player->actionPoints -= P_ITEMMOVE_COST;
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new MoveItemFromShowCaseToPlayerTS());
        return cmd;
    }
    unsigned char showcase;
    unsigned char pos;
    unsigned char cpos;
    unsigned char count;
};

/**
*@ingroup Clientcommands
*char moves items inside his inventory
*/
class MoveItemInsideInventoryTS : public BasicClientCommand {
public:
    MoveItemInsideInventoryTS() : BasicClientCommand(C_MOVEITEMINSIDEINVENTORY_TS) {
    }

    virtual ~MoveItemInsideInventoryTS() {};

    virtual void decodeData() {
        opos = getUnsignedCharFromBuffer();
        npos = getUnsignedCharFromBuffer();
        count = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug", player->name + "moves an item inside the inventory!");
#ifdef DO_UNCONSCIOUS

        if (player->IsAlive() && player->IsConscious())
#else
        if (player->IsAlive())
#endif
        {
            World::get()->moveItemBetweenBodyParts(player, opos, npos, count);
            player->actionPoints -= P_ITEMMOVE_COST;
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new MoveItemInsideInventoryTS());
        return cmd;
    }
    unsigned char opos;
    unsigned char npos;
    unsigned char count;
};

/**
*@ingroup Clientcommands
*char droppes a item from his inventory to the map
*/
class DropItemFromInventoryOnMapTS : public BasicClientCommand {
public:
    DropItemFromInventoryOnMapTS() : BasicClientCommand(C_DROPITEMFROMPLAYERONMAP_TS) {
    }

    virtual ~DropItemFromInventoryOnMapTS() {};

    virtual void decodeData() {
        pos = getUnsignedCharFromBuffer();
        xc = static_cast<short int>(getShortIntFromBuffer());
        yc = static_cast<short int>(getShortIntFromBuffer());
        zc = static_cast<short int>(getShortIntFromBuffer());
        count = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug", player->name + " throws an item from inventory on the map!");
#ifdef DO_UNCONSCIOUS

        if (player->IsConscious())
#endif
        {
            World::get()->dropItemFromPlayerOnMap(player, pos, xc, yc, zc, count);
            player->actionPoints -= P_ITEMMOVE_COST;
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new DropItemFromInventoryOnMapTS());
        return cmd;
    }
    unsigned char pos;
    short int xc,yc,zc;
    unsigned char count;
};

/**
*@ingroup Clientcommands
*char moves item from map to his inventory
*/
class MoveItemFromMapToPlayerTS : public BasicClientCommand {
public:
    MoveItemFromMapToPlayerTS() : BasicClientCommand(C_MOVEITEMFROMMAPTOPLAYER_TS) {
    }

    virtual ~MoveItemFromMapToPlayerTS() {};

    virtual void decodeData() {
        dir = getUnsignedCharFromBuffer();
        pos = getUnsignedCharFromBuffer();
        count = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug", player->name + " moves an Item from the map to the inventory!");
#ifdef DO_UNCONSCIOUS

        if (player->IsAlive() && player->IsConscious())
#else
        if (player->IsAlive())
#endif
        {
            World::get()->moveItemFromMapToPlayer(player, dir, pos, count);
            player->actionPoints -= P_ITEMMOVE_COST;
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new MoveItemFromMapToPlayerTS());
        return cmd;
    }
    unsigned char dir;
    unsigned char pos;
    unsigned char count;
};

/**
*@ingroup Clientcommands
*char moves item from map into a open showcase
*/
class MoveItemFromMapIntoShowCaseTS : public BasicClientCommand {
public:
    MoveItemFromMapIntoShowCaseTS() : BasicClientCommand(C_MOVEITEMFROMMAPINTOSHOWCASE_TS) {
    }

    virtual ~MoveItemFromMapIntoShowCaseTS() {};

    virtual void decodeData() {
        dir = getUnsignedCharFromBuffer();
        showcase = getUnsignedCharFromBuffer();
        pos = getUnsignedCharFromBuffer();
        count = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug", player->name + " moves an item from the map to the showcase!");
#ifdef DO_UNCONSCIOUS

        if (player->IsAlive() && player->IsConscious())
#else
        if (player->IsAlive())
#endif
        {
            World::get()->moveItemFromMapIntoShowcase(player, dir, showcase, pos, count);
            player->actionPoints -= P_ITEMMOVE_COST;
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new MoveItemFromMapIntoShowCaseTS());
        return cmd;
    }
    unsigned char dir;
    unsigned char showcase;
    unsigned char pos;
    unsigned char count;
};

/**
*@ingroup Clientcommands
*char moves item between two showcases
*/
class MoveItemBetweenShowCasesTS : public BasicClientCommand {
public:
    MoveItemBetweenShowCasesTS() : BasicClientCommand(C_MOVEITEMBETWEENSHOWCASES_TS) {
    }

    virtual ~MoveItemBetweenShowCasesTS() {};

    virtual void decodeData() {
        source = getUnsignedCharFromBuffer();
        spos = getUnsignedCharFromBuffer();
        dest = getUnsignedCharFromBuffer();
        dpos = getUnsignedCharFromBuffer();
        count = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug",player->name + " moves an item between showcases!");
#ifdef DO_UNCONSCIOUS

        if (player->IsAlive() && player->IsConscious())
#else
        if (player->IsAlive())
#endif
        {
            World::get()->moveItemBetweenShowcases(player, source, spos, dest, dpos, count);
            player->actionPoints -= P_ITEMMOVE_COST;
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new MoveItemBetweenShowCasesTS());
        return cmd;
    }
    unsigned char source;
    unsigned char spos;
    unsigned char dest;
    unsigned char dpos;
    unsigned char count;
};

/**
*@ingroup Clientcommands
*char droppes a item from a showcase to the map
*/
class DropItemFromShowCaseOnMapTS : public BasicClientCommand {
public:
    DropItemFromShowCaseOnMapTS() : BasicClientCommand(C_DROPITEMFROMSHOWCASEONMAP_TS) {
    }

    virtual ~DropItemFromShowCaseOnMapTS() {};

    virtual void decodeData() {
        showcase = getUnsignedCharFromBuffer();
        pos = getUnsignedCharFromBuffer();
        xc = static_cast<short int>(getShortIntFromBuffer());
        yc = static_cast<short int>(getShortIntFromBuffer());
        zc = static_cast<short int>(getShortIntFromBuffer());
        count = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug",player->name + " moves an item from showcase to the map!");
#ifdef DO_UNCONSCIOUS

        if (player->IsConscious())
#endif
        {
            World::get()->dropItemFromShowcaseOnMap(player, showcase, pos, xc, yc, zc, count);
            player->actionPoints -= P_ITEMMOVE_COST;
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new DropItemFromShowCaseOnMapTS());
        return cmd;
    }
    unsigned char showcase;
    unsigned char pos;
    short int xc,yc,zc;
    unsigned char count;
};

/**
*@ingroup Clientcommands
*char closes a showcase container
*/
class CloseContainerInShowCaseTS : public BasicClientCommand {
public:
    CloseContainerInShowCaseTS() : BasicClientCommand(C_CLOSECONTAINERINSHOWCASE_TS) {
    }

    virtual ~CloseContainerInShowCaseTS() {};

    virtual void decodeData() {
        showcase = getUnsignedCharFromBuffer();
        //getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug", player->name + " closes a container in the showcase");
#ifdef DO_UNCONSCIOUS

        if (player->IsAlive() && player->IsConscious())
#else
        if (player->IsAlive())
#endif
        {
            World::get()->closeContainerInShowcase(player, showcase);
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new CloseContainerInShowCaseTS());
        return cmd;
    }
    unsigned char showcase;
};

/**
*@ingroup Clientcommands
*char looks into a showcase container
*/
class LookIntoShowCaseContainerTS : public BasicClientCommand {
public:
    LookIntoShowCaseContainerTS() : BasicClientCommand(C_LOOKINTOSHOWCASECONTAINER_TS) {
    }

    virtual ~LookIntoShowCaseContainerTS() {};

    virtual void decodeData() {
        showcase = getUnsignedCharFromBuffer();
        pos = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug", player->name + " looks into a container in a showcase!");
#ifdef DO_UNCONSCIOUS

        if (player->IsConscious())
#endif
        {
            World::get()->lookIntoShowcaseContainer(player, showcase, pos);
            player->actionPoints -= P_LOOK_COST;
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new LookIntoShowCaseContainerTS());
        return cmd;
    }
    unsigned char showcase;
    unsigned char pos;
};

/**
*@ingroup Clientcommands
*char looks into the inventory
*/
class LookIntoInventoryTS : public BasicClientCommand {
public:
    LookIntoInventoryTS() : BasicClientCommand(C_LOOKINTOINVENTORY_TS) {
    }

    virtual ~LookIntoInventoryTS() {};

    virtual void decodeData() {
        showcase = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug", player->name + " looks into his backpack");
#ifdef DO_UNCONSCIOUS

        if (player->IsConscious())
#endif
        {
            World::get()->lookIntoBackPack(player, showcase);
            player->actionPoints -= P_LOOK_COST;
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new LookIntoInventoryTS());
        return cmd;
    }
    unsigned char showcase;
};

/**
*@ingroup Clientcommands
*char looks into a container on a map field
*/
class LookIntoContainerOnFieldTS : public BasicClientCommand {
public:
    LookIntoContainerOnFieldTS() : BasicClientCommand(C_LOOKINTOCONTAINERONFIELD_TS) {
    }

    virtual ~LookIntoContainerOnFieldTS() {};

    virtual void decodeData() {
        direction = getUnsignedCharFromBuffer();
        showcase = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug", player->name + " looks into a container on the map");
#ifdef DO_UNCONSCIOUS

        if (player->IsAlive() && player->IsConscious())
#else
        if (player->IsAlive())
#endif
        {
            World::get()->lookIntoContainerOnField(player, direction, showcase);
            player->actionPoints -= P_LOOK_COST;
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new LookIntoContainerOnFieldTS());
        return cmd;
    }
    unsigned char direction;
    unsigned char showcase;
};

/**
*@ingroup Clientcommands
*char logged out
*/
class LogOutTS : public BasicClientCommand {
public:
    /**
    *a client logged out
    */
    LogOutTS() : BasicClientCommand(C_LOGOUT_TS) {
    }

    virtual ~LogOutTS() {};

    virtual void decodeData() {
    }

    void performAction(Player *player) {
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug", player->name + " logt aus");
        player->Connection->closeConnection();
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new LogOutTS());
        return cmd;
    }
};

/**
*@ingroup Clientcommands
*char talks to another one
*/
class WhisperTS : public BasicClientCommand {
public:
    /**
    *a player talked
    */
    WhisperTS() : BasicClientCommand(C_WHISPER_TS) {
    }

    virtual ~WhisperTS() {};

    virtual void decodeData() {
        text = getStringFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        Logger::writeMessage("World_Debug", player->name + " whispers something!");
        player->talk(Character::tt_whisper, text);
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new WhisperTS());
        return cmd;
    }
    std::string text;
};

/**
*@ingroup Clientcommands
*char talks to another one
*/
class ShoutTS : public BasicClientCommand {
public:
    /**
    *a player talked
    */
    ShoutTS() : BasicClientCommand(C_SHOUT_TS) {
    }

    virtual ~ShoutTS() {};

    virtual void decodeData() {
        text = getStringFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->talk(Character::tt_yell, text);
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new ShoutTS());
        return cmd;
    }
    std::string text;
};

/**
*@ingroup Clientcommands
*char talks to another one
*/
class SayTS : public BasicClientCommand {
public:
    /**
    *a player talked
    */
    SayTS() : BasicClientCommand(C_SAY_TS) {
    }

    virtual ~SayTS() {};

    virtual void decodeData() {
        text = getStringFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        Logger::writeMessage("World_Debug", player->name + " whispers something!");

        if ((!player->hasGMRight(gmr_isnotshownasgm) && !player->isAdmin()) || !World::get()->parseGMCommands(player, text)) {
            if (!World::get()->parsePlayerCommands(player, text)) {    // did we issue a player command?
                player->talk(Character::tt_say, text);
            }
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new SayTS());
        return cmd;
    }
    std::string text;
};

/**
*@ingroup Clientcommands
*char wants a clientupdate for the map
*/
class RefreshTS : public BasicClientCommand {
public:
    /**
    *a clientupdate should be sendet to the client
    */
    RefreshTS() : BasicClientCommand(C_REFRESH_TS) {
    }

    virtual ~RefreshTS() {};

    virtual void decodeData() {
    }

    void performAction(Player *player) {
        Logger::writeMessage("World_Debug", player->name + " want sended a refresh_ts, sending map!");
        //update the current mapview
        player->sendFullMap();
        World::get()->sendAllVisibleCharactersToPlayer(player, true);
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new RefreshTS());
        return cmd;
    }
};

/**
*@ingroup Clientcommands
*char introduces to another one
*/
class IntroduceTS : public BasicClientCommand {
public:
    /**
    *a player attacked another char
    */
    IntroduceTS() : BasicClientCommand(C_INTRODUCE_TS) {
    }

    virtual ~IntroduceTS() {};

    virtual void decodeData() {
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        Logger::writeMessage("World_Debug",player->name + " introduces himself!");
#ifdef DO_UNCONSCIOUS

        if (player->IsAlive() && player->IsConscious())
#else
        if (player->IsAlive())
#endif
        {
            World::get()->introduceMyself(player);
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new IntroduceTS());
        return cmd;
    }
};

/**
*@ingroup Clientcommands
*char attacks another char
*/
class AttackPlayerTS : public BasicClientCommand {
public:
    /**
    *a player attacked another char
    */
    AttackPlayerTS() : BasicClientCommand(C_ATTACKPLAYER_TS) {

    }

    virtual ~AttackPlayerTS() {};

    virtual void decodeData() {
        enemyid = getIntFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
#ifdef DO_UNCONSCIOUS

        if (player->IsAlive() && player->GetStatus() < 10 && player->IsConscious())
#else
        if (player->IsAlive() && player->GetStatus() < 10)
#endif
        {
            player->ltAction->abortAction();

            if (player->IsAlive()) {
                player->enemyid = enemyid;
                player->attackmode = true;
                player->enemytype = Character::player;

                if (player->enemyid >= MONSTER_BASE) {
                    player->enemytype = Character::monster;
                }

                if (player->enemyid >= NPC_BASE) {
                    player->enemytype = Character::npc;
                }

                boost::shared_ptr<BasicServerCommand>cmd(new AttackAcknowledgedTC());
                player->Connection->addCommand(cmd);
                player->startMusic(FIGHTMUSIC);
                //monitoringClientList->sendCommand( new SendActionTS(player->id, player->name, 0, "Starts an attack: " + Logger::toString(player->enemyid) ) );
                World::get()->characterAttacks(player);
            } else {
                player->attackmode = false;
            }
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new AttackPlayerTS());
        return cmd;
    }
    uint32_t enemyid;

};

/**
*@ingroup Clientcommands
*char looks at a map item
*/
class LookAtMapItemTS : public BasicClientCommand {
public:
    /**
    *a spin occured special handling to get direction which is encoded in the definition byte
    */
    LookAtMapItemTS() : BasicClientCommand(C_LOOKATMAPITEM_TS) {
    }

    virtual ~LookAtMapItemTS() {};

    virtual void decodeData() {
        x = getShortIntFromBuffer();
        y = getShortIntFromBuffer();
        z = getShortIntFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        Logger::writeMessage("World_Debug",player->name + " looks at a map item.");
#ifdef DO_UNCONSCIOUS

        if (player->IsAlive() && player->IsConscious())
#else
        if (player->IsAlive())
#endif
        {
            World::get()->lookAtMapItem(player, x, y, z);
            player->actionPoints -= P_LOOK_COST;
        }

    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new LookAtMapItemTS());
        return cmd;
    }
    short int x,y,z;

};

/**
*@ingroup Clientcommands
*char starts to spin around
*/
class PSpinActionTS : public BasicClientCommand {
public:
    /**
    *a spin occured special handling to get direction which is encoded in the definition byte
    */
    PSpinActionTS(uint8_t dir) : BasicClientCommand(C_PSPINRSTART_TS) {
        direction = dir;
    }

    virtual ~PSpinActionTS() {};

    virtual void decodeData() {
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug", "Player changes his dircetion: " + player->name + " temp: " + Logger::toString(direction));

#ifdef DO_UNCONSCIOUS

        if (player->IsConscious())
#endif
        {
            if (World::get()->spinPlayer(player, direction)) {
                player->actionPoints -= P_SPIN_COST;
            }
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new PSpinActionTS(direction));
        return cmd;
    }
    uint8_t direction;
};


/**
*@ingroup Clientcommands
*a char move has occured
*/
class CharMoveTS : public BasicClientCommand {
public:
    /**
    *a character has started moving
    */
    CharMoveTS() : BasicClientCommand(C_CHARMOVE_TS) {
    }

    virtual ~CharMoveTS() {};

    virtual void decodeData() {
        charid = getIntFromBuffer();
        direction = getUnsignedCharFromBuffer();
        mode = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));

        if (charid == player->id && (mode == NORMALMOVE || mode == RUNNING)) {
            player->ltAction->abortAction();
            Logger::writeMessage("World_Debug", "Playermove from Player: " + player->name);

            if (player->getTurtleActive() && player->hasGMRight(gmr_settiles) && mode == NORMALMOVE) {
                World::get()->setNextTile(player, player->getTurtleTile());
                Logger::writeMessage("Player_Moves","Turtle was active, new tile set at pos: " + Logger::toString(player->pos.x) + "," + Logger::toString(player->pos.y) + "," + Logger::toString(player->pos.z) + " tile: " + Logger::toString(player->getTurtleTile()));
            }

            if (player->move(static_cast<Character::direction>(direction), mode)) {
                player->closeAllShowcasesOfMapContainers();
            }
        } else if (mode == PUSH) {
            player->ltAction->abortAction();
            Logger::writeMessage("World_Debug", "Player pushes another: " + player->name);

            if (player->IsAlive()) {
                if (World::get()->pushCharacter(player, charid, direction)) {
                    player->actionPoints -= P_PUSH_COST;
                }
            }
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new CharMoveTS());
        return cmd;
    }

    TYPE_OF_CHARACTER_ID charid;
    unsigned char direction;
    unsigned char mode;
};


/**
*@ingroup Clientcommands
*char starts to move a item
*/
class IMoverActionTS : public BasicClientCommand {
public:
    /**
    *a item move occured special handling to get direction which is encoded in the definition byte
    */
    IMoverActionTS(uint8_t dir) : BasicClientCommand(C_IMOVERSTART_TS) {
        direction = dir;
    }

    virtual ~IMoverActionTS() {};

    virtual void decodeData() {
        xc = static_cast<char>(getShortIntFromBuffer());
        yc = static_cast<char>(getShortIntFromBuffer());
        zc = static_cast<char>(getShortIntFromBuffer());
        count = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
        player->ltAction->abortAction();
        Logger::writeMessage("World_Debug", player->name + " tryes to move an Item!");
#ifdef DO_UNCONSCIOUS

        if (player->IsAlive() && player->IsConscious())
#else
        if (player->IsAlive())
#endif
        {
            World::get()->moveItem(player, direction, xc, yc, zc, count);
            player->actionPoints -= P_ITEMMOVE_COST;
        }
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new IMoverActionTS(direction));
        return cmd;
    }
    short int xc, yc, zc;
    uint8_t count;
    uint8_t direction;
};


//=========================================Login Command=========================
/**
*@ingroup Clientcommands
*a player tries to login
*/
class LoginCommandTS : public BasicClientCommand {
public:
    LoginCommandTS() : BasicClientCommand(C_LOGIN_TS) {
    }

    virtual ~LoginCommandTS() {};

    virtual void decodeData() {
        clientVersion = getUnsignedCharFromBuffer();
        loginName = getStringFromBuffer();
        passwort = getStringFromBuffer();
    }

    void performAction(Player *player) {
        time(&(player->lastaction));
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new LoginCommandTS());
        return cmd;
    }

    unsigned short clientVersion;
    std::string loginName;
    std::string passwort;
};
//==============================End Login Command================================

//=========================================Screen Size Command=========================
/**
*@ingroup Clientcommands
*a player tries to login
*/
class ScreenSizeCommandTS : public BasicClientCommand {
public:
    ScreenSizeCommandTS() : BasicClientCommand(C_SCREENSIZE_TS) {
    }

    virtual ~ScreenSizeCommandTS() {};

    virtual void decodeData() {
        width = getUnsignedCharFromBuffer();
        height = getUnsignedCharFromBuffer();
    }

    void performAction(Player *player) {
        // Set screen width and screen height
        player->screenwidth = width;
        player->screenheight = height;
        player->sendFullMap();
    }

    boost::shared_ptr<BasicClientCommand> clone() {
        boost::shared_ptr<BasicClientCommand>cmd(new ScreenSizeCommandTS());
        return cmd;
    }

    uint8_t width;
    uint8_t height;
};
//==============================End Screen Size Command================================

#endif
