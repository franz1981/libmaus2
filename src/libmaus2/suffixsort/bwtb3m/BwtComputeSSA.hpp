/*
    libmaus2
    Copyright (C) 2016 German Tischler

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
#if ! defined(LIBMAUS2_SUFFIXSORT_BWTB3M_BWTCOMPUTESSA_HPP)
#define LIBMAUS2_SUFFIXSORT_BWTB3M_BWTCOMPUTESSA_HPP

#include <libmaus2/aio/InputStreamFactoryContainer.hpp>
#include <libmaus2/huffman/RLDecoder.hpp>
#include <libmaus2/rank/popcnt.hpp>
#include <libmaus2/sorting/SemiExternalKeyTupleSort.hpp>
#include <libmaus2/sorting/SortingBufferedOutputFile.hpp>
#include <libmaus2/util/iterator.hpp>
#include <libmaus2/util/NumberMapSerialisation.hpp>
#include <libmaus2/util/OutputFileNameTools.hpp>

namespace libmaus2
{
	namespace suffixsort
	{
		namespace bwtb3m
		{
			struct BwtComputeSSA
			{
				static void computeSSA(
					std::string bwt,
					uint64_t const sasamplingrate,
					uint64_t const isasamplingrate,
					std::string const tmpfilenamebase,
					bool const copyinputtomemory,
					uint64_t const numthreads,
					uint64_t const maxsortmem,
					uint64_t const maxtmpfiles,
					std::ostream * logstr,
					std::string const ref_isa_fn = std::string(),
					std::string const ref_sa_fn = std::string()
				);
			};
		}
	}
}
#endif
