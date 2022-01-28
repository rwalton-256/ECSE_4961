#include <iostream>
#include <cassert>
#include <immintrin.h>
#include <new>
#include <iomanip>
#include <vector>
#include "tracer.hpp"
#include "matrix_mult.hpp"


int main( int argc, char** argv )
{
    std::vector<unsigned> sizes{ 1, 2, 1000, 10000 };
    unsigned num_threads = 6;

    for( std::vector<unsigned>::iterator i=sizes.begin(); i!= sizes.end(); i++ )
    {
        unsigned s = *i;

        Matrix<float> f_matrix(s, s);
        f_matrix.initialize( 0.0F );
        for( unsigned i=0; i<f_matrix.rows(); i++ )
        {
            for( unsigned j=0; j<f_matrix.cols(); j++ )
            {
                f_matrix[j][i] = rand() * 1.0F;
            }
        }

        Matrix<float> fr0(s, s);
        Matrix<float> fr1(s, s);
        Matrix<float> fr2(s, s);
        Matrix<float> fr3(s, s);
        Matrix<float> fr4(s, s);
        Matrix<float> fr5(s, s);

        {
            Tracer t( "fmul5_size" + std::to_string( s ) + "_threads" + std::to_string(num_threads) );
            fmul5( Matrix<float>(f_matrix, false), f_matrix, fr5, num_threads, 200, 200 );
        }
        {
            Tracer t( "fmul4_size" + std::to_string( s ) );
            fmul4( Matrix<float>(f_matrix, false), f_matrix, fr4, 296 );
        }
        {
            Tracer t( "fmul3_size" + std::to_string( s ) );
            fmul3( f_matrix, f_matrix, fr3 );
        }
        {
            Tracer t( "fmul2_size" + std::to_string( s ) );
            fmul2( f_matrix, f_matrix, fr2 );
        }
        {
            Tracer t( "fmul1_size" + std::to_string( s ) );
            mul1<float>( f_matrix, f_matrix, fr1 );
        }
        {
            Tracer t( "fmul0_size" + std::to_string( s ) );
            mul0<float>( f_matrix, f_matrix, fr0 );
        }

        assert( fr0 == fr1 );
        assert( fr0 == fr2 );
        assert( fr0 == fr3 );
        assert( fr0 == fr4 );
        assert( fr0 == fr5 );

        std::cout << "\033[34m" << "Float matrix multiplication of size " << s << "x" << s << " passed!" << "\033[39m" << std::endl;

        Matrix<double> d_matrix(s, s);
        d_matrix.initialize( 0.0F );
        for( unsigned i=0; i<d_matrix.rows(); i++ )
        {
            for( unsigned j=0; j<d_matrix.cols(); j++ )
            {
                d_matrix[j][i] = rand() * 1.0;
            }
        }

        Matrix<double> dr0(s, s);
        Matrix<double> dr1(s, s);
        Matrix<double> dr2(s, s);
        Matrix<double> dr3(s, s);

        {
            Tracer t( "dmul5_size" + std::to_string( s ) + "_threads" + std::to_string( num_threads ) );
            dmul5( Matrix<double>(d_matrix, false), d_matrix, dr3, num_threads, 200, 200 );
        }
        {
            Tracer t( "dmul5_size" + std::to_string( s ) + "_threads1" );
            dmul5( Matrix<double>(d_matrix, false), d_matrix, dr2, 1, 200, 200 );
        }
        {
            Tracer t( "dmul1_size" + std::to_string( s ) );
            mul1<double>( d_matrix, d_matrix, dr1 );
        }
        {
            Tracer t( "dmul0_size" + std::to_string( s ) );
            mul0<double>( d_matrix, d_matrix, dr0 );
        }

        assert( dr0 == dr1 );
        assert( dr0 == dr2 );
        assert( dr0 == dr3 );

        std::cout << "\033[34m" << "Double matrix multiplication of size " << s << "x" << s << " passed!" << "\033[39m" << std::endl;

        Matrix<int16_t> i16_matrix(s, s);
        i16_matrix.initialize( 0.0F );
        for( unsigned i=0; i<i16_matrix.rows(); i++ )
        {
            for( unsigned j=0; j<i16_matrix.cols(); j++ )
            {
                i16_matrix[j][i] = rand() % 3 - 1;
            }
        }

        Matrix<int16_t> i16r0(s, s);
        Matrix<int16_t> i16r1(s, s);
        Matrix<int16_t> i16r2(s, s);
        Matrix<int16_t> i16r3(s, s);

        {
            Tracer t( "i16mul5_size" + std::to_string( s ) + "_threads" + std::to_string( num_threads ) );
            i16mul5( Matrix<int16_t>(i16_matrix, false), i16_matrix, i16r3, num_threads, 200, 200 );
        }
        {
            Tracer t( "i16mul5_size" + std::to_string( s ) + "_threads1" );
            i16mul5( Matrix<int16_t>(i16_matrix, false), i16_matrix, i16r2, 1, 200, 200 );
        }
        {
            Tracer t( "i16mul1_size" + std::to_string( s ) );
            mul1<int16_t>( i16_matrix, i16_matrix, i16r1 );
        }
        {
            Tracer t( "i16mul0_size" + std::to_string( s ) );
            mul0<int16_t>( i16_matrix, i16_matrix, i16r0 );
        }

        assert( i16r0 == i16r1 );
        assert( i16r0 == i16r2 );
        assert( i16r0 == i16r3 );

        std::cout << "\033[34m" << "int16_t matrix multiplication of size " << s << "x" << s << " passed!" << "\033[39m" << std::endl;

        Matrix<int32_t> i32_matrix(s, s);
        i32_matrix.initialize( 0.0F );
        for( unsigned i=0; i<i32_matrix.rows(); i++ )
        {
            for( unsigned j=0; j<i32_matrix.cols(); j++ )
            {
                i32_matrix[j][i] = rand() % 801 - 401;
            }
        }

        Matrix<int32_t> i32r0(s, s);
        Matrix<int32_t> i32r1(s, s);
        Matrix<int32_t> i32r2(s, s);
        Matrix<int32_t> i32r3(s, s);

        {
            Tracer t( "i32mul5_size" + std::to_string( s ) + "_threads" + std::to_string( num_threads ) );
            i32mul5( Matrix<int32_t>(i32_matrix, false), i32_matrix, i32r3, num_threads, 200, 200 );
        }
        {
            Tracer t( "i32mul5_size" + std::to_string( s ) + "_threads1" );
            i32mul5( Matrix<int32_t>(i32_matrix, false), i32_matrix, i32r2, 1, 200, 200 );
        }
        {
            Tracer t( "i32mul1_size" + std::to_string( s ) );
            mul1<int32_t>( i32_matrix, i32_matrix, i32r1 );
        }
        {
            Tracer t( "i32mul0_size" + std::to_string( s ) );
            mul0<int32_t>( i32_matrix, i32_matrix, i32r0 );
        }

        assert( i32r0 == i32r1 );
        assert( i32r0 == i32r2 );
        assert( i32r0 == i32r3 );

        std::cout << "\033[34m" << "int32_t matrix multiplication of size " << s << "x" << s << " passed!" << "\033[39m" << std::endl;
    }

    return 0;
}