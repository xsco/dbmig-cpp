// Copyright (c) 2014, Adam Szmigin (adam@xsco.net)
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "semantic_version.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE semantic_version_test
#include <boost/test/unit_test.hpp>
#include <sstream>

using namespace std;
using namespace dbmig;

BOOST_AUTO_TEST_CASE (ctor_tests)
{
	const semver sv1(3);
	const semver sv2(4, 87, 9);
	const semver sv3(2, 5, 1, "rc.7", "5cf938fj3");
	
	BOOST_CHECK_EQUAL(sv1.mj(), 3);
	BOOST_CHECK_EQUAL(sv1.mn(), 0);
	BOOST_CHECK_EQUAL(sv1.pt(), 0);
	BOOST_CHECK_EQUAL(sv1.prerelease_str(), "");
	BOOST_CHECK_EQUAL(sv1.build_metadata_str(), "");
	
	BOOST_CHECK_EQUAL(sv2.mj(), 4);
	BOOST_CHECK_EQUAL(sv2.mn(), 87);
	BOOST_CHECK_EQUAL(sv2.pt(), 9);
	BOOST_CHECK_EQUAL(sv2.prerelease_str(), "");
	BOOST_CHECK_EQUAL(sv2.build_metadata_str(), "");

	BOOST_CHECK_EQUAL(sv3.mj(), 2);
	BOOST_CHECK_EQUAL(sv3.mn(), 5);
	BOOST_CHECK_EQUAL(sv3.pt(), 1);
	BOOST_CHECK_EQUAL(sv3.prerelease_str(), "rc.7");
	BOOST_CHECK_EQUAL(sv3.build_metadata_str(), "5cf938fj3");
}

