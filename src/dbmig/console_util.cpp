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

#include <console_util.hpp>
#include <stdexcept>
#include <iostream>

using namespace std;

///
/// Block for yes/no confirmation from the console and return the result
///
bool console_confirmation(const char *prompt)
{
    using namespace std;
    while (true)
    {
        cout << prompt << " [yn] ";
        string line;
        if (!getline(cin, line)) {
            throw std::runtime_error("unexpected input error");
        }
        else if (line.size() == 1 and line.find_first_of("YyNn") != line.npos) {
            return line == "Y" || line == "y";
        }
        cout << "Please answer 'y' or 'n'." << endl;
    }
}

