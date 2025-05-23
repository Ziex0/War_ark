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

#include "WaypointMgr.h"
#include "DBCacheMgr.h"
#include "DatabaseEnv.h"
#include "GridDefines.h"
#include "Log.h"
#include "StopWatch.h"

WaypointMgr::WaypointMgr() = default;

WaypointMgr::~WaypointMgr()
{
    for (WaypointPathContainer::iterator itr = _waypointStore.begin(); itr != _waypointStore.end(); ++itr)
    {
        for (WaypointPath::const_iterator it = itr->second.begin(); it != itr->second.end(); ++it)
            delete *it;

        itr->second.clear();
    }

    _waypointStore.clear();
}

WaypointMgr* WaypointMgr::instance()
{
    static WaypointMgr instance;
    return &instance;
}

void WaypointMgr::Load()
{
    StopWatch sw;

    auto result{ sDBCacheMgr->GetResult(DBCacheTable::WaypointData) };
    if (!result)
    {
        LOG_WARN("server.loading", ">> Loaded 0 waypoints. DB table `waypoint_data` is empty!");
        LOG_INFO("server.loading", " ");
        return;
    }

    uint32 count = 0;

    do
    {
        auto fields = result->Fetch();
        WaypointData* wp = new WaypointData();

        uint32 pathId = fields[0].Get<uint32>();
        WaypointPath& path = _waypointStore[pathId];

        float x = fields[2].Get<float>();
        float y = fields[3].Get<float>();
        float z = fields[4].Get<float>();
        Optional<float > o;
        if (!fields[5].IsNull())
            o = fields[5].Get<float>();

        Warhead::NormalizeMapCoord(x);
        Warhead::NormalizeMapCoord(y);

        wp->id = fields[1].Get<uint32>();
        wp->x = x;
        wp->y = y;
        wp->z = z;
        wp->orientation = o;
        wp->move_type = fields[6].Get<uint32>();

        if (wp->move_type >= WAYPOINT_MOVE_TYPE_MAX)
        {
            //LOG_ERROR("db.query", "Waypoint {} in waypoint_data has invalid move_type, ignoring", wp->id);
            delete wp;
            continue;
        }

        wp->delay = fields[7].Get<uint32>();
        wp->event_id = fields[8].Get<uint32>();
        wp->event_chance = fields[9].Get<int16>();

        path.push_back(wp);
        ++count;
    } while (result->NextRow());

    LOG_INFO("server.loading", ">> Loaded {} waypoints in {}", count, sw);
    LOG_INFO("server.loading", " ");
}

void WaypointMgr::ReloadPath(uint32 id)
{
    WaypointPathContainer::iterator itr = _waypointStore.find(id);
    if (itr != _waypointStore.end())
    {
        for (WaypointPath::const_iterator it = itr->second.begin(); it != itr->second.end(); ++it)
            delete *it;

        _waypointStore.erase(itr);
    }

    WorldDatabasePreparedStatement stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_WAYPOINT_DATA_BY_ID);

    stmt->SetData(0, id);

    PreparedQueryResult result = WorldDatabase.Query(stmt);

    if (!result)
        return;

    WaypointPath& path = _waypointStore[id];

    do
    {
        auto fields = result->Fetch();
        WaypointData* wp = new WaypointData();

        float x = fields[1].Get<float>();
        float y = fields[2].Get<float>();
        float z = fields[3].Get<float>();
        Optional<float> o;
        if (!fields[4].IsNull())
            o = fields[4].Get<float>();

        Warhead::NormalizeMapCoord(x);
        Warhead::NormalizeMapCoord(y);

        wp->id = fields[0].Get<uint32>();
        wp->x = x;
        wp->y = y;
        wp->z = z;
        wp->orientation = o;
        wp->move_type = fields[5].Get<uint32>();

        if (wp->move_type >= WAYPOINT_MOVE_TYPE_MAX)
        {
            //LOG_ERROR("db.query", "Waypoint {} in waypoint_data has invalid move_type, ignoring", wp->id);
            delete wp;
            continue;
        }

        wp->delay = fields[6].Get<uint32>();
        wp->event_id = fields[7].Get<uint32>();
        wp->event_chance = fields[8].Get<uint8>();

        path.push_back(wp);
    } while (result->NextRow());
}