BOOST_AUTO_TEST_CASE (parse_tests)
{
	const semver sv1 = semver::parse("1.2.3");
	const semver sv2 = semver::parse("4.5.6-rc123");
	const semver sv3 = semver::parse("7.8.9+wibble578374");
	const semver sv4 = semver::parse("6.0.2-alpha.1+build3927.whatever");
	const semver sv5 = semver::parse(u8"12.34.56-foo.bar.5+baz.quux.3");

	BOOST_CHECK_EQUAL(sv1.mj(), 1);
	BOOST_CHECK_EQUAL(sv1.mn(), 2);
	BOOST_CHECK_EQUAL(sv1.pt(), 3);
	BOOST_CHECK_EQUAL(sv1.prerelease_str(), "");
	BOOST_CHECK_EQUAL(sv1.build_metadata_str(), "");
	
	BOOST_CHECK_EQUAL(sv2.mj(), 4);
	BOOST_CHECK_EQUAL(sv2.mn(), 5);
	BOOST_CHECK_EQUAL(sv2.pt(), 6);
	BOOST_CHECK_EQUAL(sv2.prerelease_str(), "rc123");
	BOOST_CHECK_EQUAL(sv2.build_metadata_str(), "");

	BOOST_CHECK_EQUAL(sv3.mj(), 7);
	BOOST_CHECK_EQUAL(sv3.mn(), 8);
	BOOST_CHECK_EQUAL(sv3.pt(), 9);
	BOOST_CHECK_EQUAL(sv3.prerelease_str(), "");
	BOOST_CHECK_EQUAL(sv3.build_metadata_str(), "wibble578374");
	
	BOOST_CHECK_EQUAL(sv4.mj(), 6);
	BOOST_CHECK_EQUAL(sv4.mn(), 0);
	BOOST_CHECK_EQUAL(sv4.pt(), 2);
	BOOST_CHECK_EQUAL(sv4.prerelease_str(), "alpha.1");
	BOOST_CHECK_EQUAL(sv4.build_metadata_str(), "build3927.whatever");
	
	BOOST_CHECK_EQUAL(sv5.mj(), 12);
	BOOST_CHECK_EQUAL(sv5.mn(), 34);
	BOOST_CHECK_EQUAL(sv5.pt(), 56);
	BOOST_CHECK_EQUAL(sv5.prerelease_str(), "foo.bar.5");
	BOOST_CHECK_EQUAL(sv5.build_metadata_str(), "baz.quux.3");
	
	// Bad parses.
	BOOST_CHECK_THROW(semver::parse(""), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("."), std::domain_error);
	BOOST_CHECK_THROW(semver::parse(".."), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("0"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("1"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("0."), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("1."), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("0.0"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("0.1"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("0.0."), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("1.1."), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("1.2.3.4"), std::domain_error);
	
	BOOST_CHECK_THROW(semver::parse("00.0.0"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("0.00.0"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("0.0.00"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("01.1.1"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("1.01.1"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("1.1.01"), std::domain_error);
	
	BOOST_CHECK_THROW(semver::parse("111.222.333-"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("111.222.333-00"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("111.222.333-000"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("111.222.333-."), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("111.222.333-1."), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("111.222.333+"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("111.222.333+."), std::domain_error);
	BOOST_CHECK_THROW(semver::parse("111.222.333+1."), std::domain_error);
	
	// Only ASCII alphanumerics and hyphen are permitted in pre-release and
	// build meta-data.
	BOOST_CHECK_THROW(semver::parse(
	    "1.2.3-foo?"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse(
	    "1.2.3+foo?"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse(
	    u8"99.88.77-utf8.\u010F+more.utf8.foo"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse(
	    u8"99.88.77-utf8+more.utf8.\u0126.foo"), std::domain_error);
	BOOST_CHECK_THROW(semver::parse(
	    u8"99.88.77-utf8.\u010F+more.utf8.\u0126.foo"), std::domain_error);
}

BOOST_AUTO_TEST_CASE (to_str_tests)
{
	const semver sv1(3);
	const semver sv2(4, 87, 9);
	const semver sv3(7, 312, 3, "", "wibble872523749");
	const semver sv4(2, 5, 1, "rc.7", "5cf938fj3");
	
	// Check the to_str() method.
	BOOST_CHECK_EQUAL(sv1.to_str(), "3.0.0");
	BOOST_CHECK_EQUAL(sv2.to_str(), "4.87.9");
	BOOST_CHECK_EQUAL(sv3.to_str(), "7.312.3+wibble872523749");
	BOOST_CHECK_EQUAL(sv4.to_str(), "2.5.1-rc.7+5cf938fj3");
	
	// Check the << operator.
	stringstream ss;
	ss.str("");
	ss << sv1;
	BOOST_CHECK_EQUAL(ss.str(), "3.0.0");
	ss.str("");
	ss << sv2;
	BOOST_CHECK_EQUAL(ss.str(), "4.87.9");
	ss.str("");
	ss << sv3;
	BOOST_CHECK_EQUAL(ss.str(), "7.312.3+wibble872523749");
	ss.str("");
	ss << sv4;
	BOOST_CHECK_EQUAL(ss.str(), "2.5.1-rc.7+5cf938fj3");
}

BOOST_AUTO_TEST_CASE (self_inspection_tests)
{
	// test any self-inspection methods, e.g. is_initial_development().
	const semver sv1 = semver::parse("0.0.0");
	const semver sv2 = semver::parse("0.7.0");
	const semver sv3 = semver::parse("0.7.8");
	const semver sv4 = semver::parse("0.0.8");
	const semver sv5 = semver::parse("1.0.0");
	const semver sv6 = semver::parse("1.3.5-rc.234");
	const semver sv7 = semver::parse("0.4.0-rc.888");
	
	BOOST_CHECK(sv1.is_initial_development());
	BOOST_CHECK(sv2.is_initial_development());
	BOOST_CHECK(sv3.is_initial_development());
	BOOST_CHECK(sv4.is_initial_development());
	BOOST_CHECK(!sv5.is_initial_development());
	BOOST_CHECK(!sv6.is_initial_development());
	BOOST_CHECK(sv7.is_initial_development());
}

BOOST_AUTO_TEST_CASE (increment_tests)
{
	semver sv = semver::parse("9.9.9-rc.5+build.4827");
	sv.next_patch();
	BOOST_CHECK_EQUAL(sv.to_str(), "9.9.10");
	sv.next_minor();
	BOOST_CHECK_EQUAL(sv.to_str(), "9.10.0");
	sv.next_patch();
	BOOST_CHECK_EQUAL(sv.to_str(), "9.10.1");
	sv.next_major();
	BOOST_CHECK_EQUAL(sv.to_str(), "10.0.0");	
}

BOOST_AUTO_TEST_CASE (strict_precedence_tests)
{
	// test strict_compare
	const semver sv1 = semver::parse("1.0.0-alpha");
	const semver sv2 = semver::parse("1.0.0-alpha.1");
	const semver sv3 = semver::parse("1.0.0-alpha.beta");
	const semver sv4 = semver::parse("1.0.0-beta");
	const semver sv5 = semver::parse("1.0.0-beta.2");
	const semver sv6 = semver::parse("1.0.0-beta.11");
	const semver sv7 = semver::parse("1.0.0-rc.1");
	const semver sv8 = semver::parse("1.0.0");
	semver::strict_compare cmp;

	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv1), 0);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv2), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv3), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv4), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv5), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv6), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv7), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv8), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv1), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv2), 0);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv3), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv4), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv5), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv6), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv7), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv8), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv1), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv2), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv3), 0);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv4), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv5), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv6), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv7), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv8), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv1), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv2), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv3), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv4), 0);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv5), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv6), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv7), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv8), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv1), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv2), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv3), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv4), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv5), 0);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv6), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv7), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv8), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv1), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv2), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv3), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv4), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv5), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv6), 0);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv7), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv8), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv1), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv2), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv3), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv4), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv5), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv6), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv7), 0);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv8), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv1), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv2), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv3), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv4), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv5), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv6), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv7), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv8), 0);
}

