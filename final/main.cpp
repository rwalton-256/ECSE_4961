#include "b_plus.hpp"

#include "tracer.hpp"

#include <fstream>
#include <cstring>

int main()
{
    B_Tree b( "foo.dtb" );

    B_Tree::Val v;

    {
        Tracer t( "test" );

        for( size_t i=0; i<10000000; i++ )
        {
            if( ! (i%10000) )
            {
                t.update_progress( static_cast<float>(i) / 10000000 );
            }

            B_Tree::Key k = ( rand() << 16 ) | ( rand() & 0xffff ) & 0xffffffff;

            snprintf( (char*)v.val, sizeof( v.val ), "0x%08x", k );

            t.begin_trace( "Insert" );
            b.insert( k, v );
            t.end_trace();
        }
    }

    {
        Tracer t( "test2" );
        for( size_t i=0; i<10000000; i++ )
        {
            if( ! (i%10000) )
            {
                t.update_progress( static_cast<float>(i) / 10000000 );
            }

            //B_Tree::Key k = ( rand() << 16 ) | ( rand() & 0xffff ) & 0xffffffff;
            B_Tree::Key k = i;

            t.begin_trace( "Find" );
            bool res = b.find( k, v );
            t.end_trace();

            if( res )
            {
                //std::cout << "Found! " << std::hex << k << " " << v.val << std::endl;
            }
            else
            {
                //std::cout << "Not found" << std::endl;
            }
        }
    }

    std::cout << std::dec << static_cast<float>(b._mHeader._mNumLeafNodes)/b._mLeafNodeMap._mTableSize << std::endl;
}
