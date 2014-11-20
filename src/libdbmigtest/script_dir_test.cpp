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

#include "script_dir.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE script_dir_test
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include "pair_special.hpp"

// Specialise some non-member operators for pair<semver, string>, so that we
// can pass pairs to the macros directly.
namespace std {
    bool operator ==(const pair<dbmig::semver, string> &v1,
                     const pair<dbmig::semver, string> &v2)
    {
        return v1.first == v2.first && v1.second == v2.second;
    }
} // namespace std

using namespace std;
using namespace dbmig;


BOOST_AUTO_TEST_CASE (scriptdir1_ctor_goodpath)
{
    // ctor on a good (existent) path
    script_dir sd1("data/scriptdir1");
}

BOOST_AUTO_TEST_CASE (scriptdir1_ctor_badpath)
{
    // Expect filesystem_error to be thrown if we try to construct a script_dir
    // on a bad or non-existent path
    BOOST_CHECK_THROW(script_dir{"data/nonexistent/path"},
        boost::filesystem::filesystem_error);
    
    // Expect filesystem_error to be thrown if we try to construct a script_dir
    // on a file path (as opposed to a directory)
    BOOST_CHECK_THROW(script_dir{"data/regularfile.txt"},
        boost::filesystem::filesystem_error);
}

BOOST_AUTO_TEST_CASE (scriptdir1_path)
{
    script_dir sd1("data/scriptdir1");
    BOOST_CHECK_EQUAL(sd1.path(), "data/scriptdir1");
}

BOOST_AUTO_TEST_CASE (scriptdir1_iter_begin_end)
{
    // Iterate over all contents of the script dir
    script_dir sd1("data/scriptdir1");
    
    auto iter = sd1.begin();
    auto end = sd1.end();
    
    auto exp1 = pair<semver, string>{
        semver{43, 209, 153, "", "script.3912"},
        "43.209.153/script.3912_wibble.sql"};
    auto exp2 = pair<semver, string>{
        semver{43, 209, 153, "", "script.3914"},
        "43.209.153/3914_wobble.sql"};
    auto exp3 = pair<semver, string>{
        semver{43, 209, 153, "", "script.17389"},
        "43.209.153+script.17389_foo.sql"};
    auto exp4 = pair<semver, string>{
        semver{43, 210, 0, "", "script.88"},
        "43.210.0/script.0088.sql"};
    auto exp5 = pair<semver, string>{
        semver{43, 210, 1, "", "script.972"},
        "43.210.1/43.210.1+script.972_quux.sql"};
    
    // Note that direct comparison between pairs is only possible because we
    // provided a specialisation of the relevant == and << operators (see top).
    BOOST_CHECK_EQUAL(*iter++, exp1);
    BOOST_CHECK_EQUAL(*iter++, exp2);
    BOOST_CHECK_EQUAL(*iter++, exp3);
    BOOST_CHECK_EQUAL(*iter++, exp4);
    BOOST_CHECK_EQUAL(*iter++, exp5);
    BOOST_CHECK(iter == end);
}

BOOST_AUTO_TEST_CASE (scriptdir1_iter_cbegin_cend)
{
    // Iterate over all contents of the script dir
    script_dir sd1("data/scriptdir1");
    
    auto iter = sd1.cbegin();
    auto end = sd1.cend();
    
    auto exp1 = pair<semver, string>{
        semver{43, 209, 153, "", "script.3912"},
        "43.209.153/script.3912_wibble.sql"};
    auto exp2 = pair<semver, string>{
        semver{43, 209, 153, "", "script.3914"},
        "43.209.153/3914_wobble.sql"};
    auto exp3 = pair<semver, string>{
        semver{43, 209, 153, "", "script.17389"},
        "43.209.153+script.17389_foo.sql"};
    auto exp4 = pair<semver, string>{
        semver{43, 210, 0, "", "script.88"},
        "43.210.0/script.0088.sql"};
    auto exp5 = pair<semver, string>{
        semver{43, 210, 1, "", "script.972"},
        "43.210.1/43.210.1+script.972_quux.sql"};
    
    // Note that direct comparison between pairs is only possible because we
    // provided a specialisation of the relevant == operator (see top).
    BOOST_CHECK_EQUAL(*iter++, exp1);
    BOOST_CHECK_EQUAL(*iter++, exp2);
    BOOST_CHECK_EQUAL(*iter++, exp3);
    BOOST_CHECK_EQUAL(*iter++, exp4);
    BOOST_CHECK_EQUAL(*iter++, exp5);
    BOOST_CHECK(iter == end);
}

