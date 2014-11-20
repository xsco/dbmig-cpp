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

#include "repository.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE repository_test
#include <boost/test/unit_test.hpp>
#include <sstream>
#include "pair_special.hpp"

#include "exception.hpp"


using namespace std;
using namespace dbmig;


BOOST_AUTO_TEST_CASE (repo4_ctor)
{
    repository r4("data/repo4");
}

BOOST_AUTO_TEST_CASE (repo4_path_defaulting_tests)
{
    repository r4("data/repo4");
    BOOST_CHECK_EQUAL(r4.latest_schema_path(),  "data/repo4/latest");
    BOOST_CHECK_EQUAL(r4.upgrade_script_path(), "data/repo4/upgrade");
    BOOST_CHECK_EQUAL(r4.install_script_path(), "data/repo4/install");
}

BOOST_AUTO_TEST_CASE (repo2_path_explicit_tests)
{
    repository r2(
        "data/repo2/foo/bar/fnoozle",
        "data/repo2/foo/bar/wibble",
        "data/repo2/scruff/sniff/floozle");
    BOOST_CHECK_EQUAL(r2.latest_schema_path(),  "data/repo2/foo/bar/fnoozle");
    BOOST_CHECK_EQUAL(r2.upgrade_script_path(), "data/repo2/foo/bar/wibble");
    BOOST_CHECK_EQUAL(r2.install_script_path(), "data/repo2/scruff/sniff/floozle");
}

BOOST_AUTO_TEST_CASE (repo4_latest_version)
{
    repository r4("data/repo4");
    BOOST_CHECK_EQUAL(r4.latest_version(), semver::parse("2.45.1+script.1"));
}

BOOST_AUTO_TEST_CASE (repo2_latest_version)
{    
    repository r2(
        "data/repo2/foo/bar/fnoozle",
        "data/repo2/foo/bar/wibble",
        "data/repo2/scruff/sniff/floozle");
    semver v0 = semver::zero();
    BOOST_CHECK_EQUAL(r2.latest_version(), v0);
}

BOOST_AUTO_TEST_CASE (repo4_nearest_install_script)
{
    repository r4("data/repo4");
    pair<const semver, string> exp1{
        semver{2, 44, 2, "", "script.57"},
        "2.44.2/2.44.2+script.0057_install.sql"};
    
    auto range1 = r4.nearest_install_script(semver{1, 0, 0});
    auto iter1 = range1.first;
    BOOST_CHECK(iter1 == range1.second);
    
    auto range2 = r4.nearest_install_script(semver{2, 44, 2});
    auto iter2 = range2.first;
    auto iter2_exp1 = *iter2++;
    BOOST_CHECK_EQUAL(iter2_exp1, exp1);
    BOOST_CHECK(iter2 == range2.second);

    auto range3 = r4.nearest_install_script(semver{3, 0, 0});
    auto iter3 = range3.first;
    auto iter3_exp1 = *iter3++;
    BOOST_CHECK_EQUAL(iter3_exp1, exp1);
    BOOST_CHECK(iter3 == range3.second);
    
    auto range4 = r4.nearest_install_script(
            semver{2, 44, 2, "", "script.57"});
    auto iter4 = range4.first;
    auto iter4_exp1 = *iter4++;
    BOOST_CHECK_EQUAL(iter4_exp1, exp1);
    BOOST_CHECK(iter4 == range4.second);
}

BOOST_AUTO_TEST_CASE (repo2_nearest_install_script)
{
    repository r2(
        "data/repo2/foo/bar/fnoozle",
        "data/repo2/foo/bar/wibble",
        "data/repo2/scruff/sniff/floozle");

    auto range = r2.nearest_install_script(semver{1, 0, 0});
    
    BOOST_CHECK(range.first == range.second);
}

