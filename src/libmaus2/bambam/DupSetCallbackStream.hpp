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
#if ! defined(LIBMAUS2_BAMBAM_DUPSETCALLBACKSTREAM_HPP)
#define LIBMAUS2_BAMBAM_DUPSETCALLBACKSTREAM_HPP

#include <libmaus2/bambam/DupSetCallback.hpp>
#include <libmaus2/bambam/DuplicationMetrics.hpp>
#include <libmaus2/aio/SynchronousGenericOutput.hpp>
#include <libmaus2/aio/SynchronousGenericInput.hpp>

namespace libmaus2
{
	namespace bambam
	{
		struct DupSetCallbackStream : public ::libmaus2::bambam::DupSetCallback
		{
			std::string const filename;
			libmaus2::aio::SynchronousGenericOutput<uint64_t>::unique_ptr_type SGO;
			std::map<uint64_t,::libmaus2::bambam::DuplicationMetrics> & metrics;
			uint64_t numdup;
			::libmaus2::bitio::BitVector::unique_ptr_type B;

			DupSetCallbackStream(
				std::string const & rfilename,
				std::map<uint64_t,::libmaus2::bambam::DuplicationMetrics> & rmetrics
			) : filename(rfilename), SGO(new libmaus2::aio::SynchronousGenericOutput<uint64_t>(filename,8*1024)), metrics(rmetrics), numdup(0) /* unpairedreadduplicates(), readpairduplicates(), metrics(rmetrics) */ {}

			void operator()(::libmaus2::bambam::ReadEnds const & A)
			{
				SGO->put(A.getRead1IndexInFile());
				numdup++;

				if ( A.isPaired() )
				{
					SGO->put(A.getRead2IndexInFile());
					numdup++;
					metrics[A.getLibraryId()].readpairduplicates++;
				}
				else
				{
					metrics[A.getLibraryId()].unpairedreadduplicates++;
				}
			}

			uint64_t getNumDups() const
			{
				return numdup;
			}
			void addOpticalDuplicates(uint64_t const libid, uint64_t const count)
			{
				metrics[libid].opticalduplicates += count;
			}

			bool isMarked(uint64_t const i) const
			{
				return (*B)[i];
			}

			void flush(uint64_t const n)
			{
				SGO->flush();
				SGO.reset();

				::libmaus2::bitio::BitVector::unique_ptr_type tB(new ::libmaus2::bitio::BitVector(n));
				B = UNIQUE_PTR_MOVE(tB);
				for ( uint64_t i = 0; i < n; ++i )
					B->set(i,false);

				libmaus2::aio::SynchronousGenericInput<uint64_t> SGI(filename,8*1024);
				uint64_t v;
				numdup = 0;
				while ( SGI.getNext(v) )
				{
					if ( ! B->get(v) )
						numdup++;
					B->set(v,true);
				}
			}
		};
	}
}
#endif