// TODO - rbegin/rend
// TODO - crbegin/crend

BOOST_AUTO_TEST_CASE (scriptdir1_first_greater)
{
    script_dir sd1("data/scriptdir1");
    auto exp1 = pair<semver, string>{
        semver{43, 209, 153, "", "script.3912"},
        "43.209.153/script.3912_wibble.sql"};
    auto exp2 = pair<semver, string>{
        semver{43, 209, 153, "", "script.3914"},
        "43.209.153/3914_wobble.sql"};
    auto exp3 = pair<semver, string>{
        semver{43, 209, 153, "", "script.17389"},
        "43.209.153+script.17389_foo.sql"};
    auto exp4 = pair<semver, string>{
        semver{43, 210, 0, "", "script.88"},
        "43.210.0/script.0088.sql"};
    auto exp5 = pair<semver, string>{
        semver{43, 210, 1, "", "script.972"},
        "43.210.1/43.210.1+script.972_quux.sql"};
    auto end = sd1.end();
    
    // Variations that should point to the first element.
    auto lb11 = sd1.first_greater(semver{43, 209, 153});
    auto lb12 = sd1.first_greater(semver{43, 209, 153, "", "script.3911"});
    auto lb13 = sd1.first_greater(semver{1, 0, 0});
    BOOST_CHECK_EQUAL(*lb11, exp1);
    BOOST_CHECK_EQUAL(*lb12, exp1);
    BOOST_CHECK_EQUAL(*lb13, exp1);
    
    // Variations that should point to the second element.
    auto lb21 = sd1.first_greater(semver{43, 209, 153, "", "script.3912"});
    auto lb22 = sd1.first_greater(semver{43, 209, 153, "", "script.3913"});
    BOOST_CHECK_EQUAL(*lb21, exp2);
    BOOST_CHECK_EQUAL(*lb22, exp2);
    
    // Variations that should point to the third element.
    auto lb31 = sd1.first_greater(semver{43, 209, 153, "", "script.3914"});
    auto lb32 = sd1.first_greater(semver{43, 209, 153, "", "script.10000"});
    auto lb33 = sd1.first_greater(semver{43, 209, 153, "", "script.17388"});
    BOOST_CHECK_EQUAL(*lb31, exp3);
    BOOST_CHECK_EQUAL(*lb32, exp3);
    BOOST_CHECK_EQUAL(*lb33, exp3);
    
    // Variations that should point to the fourth element.
    auto lb41 = sd1.first_greater(semver{43, 209, 153, "", "script.17389"});
    auto lb42 = sd1.first_greater(semver{43, 209, 154});
    auto lb43 = sd1.first_greater(semver{43, 210, 0});
    BOOST_CHECK_EQUAL(*lb41, exp4);
    BOOST_CHECK_EQUAL(*lb42, exp4);
    BOOST_CHECK_EQUAL(*lb43, exp4);
    
    // Variations that should point to the fifth element.
    auto lb51 = sd1.first_greater(semver{43, 210, 1});
    auto lb52 = sd1.first_greater(semver{43, 210, 1, "", "script.1"});
    auto lb53 = sd1.first_greater(semver{43, 210, 1, "", "script.971"});
    BOOST_CHECK_EQUAL(*lb51, exp5);
    BOOST_CHECK_EQUAL(*lb52, exp5);
    BOOST_CHECK_EQUAL(*lb53, exp5);
    
    // Variations that should point beyond any elements.
    auto lb61 = sd1.first_greater(semver{43, 210, 1, "", "script.972"});
    auto lb62 = sd1.first_greater(semver{43, 210, 2});
    auto lb63 = sd1.first_greater(semver{43, 211, 0});
    auto lb64 = sd1.first_greater(semver{44, 0, 0});
    BOOST_CHECK(lb61 == end);
    BOOST_CHECK(lb62 == end);
    BOOST_CHECK(lb63 == end);
    BOOST_CHECK(lb64 == end);
}

