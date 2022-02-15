#include <zstd.h>
#include <vector>
#include <algorithm>
#include <thread>
#include <atomic>

#include "zstd_comp.hpp"

#include <cstring>
#include <string>

void compress_stream( std::istream& is, std::ostream& os, size_t uncomp_block_size, size_t num_threads )
{
    struct Compression_Block
    {
        void* _mUncompressed;
        void* _mCompressed;
        size_t _mUncompressedSize;
        size_t _mCompressedSize;
        int _mBlockID;
        bool operator< ( Compression_Block& b )
        {
            return _mBlockID < b._mBlockID;
        }
    };

    size_t comp_block_size = ZSTD_compressBound( uncomp_block_size );

    uint8_t* mem_block = new uint8_t[ ( uncomp_block_size + comp_block_size ) * num_threads + 0 ];

    int block_count = 0, end_block = -1, next_block = 0;

    std::thread** compression_threads;
    thread_safe_queue<Compression_Block> to_compress_queue, to_store_queue;
    std::vector<Compression_Block> empty_blocks_vect, blocks_to_store_vect;
    std::atomic<int> threads_working;

    auto thread_func = [&]()
    {
        Compression_Block cb;
        while( to_compress_queue.pop_front( cb ) )
        {
            cb._mCompressedSize = ZSTD_compress( cb._mCompressed, comp_block_size, cb._mUncompressed, cb._mUncompressedSize, 5 );
            to_store_queue.push_back( cb );
        }
        threads_working--;
    };

    compression_threads = new std::thread*[num_threads];
    for( unsigned i=0; i<num_threads; i++ )
    {
        compression_threads[i] = new std::thread( thread_func );
        compression_threads[i]->detach();
    }
    threads_working = num_threads;

    for( unsigned i=0; i<num_threads; i++ )
    {
        Compression_Block cb =
        {
            ._mUncompressed = mem_block + i * ( uncomp_block_size + comp_block_size ),
            ._mCompressed = mem_block + i * ( uncomp_block_size + comp_block_size ) + uncomp_block_size,
            ._mBlockID = block_count
        };
        empty_blocks_vect.push_back( cb );
    }

    while( true )
    {
        // Read blocks in
        while( empty_blocks_vect.size() && end_block == -1 )
        {
            Compression_Block cb = *empty_blocks_vect.rbegin();
            empty_blocks_vect.pop_back();

            is.read( (char*)cb._mUncompressed, uncomp_block_size );
            cb._mUncompressedSize = is.gcount();
            cb._mBlockID = block_count;

            to_compress_queue.push_back( cb );

            if( is.eof() )
            {
                end_block = block_count;
                to_compress_queue.write_done();
            }
            else
            {
                block_count++;
            }
        }

        // Get block from to store queue, store in local vector
        if( to_store_queue.size() )
        {
            Compression_Block cb;
            to_store_queue.pop_front( cb );

            blocks_to_store_vect.push_back( cb );
        }

        // Write compressed data to file, in order it was read from the original file
        if( blocks_to_store_vect.size() )
        {
            std::sort( blocks_to_store_vect.rbegin(), blocks_to_store_vect.rend() );

            Compression_Block cb = *blocks_to_store_vect.rbegin();
            if( cb._mBlockID == next_block )
            {
                blocks_to_store_vect.pop_back();

                os.write( (char*)cb._mCompressed, cb._mCompressedSize );
                empty_blocks_vect.push_back( cb );
                next_block++;
            }
        }

        if( !threads_working.load() && !to_store_queue.size() && !blocks_to_store_vect.size() )
        {
            break;
        }
    }

    for( unsigned i=0; i<num_threads; i++ )
    {
        delete compression_threads[i];
    }
    delete [] compression_threads;

    delete [] mem_block;
}
