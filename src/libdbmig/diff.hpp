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

#ifndef DBMIG_DIFF_INCLUDED
#define DBMIG_DIFF_INCLUDED

namespace dbmig
{
    ///
    /// Perform a diff against two sorted ranges.
    ///
    /// Appropriate functors are called back depending on whether there are
    /// items in the first range but not the second, in the second range but
    /// not the first, and vice versa.
    ///
    template<
        typename InputIterator1,
        typename InputIterator2,
        typename FirstNotInSecond,
        typename SecondNotInFirst,
        typename InFirstAndSecond,
        typename Compare,
        typename Equality>
    void diff(InputIterator1 b1, InputIterator1 e1,
              InputIterator2 b2, InputIterator2 e2,
              FirstNotInSecond func_a,
              SecondNotInFirst func_b,
              InFirstAndSecond func_c,
              Compare comp,
              Equality eq)
    {
        while (b1 != e1 || b2 != e2) {
            if (b1 == e1) {
                // Reached the end of the first list, but not the second.
                func_b(*b2);
                ++b2;
            }
            else if (b2 == e2) {
                // Reached the end of the second list, but not the first.
                func_a(*b1);
                ++b1;
            }
            else if (comp(*b1, *b2)) {
                // Something in the first range but not in the second.
                func_a(*b1);
                ++b1;
            }
            else if (!eq(*b1, *b2)) {
                // Something in the second range but not in the first.
                func_b(*b2);
                ++b2;
            }
            else {
                // Something in both lists.
                func_c(*b1, *b2);
                ++b1;
                ++b2;
            }
        }
    }
}

#endif // DBMIG_DIFF_INCLUDED