BOOST_AUTO_TEST_CASE (repo4_upgrade_scripts)
{
    repository r4("data/repo4");
    pair<const semver, string> exp1{
        semver{2, 44, 3, "", "script.1"},
        "2.44.3/0001_foo.sql"};
    pair<const semver, string> exp2{
        semver{2, 44, 3, "", "script.2"},
        "2.44.3/0002_bar.sql"};
    pair<const semver, string> exp3{
        semver{2, 45, 0, "", "script.1"},
        "2.45.0/0001_quux.sql"};
    
    auto range1 = r4.upgrade_scripts(semver{2,44,3},
                                     semver{2,45,0,"","script.1"});
    auto iter1 = range1.first;
    auto iter1_exp1 = *iter1++;
    auto iter1_exp2 = *iter1++;
    auto iter1_exp3 = *iter1++;
    
    BOOST_CHECK_EQUAL(iter1_exp1, exp1);
    BOOST_CHECK_EQUAL(iter1_exp2, exp2);
    BOOST_CHECK_EQUAL(iter1_exp3, exp3);
    BOOST_CHECK(iter1 == range1.second);
    
    auto range2 = r4.upgrade_scripts(semver{2,44,3,"","script.1"},
                                     semver{2,45,0});
    auto iter2 = range2.first;
    auto iter2_exp1 = *iter2++;
    auto iter2_exp2 = *iter2++;
    
    BOOST_CHECK_EQUAL(iter2_exp1, exp2);
    BOOST_CHECK_EQUAL(iter2_exp2, exp3);
    BOOST_CHECK(iter2 == range2.second);
}

BOOST_AUTO_TEST_CASE (repo2_upgrade_scripts)
{
    repository r2(
        "data/repo2/foo/bar/fnoozle",
        "data/repo2/foo/bar/wibble",
        "data/repo2/scruff/sniff/floozle");
    
    auto range = r2.upgrade_scripts(semver{0,0,0}, semver{1,0,0});
    
    BOOST_CHECK(range.first == range.second);
}

BOOST_AUTO_TEST_CASE (repo4_upgrade_script_at)
{
    repository r4("data/repo4");
    pair<const semver, string> exp1{
        semver{2, 44, 3, "", "script.1"},
        "2.44.3/0001_foo.sql"};
    pair<const semver, string> exp2{
        semver{2, 44, 3, "", "script.2"},
        "2.44.3/0002_bar.sql"};
    pair<const semver, string> exp3{
        semver{2, 45, 0, "", "script.1"},
        "2.45.0/0001_quux.sql"};
    
    auto range1 = r4.upgrade_script_at(semver{2,44,3,"","script.1"});
    auto iter1 = range1.first;
    auto iter1_exp = *iter1++;
    BOOST_CHECK_EQUAL(iter1_exp, exp1);
    BOOST_CHECK(iter1 == range1.second);
    
    auto range2 = r4.upgrade_script_at(semver{2,44,3,"","script.2"});
    auto iter2 = range2.first;
    auto iter2_exp = *iter2++;
    BOOST_CHECK_EQUAL(iter2_exp, exp2);
    BOOST_CHECK(iter2 == range2.second);
    
    auto range3 = r4.upgrade_script_at(semver{2,45,0,"","script.1"});
    auto iter3 = range3.first;
    auto iter3_exp = *iter3++;
    BOOST_CHECK_EQUAL(iter3_exp, exp3);
    BOOST_CHECK(iter3 == range3.second);
    
    auto range4 = r4.upgrade_script_at(semver{2,44,3});
    auto range5 = r4.upgrade_script_at(semver{2,44,3,"","script.3"});
    auto range6 = r4.upgrade_script_at(semver{2,45,0,"","script.7"});
    
    BOOST_CHECK(range4.first == range4.second);
    BOOST_CHECK(range5.first == range5.second);
    BOOST_CHECK(range6.first == range6.second);
}