BOOST_AUTO_TEST_CASE (metadata_precedence_tests)
{
    // test metadata_compare
	const semver sv1 = semver::parse("1.0.0-beta.5+fnoozle");
	const semver sv2 = semver::parse("1.0.0-beta.5+fnoozle.1");
	const semver sv3 = semver::parse("1.0.0-beta.5");
	const semver sv4 = semver::parse("1.0.0+fnoozle");
	const semver sv5 = semver::parse("1.0.0+fnoozle.2");
	const semver sv6 = semver::parse("1.0.0+fnoozle.11");
	const semver sv7 = semver::parse("1.0.0+moistly.1");
	const semver sv8 = semver::parse("1.0.0");
	semver::metadata_compare cmp;
	
	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv1), 0);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv2), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv3), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv4), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv5), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv6), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv7), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv1, sv8), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv1), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv2), 0);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv3), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv4), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv5), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv6), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv7), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv2, sv8), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv1), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv2), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv3), 0);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv4), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv5), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv6), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv7), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv3, sv8), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv1), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv2), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv3), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv4), 0);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv5), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv6), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv7), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv4, sv8), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv1), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv2), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv3), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv4), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv5), 0);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv6), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv7), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv5, sv8), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv1), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv2), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv3), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv4), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv5), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv6), 0);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv7), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv6, sv8), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv1), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv2), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv3), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv4), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv5), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv6), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv7), 0);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv7, sv8), -1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv1), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv2), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv3), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv4), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv5), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv6), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv7), 1);
	BOOST_CHECK_EQUAL(cmp.compare_to(sv8, sv8), 0);
}

BOOST_AUTO_TEST_CASE (comparison_operator_tests)
{
	const semver sv1 = semver::parse("1.0.0");
	const semver sv2 = semver::parse("1.1.0");
	const semver sv3 = semver::parse("1.1.1");
	const semver sv4 = semver::parse("1.1.2-foo.1");
	const semver sv5 = semver::parse("1.1.2-foo.bar+something");
	const semver sv6 = semver::parse("1.1.2-foo.bar");
	const semver sv7 = semver::parse("1.1.2");
	
	const semver sv8 = semver::parse("1.0.0");
	const semver sv9 = semver::parse("1.1.2-foo.1");
	const semver sv10 = semver::parse("1.1.2-foo.bar+something");

    BOOST_CHECK(sv1 == sv8);
    BOOST_CHECK(sv4 == sv9);
    BOOST_CHECK(sv5 == sv10);
    
    BOOST_CHECK(sv1 == sv1);
    BOOST_CHECK(sv2 == sv2);
    BOOST_CHECK(sv3 == sv3);
    BOOST_CHECK(sv4 == sv4);
    BOOST_CHECK(sv5 == sv5);
    BOOST_CHECK(sv6 == sv6);
    BOOST_CHECK(sv7 == sv7);
    
	BOOST_CHECK(sv1 < sv2);
	BOOST_CHECK(sv2 < sv3);
	BOOST_CHECK(sv3 < sv4);
	BOOST_CHECK(sv4 < sv5);
	BOOST_CHECK(sv5 < sv6);
	BOOST_CHECK(sv6 < sv7);
	
	BOOST_CHECK(sv1 <= sv2);
	BOOST_CHECK(sv2 <= sv3);
	BOOST_CHECK(sv3 <= sv4);
	BOOST_CHECK(sv4 <= sv5);
	BOOST_CHECK(sv5 <= sv6);
	BOOST_CHECK(sv6 <= sv7);
	
	BOOST_CHECK(sv2 > sv1);
	BOOST_CHECK(sv3 > sv2);
	BOOST_CHECK(sv4 > sv3);
	BOOST_CHECK(sv5 > sv4);
	BOOST_CHECK(sv6 > sv5);
	BOOST_CHECK(sv7 > sv6);
	
	BOOST_CHECK(sv2 >= sv1);
	BOOST_CHECK(sv3 >= sv2);
	BOOST_CHECK(sv4 >= sv3);
	BOOST_CHECK(sv5 >= sv4);
	BOOST_CHECK(sv6 >= sv5);
	BOOST_CHECK(sv7 >= sv6);
}
