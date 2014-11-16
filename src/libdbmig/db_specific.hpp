/*
    dbmig - Database schema migration tool
    Copyright (C) 2012-2014  Adam Szmigin (adam.szmigin@xsco.net)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DBMIG_CHANGELOG_SQL_INCLUDED
#define DBMIG_CHANGELOG_SQL_INCLUDED

#include <string>

namespace dbmig {

    struct db_specific
    {
        std::string default_changelog_table;
        std::string changelog_exists_sql;
        std::string create_changelog_sql;
        std::string drop_changelog_sql;
        std::string latest_version_sql;
        std::string previous_version_sql;
        std::string rollback_steps_sql;
        std::string contiguous_history_sql;
        std::string insert_sql;
    };

    const db_specific &get_db_specific(const std::string &backend);

} // namespace dbmig

#endif // DBMIG_CHANGELOG_SQL_INCLUDED

