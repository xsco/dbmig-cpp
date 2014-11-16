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

#include <iostream>
#include <sstream>
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "hash.hpp"

using namespace std;

namespace dbmig {

// Regex that can match statement or batch delimiters in most SQL scripts.
static const boost::regex stmt_delim_exp{
        "'.*(?:go|;).*'|\".*(?:go|;).*\"|(go|;)",
        boost::regex::icase};

///
/// Struct that can be fed lines from a script file, and will parse them into
/// the individual statements (according to statement and/or batch separators).
///
template <typename OutputIterator>
class statement_buffer
{
public:
    statement_buffer(OutputIterator oi) : oi_(oi)
    {}
    
    ///
    /// Append a line to the statement buffer.
    ///
    void append(const std::string &line)
    {
        buf_.append(line);
        
        // Can we parse any statements?
        boost::smatch results;
        while (boost::regex_search(buf_, results, stmt_delim_exp) &&
            !results.empty()) {
            // Found a statement (or batch) terminator.
            auto sm = results[0];
            auto pos_beg = buf_.cbegin();
            int pos_curr_end = std::distance(pos_beg, sm.first);
            int pos_next_beg = std::distance(pos_beg, sm.second);
            // Extract the delimited statement, trim, and store.
            auto stmt = buf_.substr(0, pos_curr_end);
            boost::trim(stmt);
            // Erase the statement from the buffer.
            buf_.erase(0, pos_next_beg);
            
            // Write to the OutputIterator and increment.
            *oi_++ = stmt;
        }
    }
    
    ///
    /// Finalise anything left in the statement buffer.
    ///
    void finalise()
    {
        // See if anything is left in the statement buffer.
        boost::trim(buf_);
        if (!buf_.empty()) {
            // Add the last statement.
            *oi_++ = buf_;
            buf_.clear();
        }
    }
    
private:
    std::string buf_;
    OutputIterator oi_;
};

///
/// Make a statement_buffer (object generator pattern).
///
template<typename OutputIterator>
static statement_buffer<OutputIterator>
make_statement_buffer(OutputIterator oi)
{
    return statement_buffer<OutputIterator>{oi};
}


///
/// Variation of std::getline() that will cope with the different line
/// endings that various platforms might throw at us.
///
static std::istream &
multiplatform_getline(istream &is, string &t, string &line_ending)
{
    // Implementation adapted from http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf/6089413#6089413
    
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    istream::sentry se(is, true);
    streambuf* sb = is.rdbuf();

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
            if (t.empty())
                // Set failbit so that boolean evaluation of the stream is false
                is.setstate(ios::eofbit | ios::failbit);
            line_ending = "";
            return is;
        default:
            t += (char)c;
        }
    }
}


///
/// Specialisation of constructor for scripts in "install" mode
///
template <>
script_statements<script_action::install>::script_statements(istream &is)
{
    // Read all lines.
    sha256_hash sum;
    string line, line_ending;
    auto stmt_buf = make_statement_buffer(std::back_inserter(statements_));
    
    while (multiplatform_getline(is, line, line_ending))
    {
        stmt_buf.append(line + "\n"); // Ok to sanitise line endings
    
        // Add lines (and ending) to hash.
        sum.update(line);
        sum.update(line_ending);
    }
    
    // Finalise anything left in the buffer.
    stmt_buf.finalise();
    
    // Calculate digest.
    sum.finalise();
    // Encode to hex and store.
    sum.hex_encode(sha256_sum_);
}

///
/// Specialisation of constructor for scripts in "upgrade" mode
///
template <>
script_statements<script_action::upgrade>::script_statements(istream &is)
{
    // Read all lines, up until we find the magic text.
    sha256_hash sum;
    string line, line_ending;
    auto stmt_buf = make_statement_buffer(std::back_inserter(statements_));
    
    // Record upgrade lines.
    while (multiplatform_getline(is, line, line_ending))
    {
        // Add to hash.
        sum.update(line);
        sum.update(line_ending);
        
        if (line.find(script_partition_marker) != std::string::npos)
            break;
        
        // Record the line.
        stmt_buf.append(line + "\n");
    }
    // The remaining lines form part of the hash, but the statements there
    // aren't parsed.
    while (multiplatform_getline(is, line, line_ending))
    {
        // Add to hash.
        sum.update(line);
        sum.update(line_ending);
    }
    
    stmt_buf.finalise();
    
    // Calculate digest.
    sum.finalise();
    // Encode to hex and store.
    sum.hex_encode(sha256_sum_);
}

///
/// Specialisation of constructor for scripts in "rollback" mode
///
/// Note that the hash that is built up in rollback mode is actually
/// the hash of the 'upgrade' portion of the script.  This is because when
/// rolling back, it is beneficial to check that what the current script to
/// be rolled back has not changed from when it was applied to the database,
/// and the hash of the upgrade part is the way to make that check.
///
template <>
script_statements<script_action::rollback>::script_statements(istream &is)
{
    // Read all lines, from the magic text until the end.
    sha256_hash sum;
    string line, line_ending;
    auto stmt_buf = make_statement_buffer(std::back_inserter(statements_));
    
    while (multiplatform_getline(is, line, line_ending))
    {
        // Add to hash.
        sum.update(line);
        sum.update(line_ending);
            
        if (line.find(script_partition_marker) != string::npos)
            break;
    }
    while (multiplatform_getline(is, line, line_ending))
    {
        // Add to hash.
        sum.update(line);
        sum.update(line_ending);
        
        // Record the line.
        stmt_buf.append(line + "\n");
    }
    
    stmt_buf.finalise();
    
    // Calculate digest.
    sum.finalise();
    // Encode to hex and store.
    sum.hex_encode(sha256_sum_);
}

} // dbmig namespace

