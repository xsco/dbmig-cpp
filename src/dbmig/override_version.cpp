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

#include <iostream>
#include <sstream>
#include <string>
#include <changelog.hpp>

#include "console_util.hpp"

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
    const std::string &version_str)
{
    // Parse the version string to a real version.
    auto version = dbmig::semver::parse(version_str);
    
    dbmig::changelog cl{conn_str, changeset};
    if (verbose && !cl.installed()) {
        std::cout << "No changelog table currently exists - it will be created"
            << std::endl;
    }
    
    // Get the existing version.
    auto existing_version = cl.version();
    if (verbose) {
        if (existing_version.is_zero())
            std::cout << "No existing version installed" << std::endl;
        else
            std::cout << "Existing version is " << existing_version << std::endl;
    }
    
    if (!force) {
        // Prompt for confirmation
        std::stringstream ss;
        if (existing_version.is_zero())
            ss << "Forcibly override to " << version << "?";
        else
            ss << "Forcibly override from " << existing_version
                << " to " << version << "?";

        if (!console_confirmation(ss.str().c_str()))
            return;
    }
    
    cl.override_version(version);
    
    if (verbose) {
        if (existing_version.is_zero())
            std::cout << "Version overridden to " << version << std::endl;
        else
            std::cout << "Version overridden from " << existing_version
                      << " to " << version << std::endl;
    }
}

