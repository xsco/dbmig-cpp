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
#include "semantic_version.hpp"
#include "script_dir.hpp"
#include "hash.hpp"
#include "getline.hpp"
#include "statement_buffer.hpp"

namespace dbmig
{
    // TODO - what about different SQL dialects?
    const std::string script_partition_marker = "--//@UNDO";
    
    ///
    /// Class representing a range of lines from a script
    ///
    class script_statements
    {
    public:
        typedef std::vector<std::string> list_type;
        typedef list_type::const_iterator iterator;
        
        script_statements(const list_type &statements,
                          const std::string sha256_sum) :
            statements_(statements),
            sha256_sum_(sha256_sum)
        {}

        /// Iterator pointing to the first statement
        iterator begin() { return statements_.cbegin(); }
        /// Iterator pointing beyond the last statement
        iterator end()   { return statements_.cend();   }
        
        const std::string &sha256_sum() const
        {
            return sha256_sum_;
        }
        
    private:
        // Store all lines pre-read.  Not efficient for large files!
        const list_type statements_;
        const std::string sha256_sum_;
    };
    
    // TODO - split the "read_X_statements" functions into a separate header!
    
    ///
    /// Read statements from a stream in "install" mode
    ///
    template<typename InputStream>
    script_statements read_install_statements(InputStream &is)
    {
        // Read all lines.
        sha256_hash sum;
        std::string line, line_ending;
        script_statements::list_type statements;
        auto stmt_buf = make_statement_buffer(std::back_inserter(statements));
        
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
        // Encode to hex.
        std::string sum_hex;
        sum.hex_encode(sum_hex);
        return script_statements{statements, sum_hex};
    }

    ///
    /// Read statements from a stream in "upgrade" mode
    ///
    template<typename InputStream>
    script_statements read_upgrade_statements(InputStream &is)
    {
        // Read all lines, up until we find the magic text.
        sha256_hash sum;
        std::string line, line_ending;
        script_statements::list_type statements;
        auto stmt_buf = make_statement_buffer(std::back_inserter(statements));
        
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
        // Encode to hex.
        std::string sum_hex;
        sum.hex_encode(sum_hex);
        return script_statements{statements, sum_hex};
    }

    ///
    /// Read statements from a stream in "rollback" mode
    ///
    /// Note that the hash that is built up in rollback mode is actually
    /// the hash of the 'upgrade' portion of the script.  This is because when
    /// rolling back, it is beneficial to check that what the current script to
    /// be rolled back has not changed from when it was applied to the database,
    /// and the hash of the upgrade part is the way to make that check.
    ///
    template<typename InputStream>
    script_statements read_rollback_statements(InputStream &is)
    {
        // Read all lines, from the magic text until the end.
        sha256_hash sum;
        std::string line, line_ending;
        script_statements::list_type statements;
        auto stmt_buf = make_statement_buffer(std::back_inserter(statements));
        
        while (multiplatform_getline(is, line, line_ending))
        {
            // Add to hash.
            sum.update(line);
            sum.update(line_ending);
                
            if (line.find(script_partition_marker) != std::string::npos)
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
        // Encode to hex.
        std::string sum_hex;
        sum.hex_encode(sum_hex);
        return script_statements{statements, sum_hex};
    }
}

#endif // DBMIG_SCRIPT_STREAM_INCLUDED

