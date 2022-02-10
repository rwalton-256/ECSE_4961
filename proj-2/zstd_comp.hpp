#include <thread>
#include <iostream>
#include <atomic>

#include "thread_safe_queue.hpp"

class ZSTD_comp
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
public:
    size_t _mUBlockSize, _mCBlockSize;
    std::thread** _mCompressionThreads;
    thread_safe_queue<Compression_Block> _mToCompress, _mToStore;
    std::atomic<int> _mThreadsWorking;
    void compress_stream( std::istream& is, std::ostream& os, size_t block_size, size_t num_threads );
    void _mThreadFunc();
};

