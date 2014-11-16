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

#include <db_specific.hpp>
#include <sstream>
#include <unordered_map>
#include <stdexcept>

using namespace std;

namespace dbmig {

const db_specific &get_db_specific(const std::string &backend)
{
    // TODO - consider embedding the SQL in the executable and edit .sql files?
    // Automake:  http://stackoverflow.com/questions/6450830/custom-command-to-generate-object-o-from-binary-file-using-autoconf
    // Assembler: http://stackoverflow.com/questions/4864866/c-c-with-gcc-statically-add-resource-files-to-executable-library

    typedef unordered_map<string, const db_specific> sql_map;
    
    static const sql_map m {
        { "postgresql",
            {
                // default_changelog_table
                "dbmig_changelog",
                // changelog_exists_sql
                R"SQL(
SELECT COUNT(*) AS cnt
FROM information_schema.tables
WHERE table_schema = 'public'
AND table_name = 'dbmig_changelog'
)SQL",
                // create_changelog_sql
                R"SQL(
CREATE TABLE dbmig_changelog (
    changelog_id SERIAL,
    changeset VARCHAR(100) NOT NULL,
    applied TIMESTAMP WITH TIME ZONE NOT NULL,
    decommissioned TIMESTAMP WITH TIME ZONE NULL,
    script_path VARCHAR(255) NOT NULL,
    action VARCHAR(10) NOT NULL,
    from_version VARCHAR(255) NULL,
    to_version VARCHAR(255) NOT NULL,
    sha256_hash VARCHAR(64) NOT NULL,
    changed_by VARCHAR(255) NOT NULL,
    time_taken INTERVAL NOT NULL
);
CREATE FUNCTION dbmig_changelog_decom_func()
RETURNS TRIGGER AS
$BODY$
BEGIN
    UPDATE dbmig_changelog SET decommissioned = new.applied
    WHERE changelog_id < new.changelog_id
    AND changeset = new.changeset
    AND decommissioned IS NULL;
    RETURN NULL;
END;
$BODY$
LANGUAGE plpgsql;
CREATE TRIGGER dbmig_apply_decom
AFTER INSERT ON dbmig_changelog
FOR EACH ROW
EXECUTE PROCEDURE dbmig_changelog_decom_func();
)SQL",
                // drop_changelog_sql
                R"SQL(
BEGIN TRANSACTION;
DROP TRIGGER dbmig_apply_decom ON dbmig_changelog;
DROP FUNCTION dbmig_changelog_decom_func();
DROP TABLE dbmig_changelog;
COMMIT TRANSACTION;
)SQL",
                // latest_version_sql
                R"SQL(
SELECT to_version AS ver
FROM dbmig_changelog
WHERE changeset = :changeset
ORDER BY changelog_id DESC
LIMIT 1
)SQL",
                // previous_version_sql
                R"SQL(
SELECT from_version AS ver
FROM dbmig_changelog
WHERE changeset = :changeset
ORDER BY changelog_id DESC
LIMIT 1
)SQL",
                // rollback_steps_sql
                R"SQL(
WITH last_install AS (
    SELECT cl.changelog_id
    FROM dbmig_changelog cl
    WHERE cl.action IN ('install', 'override')
    AND cl.changeset = :changeset
    ORDER BY cl.changelog_id DESC
    LIMIT 1
), rollback_target AS (
    SELECT cl.changelog_id
    FROM dbmig_changelog cl
    INNER JOIN last_install cl_li ON (cl.changelog_id > cl_li.changelog_id)
    WHERE cl.action = 'upgrade'
    AND cl.changeset = :changeset
    AND cl.from_version = :rollback_ver
    LIMIT 1
)
SELECT cl.action, cl.from_version, cl.to_version, cl.sha256_hash
FROM dbmig_changelog cl
INNER JOIN last_install cl_li ON (cl.changelog_id > cl_li.changelog_id)
INNER JOIN rollback_target cl_rbt ON (cl.changelog_id >= cl_rbt.changelog_id)
WHERE cl.changeset = :changeset
ORDER BY cl.changelog_id DESC
)SQL",
                // contiguous_history_sql
                R"SQL(
WITH last_install AS (
    SELECT cl.changelog_id
    FROM dbmig_changelog cl
    WHERE cl.action IN ('install', 'override')
    AND cl.changeset = :changeset
    ORDER BY cl.changelog_id DESC
    LIMIT 1
)
SELECT cl.script_path, cl.action, cl.from_version, cl.to_version, cl.sha256_hash
FROM dbmig_changelog cl
INNER JOIN last_install cl_li ON (cl.changelog_id >= cl_li.changelog_id)
WHERE cl.changeset = :changeset
ORDER BY cl.changelog_id DESC
)SQL",
                // insert_sql
                R"SQL(
INSERT INTO dbmig_changelog (
    changeset, applied, script_path, action, from_version, to_version,
    sha256_hash, changed_by, time_taken)
VALUES (
    :changeset, :applied, :script_path, :action, :from_version, :to_version,
    :sha256_hash, current_user, :time_taken)
)SQL"
            }
        }
    };
    
    sql_map::const_iterator ci = m.find(backend);
    if (ci == m.end())
    {
        // The backend is not supported.
        throw invalid_argument("backend " + backend + " is not supported");
    }
    
    return ci->second;
}

} // namespace dbmig
