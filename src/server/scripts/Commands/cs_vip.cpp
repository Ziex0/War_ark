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

#include "CharacterCache.h"
#include "Chat.h"
#include "Creature.h"
#include "GameLocale.h"
#include "GameTime.h"
#include "ScriptObject.h"
#include "Timer.h"
#include "Vip.h"

using namespace Warhead::ChatCommands;

class vip_commandscript : public CommandScript
{
public:
    vip_commandscript() : CommandScript("vip_commandscript") { }

    [[nodiscard]] ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable vipReloadCommandTable =
        {
            { "rates",      HandleVipReloadRatesCommand,    SEC_ADMINISTRATOR,  Console::Yes },
            { "vendors",    HandleVipReloadVendorsCommand,  SEC_ADMINISTRATOR,  Console::Yes }
        };

        static ChatCommandTable vipListCommandTable =
        {
            { "rates",      HandleVipListRatesCommand,   SEC_ADMINISTRATOR,  Console::Yes }
        };

        static ChatCommandTable vipVendorCommandTable =
        {
            { "info",      HandleVipVendorInfoCommand,   SEC_ADMINISTRATOR,  Console::No },
            { "add",       HandleVipVendorAddCommand,    SEC_ADMINISTRATOR,  Console::No },
            { "delete",    HandleVipVendorDeleteCommand, SEC_ADMINISTRATOR,  Console::No }
        };

        static ChatCommandTable vipCommandTable =
        {
            { "add",        HandleVipAddCommand,        SEC_ADMINISTRATOR,  Console::Yes },
            { "delete",     HandleVipDeleteCommand,     SEC_ADMINISTRATOR,  Console::Yes },
            { "unbind",     HandleVipUnbindCommand,     SEC_PLAYER,         Console::No },
            { "info",       HandleVipInfoCommand,       SEC_PLAYER,         Console::Yes },
            { "menu",       HandleVipMenuCommand,       SEC_PLAYER,         Console::No },
            { "list",       vipListCommandTable },
            { "vendor",     vipVendorCommandTable },
            { "reload",     vipReloadCommandTable },
        };

        static ChatCommandTable commandTable =
        {
            { "vip", vipCommandTable }
        };

