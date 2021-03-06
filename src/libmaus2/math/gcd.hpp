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

#if ! defined(GCD_HPP)
#define GCD_HPP

#include <libmaus2/types/types.hpp>

namespace libmaus2
{
	namespace math
	{
		inline uint64_t gcd(uint64_t a, uint64_t b)
		{
			if ( b == 0 )
				return a;
			else
				return gcd(b,a%b);
		}
		inline uint64_t lcm(uint64_t a, uint64_t b)
		{
			return (a*b)/gcd(a,b);
		}
	}
}
#endif