BOOST_AUTO_TEST_CASE (repo3_upgrade_scripts_contiguous)
{
    // The below scripts in repository 3 should all be contiguous, and not
    // present any problems when asking for upgrade scripts across their range.
    repository repo3{"data/repo3"};
    pair<const semver, string> e1{
        semver{5, 0, 0, "", "script.5"}, "5.0.0+script.5.sql"};
    pair<const semver, string> e2{
        semver{5, 0, 0, "", "script.6"}, "5.0.0+script.6.sql"};
    pair<const semver, string> e3{
        semver{5, 0, 0, "", "script.100"}, "5.0.0+script.100.sql"};
    pair<const semver, string> e4{
        semver{5, 0, 1, "", "script.1"}, "5.0.1+script.1.sql"};
    pair<const semver, string> e5{
        semver{5, 1, 0, "", "script.1"}, "5.1.0+script.1.sql"};
    pair<const semver, string> e6{
        semver{6, 0, 0, "", "script.1"}, "6.0.0+script.1.sql"};
    
    // Single-number increment in script number is good.
    auto range1 = repo3.upgrade_scripts(
        semver{5,0,0}, semver{5,0,0,"","script.6"});
    auto iter1 = range1.first;
    auto iter1_exp1 = *iter1++;
    auto iter1_exp2 = *iter1++;
    BOOST_CHECK_EQUAL(iter1_exp1, e1);
    BOOST_CHECK_EQUAL(iter1_exp2, e2);
    BOOST_CHECK(iter1 == range1.second);
    
    // Multiple-number increment in script number is good.
    auto range2 = repo3.upgrade_scripts(
        semver{5,0,0,"","script.5"}, semver{5,0,0,"","script.100"});
    auto iter2 = range2.first;
    auto iter2_exp1 = *iter2++;
    auto iter2_exp2 = *iter2++;
    BOOST_CHECK_EQUAL(iter2_exp1, e2);
    BOOST_CHECK_EQUAL(iter2_exp2, e3);
    BOOST_CHECK(iter2 == range2.second);
    
    // Increment in patch number is good.
    auto range3 = repo3.upgrade_scripts(
        semver{5,0,0,"","script.6"}, semver{5,0,1});
    auto iter3 = range3.first;
    auto iter3_exp1 = *iter3++;
    auto iter3_exp2 = *iter3++;
    BOOST_CHECK_EQUAL(iter3_exp1, e3);
    BOOST_CHECK_EQUAL(iter3_exp2, e4);
    BOOST_CHECK(iter3 == range3.second);
    
    // Increment in minor number is good.
    auto range4 = repo3.upgrade_scripts(
        semver{5,0,1}, semver{5,1,0});
    auto iter4 = range4.first;
    auto iter4_exp1 = *iter4++;
    auto iter4_exp2 = *iter4++;
    BOOST_CHECK_EQUAL(iter4_exp1, e4);
    BOOST_CHECK_EQUAL(iter4_exp2, e5);
    BOOST_CHECK(iter4 == range4.second);
    
    // Increment in major number is good.
    auto range5 = repo3.upgrade_scripts(
        semver{5,1,0}, semver{6,0,0});
    auto iter5 = range5.first;
    auto iter5_exp1 = *iter5++;
    auto iter5_exp2 = *iter5++;
    BOOST_CHECK_EQUAL(iter5_exp1, e5);
    BOOST_CHECK_EQUAL(iter5_exp2, e6);
    BOOST_CHECK(iter5 == range5.second);
}

BOOST_AUTO_TEST_CASE (repo3_upgrade_scripts_noncontiguous)
{
    // Repository 3 contains some scripts which are not contiguous, and hence
    // should throw up errors if one were to try and collect upgrade scripts
    // over their range.
    repository repo3{"data/repo3"};

    BOOST_CHECK_THROW(repo3.upgrade_scripts(semver{13,0,0}, semver{13,0,2}),
        dbmig::script_noncontiguous);
    BOOST_CHECK_THROW(repo3.upgrade_scripts(semver{23,0,0}, semver{23,2,0}),
        dbmig::script_noncontiguous);
    BOOST_CHECK_THROW(repo3.upgrade_scripts(semver{33,0,0}, semver{35,0,0}),
        dbmig::script_noncontiguous);
}

