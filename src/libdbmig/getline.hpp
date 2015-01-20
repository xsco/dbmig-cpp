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

#ifndef DBMIG_GETLINE_INCLUDED
#define DBMIG_GETLINE_INCLUDED

#include <string>
#include <ios>
#include <streambuf>

namespace dbmig
{
    ///
    /// Variation of std::getline() that will cope with the different line
    /// endings that various platforms might throw at us.
    ///
    template <typename InputStream>
    InputStream &
    multiplatform_getline(InputStream &is,
                          std::string &line,
                          std::string &line_ending)
    {
        // Implementation adapted from http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf/6089413#6089413
        
        line.clear();

        // The characters in the stream are read one-by-one using a std::streambuf.
        // That is faster than reading them one-by-one using the std::istream.
        // Code that uses streambuf this way must be guarded by a sentry object.
        // The sentry object performs various tasks,
        // such as thread synchronization and updating the stream state.

        typename InputStream::sentry se(is, true);
        std::streambuf* sb = is.rdbuf();

        for(;;) {
            int c = sb->sbumpc();
            switch (c) {
            case '\n':
                line_ending = "\n";
                return is;
            case '\r':
                if(sb->sgetc() == '\n') {
                    line_ending = "\r\n";
                    sb->sbumpc();
                }
                else {
                    line_ending = "\r";
                }
                return is;
            case EOF:
                // Also handle the case when the last line has no line ending
                if (line.empty())
                    // Set failbit so that boolean evaluation of the stream is false
                    is.setstate(std::ios::eofbit | std::ios::failbit);
                line_ending = "";
                return is;
            default:
                line += (char)c;
            }
        }
    }
}

#endif // DBMIG_GETLINE_INCLUDED

