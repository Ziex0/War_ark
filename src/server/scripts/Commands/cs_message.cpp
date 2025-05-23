/*
 * This file is part of the WarheadCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/* ScriptData
Name: message_commandscript
%Complete: 100
Comment: All message related commands
Category: commandscripts
EndScriptData */

#include "Channel.h"
#include "Chat.h"
#include "ChatTextBuilder.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptObject.h"
#include "WorldSession.h"

using namespace Warhead::ChatCommands;

class message_commandscript : public CommandScript
{
public:
    message_commandscript() : CommandScript("message_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable commandTable =
        {
            { "nameannounce",   HandleNameAnnounceCommand,   SEC_GAMEMASTER, Console::Yes },
            { "gmnameannounce", HandleGMNameAnnounceCommand, SEC_GAMEMASTER, Console::Yes },
            { "announce",       HandleAnnounceCommand,       SEC_GAMEMASTER, Console::Yes },
            { "gmannounce",     HandleGMAnnounceCommand,     SEC_GAMEMASTER, Console::Yes },
            { "notify",         HandleNotifyCommand,         SEC_GAMEMASTER, Console::Yes },
            { "gmnotify",       HandleGMNotifyCommand,       SEC_GAMEMASTER, Console::Yes },
            { "whispers",       HandleWhispersCommand,       SEC_MODERATOR,  Console::No },
        };
        return commandTable;
    }

    static bool HandleNameAnnounceCommand(ChatHandler* handler, Tail message)
    {
        if (message.empty())
            return false;

        std::string name("Console");
        if (WorldSession* session = handler->GetSession())
            name = session->GetPlayer()->GetName();

        Warhead::Text::SendWorldText(LANG_ANNOUNCE_COLOR, name, std::string_view(message));

        return true;
    }

    static bool HandleGMNameAnnounceCommand(ChatHandler* handler, Tail message)
    {
        if (message.empty())
            return false;

        std::string name("Console");
        if (WorldSession* session = handler->GetSession())
            name = session->GetPlayer()->GetName();

        Warhead::Text::SendGMText(LANG_ANNOUNCE_COLOR, name, std::string_view(message));

        return true;
    }

    // global announce
    static bool HandleAnnounceCommand(ChatHandler* handler, Tail message)
    {
        if (message.empty())
            return false;

        sWorld->SendServerMessage(SERVER_MSG_STRING, Warhead::StringFormat(fmt::runtime(handler->GetWarheadString(LANG_SYSTEMMESSAGE)), std::string_view(message)));
        return true;
    }

    // announce to logged in GMs
    static bool HandleGMAnnounceCommand(ChatHandler* /*handler*/, Tail message)
    {
        if (message.empty())
            return false;

        Warhead::Text::SendGMText(LANG_GM_BROADCAST, std::string_view(message));

        return true;
    }

    // send on-screen notification to players
    static bool HandleNotifyCommand(ChatHandler* handler, Tail message)
    {
        if (message.empty())
            return false;

        std::string str = handler->GetWarheadString(LANG_GLOBAL_NOTIFY);
        str += message;

        WorldPacket data(SMSG_NOTIFICATION, (str.size() + 1));
        data << str;
        sWorld->SendGlobalMessage(&data);

        return true;
    }

    // send on-screen notification to GMs
    static bool HandleGMNotifyCommand(ChatHandler* handler, Tail message)
    {
        if (message.empty())
            return false;

        std::string str = handler->GetWarheadString(LANG_GM_NOTIFY);
        str += message;

        WorldPacket data(SMSG_NOTIFICATION, (str.size() + 1));
        data << str;
        sWorld->SendGlobalGMMessage(&data);

        return true;
    }

    // Enable/Disable accepting whispers (for GM)
    static bool HandleWhispersCommand(ChatHandler* handler, Optional<Variant<bool, EXACT_SEQUENCE("remove")>> operationArg, Optional<std::string> playerNameArg)
    {
        if (!operationArg)
        {
            handler->PSendSysMessage(LANG_COMMAND_WHISPERACCEPTING, handler->GetSession()->GetPlayer()->isAcceptWhispers() ?  handler->GetWarheadString(LANG_ON) : handler->GetWarheadString(LANG_OFF));
            return true;
        }

        if (operationArg->holds_alternative<bool>())
        {
            if (operationArg->get<bool>())
            {
                handler->GetSession()->GetPlayer()->SetAcceptWhispers(true);
                handler->SendSysMessage(LANG_COMMAND_WHISPERON);
                return true;
            }
            else
            {
                // Remove all players from the Gamemaster's whisper whitelist
                handler->GetSession()->GetPlayer()->ClearWhisperWhiteList();
                handler->GetSession()->GetPlayer()->SetAcceptWhispers(false);
                handler->SendSysMessage(LANG_COMMAND_WHISPEROFF);
                return true;
            }
        }

        if (operationArg->holds_alternative<EXACT_SEQUENCE("remove")>())
        {
            if (!playerNameArg)
                return false;

            if (normalizePlayerName(*playerNameArg))
            {
                if (Player* player = ObjectAccessor::FindPlayerByName(*playerNameArg))
                {
                    handler->GetSession()->GetPlayer()->RemoveFromWhisperWhiteList(player->GetGUID());
                    handler->PSendSysMessage(LANG_COMMAND_WHISPEROFFPLAYER, playerNameArg->c_str());
                    return true;
                }
                else
                {
                    handler->PSendSysMessage(LANG_PLAYER_NOT_FOUND, playerNameArg->c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }
        }
        handler->SendSysMessage(LANG_USE_BOL);
        handler->SetSentErrorMessage(true);
        return false;
    }
};

void AddSC_message_commandscript()
{
    new message_commandscript();
}