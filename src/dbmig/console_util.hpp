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

#ifndef DBMIG_CONSOLE_UTIL_INCLUDED
#define DBMIG_CONSOLE_UTIL_INCLUDED

#include <string>
#include <exception>

///
/// Utility exception class when the user chooses to halt execution
///
struct user_driven_cancel : public std::exception
{
    user_driven_cancel() {}
    virtual const char *what() const noexcept {
        return "user cancelled the operation";
    }
};

///
/// Block for yes/no confirmation from the console and return the result
///
bool console_confirmation(const char *prompt);

#endif // DBMIG_CONSOLE_UTIL_INCLUDED

