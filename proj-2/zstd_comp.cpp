#include <zstd.h>
#include <vector>
#include <algorithm>

#include "zstd_comp.hpp"

#include <cstring>
#include <string>

void ZSTD_comp::compress_stream( std::istream& is, std::ostream& os, size_t block_size, size_t num_threads )
{
    _mBlockSize = block_size;
    uint8_t* mem_block = new uint8_t[ _mBlockSize * 2 * num_threads + 0 ];

    int block_count = 0, end_block = -1, next_block = 0;

    _mCompressionThreads = new std::thread*[num_threads];
    for( unsigned i=0; i<num_threads; i++ )
    {
        _mCompressionThreads[i] = new std::thread( [&](){ _mThreadFunc(); } );
        _mCompressionThreads[i]->detach();
    }
    _mThreadsWorking = num_threads;

    std::vector<Compression_Block> empty_blocks, blocks_to_store;
    for( unsigned i=0; i<2*num_threads; i+=2 )
    {
        Compression_Block cb =
        {
            ._mUncompressed = mem_block + ( i ) * _mBlockSize,
            ._mCompressed = mem_block + ( i + 1 ) * _mBlockSize,
            ._mBlockID = block_count
        };
        empty_blocks.push_back( cb );
    }

    while( true )
    {
        // Read blocks in
        while( empty_blocks.size() && end_block == -1 )
        {
            Compression_Block cb = *empty_blocks.rbegin();
            empty_blocks.pop_back();

            is.read( (char*)cb._mUncompressed, _mBlockSize );
            cb._mUncompressedSize = is.gcount();
            cb._mBlockID = block_count;

            _mToCompress.push_back( cb );

            if( is.eof() )
            {
                end_block = block_count;
                _mToCompress.write_done();
            }
            else
            {
                block_count++;
            }
        }

        // Get block from to store queue, store in local vector
        if( _mToStore.size() )
        {
            Compression_Block cb;
            _mToStore.pop_front( cb );

            blocks_to_store.push_back( cb );
        }

        // Write compressed data to file, in order it was read from the original file
        if( blocks_to_store.size() )
        {
            std::sort( blocks_to_store.rbegin(), blocks_to_store.rend() );

            Compression_Block cb = *blocks_to_store.rbegin();
            if( cb._mBlockID == next_block )
            {
                blocks_to_store.pop_back();

                os.write( (char*)cb._mCompressed, cb._mCompressedSize );
                empty_blocks.push_back( cb );
                next_block++;
            }
        }

        if( !_mThreadsWorking.load() && !_mToStore.size() && !blocks_to_store.size() )
        {
            break;
        }
    }

    for( unsigned i=0; i<num_threads; i++ )
    {
        delete _mCompressionThreads[i];
    }
    delete [] _mCompressionThreads;

    delete [] mem_block;
}

void ZSTD_comp::_mThreadFunc()
{
    Compression_Block cb;
    while( _mToCompress.pop_front( cb ) )
    {
        cb._mCompressedSize = ZSTD_compress( cb._mCompressed, _mBlockSize, cb._mUncompressed, cb._mUncompressedSize, 1 );
        _mToStore.push_back( cb );
    }
    _mThreadsWorking--;
}
