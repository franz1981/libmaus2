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
#if ! defined(LIBMAUS2_SORTING_PARALLELRUNLENGTHRADIXSORT_HPP)
#define LIBMAUS2_SORTING_PARALLELRUNLENGTHRADIXSORT_HPP

#include <libmaus2/sorting/ParallelRunLengthRadixUnsort.hpp>
#include <libmaus2/math/numbits.hpp>
#include <libmaus2/huffman/RLInitType.hpp>
#include <libmaus2/math/lowbits.hpp>
#include <libmaus2/huffman/IndexDecoderDataArray.hpp>
#include <libmaus2/util/PrefixSums.hpp>

namespace libmaus2
{
	namespace sorting
	{
		struct ParallelRunLengthRadixSort
		{
			template<
				typename decoder_type,
				typename encoder_type,
				typename projector_type,
				typename key_encoder_type,
				typename key_decoder_type,
				bool store_key_bits
			>
			static std::vector<std::string> parallelRadixSort(
				std::vector<std::string> Vfn,
				uint64_t const tnumthreads,
				uint64_t const maxfiles,
				bool deleteinput,
				libmaus2::util::TempFileNameGenerator & tmpgen,
				// block size for output
				uint64_t bs,
				// range restriction
				uint64_t ilow,
				uint64_t ihigh,
				uint64_t maxsym,
				bool maxsymvalid,
				uint64_t const keybs,
				uint64_t * rmaxsym = 0,
				libmaus2::sorting::ParallelRunLengthRadixUnsort * unsortinfo = 0,
				uint64_t const unsortthreads = 1
			)
			{
				if ( unsortinfo )
				{
					unsortinfo->unsortthreads = unsortthreads;
					unsortinfo->totalsyms = ihigh-ilow;
				}

				// we need at least 3 fils per thread (one input, two output. plus one if we store key bits)
				uint64_t const maxthreads = maxfiles / (3+(store_key_bits?1:0));
				// compute maximum number of threads we can actually use
				uint64_t const numthreads = std::min(tnumthreads,maxthreads);

				if ( ! numthreads )
				{
					libmaus2::exception::LibMausException lme;
					lme.getStream() << "parallelRadixSort: maxfiles=" << maxfiles << " is too small (needs to be at least 3)" << std::endl;
					lme.finish();
					throw lme;
				}

				// maximum number of files per thread
				uint64_t const filesperthread = maxfiles / numthreads;
				// maximum number of output files per thread
				uint64_t const ofilesperthread = filesperthread - 1 - (store_key_bits?1:0);

				uint64_t tfilebits = 1;
				uint64_t ofilecnt = 2;

				while ( ofilecnt * 2 <= ofilesperthread )
				{
					ofilecnt *= 2;
					tfilebits += 1;
				}
				assert ( ofilecnt <= ofilesperthread );

				// total symbol bits
				uint64_t totalsymbits = maxsymvalid ? libmaus2::math::numbits(maxsym) : libmaus2::math::numbits(std::numeric_limits<uint64_t>::max());

				libmaus2::huffman::RLInitType::rl_init_type rli = decoder_type::rl_init_type_v;

				uint64_t const numinsyms = ihigh-ilow;

				unsigned int rshift = 0;
				while ( rshift < totalsymbits )
				{
					// rest bits to sort by
					uint64_t const restbits = totalsymbits - rshift;
					// minimum rounds left
					uint64_t const minrounds = (restbits + tfilebits - 1)/tfilebits;
					// number of file bits per round/this round
					uint64_t const filebits = (restbits + minrounds -1)/minrounds;
					// sanity checks
					assert ( (1ull << filebits) <= ofilesperthread );
					assert ( minrounds * filebits + rshift >= totalsymbits );

					uint64_t const outfilesperthread = (1ull<<filebits);
					uint64_t const totaloutfiles = numthreads * outfilesperthread;

					libmaus2::autoarray::AutoArray < typename encoder_type::unique_ptr_type > Aoutfiles(totaloutfiles);

					unsigned int const roundbits = filebits;
					uint64_t const rmask = ::libmaus2::math::lowbits(roundbits);

					uint64_t const isize = ihigh-ilow;
					uint64_t const packsize = (isize+numthreads-1)/numthreads;
					uint64_t const runthreads = (isize+packsize-1)/packsize;

					std::vector<std::string> Vout(runthreads * outfilesperthread);
					std::vector<std::string> Vkey(runthreads);

					for ( uint64_t i = 0; i < outfilesperthread; ++i )
						for ( uint64_t t = 0; t < runthreads; ++t )
						{
							// sym major
							uint64_t const fnid = i * runthreads + t;
							Vout[fnid] = tmpgen.getFileName(true);
							// thread major
							uint64_t const tid = t*outfilesperthread+i;
							typename encoder_type::unique_ptr_type Tenc(new encoder_type(Vout[fnid],bs));
							Aoutfiles[tid] = UNIQUE_PTR_MOVE(Tenc);
						}

					libmaus2::autoarray::AutoArray < typename key_encoder_type::unique_ptr_type > Akeyfiles(store_key_bits ? runthreads : 0);
					if ( store_key_bits )
						for ( uint64_t t = 0; t < runthreads; ++t )
						{
							Vkey[t] = tmpgen.getFileName(true);
							typename key_encoder_type::unique_ptr_type Tenc(new key_encoder_type(Vkey[t],keybs));
							Akeyfiles[t] = UNIQUE_PTR_MOVE(Tenc);
						}

					::libmaus2::huffman::IndexDecoderDataArray IDDA(Vfn,numthreads);
					uint64_t volatile cmaxsym = 0;
					libmaus2::parallel::PosixSpinLock cmaxsymlock;

					// global key histogram
					libmaus2::autoarray::AutoArray<uint64_t> GKhist(store_key_bits ? outfilesperthread : 0);
					libmaus2::parallel::PosixSpinLock GKhistlock;

					#if defined(_OPENMP)
					#pragma omp parallel for num_threads(runthreads)
					#endif
					for ( uint64_t t = 0; t < runthreads; ++t )
					{
						uint64_t const tlow = ilow + t * packsize;
						uint64_t const thigh = std::min( tlow+packsize, ihigh );
						uint64_t const trange = thigh-tlow;

						typename encoder_type::unique_ptr_type * enc = Aoutfiles.begin() + t * outfilesperthread;
						typename decoder_type::run_type R;

						decoder_type dec(IDDA,tlow,rli);
						key_encoder_type *Kenc = store_key_bits ? Akeyfiles[t].get() : 0;

						libmaus2::autoarray::AutoArray<uint64_t> Khist(store_key_bits ? outfilesperthread : 0);

						if ( maxsymvalid )
						{
							switch ( rli )
							{
								case decoder_type::rl_init_type_k:
								{
									for ( uint64_t i = 0; i < trange; ++i )
									{
										dec.decodeRun(R);
										uint64_t const key = (projector_type::project(R) >> rshift) & rmask;
										enc[key]->encodeRun(R);
										if ( store_key_bits )
										{
											Kenc->encodeRun(typename key_encoder_type::run_type(key,R.rlen));
											Khist[key] += R.rlen;
											//libmaus2::parallel::ScopePosixSpinLock slock(libmaus2::aio::StreamLock::cerrlock);
											//std::cerr << "here " << key << " " << R.rlen << std::endl;
										}
									}
									break;
								}
								case decoder_type::rl_init_type_v:
								{
									uint64_t todo = trange;

									while ( todo )
									{
										dec.decodeRun(R);
										uint64_t const key = (projector_type::project(R) >> rshift) & rmask;
										R.rlen = std::min(todo,R.rlen);
										enc[key]->encodeRun(R);
										if ( store_key_bits )
										{
											Kenc->encodeRun(typename key_encoder_type::run_type(key,R.rlen));
											Khist[key] += R.rlen;
										}
										todo -= R.rlen;
									}
									break;
								}
							}
						}
						else
						{
							uint64_t lmaxsym = 0;

							switch ( rli )
							{
								case decoder_type::rl_init_type_k:
								{
									for ( uint64_t i = 0; i < trange; ++i )
									{
										dec.decodeRun(R);
										uint64_t const key = (projector_type::project(R) >> rshift) & rmask;
										enc[key]->encodeRun(R);
										if ( store_key_bits )
										{
											Kenc->encodeRun(typename key_encoder_type::run_type(key,R.rlen));
											Khist[key] += R.rlen;
										}
										if ( R.sym > static_cast<int64_t>(lmaxsym) )
											lmaxsym = R.sym;
									}
									break;
								}
								case decoder_type::rl_init_type_v:
								{
									uint64_t todo = trange;

									while ( todo )
									{
										dec.decodeRun(R);
										uint64_t const key = (projector_type::project(R) >> rshift) & rmask;
										R.rlen = std::min(todo,R.rlen);
										enc[key]->encodeRun(R);
										if ( store_key_bits )
										{
											Kenc->encodeRun(typename key_encoder_type::run_type(key,R.rlen));
											Khist[key] += R.rlen;
										}
										todo -= R.rlen;
										if ( R.sym > static_cast<int64_t>(lmaxsym) )
											lmaxsym = R.sym;
									}
									break;
								}
							}

							cmaxsymlock.lock();
							if ( lmaxsym > cmaxsym )
								cmaxsym = lmaxsym;
							cmaxsymlock.unlock();

						}

						for ( uint64_t i = 0; i < outfilesperthread; ++i )
							(Aoutfiles.begin() + t * outfilesperthread)[i].reset();
						if ( store_key_bits )
							Akeyfiles[t].reset();

						GKhistlock.lock();
						for ( uint64_t i = 0; i < Khist.size(); ++i )
							GKhist[i] += Khist[i];
						GKhistlock.unlock();
					}

					if ( deleteinput )
						for ( uint64_t i = 0; i < Vfn.size(); ++i )
							libmaus2::aio::FileRemoval::removeFile(Vfn[i]);
					deleteinput = true;

					if ( ! maxsymvalid )
					{
						// std::cerr << "[V] setting maxsym to " << cmaxsym << std::endl;
						maxsym = cmaxsym;
						maxsymvalid = true;
						totalsymbits = libmaus2::math::numbits(maxsym);
					}

					if ( store_key_bits )
					{
						assert ( std::accumulate(GKhist.begin(),GKhist.end(),0ull) == numinsyms );

						if ( unsortinfo )
						{
							libmaus2::sorting::ParallelRunLengthRadixUnsort::UnsortLevel UL;
							UL.keyseqfn = Vkey;
							UL.bits = std::min(filebits,totalsymbits);
							uint64_t const outfilesperthread = (1ull<<UL.bits);
							UL.Ghist = std::vector<uint64_t>(GKhist.begin(),GKhist.begin() + outfilesperthread);

							libmaus2::autoarray::AutoArray<uint64_t> Fhist(outfilesperthread * (unsortthreads+1) + 1);

							libmaus2::autoarray::AutoArray < std::pair<uint64_t,uint64_t> > Athreadint(unsortthreads);

							::libmaus2::huffman::IndexDecoderDataArray IDDA(Vkey,tnumthreads);
							uint64_t const symsperthread = (numinsyms + unsortthreads - 1)/unsortthreads;
							#if defined(_OPENMP)
							#pragma omp parallel for schedule(dynamic,1) num_threads(unsortthreads)
							#endif
							for ( uint64_t i = 0; i < unsortthreads; ++i )
							{
								uint64_t const low = std::min(i * symsperthread,numinsyms);
								uint64_t const high = std::min(low+symsperthread,numinsyms);
								uint64_t * Khist = Fhist.begin() + i * outfilesperthread;
								Athreadint[i] = std::pair<uint64_t,uint64_t>(low,high);

								if ( high > low )
								{
									key_decoder_type dec(IDDA,low,key_decoder_type::rl_init_type_v);
									uint64_t todo = high-low;
									typename key_decoder_type::run_type R;
									while ( todo )
									{
										dec.decodeRun(R);
										uint64_t const av = std::min(todo,R.rlen);
										Khist [ R.sym ] += av;
										todo -= av;
									}
								}
							}

							assert ( std::accumulate(Fhist.begin(),Fhist.end(),0ull) == numinsyms );

							for ( uint64_t i = 0; i < outfilesperthread; ++i )
							{
								uint64_t s = 0;
								uint64_t * p = Fhist.begin() + i;
								for ( uint64_t j = 0; j < (unsortthreads+1); ++j )
								{
									uint64_t const t = *p;
									*p = s;
									s += t;
									p += outfilesperthread;
								}
							}

							for ( uint64_t i = 0; i < outfilesperthread; ++i )
								assert ( Fhist[ outfilesperthread * unsortthreads + i ] ==  GKhist[i] );

							libmaus2::util::PrefixSums::prefixSums(
								Fhist.begin() + outfilesperthread * unsortthreads,
								Fhist.begin() + outfilesperthread * unsortthreads + (outfilesperthread) + 1
							);

							assert ( Fhist[outfilesperthread * unsortthreads + outfilesperthread] == numinsyms );

							for ( uint64_t i = 0; i < outfilesperthread; ++i )
							{
								uint64_t const add = Fhist[outfilesperthread * unsortthreads + i];
								assert ( (i!=0) || (add == 0) );

								uint64_t * p = Fhist.begin() + i;
								for ( uint64_t j = 0; j < unsortthreads; ++j )
								{
									*p += add;
									p += outfilesperthread;
								}
							}

							// std::cerr << "here" << std::endl;

							UL.Ohist = std::vector<uint64_t>(Fhist.begin(),Fhist.end());
							UL.Vthreadint = std::vector < std::pair<uint64_t,uint64_t> >(Athreadint.begin(),Athreadint.end());
							unsortinfo->levels.push_back(UL);
						}
					}

					Vfn = Vout;

					rli = decoder_type::rl_init_type_k;
					ilow = 0;
					ihigh = decoder_type::getKLength(Vfn,numthreads);
					rshift += roundbits;

				}

				assert ( maxsymvalid );

				if ( rmaxsym )
					*rmaxsym = maxsym;

				return Vfn;
			}
		};
	}
}
#endif
