/*
    libmaus
    Copyright (C) 2009-2015 German Tischler
    Copyright (C) 2011-2015 Genome Research Limited

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
#if ! defined(LIBMAUS_BAMBAM_PARALLEL_CRAMENCODINGSUPPORTDATA_HPP)
#define LIBMAUS_BAMBAM_PARALLEL_CRAMENCODINGSUPPORTDATA_HPP

#include <libmaus/parallel/PosixSpinLock.hpp>
#include <libmaus/parallel/LockedGrowingFreeList.hpp>
#include <libmaus/bambam/parallel/CramPassPointerObject.hpp>
#include <libmaus/bambam/parallel/CramPassPointerObjectAllocator.hpp>
#include <libmaus/bambam/parallel/CramPassPointerObjectTypeInfo.hpp>
#include <libmaus/bambam/parallel/CramOutputBlock.hpp>
#include <libmaus/bambam/parallel/CramOutputBlockIdComparator.hpp>
#include <set>

namespace libmaus
{
	namespace bambam
	{
		namespace parallel
		{
			struct CramEncodingSupportData
			{
				uint64_t volatile cramtokens;
				libmaus::parallel::PosixSpinLock cramtokenslock;

				libmaus::parallel::LockedGrowingFreeList<
					libmaus::bambam::parallel::CramPassPointerObject,
					libmaus::bambam::parallel::CramPassPointerObjectAllocator,
					libmaus::bambam::parallel::CramPassPointerObjectTypeInfo>
					passPointerFreeList;

				std::map<uint64_t,CramPassPointerObject::shared_ptr_type> passPointerActive;
				libmaus::parallel::PosixSpinLock passPointerActiveLock;
				
				void * context;

				std::multimap<size_t,CramOutputBlock::shared_ptr_type> outputBlockFreeList;
				libmaus::parallel::PosixSpinLock outputBlockFreeListLock;
				
				std::set<CramOutputBlock::shared_ptr_type,CramOutputBlockIdComparator> outputBlockPendingList;
				libmaus::parallel::PosixSpinLock outputBlockPendingListLock;

				uint64_t volatile outputBlockUnfinished;
				libmaus::parallel::PosixSpinLock outputBlockUnfinishedLock;

				std::pair<int64_t volatile,uint64_t volatile> outputWriteNext;
				libmaus::parallel::PosixSpinLock outputWriteNextLock;
												
				bool getCramEncodingToken()
				{
					bool ok = false;
					
					cramtokenslock.lock();
					if ( cramtokens )
					{
						ok = true;
						cramtokens -= 1;
					}
					cramtokenslock.unlock();
					
					return ok;
				}
				
				void putCramEncodingToken()
				{
					cramtokenslock.lock();
					cramtokens += 1;
					cramtokenslock.unlock();
				}

				CramEncodingSupportData(size_t const numtokens)
				: 
				  cramtokens(numtokens),
				  context(0),
				  outputWriteNext(-1,0)
				{
				
				}
			};
		}
	}
}
#endif
