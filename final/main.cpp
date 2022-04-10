#include "b_plus.hpp"

#include "tracer.hpp"

#include <fstream>
#include <cstring>

int main()
{
    B_Tree b;

    B_Tree::Val v;

    {
        Tracer t( "test" );

        for( size_t i=0; i<4000000; i++ )
        {
            B_Tree::Key k = ( rand() << 16 ) | ( rand() & 0xffff ) & 0xffffffff;

            snprintf( (char*)v.val, sizeof( v.val ), "0x%08x", k );

            b.insert( k, v );
        }
    }

    {
        Tracer t( "test2" );
        for( size_t i=0; i<1000000; i++ )
        {
            B_Tree::Key k = ( rand() << 16 ) | ( rand() & 0xffff ) & 0xffffffff;

            if( b.find( k, v ) )
            {
                std::cout << "Found! " << std::hex << k << " " << v.val << std::endl;
            }
            else
            {
                //std::cout << "Not found" << std::endl;
            }
        }
    }
}
