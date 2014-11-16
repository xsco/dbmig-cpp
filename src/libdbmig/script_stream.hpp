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

#ifndef DBMIG_SCRIPT_STREAM_INCLUDED
#define DBMIG_SCRIPT_STREAM_INCLUDED

#include <string>
#include <istream>
#include <vector>
#include "script_action.hpp"
#include "semantic_version.hpp"
#include "script_dir.hpp"

namespace dbmig
{
    // TODO - what about different SQL dialects?
    const std::string script_partition_marker = "--//@UNDO";
    
    ///
    /// Class representing a range of lines from a script
    ///
    template <script_action T>
    class script_statements
    {
        // Pre-read all lines into a vector.  Not efficient, but works for now.
        // TODO - consider making this lazy-load?
        typedef std::vector<std::string> statements_list;
    public:
        typedef statements_list::const_iterator iterator;
        
        script_statements(std::istream &is);
        
        iterator begin() { return statements_.cbegin(); }
        iterator end()   { return statements_.cend();   }
        
        const std::string &sha256_sum() const
        {
            return sha256_sum_;
        }
        
    private:
        statements_list statements_;
        std::string sha256_sum_;
    };
    
    typedef script_statements<script_action::install> install_statements;
    typedef script_statements<script_action::upgrade> upgrade_statements;
    typedef script_statements<script_action::rollback> rollback_statements;
}

#endif // DBMIG_SCRIPT_STREAM_INCLUDED

