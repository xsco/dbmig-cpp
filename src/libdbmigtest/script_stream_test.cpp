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

#include "script_stream.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE script_stream_test
#include <boost/test/unit_test.hpp>
#include <fstream>
#include "pair_special.hpp"


using namespace dbmig;


BOOST_AUTO_TEST_CASE (repo4_install_nonempty)
{
    // Get ifstream on an example script path.
    auto path = "data/repo4/install/2.44.2/2.44.2+script.0057_install.sql";
    std::ifstream ifs{path};

    // Check lines
    auto statements = install_statements(ifs);
    auto b = statements.begin();
    auto e = statements.end();
    BOOST_CHECK_EQUAL(*b++, "SELECT 'foo'");
    BOOST_CHECK(b != e);
    BOOST_CHECK_EQUAL(*b++, "SELECT 'bar'");
    BOOST_CHECK(b == e);
    
    // Check hash
    BOOST_CHECK_EQUAL(
        statements.sha256_sum(),
        "f9851f80e0e17c734469b86ea96045f01cb82a68c44966ea9fb7f9999b4cb125");
    
    ifs.close();
}

BOOST_AUTO_TEST_CASE (repo4_upgrade_nonempty)
{
    // Get ifstream on the script path.
    auto path = "data/repo4/upgrade/2.44.3/0001_foo.sql";
    std::ifstream ifs{path};
    
    // Check lines
    auto statements = upgrade_statements(ifs);
    auto b = statements.begin();
    auto e = statements.end();
    BOOST_CHECK_EQUAL(*b++, "SELECT 'foo'");
    BOOST_CHECK(b != e);
    BOOST_CHECK_EQUAL(*b++, "SELECT 'bar'");
    BOOST_CHECK(b == e);
    
    // Check hash
    BOOST_CHECK_EQUAL(
        statements.sha256_sum(),
        "3e1a6103040506d0e526880e739546567eb8282de831e8ae666a5fb2baf8849a");
    
    ifs.close();
}

BOOST_AUTO_TEST_CASE (repo4_upgrade_empty)
{
    // Get ifstream on the script path.
    auto path = "data/repo4/upgrade/2.44.3/0002_bar.sql";
    std::ifstream ifs{path};
    
    // Check lines
    auto statements = upgrade_statements(ifs);
    auto b = statements.begin();
    auto e = statements.end();
    BOOST_CHECK(b == e);
    
    // Check hash
    BOOST_CHECK_EQUAL(
        statements.sha256_sum(),
        "3396754a69a86991d06dbe921bcd84e0316c39a843294c1667a324b7b22f3a70");
    
    ifs.close();
}

BOOST_AUTO_TEST_CASE (repo4_rollback_nonempty)
{
    // Get ifstream on the script path.
    auto path = "data/repo4/upgrade/2.44.3/0001_foo.sql";
    std::ifstream ifs{path};
    
    // Check lines
    auto statements = rollback_statements(ifs);
    auto b = statements.begin();
    auto e = statements.end();
    BOOST_CHECK_EQUAL(*b++, "SELECT 'baz'");
    BOOST_CHECK(b != e);
    BOOST_CHECK_EQUAL(*b++, "SELECT 'quux'");
    BOOST_CHECK(b == e);
    
    // Check hash
    BOOST_CHECK_EQUAL(
        statements.sha256_sum(),
        "3e1a6103040506d0e526880e739546567eb8282de831e8ae666a5fb2baf8849a");
    
    ifs.close();
}

BOOST_AUTO_TEST_CASE (repo4_rollback_empty)
{
    // Get ifstream on the script path.
    auto path = "data/repo4/upgrade/2.44.3/0002_bar.sql";
    std::ifstream ifs{path};
    
    // Check lines
    auto statements = rollback_statements(ifs);
    auto b = statements.begin();
    auto e = statements.end();
    BOOST_CHECK(b == e);
    
    // Check hash
    BOOST_CHECK_EQUAL(
        statements.sha256_sum(),
        "3396754a69a86991d06dbe921bcd84e0316c39a843294c1667a324b7b22f3a70");
    
    ifs.close();
}

// TODO - add test cases with files that use different EOL encodings

