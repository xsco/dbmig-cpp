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

#ifndef DBMIG_CLI_SERVICES_INCLUDED
#define DBMIG_CLI_SERVICES_INCLUDED


///
/// Show the currently-installed version of a given database
///
void show(
    const std::string &conn_str,
    const std::string &changeset,
    const bool verbose);

///
/// Check the compatibility of a repository with a given database
///
void check(
    const std::string &conn_str,
    const std::string &changeset,
    const bool verbose,
    const std::string &repository_path);

///
/// Forcibly override the version in a given database
///
/// This service should not be used lightly: it will invalidate any previous
/// change history in the target database, and will forcibly set the version
/// to the one specified, without making any further schema changes.
///
/// The behaviour offered by this service can be useful when bringing an
/// existing database installation under changelog control for the first time,
/// or as an absolute last resort to set a known version if the schema and
/// changelog somehow become out of sync.
///
void override_version(
    const std::string &conn_str,
    const std::string &changeset,
    const bool verbose,
    const bool force,
    const std::string &version_str);

///
/// Migrate a database to the latest version
///
void migrate(
    const std::string &conn_str,
    const std::string &changeset,
    const bool verbose,
    const bool force,
    const std::string &repository_path);

///
/// Migrate a database to a given version
///
void migrate(
    const std::string &conn_str,
    const std::string &changeset,
    const bool verbose,
    const bool force,
    const std::string &repository_path,
    const std::string &version_str);

#endif // DBMIG_CLI_SERVICES_INCLUDED