BOOST_AUTO_TEST_CASE (scriptdir1_last_less_equal)
{
    script_dir sd1("data/scriptdir1");
    auto exp1 = pair<semver, string>{
        semver{43, 209, 153, "", "script.3912"},
        "43.209.153/script.3912_wibble.sql"};
    auto exp2 = pair<semver, string>{
        semver{43, 209, 153, "", "script.3914"},
        "43.209.153/3914_wobble.sql"};
    auto exp3 = pair<semver, string>{
        semver{43, 209, 153, "", "script.17389"},
        "43.209.153+script.17389_foo.sql"};
    auto exp4 = pair<semver, string>{
        semver{43, 210, 0, "", "script.88"},
        "43.210.0/script.0088.sql"};
    auto exp5 = pair<semver, string>{
        semver{43, 210, 1, "", "script.972"},
        "43.210.1/43.210.1+script.972_quux.sql"};
    auto end = sd1.end();
    
    // Variations that should point to the first element, i.e. no match.
    auto lb11 = sd1.last_less_equal(semver{1, 0, 0});
    BOOST_CHECK_EQUAL(*lb11, exp1);
    
    // Variations that should point beyond the first element.
    auto lb21 = sd1.last_less_equal(semver{43, 209, 153, "", "script.3912"});
    auto lb22 = sd1.last_less_equal(semver{43, 209, 153, "", "script.3913"});
    BOOST_CHECK_EQUAL(*lb21, exp2);
    BOOST_CHECK_EQUAL(*lb22, exp2);
    
    // Variations that should point beyond the second element.
    auto lb31 = sd1.last_less_equal(semver{43, 209, 153, "", "script.3914"});
    auto lb32 = sd1.last_less_equal(semver{43, 209, 153, "", "script.3915"});
    auto lb33 = sd1.last_less_equal(semver{43, 209, 153, "", "script.10000"});
    BOOST_CHECK_EQUAL(*lb31, exp3);
    BOOST_CHECK_EQUAL(*lb32, exp3);
    BOOST_CHECK_EQUAL(*lb33, exp3);
    
    // Variations that should point beyond the third element.
    auto lb41 = sd1.last_less_equal(semver{43, 209, 153, "", "script.17389"});
    auto lb42 = sd1.last_less_equal(semver{43, 209, 153, "", "script.17390"});
    auto lb43 = sd1.last_less_equal(semver{43, 209, 153});
    auto lb44 = sd1.last_less_equal(semver{43, 209, 154});
    BOOST_CHECK_EQUAL(*lb41, exp4);
    BOOST_CHECK_EQUAL(*lb42, exp4);
    BOOST_CHECK_EQUAL(*lb43, exp4);
    BOOST_CHECK_EQUAL(*lb44, exp4);
    
    // Variations that should point beyond the fourth element.
    auto lb51 = sd1.last_less_equal(semver{43, 210, 0});
    auto lb52 = sd1.last_less_equal(semver{43, 210, 1, "", "script.971"});
    BOOST_CHECK_EQUAL(*lb51, exp5);
    BOOST_CHECK_EQUAL(*lb52, exp5);
    
    // Variations that should point beyond the fifth element.
    auto lb61 = sd1.last_less_equal(semver{43, 210, 1, "", "script.972"});
    auto lb62 = sd1.last_less_equal(semver{43, 210, 1, "", "script.973"});
    auto lb63 = sd1.last_less_equal(semver{43, 210, 1});
    auto lb64 = sd1.last_less_equal(semver{43, 210, 2});
    auto lb65 = sd1.last_less_equal(semver{43, 211, 0});
    auto lb66 = sd1.last_less_equal(semver{44, 0, 0});
    BOOST_CHECK(lb61 == end);
    BOOST_CHECK(lb62 == end);
    BOOST_CHECK(lb63 == end);
    BOOST_CHECK(lb64 == end);
    BOOST_CHECK(lb65 == end);
    BOOST_CHECK(lb66 == end);
}

