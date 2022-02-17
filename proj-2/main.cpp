#include <fstream>

#include "zstd_comp.hpp"

int main( int argc, char** argv )
{
    std::ifstream in( argv[1] );
    std::ofstream out( argv[2] );
    compress_stream( in, out, 16000, 6 );

    return 0;
}
