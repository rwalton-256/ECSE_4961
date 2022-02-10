#include <fstream>

#include "zstd_comp.hpp"

int main( int argc, char** argv )
{
    std::ifstream in( argv[1] );
    std::ofstream out( argv[2] );
    ZSTD_comp comp;
    std::cout << argv[2];
    comp.compress_stream( in, out, 16000, 16 );

    return 0;
}