BOOST_AUTO_TEST_CASE (scriptdir1_range)
{
    script_dir sd1("data/scriptdir1");
    auto exp1 = pair<semver, string>{
        semver{43, 209, 153, "", "script.3912"},
        "43.209.153/script.3912_wibble.sql"};
    auto exp2 = pair<semver, string>{
        semver{43, 209, 153, "", "script.3914"},
        "43.209.153/3914_wobble.sql"};
    auto exp3 = pair<semver, string>{
        semver{43, 209, 153, "", "script.17389"},
        "43.209.153+script.17389_foo.sql"};
    auto exp4 = pair<semver, string>{
        semver{43, 210, 0, "", "script.88"},
        "43.210.0/script.0088.sql"};
    auto exp5 = pair<semver, string>{
        semver{43, 210, 1, "", "script.972"},
        "43.210.1/43.210.1+script.972_quux.sql"};
    
    // Should return no scripts.
    auto p11 = sd1.range(semver{1,0,0}, semver{1,0,0});
    BOOST_CHECK(p11.first == p11.second);
    auto p12 = sd1.range(semver{2,0,0}, semver{3,0,0});
    BOOST_CHECK(p12.first == p12.second);
    auto p13 = sd1.range(semver{43,209,153,"","script.3912"},
                         semver{43,209,153,"","script.3912"});
    BOOST_CHECK(p13.first == p13.second);
    
    // Should return exactly one script.
    auto p21 = sd1.range(semver{43,209,153},
                         semver{43,209,153,"","script.3912"});
    BOOST_CHECK_EQUAL(*p21.first++, exp1);
    BOOST_CHECK(p21.first == p21.second);
    auto p22 = sd1.range(semver{43,209,153,"","script.3911"},
                         semver{43,209,153,"","script.3912"});
    BOOST_CHECK_EQUAL(*p22.first++, exp1);
    BOOST_CHECK(p22.first == p22.second);
    
    // Should return some scripts of a X.Y.Z version
    auto p31 = sd1.range(semver{43,209,153,"","script.3912"},
                         semver{43,209,153,"","script.17389"});
    BOOST_CHECK_EQUAL(*p31.first++, exp2);
    BOOST_CHECK_EQUAL(*p31.first++, exp3);
    BOOST_CHECK(p31.first == p31.second);
    
    // Should return all scripts of a single X.Y.Z version.
    auto p41 = sd1.range(semver{43,209,153}, semver{43,209,153});
    BOOST_CHECK_EQUAL(*p41.first++, exp1);
    BOOST_CHECK_EQUAL(*p41.first++, exp2);
    BOOST_CHECK_EQUAL(*p41.first++, exp3);
    BOOST_CHECK(p41.first == p41.second);
    auto p42 = sd1.range(semver{43,209,152}, semver{43,209,153});
    BOOST_CHECK_EQUAL(*p42.first++, exp1);
    BOOST_CHECK_EQUAL(*p42.first++, exp2);
    BOOST_CHECK_EQUAL(*p42.first++, exp3);
    BOOST_CHECK(p42.first == p42.second);
    
    // Should return over a wider range.
    auto p51 = sd1.range(semver{43,209,153}, semver{43,210,1});
    BOOST_CHECK_EQUAL(*p51.first++, exp1);
    BOOST_CHECK_EQUAL(*p51.first++, exp2);
    BOOST_CHECK_EQUAL(*p51.first++, exp3);
    BOOST_CHECK_EQUAL(*p51.first++, exp4);
    BOOST_CHECK_EQUAL(*p51.first++, exp5);
    BOOST_CHECK(p51.first == p51.second);
}