        return commandTable;
    }

    static bool HandleVipAddCommand(ChatHandler* handler, Optional<PlayerIdentifier> target, uint32 duration, uint8 level)
    {
        if (!sVip->IsEnable())
        {
            handler->PSendSysMessage("> Module disabled");
            return true;
        }

        if (!target)
        {
            handler->PSendSysMessage("> The player's name is not specified, and a name will be chosen based on the target.");
            target = PlayerIdentifier::FromTargetOrSelf(handler);
        }

        if (!target)
        {
            handler->PSendSysMessage("> No target selected.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        auto data = sCharacterCache->GetCharacterCacheByGuid(target->GetGUID());

        if (!data)
        {
            handler->PSendSysMessage("> Player {} not found in the database.", target->GetName());
            return false;
        }

        if (sVip->IsVip(data->AccountId))
            handler->PSendSysMessage("> The player {} has already been a VIP..", target->GetName());

        if (sVip->Add(data->AccountId, GameTime::GetGameTime() + Days(duration), level, true))
        {
            handler->PSendSysMessage("> Update of the Vip account status for player {}.", target->GetName());
            handler->PSendSysMessage("> Level {}. Remaining time {}.", level, Warhead::Time::ToTimeString(Days(duration)));
            return true;
        }

        return false;
    }

    static bool HandleVipUnbindCommand(ChatHandler* handler)
    {
        if (!sVip->IsEnable())
        {
            handler->PSendSysMessage("> Module disabled");
            return true;
        }

        sVip->UnBindInstances(handler->GetPlayer());
        return true;
    }

    static bool HandleVipDeleteCommand(ChatHandler* handler, Optional<PlayerIdentifier> target)
    {
        if (!sVip->IsEnable())
        {
            handler->PSendSysMessage("> Module disabled");
            return true;
        }

        if (!target)
        {
            handler->PSendSysMessage("> No player name is specified, the name will be selected from the target");
            target = PlayerIdentifier::FromTargetOrSelf(handler);
        }

        if (!target)
        {
            handler->PSendSysMessage("> No target selected");
            handler->SetSentErrorMessage(true);
            return false;
        }

        auto data = sCharacterCache->GetCharacterCacheByGuid(target->GetGUID());

        if (!data)
        {
            handler->PSendSysMessage("> Player {} is not found in the database", target->GetName());
            return false;
        }

        if (!sVip->IsVip(data->AccountId))
        {
            handler->PSendSysMessage("> Player {} does not have Vip status", target->GetName());
            return true;
        }

        sVip->UnSet(data->AccountId);

        handler->PSendSysMessage("> Player {} no longer has Vip", target->GetName());
        return true;
    }

    static bool HandleVipInfoCommand(ChatHandler* handler, Optional<PlayerIdentifier> target)
    {
        if (!sVip->IsEnable())
        {
            handler->PSendSysMessage("> Module disabled");
            return true;
        }

        if (!target)
        {
            handler->PSendSysMessage("> No player name is specified, the name will be selected from the target");
            target = PlayerIdentifier::FromTargetOrSelf(handler);
        }

        if (!target)
        {
            handler->PSendSysMessage("> Не выбрана цель");
            handler->SetSentErrorMessage(true);
            return false;
        }

        sVip->SendVipInfo(handler, target->GetGUID());
        return true;
    }

    static bool HandleVipListRatesCommand(ChatHandler* handler)
    {
        if (!sVip->IsEnable())
        {
            handler->PSendSysMessage("> Module disabled");
            return true;
        }

        sVip->SendVipListRates(handler);
        return true;
    }

    static bool HandleVipReloadRatesCommand(ChatHandler* handler)
    {
        if (!sVip->IsEnable())
        {
            handler->PSendSysMessage("> Module disabled");
            return true;
        }

        sVip->LoadRates();
        handler->PSendSysMessage("> VIP rates have been reset");
        return true;
    }

    static bool HandleVipReloadVendorsCommand(ChatHandler* handler)
    {
        if (!sVip->IsEnable())
        {
            handler->PSendSysMessage("> Module disabled");
            return true;
        }

        sVip->LoadVipVendors();
        handler->PSendSysMessage("> VIP vendors rebooted");
        return true;
    }

    static bool HandleVipVendorInfoCommand(ChatHandler* handler)
    {
        if (!sVip->IsEnable())
        {
            handler->PSendSysMessage("> Module disabled");
            return true;
        }

        Creature* vendor = handler->getSelectedCreature();
        if (!vendor)
        {
            handler->SendSysMessage("You need to choose a creature");
            return true;
        }

        auto creatureEntry = vendor->GetEntry();
        auto creatureName = sGameLocale->GetCreatureNamelocale(vendor->GetEntry(), handler->GetSessionDbLocaleIndex());

        if (!vendor->IsVendor())
        {
            handler->PSendSysMessage("# Creature `{}` - '{}' is not a vendor", creatureEntry, creatureName);
            return true;
        }

        if (!sVip->IsVipVendor(creatureEntry))
        {
            handler->PSendSysMessage("# Creature `{}` - '{}' is not a VIP vendor", creatureEntry, creatureName);
            return true;
        }

        handler->PSendSysMessage("# Creature `{}` - '{}' available for VIP '{}' level and above", creatureEntry, creatureName, sVip->GetVendorVipLevel(creatureEntry));
        return true;
    }

    static bool HandleVipVendorAddCommand(ChatHandler* handler, uint8 vendorVipLevel)
    {
        if (!sVip->IsEnable())
        {
            handler->PSendSysMessage("> Module disabled");
            return true;
        }

        Creature* vendor = handler->getSelectedCreature();
        if (!vendor)
        {
            handler->SendSysMessage("You need to select Creature");
            return true;
        }

        auto creatureEntry = vendor->GetEntry();
        auto creatureName = sGameLocale->GetCreatureNamelocale(vendor->GetEntry(), handler->GetSessionDbLocaleIndex());

        if (!vendor->IsVendor())
        {
            handler->PSendSysMessage("# Creature `{}` - '{}' is not a vendor", creatureEntry, creatureName);
            return true;
        }

        if (sVip->IsVipVendor(creatureEntry))
        {
            handler->PSendSysMessage("# Creature `{}` - '{}' was already a VIP vendor", creatureEntry, creatureName);
            sVip->DeleteVendorVipLevel(creatureEntry);
        }

        sVip->AddVendorVipLevel(creatureEntry, vendorVipLevel);
        handler->PSendSysMessage("# Creature `{}` - '{}' now available for VIP '{}' level and above", creatureEntry, creatureName, sVip->GetVendorVipLevel(creatureEntry));
        return true;
    }

    static bool HandleVipVendorDeleteCommand(ChatHandler* handler)
    {
        if (!sVip->IsEnable())
        {
            handler->PSendSysMessage("> Module disabled");
            return true;
        }

        Creature* vendor = handler->getSelectedCreature();
        if (!vendor)
        {
            handler->SendSysMessage("Need to choose Creature");
            return true;
        }

        auto creatureEntry = vendor->GetEntry();
        auto creatureName = sGameLocale->GetCreatureNamelocale(vendor->GetEntry(), handler->GetSessionDbLocaleIndex());

        if (!vendor->IsVendor())
        {
            handler->PSendSysMessage("# Creature `{}` - '{}' is not a vendor", creatureEntry, creatureName);
            return true;
        }

        if (!sVip->IsVipVendor(creatureEntry))
        {
            handler->PSendSysMessage("# Creature `{}` - '{}' is not a VIP vendor", creatureEntry, creatureName);
            return true;
        }

        sVip->DeleteVendorVipLevel(creatureEntry);
        handler->PSendSysMessage("# Creature `{}` - '{}' available to everyone", creatureEntry, creatureName);
        return true;
    }

    static bool HandleVipMenuCommand(ChatHandler* handler)
    {
        if (!sVip->IsEnable())
        {
            handler->PSendSysMessage("> Module disabled");
            return true;
        }

        sVip->SendVipMenu(handler->GetPlayer());
        return true;
    }
};

void AddSC_vip_commandscript()
{
    new vip_commandscript();
}
