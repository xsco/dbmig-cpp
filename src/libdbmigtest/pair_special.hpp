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

#ifndef DBMIG_TEST_PAIR_SPECIAL_INCLUDED
#define DBMIG_TEST_PAIR_SPECIAL_INCLUDED

#include <utility>

// Politely inform boost how to log std::pairs
namespace boost {
namespace test_tools {

template<typename K, typename V>
struct print_log_value<std::pair<K, V> >
{
    void operator()( std::ostream& os,
        std::pair<K, V> const& p)
    {
        os << '(' << p.first << ',' << p.second << ')';
    }
};

} // test_tools
} // boost

#endif // DBMIG_TEST_PAIR_SPECIAL_INCLUDED

