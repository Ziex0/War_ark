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

// This is where scripts' loading functions should be declared:
// void MyExampleScript()

// The name of this function should match:
// void Add${NameOfDirectory}Scripts()

void AddSC_Teleporter();
void AddSC_NPC_VisualWeapon();
void AddSC_NPC_Funny();

void AddSC_beastmaster_npc();


// Custom part

void AddCustomScripts()
{
    AddSC_Teleporter();
	AddSC_NPC_VisualWeapon();
	AddSC_NPC_Funny();
	
	AddSC_beastmaster_npc();
}
