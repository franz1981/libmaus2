/*
    libmaus
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
#if ! defined(LIBMAUS_BAMBAM_READENDS_HPP)
#define LIBMAUS_BAMBAM_READENDS_HPP

#include <libmaus/types/types.hpp>
#include <libmaus/util/utf8.hpp>
#include <libmaus/util/NumberSerialisation.hpp>
#include <libmaus/bambam/BamAlignment.hpp>
#include <libmaus/util/DigitTable.hpp>
#include <map>
#include <cstring>
#include <sstream>

namespace libmaus
{
	namespace bambam
	{
		struct ReadEnds;
	}
}

namespace libmaus
{
	namespace bambam
	{
		std::ostream & operator<<(std::ostream & out, libmaus::bambam::ReadEnds const & RE);
	}
}

namespace libmaus
{
	namespace bambam
	{
		struct OpticalComparator;

		/**
		 * base class for ReadEnds
		 **/
		struct ReadEndsBase
		{
			//! this type
			typedef ReadEndsBase this_type;
			//! digit table
			static ::libmaus::util::DigitTable const D;
			
			//! friend output iterator
			friend std::ostream & ::libmaus::bambam::operator<<(std::ostream & out, ::libmaus::bambam::ReadEnds const & RE);
			//! friend comparator
			friend struct OpticalComparator;
			
			//! orientation enum
			enum read_end_orientation { F=0, R=1, FF=2, FR=3, RF=4, RR=5 };
			
			private:
			//! library id
			uint16_t libraryId;
			//! tag id
			uint64_t tagId;
			//! sequence id for end 1
			uint32_t read1Sequence;
			//! coordinate for end 1
			uint32_t read1Coordinate;
			//! orientation
			read_end_orientation orientation;
			//! sequence id for end 2
			uint32_t read2Sequence;
			//! coordinate for end 2
			uint32_t read2Coordinate;

			//! line number of end 1
			uint64_t read1IndexInFile;
			//! line number of end 2
			uint64_t read2IndexInFile;

			//! score
			uint32_t score;
			//! read group id
			uint16_t readGroup;
			//! optical tile number
			uint16_t tile;
			//! optical x coordinate
			uint32_t x;
			//! optical y coordinate
			uint32_t y;
			
			//! shift to make signed numbers non-negative
			static int64_t const signshift = (-(1l<<29))+1;

			public:
			/**
			 * shift n to make it non-negative
			 *
			 * @param n number
			 * @return n - signshift
			 **/
			static uint32_t signedEncode(int32_t const n)
			{
				return static_cast<uint32_t>(static_cast<int64_t>(n) - signshift);
			}
			
			/**
			 * shift n generated by signedEncode back to its original value
			 *
			 * @param n number
			 * @return n + signshift
			 **/
			static int32_t signedDecode(uint32_t const n)
			{
				return static_cast<int32_t>(static_cast<int64_t>(n) + signshift);
			}

			/**
			 * @return true if object is paired (i.e. read2Sequence != 0)
			 **/
			bool isPaired() const
			{
				return read2Sequence != 0;
			}
			
			/**
			 * @return coordinates for read 1
			 **/
			std::pair<int64_t,int64_t> getCoord1() const
			{
				return std::pair<int64_t,int64_t>(read1Sequence,getRead1Coordinate());
			}
			/**
			 * @return coordinates for read 2
			 **/
			std::pair<int64_t,int64_t> getCoord2() const
			{
				return std::pair<int64_t,int64_t>(read2Sequence,getRead2Coordinate());
			}
			
			/**
			 * @return library id
			 **/
			uint16_t getLibraryId() const { return libraryId; }
			/**
			 * @return tag id
			 **/
			uint64_t getTagId() const { return tagId; }
			/**
			 * @return orientation
			 **/
			read_end_orientation getOrientation() const { return orientation; }
			/**
			 * @return sequence id for read 1
			 **/
			uint32_t getRead1Sequence() const { return read1Sequence; }
			/**
			 * @return coordinate for read 1
			 **/
			int32_t getRead1Coordinate() const { return signedDecode(read1Coordinate); }
			/**
			 * @return sequence id for read 2
			 **/
			uint32_t getRead2Sequence() const { return read2Sequence; }
			/**
			 * @return coordinate for read 2
			 **/
			int32_t getRead2Coordinate() const { return signedDecode(read2Coordinate); }
			/**
			 * @return original line number of read 1
			 **/
			uint64_t getRead1IndexInFile() const { return read1IndexInFile; }
			/**
			 * @return original line number of read 2
			 **/
			uint64_t getRead2IndexInFile() const { return read2IndexInFile; }
			/**
			 * @return score
			 **/
			uint32_t getScore() const { return score; }
			/**
			 * @return read group
			 **/
			uint16_t getReadGroup() const { return readGroup; }
			/**
			 * @return tile
			 **/
			uint16_t getTile() const { return tile; }
			/**
			 * @return x coordinate
			 **/
			uint32_t getX() const { return x; }
			/**
			 * @return y coordinate
			 **/
			uint32_t getY() const { return y; }
			
			/**
			 * constructor for invalid/empty object
			 **/
			ReadEndsBase()
			{
				reset();
			}
			
			/**
			 * reset/invalidate object
			 **/
			void reset()
			{
				memset(this,0,sizeof(*this));	
			}
			
			/**
			 * comparator smaller
			 *
			 * @param o other ReadEndsBase object
			 * @result *this < o lexicographically along the attributes as ordered in memory (see above)
			 **/
			bool operator<(ReadEndsBase const & o) const 
			{
				if ( libraryId != o.libraryId ) return libraryId < o.libraryId;
				if ( tagId != o.tagId ) return tagId < o.tagId;
				if ( read1Sequence != o.read1Sequence ) return read1Sequence < o.read1Sequence;
				if ( read1Coordinate != o.read1Coordinate ) return read1Coordinate < o.read1Coordinate;
				if ( orientation != o.orientation ) return orientation < o.orientation;
				if ( read2Sequence != o.read2Sequence ) return read2Sequence < o.read2Sequence;
				if ( read2Coordinate != o.read2Coordinate ) return read2Coordinate < o.read2Coordinate;
				if ( read1IndexInFile != o.read1IndexInFile ) return read1IndexInFile < o.read1IndexInFile;
				if ( read2IndexInFile != o.read2IndexInFile ) return read2IndexInFile < o.read2IndexInFile;

				return false;
			}

			/**
			 * comparator greater
			 *
			 * @param o other ReadEndsBase object
			 * @result *this > o lexicographically along the attributes as ordered in memory (see above)
			 **/
			bool operator>(ReadEndsBase const & o) const 
			{
				if ( libraryId != o.libraryId ) return libraryId > o.libraryId;
				if ( tagId != o.tagId ) return tagId > o.tagId;
				if ( read1Sequence != o.read1Sequence ) return read1Sequence > o.read1Sequence;
				if ( read1Coordinate != o.read1Coordinate ) return read1Coordinate > o.read1Coordinate;
				if ( orientation != o.orientation ) return orientation > o.orientation;
				if ( read2Sequence != o.read2Sequence ) return read2Sequence > o.read2Sequence;
				if ( read2Coordinate != o.read2Coordinate ) return read2Coordinate > o.read2Coordinate;
				if ( read1IndexInFile != o.read1IndexInFile ) return read1IndexInFile > o.read1IndexInFile;
				if ( read2IndexInFile != o.read2IndexInFile ) return read2IndexInFile > o.read2IndexInFile;

				return false;
			}
			
			/**
			 * comparator equals
			 *
			 * @param o other ReadEndsBase object
			 * @result *this == o lexicographically along the attributes as ordered in memory (see above) up to the line numbers
			 **/
			bool operator==(ReadEndsBase const & o) const
			{
				if ( libraryId != o.libraryId ) return false;
				if ( tagId != o.tagId ) return false;
				if ( read1Sequence != o.read1Sequence ) return false;
				if ( read1Coordinate != o.read1Coordinate ) return false;
				if ( orientation != o.orientation ) return false;
				if ( read2Sequence != o.read2Sequence ) return false;
				if ( read2Coordinate != o.read2Coordinate ) return false;
				if ( read1IndexInFile != o.read1IndexInFile ) return false;
				if ( read2IndexInFile != o.read2IndexInFile ) return false;
				
				return true;
			}

			/**
			 * comparator not equals
			 *
			 * @param o other ReadEndsBase object
			 * @result *this != o lexicographically along the attributes as ordered in memory (see above) up to the line numbers
			 **/
			bool operator!=(ReadEndsBase const & o) const
			{
				return ! (*this == o);
			}

			/**
			 * parse optical information (if any) into tile, x and y
			 **/
			static bool parseOptical(uint8_t const * readname, uint16_t & tile, uint32_t & x, uint32_t & y)
			{
				size_t const l = strlen(reinterpret_cast<char const *>(readname));
				
				if ( parseReadNameValid(readname,readname+l) )
				{
					parseReadNameTile(readname,readname+l,tile,x,y);
					return tile != 0;
				}
				else
				{
					return false;
				}
			}

			private:
			/**
			 * determine if readname contains optical parameters
			 *
			 * @param readname name start pointer
			 * @param readnamee name end pointer
			 * @return true iff name has optical fields (number of ":" in name is >= 2 and <= 4)
			 **/
			static bool parseReadNameValid(uint8_t const * readname, uint8_t const * readnamee)
			{
				int cnt[2] = { 0,0 };

				for ( uint8_t const * c = readname; c != readnamee; ++c )
					cnt [ (static_cast<int>(*c) - ':') == 0 ] ++;

				bool const rnparseok = (cnt[1] <= 4) && (cnt[1] >= 2);
				
				return rnparseok;
			}

			/**
			 * parse optical parameters from read name
			 *
			 * assumes tile, x and y are separated by the last 2 ":" in the read name
			 *
			 * @param readname name start pointer
			 * @param readnamee name end pointer
			 * @param RE ReadEndsBase object to be filled
			 **/
			static void parseReadNameTile(uint8_t const * readname, uint8_t const * readnamee, ::libmaus::bambam::ReadEndsBase & RE)
			{
				uint8_t const * sem[4];
				sem[2] = readname;
				uint8_t const ** psem = &sem[0];
				uint8_t const * c = readnamee;
				for ( --c; c >= readname; --c )
					if ( *c == ':' )
						*(psem++) = c+1;
				
				uint8_t const * t = sem[2];
				RE.tile = 0;
				while ( D[*t] )
				{
					RE.tile *= 10;
					RE.tile += *(t++)-'0';
				}
				RE.tile += 1;

				t = sem[1];
				RE.x = 0;
				while ( D[*t] )
				{
					RE.x *= 10;
					RE.x += *(t++)-'0';
				}

				t = sem[0];
				RE.y = 0;
				while ( D[*t] )
				{
					RE.y *= 10;
					RE.y += *(t++)-'0';
				}			
			}

			/**
			 * parse optical parameters from read name
			 *
			 * assumes tile, x and y are separated by the last 2 ":" in the read name
			 *
			 * @param readname name start pointer
			 * @param readnamee name end pointer
			 * @param RE ReadEndsBase object to be filled
			 **/
			static void parseReadNameTile(
				uint8_t const * readname, 
				uint8_t const * readnamee, 
				uint16_t & tile,
				uint32_t & x,
				uint32_t & y
			)
			{
				uint8_t const * sem[4];
				sem[2] = readname;
				uint8_t const ** psem = &sem[0];
				uint8_t const * c = readnamee;
				for ( --c; c >= readname; --c )
					if ( *c == ':' )
						*(psem++) = c+1;
				
				uint8_t const * t = sem[2];
				while ( D[*t] )
				{
					tile *= 10;
					tile += *(t++)-'0';
				}
				tile += 1;

				t = sem[1];
				while ( D[*t] )
				{
					x *= 10;
					x += *(t++)-'0';
				}

				t = sem[0];
				while ( D[*t] )
				{
					y *= 10;
					y += *(t++)-'0';
				}			
			}
			
			/**
			 * fill common parts between fragment and pair ReadEndsBase objects
			 *
			 * @param p alignment
			 * @param RE ReadEndsBase object to be filled
			 **/
			static void fillCommon(
				::libmaus::bambam::BamAlignment const & p, 
				::libmaus::bambam::ReadEndsBase & RE
			)
			{
				RE.read1Sequence = p.getRefIDChecked() + 1;
				RE.read1Coordinate = signedEncode(p.getCoordinate() + 1);
				RE.read1IndexInFile = p.getRank();

				uint8_t const * const readname = reinterpret_cast<uint8_t const *>(p.getName());
				uint8_t const * const readnamee = readname + (p.getLReadName()-1);
				
				// parse tile, x, y
				if ( parseReadNameValid(readname,readnamee) )
					parseReadNameTile(readname,readnamee,RE);			
			}

			public:
			/**
			 * fill fragment type ReadEndsBase object
			 *
			 * @param p alignment
			 * @param header BAM header
			 * @param RE ReadEndsBase object to be filled
			 * @param tagId tag id for object
			 **/
			static void fillFrag(
				::libmaus::bambam::BamAlignment const & p, 
				::libmaus::bambam::BamHeader const & header,
				::libmaus::bambam::ReadEndsBase & RE,
				uint64_t const rtagid = 0
			)
			{
				fillCommon(p,RE);
				
				RE.orientation = p.isReverse() ? ::libmaus::bambam::ReadEndsBase::R : ::libmaus::bambam::ReadEndsBase::F;

				RE.score = p.getScore();
				
				if ( p.isPaired() && (!p.isMateUnmap()) )
					RE.read2Sequence = p.getNextRefIDChecked() + 1;
					
				int64_t const rg = p.getReadGroupId(header);
				RE.readGroup = rg + 1;
				RE.libraryId = header.getLibraryId(rg);
				RE.tagId = rtagid;
			}

			/**
			 * fill pair type ReadEndsBase object
			 *
			 * @param p first alignment
			 * @param q second alignment
			 * @param header BAM header
			 * @param RE ReadEndsBase object to be filled
			 **/
			static void fillFragPair(
				::libmaus::bambam::BamAlignment const & p, 
				::libmaus::bambam::BamAlignment const & q, 
				::libmaus::bambam::BamHeader const & header,
				::libmaus::bambam::ReadEndsBase & RE,
				uint64_t const rtagId = 0
			)
			{
				fillCommon(p,RE);

				RE.read2Sequence = q.getRefIDChecked() + 1;
				RE.read2Coordinate = signedEncode(q.getCoordinate() + 1);
				RE.read2IndexInFile = q.getRank();
				
				if ( ! p.isReverse() )
					if ( ! q.isReverse() )
						RE.orientation = ::libmaus::bambam::ReadEndsBase::FF;
					else
						RE.orientation = ::libmaus::bambam::ReadEndsBase::FR;
				else
					if ( ! q.isReverse() )
						RE.orientation = ::libmaus::bambam::ReadEndsBase::RF;
					else
						RE.orientation = ::libmaus::bambam::ReadEndsBase::RR;
				
				
				RE.score = p.getScore() + q.getScore();
				
				if ( p.isPaired() && (!p.isMateUnmap()) )
					RE.read2Sequence = p.getNextRefIDChecked() + 1;
				
				int64_t const rg = p.getReadGroupId(header);
								
				RE.readGroup = rg + 1;
				RE.libraryId = header.getLibraryId(rg);
				RE.tagId = rtagId;				
			}
			
			#define READENDSBASECOMPACT

			/**
			 * decode ReadEndsBase object from compacted form
			 *
			 * @param G input stream
			 **/
			template<typename get_type>
			void get(get_type & G)
			{
				#if defined(READENDSBASECOMPACT)
				this->libraryId = ::libmaus::util::UTF8::decodeUTF8Unchecked(G);
				this->tagId = ::libmaus::util::NumberSerialisation::deserialiseNumber(G);
				this->read1Sequence = ::libmaus::util::UTF8::decodeUTF8Unchecked(G);
				this->read1Coordinate = ::libmaus::util::UTF8::decodeUTF8Unchecked(G);
				this->orientation = static_cast<read_end_orientation>(G.get());
				
				this->read2Sequence = ::libmaus::util::UTF8::decodeUTF8Unchecked(G);
				this->read2Coordinate = ::libmaus::util::UTF8::decodeUTF8Unchecked(G);
				
				this->read1IndexInFile = ::libmaus::util::NumberSerialisation::deserialiseNumber(G);
				this->read2IndexInFile = ::libmaus::util::NumberSerialisation::deserialiseNumber(G);

				this->score = ::libmaus::util::UTF8::decodeUTF8Unchecked(G);
				this->readGroup = ::libmaus::util::UTF8::decodeUTF8Unchecked(G);
				
				this->tile = ::libmaus::util::UTF8::decodeUTF8Unchecked(G);

				this->x = ::libmaus::util::NumberSerialisation::deserialiseNumber(G,4);
				this->y = ::libmaus::util::NumberSerialisation::deserialiseNumber(G,4);
				#else
				G.read(reinterpret_cast<char *>(this),sizeof(*this));
				#endif
			}

			/**
			 * encode ReadEndsBase object to compacted form
			 *
			 * @param P output stream
			 **/
			template<typename put_type>
			void put(put_type & P) const
			{
				#if defined(READENDSBASECOMPACT)
				::libmaus::util::UTF8::encodeUTF8(this->libraryId,P);

				::libmaus::util::NumberSerialisation::serialiseNumber(P,this->tagId);

				::libmaus::util::UTF8::encodeUTF8(this->read1Sequence,P);
				::libmaus::util::UTF8::encodeUTF8(this->read1Coordinate,P);
				P.put(static_cast<uint8_t>(this->orientation));

				::libmaus::util::UTF8::encodeUTF8(this->read2Sequence,P);
				::libmaus::util::UTF8::encodeUTF8(this->read2Coordinate,P);

				::libmaus::util::NumberSerialisation::serialiseNumber(P,this->read1IndexInFile);
				::libmaus::util::NumberSerialisation::serialiseNumber(P,this->read2IndexInFile);
				
				::libmaus::util::UTF8::encodeUTF8(this->score,P);
				::libmaus::util::UTF8::encodeUTF8(this->readGroup,P);
				
				::libmaus::util::UTF8::encodeUTF8(this->tile,P);

				::libmaus::util::NumberSerialisation::serialiseNumber(P,this->x,4);
				::libmaus::util::NumberSerialisation::serialiseNumber(P,this->y,4);					
				#else
				P.write(reinterpret_cast<char const *>(this),sizeof(*this));				
				#endif
			}
		};

		/**
		 * ReadEnds class (used for duplicate marking); this class contains partial information from
		 * a pair of reads
		 **/
		struct ReadEnds : public ReadEndsBase
		{
			//! first alignment if copied
			BamAlignment::shared_ptr_type p;
			//! second alignment if copied
			BamAlignment::shared_ptr_type q;
			
			/**
			 * constructor for empty/invalid empty
			 **/
			ReadEnds() : ReadEndsBase()
			{
			}
			
			/**
			 * reset object to invalid/empty state
			 **/
			void reset()
			{
				ReadEndsBase::reset();
				p.reset();
				q.reset();
			}
			
			/**
			 * recode object (run thorugh an encode and decode cycle; used for debugging, 
			 * recoded object should be identical to original object)
			 *
			 * @return recoded object
			 **/
			ReadEnds recode() const
			{
				std::ostringstream ostr;
				put(ostr);
				std::istringstream istr(ostr.str());
				ReadEnds RE;
				RE.get(istr);
				return RE;
			}

			/**
			 * decode ReadEnds object from stream G
			 *
			 * @param G input stream
			 **/
			template<typename get_type>
			void get(get_type & G)
			{
				ReadEndsBase::get(G);
				
				uint64_t numal = G.get();
				
				if ( numal > 0 )
					p = BamAlignment::shared_ptr_type(new BamAlignment(G));
				if ( numal > 1 )
					q = BamAlignment::shared_ptr_type(new BamAlignment(G));
			}

			/**
			 * encode ReadEnds object to output stream P
			 *
			 * @param P output stream
			 **/
			template<typename put_type>
			void put(put_type & P) const
			{
				ReadEndsBase::put(P);

				unsigned int const havep = ((p.get() != 0) ? 1 : 0);
				unsigned int const haveq = ((q.get() != 0) ? 1 : 0);
				uint64_t const numal = havep+haveq;
				P.put(static_cast<uint8_t>(numal));
					
				if ( havep )
					p->serialise(P);
				if ( haveq )
					q->serialise(P);
			}

			/**
			 * constructor for fragment type ReadEnds object
			 *
			 * @param p alignment
			 * @param header BAM header
			 * @param copyAlignment copy alignment to object
			 **/
			ReadEnds(
				::libmaus::bambam::BamAlignment const & p, 
				::libmaus::bambam::BamHeader const & header,
				bool const copyAlignment = false,
				uint64_t const rtagId = 0
			)
			{
				reset();
				fillFrag(p,header,*this,rtagId);
				if ( copyAlignment )
					 this->p = p.sclone();
			}

			/**
			 * constructor for pair type ReadEnds object
			 *
			 * @param p first alignment
			 * @param q second alignment
			 * @param header BAM header
			 * @param copyAlignment copy alignment to object
			 **/
			ReadEnds(
				::libmaus::bambam::BamAlignment const & p, 
				::libmaus::bambam::BamAlignment const & q, 
				::libmaus::bambam::BamHeader const & header,
				bool const copyAlignment = false,
				uint64_t const rtagId = 0
			)
			{
				reset();
				fillFragPair(p,q,header,*this,rtagId);
				if ( copyAlignment )
				{
					 this->p = p.sclone();
					 this->q = q.sclone();
				}
			}	
		};
		
		struct ReadEndsBasePointerComparator
		{
			bool operator()(ReadEndsBase const * A, ReadEndsBase const * B) const
			{
				return (*A) < (*B);
			}
		};
	}
}

/**
 * format orientation for output stream
 *
 * @param out output stream
 * @param reo read ends orientation
 * @return out
 **/
namespace libmaus
{
	namespace bambam
	{
		std::ostream & operator<<(std::ostream & out, libmaus::bambam::ReadEnds::read_end_orientation reo);
	}
}
/**
 * format read ends object for output stream
 *
 * @param out output stream
 * @param RE read ends object
 * @return out
 **/
namespace libmaus
{
	namespace bambam
	{
		std::ostream & operator<<(std::ostream & out, libmaus::bambam::ReadEnds const & RE);
	}
}
#endif
