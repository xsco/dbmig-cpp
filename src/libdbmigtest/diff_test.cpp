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

#include "diff.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE repository_test
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <vector>
#include <utility>
#include "pair_special.hpp"

typedef std::vector<int> int_list;
typedef std::vector<short> short_list;
typedef std::vector<std::pair<int, short>> int_short_pair_list;

struct int_short_cmp
{
    bool operator() (int i, short s) { return i < (int)s; }
};
struct int_short_eq
{
    bool operator() (int i, short s) { return i == (int)s; }
};

BOOST_AUTO_TEST_CASE (equal_lists)
{
    int_list il   = { 1, 2, 3, 4, 5 };
    short_list sl = { 1, 2, 3, 4, 5 };
    int_list ao;
    short_list bo;
    int_short_pair_list co;
    int_short_cmp cmp;
    int_short_eq eq;
    
    dbmig::diff(il.begin(), il.end(), sl.begin(), sl.end(),
                [&](int &i) { ao.push_back(i); },
                [&](short &s) { bo.push_back(s); },
                [&](int &i, short &s) { co.push_back(std::make_pair(i, s)); },
                cmp, eq);
    BOOST_CHECK_EQUAL(ao.size(), 0);
    BOOST_CHECK_EQUAL(bo.size(), 0);
    BOOST_CHECK_EQUAL(co.size(), 5);
    BOOST_CHECK_EQUAL(co[0].first,  1);
    BOOST_CHECK_EQUAL(co[0].second, 1);
    BOOST_CHECK_EQUAL(co[1].first,  2);
    BOOST_CHECK_EQUAL(co[1].second, 2);
    BOOST_CHECK_EQUAL(co[2].first,  3);
    BOOST_CHECK_EQUAL(co[2].second, 3);
    BOOST_CHECK_EQUAL(co[3].first,  4);
    BOOST_CHECK_EQUAL(co[3].second, 4);
    BOOST_CHECK_EQUAL(co[4].first,  5);
    BOOST_CHECK_EQUAL(co[4].second, 5);
}

BOOST_AUTO_TEST_CASE (some_only_in_first)
{
    int_list il   = { 1, 2, 3, 4, 5 };
    short_list sl = {    2,    4    };
    int_list ao;
    short_list bo;
    int_short_pair_list co;
    int_short_cmp cmp;
    int_short_eq eq;
    
    dbmig::diff(il.begin(), il.end(), sl.begin(), sl.end(),
                [&](int &i) { ao.push_back(i); },
                [&](short &s) { bo.push_back(s); },
                [&](int &i, short &s) { co.push_back(std::make_pair(i, s)); },
                cmp, eq);
    BOOST_CHECK_EQUAL(ao.size(), 3);
    BOOST_CHECK_EQUAL(bo.size(), 0);
    BOOST_CHECK_EQUAL(co.size(), 2);
    BOOST_CHECK_EQUAL(ao[0],        1);
    BOOST_CHECK_EQUAL(ao[1],        3);
    BOOST_CHECK_EQUAL(ao[2],        5);
    BOOST_CHECK_EQUAL(co[0].first,  2);
    BOOST_CHECK_EQUAL(co[0].second, 2);
    BOOST_CHECK_EQUAL(co[1].first,  4);
    BOOST_CHECK_EQUAL(co[1].second, 4);
}

BOOST_AUTO_TEST_CASE (some_only_in_second)
{
    int_list il   = {    2,    4    };
    short_list sl = { 1, 2, 3, 4, 5 };
    int_list ao;
    short_list bo;
    int_short_pair_list co;
    int_short_cmp cmp;
    int_short_eq eq;
    
    dbmig::diff(il.begin(), il.end(), sl.begin(), sl.end(),
                [&](int &i) { ao.push_back(i); },
                [&](short &s) { bo.push_back(s); },
                [&](int &i, short &s) { co.push_back(std::make_pair(i, s)); },
                cmp, eq);
    BOOST_CHECK_EQUAL(ao.size(), 0);
    BOOST_CHECK_EQUAL(bo.size(), 3);
    BOOST_CHECK_EQUAL(co.size(), 2);
    BOOST_CHECK_EQUAL(bo[0],        1);
    BOOST_CHECK_EQUAL(bo[1],        3);
    BOOST_CHECK_EQUAL(bo[2],        5);
    BOOST_CHECK_EQUAL(co[0].first,  2);
    BOOST_CHECK_EQUAL(co[0].second, 2);
    BOOST_CHECK_EQUAL(co[1].first,  4);
    BOOST_CHECK_EQUAL(co[1].second, 4);
}

BOOST_AUTO_TEST_CASE (lists_mismatch)
{
    int_list il   = {    2, 3, 4    };
    short_list sl = { 1,    3,    5 };
    int_list ao;
    short_list bo;
    int_short_pair_list co;
    int_short_cmp cmp;
    int_short_eq eq;
    
    dbmig::diff(il.begin(), il.end(), sl.begin(), sl.end(),
                [&](int &i) { ao.push_back(i); },
                [&](short &s) { bo.push_back(s); },
                [&](int &i, short &s) { co.push_back(std::make_pair(i, s)); },
                cmp, eq);
    BOOST_CHECK_EQUAL(ao.size(), 2);
    BOOST_CHECK_EQUAL(bo.size(), 2);
    BOOST_CHECK_EQUAL(co.size(), 1);
    BOOST_CHECK_EQUAL(ao[0],        2);
    BOOST_CHECK_EQUAL(ao[1],        4);
    BOOST_CHECK_EQUAL(bo[0],        1);
    BOOST_CHECK_EQUAL(bo[1],        5);
    BOOST_CHECK_EQUAL(co[0].first,  3);
    BOOST_CHECK_EQUAL(co[0].second, 3);
}

