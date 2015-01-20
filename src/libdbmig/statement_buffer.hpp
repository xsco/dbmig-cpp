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

#ifndef DBMIG_STATEMENT_BUFFER_INCLUDED
#define DBMIG_STATEMENT_BUFFER_INCLUDED

#include <string>
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace dbmig
{
    ///
    /// Struct that can be fed lines from a stream, and will parse them into the
    /// individual statements (according to statement and/or batch separators).
    ///
    template <typename OutputIterator>
    class statement_buffer
    {
    public:
        ///
        /// Instantiate a statement buffer using a delimiter regex that can
        /// match statement or batch delimiters in most SQL scripts.
        ///
        statement_buffer(OutputIterator oi) :
            oi_{oi},
            delim_regex_{"'.*(?:go|;).*'|\".*(?:go|;).*\"|(go|;)",
                         boost::regex::icase}
        {}
        statement_buffer(OutputIterator oi, std::string delim_regex_str) :
            oi_{oi},
            delim_regex_{delim_regex_str, boost::regex::icase}
        {}
        
        ///
        /// Append a line to the statement buffer.
        ///
        void append(const std::string &line)
        {
            buf_.append(line);
            
            // Can we parse any statements?
            boost::smatch results;
            while (boost::regex_search(buf_, results, delim_regex_) &&
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
        OutputIterator oi_;
        const boost::regex delim_regex_;
        std::string buf_;
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
}

#endif // DBMIG_STATEMENT_BUFFER_INCLUDED

