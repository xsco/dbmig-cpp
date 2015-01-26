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

#include <nowide/iostream.hpp>
#include <string>
#include <changelog.hpp>

using nowide::cout;
using std::endl;

///
/// Show the currently-installed version of a given database
///
void show(
    const std::string &conn_str,
    const std::string &changeset,
    const bool verbose)
{
    dbmig::changelog cl{conn_str, changeset};

    if (verbose && !cl.installed()) {
        cout << "No changelog table currently exists" << endl;
    }
    
    auto v = cl.version();
    if (v.is_zero())
        cout << "Version installed: (not installed)" << endl;
    else
        cout << "Version installed: " << v << endl;
}

