#include <iostream>
#include <cassert>
#include <immintrin.h>
#include <new>
#include <iomanip>
#include "tracer.hpp"
#include "matrix_mult.hpp"


int main( int argc, char** argv )
{
    unsigned s = 10000;

    Matrix<float> x(s, s);
    x.initialize( 0.0F );
    for( unsigned i=0; i<x.rows(); i++ )
    {
        for( unsigned j=0; j<x.cols(); j++ )
        {
            x[j][i] = rand() * 1.0F;
        }
    }

    Matrix<float> r0(s, s);
    Matrix<float> r1(s, s);
    Matrix<float> r2(s, s);
    Matrix<float> r3(s, s);
    Matrix<float> r4(s, s);
    Matrix<float> r5(s, s);

    {
        Tracer t( "mul5" );
        mul5<float>( Matrix<float>(x, false), x, r5, 6, 296, 168 );
    }
    {
        Tracer t( "mul4" );
        mul4<float>( Matrix<float>(x, false), x, r4, 296 );
    }
    {
        Tracer t( "mul3" );
        mul3<float>( x, x, r3 );
    }
    {
        Tracer t( "mul2" );
        mul2<float>( x, x, r2 );
    }
    {
        Tracer t( "mul1" );
        mul1<float>( x, x, r1 );
    }
    {
        Tracer t( "mul0" );
        mul0<float>( x, x, r0 );
    }

    assert( r0 == r1 );
    assert( r0 == r2 );
    assert( r0 == r3 );
    assert( r0 == r4 );
    assert( r0 == r5 );

    return 0;
}