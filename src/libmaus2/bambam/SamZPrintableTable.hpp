/*
    libmaus2
    Copyright (C) 2009-2013 German Tischler
    Copyright (C) 2011-2013 Genome Research Limited

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
#if ! defined(LIBMAUS2_UTIL_SAMZPRINTABLETABLE_HPP)
#define LIBMAUS2_UTIL_SAMZPRINTABLETABLE_HPP

#include <cstring>
#include <libmaus2/types/types.hpp>

namespace libmaus2
{
	namespace bambam
	{
		/**
		 * class containing a digit table; the contained array A
		 * is such that A[i] == true iff isprint(i) for
		 * 0 <= i < 256
		 **/
		struct SamZPrintableTable
		{
			//! digit table
			char A[256];

			/**
			 * constructor
			 */
			SamZPrintableTable();

			/**
			 * check whether i is a digit
			 *
			 * @param i character number such that 0 <= i < 256
			 * @return true iff i designates a digit
			 **/
			bool operator[](uint8_t const i) const
			{
				return A[i];
			}
		};
	}
}
#endif
