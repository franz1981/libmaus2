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
#if ! defined(LIBMAUS_BAMBAM_PARALLEL_GENERICINPUTCONTROLBLOCKCOMPRESSIONWORKPACKAGEDISPATCHER_HPP)
#define LIBMAUS_BAMBAM_PARALLEL_GENERICINPUTCONTROLBLOCKCOMPRESSIONWORKPACKAGEDISPATCHER_HPP

#include <libmaus/bambam/parallel/GenericInputControlPutCompressorInterface.hpp>
#include <libmaus/bambam/parallel/GenericInputControlGetCompressorInterface.hpp>
#include <libmaus/bambam/parallel/GenericInputControlBlockCompressionWorkPackageReturnInterface.hpp>
#include <libmaus/bambam/parallel/GenericInputControlBlockCompressionWorkPackage.hpp>
#include <libmaus/bambam/parallel/GenericInputControlBlockCompressionFinishedInterface.hpp>
#include <libmaus/parallel/SimpleThreadWorkPackageDispatcher.hpp>

namespace libmaus
{
	namespace bambam
	{
		namespace parallel
		{
			struct GenericInputControlBlockCompressionWorkPackageDispatcher : public libmaus::parallel::SimpleThreadWorkPackageDispatcher
			{
				typedef GenericInputControlBlockCompressionWorkPackageDispatcher this_type;
				typedef libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
				typedef libmaus::util::shared_ptr<this_type>::type shared_ptr_type;

				GenericInputControlBlockCompressionWorkPackageReturnInterface & packageReturnInterface;
				GenericInputControlBlockCompressionFinishedInterface & blockCompressedInterface;
				GenericInputControlGetCompressorInterface & getCompressorInterface;
				GenericInputControlPutCompressorInterface & putCompressorInterface;
			
				GenericInputControlBlockCompressionWorkPackageDispatcher(
					GenericInputControlBlockCompressionWorkPackageReturnInterface & rpackageReturnInterface,
					GenericInputControlBlockCompressionFinishedInterface & rblockCompressedInterface,
					libmaus::bambam::parallel::GenericInputControlGetCompressorInterface & rgetCompressorInterface,
					libmaus::bambam::parallel::GenericInputControlPutCompressorInterface & rputCompressorInterface
				)
				: packageReturnInterface(rpackageReturnInterface), blockCompressedInterface(rblockCompressedInterface),
				  getCompressorInterface(rgetCompressorInterface), putCompressorInterface(rputCompressorInterface)
				{
				}
			
				void dispatch(libmaus::parallel::SimpleThreadWorkPackage * P, libmaus::parallel::SimpleThreadPoolInterfaceEnqueTermInterface & /* tpi */)
				{
					assert ( dynamic_cast<GenericInputControlBlockCompressionWorkPackage *>(P) != 0 );
					GenericInputControlBlockCompressionWorkPackage * BP = dynamic_cast<GenericInputControlBlockCompressionWorkPackage *>(P);
			
					GenericInputControlCompressionPending & GICCP = BP->GICCP;
					libmaus::lz::BgzfDeflateZStreamBase::shared_ptr_type compressor = getCompressorInterface.genericInputControlGetCompressor();		
					libmaus::lz::BgzfDeflateOutputBufferBase & outblock = *(GICCP.outblock);
					std::pair<uint8_t *,uint8_t *> R = GICCP.P;

					libmaus::lz::BgzfDeflateZStreamBaseFlushInfo const info = compressor->flush(R.first,R.second,outblock);
					GICCP.flushinfo = info;
										
					putCompressorInterface.genericInputControlPutCompressor(compressor);
					blockCompressedInterface.genericInputControlBlockCompressionFinished(BP->GICCP);
					packageReturnInterface.genericInputControlBlockCompressionWorkPackageReturn(BP);
				}
			};
		}
	}
}
#endif
