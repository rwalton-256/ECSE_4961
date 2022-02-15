#include <iostream>

#include "thread_safe_queue.hpp"

void compress_stream( std::istream& is, std::ostream& os, size_t block_size, size_t num_threads );

