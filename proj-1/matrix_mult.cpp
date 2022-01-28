#include <iostream>
#include <cassert>
#include <immintrin.h>
#include <new>
#include <iomanip>
#include "tracer.hpp"
#include "matrix_mult.hpp"


bool operator==( const Matrix<float>& a, const Matrix<float>& b )
{
    if( a.rows() != b.rows() || a.cols() != b.cols() )
        return false;
    for( unsigned i=0; i<a.rows(); i++ )
    {
        for( unsigned j=0; j<b.cols(); j++ )
        {
            if( std::abs( a.at( i, j ) - b.at( i, j ) ) > std::abs( a.at( i, j ) ) / 1000000.0F )
            {
                return false;
            }
        }
    }
    return true;
}

bool operator==( const Matrix<double>& a, const Matrix<double>& b )
{
    if( a.rows() != b.rows() || a.cols() != b.cols() )
        return false;
    for( unsigned i=0; i<a.rows(); i++ )
    {
        for( unsigned j=0; j<b.cols(); j++ )
        {
            if( std::abs( a.at( i, j ) - b.at( i, j ) ) > std::abs( a.at( i, j ) ) / 1000000.0F && std::abs( a.at( i, j ) - b.at( i, j ) ) > 0.0000001F )
            {
                std::cout << i << " " << j << " " << a.at( i, j ) << " " << b.at( i, j ) << std::endl;
                return false;
            }
        }
    }
    return true;
}

bool operator==( const Matrix<int16_t>& a, const Matrix<int16_t>& b )
{
    if( a.rows() != b.rows() || a.cols() != b.cols() )
        return false;
    for( unsigned i=0; i<a.rows(); i++ )
    {
        for( unsigned j=0; j<b.cols(); j++ )
        {
            if( a.at( i, j ) != b.at( i, j ) )
            {
                return false;
            }
        }
    }
    return true;
}

bool operator==( const Matrix<int32_t>& a, const Matrix<int32_t>& b )
{
    if( a.rows() != b.rows() || a.cols() != b.cols() )
        return false;
    for( unsigned i=0; i<a.rows(); i++ )
    {
        for( unsigned j=0; j<b.cols(); j++ )
        {
            if( a.at( i, j ) != b.at( i, j ) )
            {
                return false;
            }
        }
    }
    return true;
}

void fmul2( const Matrix<float>& a, const Matrix<float>& b, Matrix<float>& res )
{
    assert( a.rows() == res.rows() );
    assert( b.cols() == res.cols() );
    assert( a.cols() == b.rows() );

    assert( a.get_major_dim() == true );
    assert( b.get_major_dim() == true );

    unsigned val_align = a.alignment();

    res.initialize( 0.0F );
    for( unsigned i=0; i<res.rows(); i+=val_align )
    {
        for( unsigned j=0; j<res.cols(); j+=val_align )
        {
            for( unsigned k=0; k<a.cols(); k+=val_align )
            {
                __m256 _r[8];
                for( unsigned l=0; l<8; l++ )
                {
                    // Load in result matrix diagonals
                    _r[l] = _mm256_set_ps( res[i+7][j+((l+7)%8)],
                                           res[i+6][j+((l+6)%8)],
                                           res[i+5][j+((l+5)%8)],
                                           res[i+4][j+((l+4)%8)],
                                           res[i+3][j+((l+3)%8)],
                                           res[i+2][j+((l+2)%8)],
                                           res[i+1][j+((l+1)%8)],
                                           res[i+0][j+((l+0)%8)]  );
                }
                __m256 _a, _b;
                for( unsigned kk=0; kk<8; kk++ )
                {
                    // Load in column vector from a and row vector from b
                    _a = _mm256_set_ps( a[i+7][k+kk],
                                        a[i+6][k+kk],
                                        a[i+5][k+kk],
                                        a[i+4][k+kk],
                                        a[i+3][k+kk],
                                        a[i+2][k+kk],
                                        a[i+1][k+kk],
                                        a[i  ][k+kk]  );
                    _b = _mm256_set_ps( b[k+kk][j+7],
                                        b[k+kk][j+6],
                                        b[k+kk][j+5],
                                        b[k+kk][j+4],
                                        b[k+kk][j+3],
                                        b[k+kk][j+2],
                                        b[k+kk][j+1],
                                        b[k+kk][j  ]  );
                    
                    // The vector product yields a diagonal of the result
                    // matrix, rotate one vector each time to yield all diagonals

                    for( unsigned l=0; l<8; l++ )
                    {
                        _r[l] = _mm256_fmadd_ps( _a, _b, _r[l] );
                        _b = _mm256_permutevar8x32_ps( _b, _mm256_set_epi32(0,7,6,5,4,3,2,1));
                    }
                }

                __attribute__ ((aligned (32))) float out[8][8];

                // Export diagonals
                for( unsigned l=0; l<8; l++ )
                {
                    _mm256_store_ps( out[l], _r[l] );
                }

                for( unsigned ii=0; ii<8; ii++ )
                {
                    for( unsigned jj=0; jj<8; jj++ )
                    {
                        res[i+ii][j+jj] = out[(jj-ii+8)%8][ii];
                    }
                }
            }
        }
    }
}

void fmul3( const Matrix<float>& a, const Matrix<float>& b, Matrix<float>& res, unsigned block_width  )
{
    assert( a.rows() == res.rows() );
    assert( b.cols() == res.cols() );
    assert( a.cols() == b.rows() );

    assert( a.get_major_dim() == true );
    assert( b.get_major_dim() == true );

    unsigned val_align = a.alignment();

    res.initialize( 0.0F );
    for( unsigned i=0; i<res.rows(); i+=block_width )
    {
        for( unsigned j=0; j<res.cols(); j+=block_width )
        {
            for( unsigned k=0; k<res.rows(); k+=block_width )
            {
                for( unsigned ii=i; ii<i+block_width && ii<res.rows(); ii+=val_align )
                {
                    for( unsigned jj=j; jj<j+block_width && jj<res.cols(); jj+=val_align )
                    {
                        __m256 _r[8];
                        for( unsigned l=0; l<8; l++ )
                        {
                            // Load in result matrix diagonals
                            _r[l] = _mm256_set_ps( res[ii+7][jj+((l+7)%8)],
                                                    res[ii+6][jj+((l+6)%8)],
                                                    res[ii+5][jj+((l+5)%8)],
                                                    res[ii+4][jj+((l+4)%8)],
                                                    res[ii+3][jj+((l+3)%8)],
                                                    res[ii+2][jj+((l+2)%8)],
                                                    res[ii+1][jj+((l+1)%8)],
                                                    res[ii+0][jj+((l+0)%8)]  );
                        }
                        for( unsigned kk=k; kk<k+block_width && kk<a.cols(); kk++ )
                        {
                            __m256 _a, _b;
                            // Load in column vector from a and row vector from b
                            _a = _mm256_set_ps( a[ii+7][kk],
                                                a[ii+6][kk],
                                                a[ii+5][kk],
                                                a[ii+4][kk],
                                                a[ii+3][kk],
                                                a[ii+2][kk],
                                                a[ii+1][kk],
                                                a[ii  ][kk]  );
                            _b = _mm256_set_ps( b[kk][jj+7],
                                                b[kk][jj+6],
                                                b[kk][jj+5],
                                                b[kk][jj+4],
                                                b[kk][jj+3],
                                                b[kk][jj+2],
                                                b[kk][jj+1],
                                                b[kk][jj  ]  ); 
                            // The vector product yields a diagonal of the result
                            // matrix, rotate one vector each time to yield all diagonals

                            for( unsigned l=0; l<8; l++ )
                            {
                                _r[l] = _mm256_fmadd_ps( _a, _b, _r[l] );
                                _b = _mm256_permutevar8x32_ps( _b, _mm256_set_epi32(0,7,6,5,4,3,2,1));
                            }
                        }
                        __attribute__ ((aligned (32))) float out[8][8];

                        // Export diagonals
                        for( unsigned l=0; l<8; l++ )
                        {
                            _mm256_store_ps( out[l], _r[l] );
                        }

                        for( unsigned iii=0; iii<8; iii++ )
                        {
                            for( unsigned jjj=0; jjj<8; jjj++ )
                            {
                                res[ii+iii][jj+jjj] = out[(jjj-iii+8)%8][iii];
                            }
                        }
                    }
                }
            }
        }
    }
}

void fmul4( const Matrix<float>& a, const Matrix<float>& b, Matrix<float>& res, unsigned block_width )
{
    assert( a.rows() == res.rows() );
    assert( b.cols() == res.cols() );
    assert( a.cols() == b.rows() );

    assert( a.get_major_dim() == false );
    assert( b.get_major_dim() == true );

    unsigned val_align = a.alignment();

    res.initialize( 0.0F );

    for( unsigned i=0; i<res.rows(); i+=block_width )
    {
        for( unsigned j=0; j<res.cols(); j+=block_width )
        {
            for( unsigned k=0; k<res.rows(); k+=block_width )
            {
                for( unsigned ii=i; ii<i+block_width && ii<res.rows(); ii+=val_align )
                {
                    for( unsigned jj=j; jj<j+block_width && jj<res.cols(); jj+=val_align )
                    {
                        __m256 _r[8];
                        for( unsigned l=0; l<8; l++ )
                        {
                            // Load in result matrix diagonals
                            _r[l] = _mm256_set_ps( res[ii+7][jj+((l+7)%8)],
                                                    res[ii+6][jj+((l+6)%8)],
                                                    res[ii+5][jj+((l+5)%8)],
                                                    res[ii+4][jj+((l+4)%8)],
                                                    res[ii+3][jj+((l+3)%8)],
                                                    res[ii+2][jj+((l+2)%8)],
                                                    res[ii+1][jj+((l+1)%8)],
                                                    res[ii+0][jj+((l+0)%8)]  );
                        }

                        for( unsigned kk=k; kk<k+block_width && kk<a.cols(); kk++ )
                        {
                            __m256 _a, _b;
                            // Load in column vector from a and row vector from b
                            _a = _mm256_load_ps( &a[kk][ii] );
                            _b = _mm256_load_ps( &b[kk][jj] );

                            // The vector product yields a diagonal of the result
                            // matrix, rotate one vector each time to yield all diagonals
                            for( unsigned l=0; l<8; l++ )
                            {
                                _r[l] = _mm256_fmadd_ps( _a, _b, _r[l] );
                                _b = _mm256_permutevar8x32_ps( _b, _mm256_set_epi32(0,7,6,5,4,3,2,1));
                            }
                        }
                        __attribute__ ((aligned (32))) float out[8][8];

                        // Export diagonals
                        for( unsigned l=0; l<8; l++ )
                        {
                            _mm256_store_ps( out[l], _r[l] );
                        }

                        for( unsigned iii=0; iii<8; iii++ )
                        {
                            for( unsigned jjj=0; jjj<8; jjj++ )
                            {
                                res[ii+iii][jj+jjj] = out[(jjj-iii+8)%8][iii];
                            }
                        }
                    }
                }
            }
        }
    }
}

#include <vector>
#include <thread>
#include "thread_safe_queue.hpp"

void fmul5( const Matrix<float>& a, const Matrix<float>& b, Matrix<float>& res, unsigned num_threads, unsigned block_width, unsigned block_depth )
{
    struct bar
    {
        unsigned i, j;
        bar() : i(0), j(0) {}
        bar( unsigned _aI, unsigned _aJ ) : i(_aI), j( _aJ ) {}
    };

    assert( a.rows() == res.rows() );
    assert( b.cols() == res.cols() );
    assert( a.cols() == b.rows() );

    assert( a.get_major_dim() == false );
    assert( b.get_major_dim() == true );

    unsigned val_align = a.alignment();

    res.initialize( 0.0F );

    auto foo = [&]( unsigned i, unsigned j )
    {
        for( unsigned k=0; k<res.rows(); k+=block_depth )
        {
            for( unsigned ii=i; ii<i+block_width && ii<res.rows(); ii+=val_align )
            {
                for( unsigned jj=j; jj<j+block_width && jj<res.cols(); jj+=val_align )
                {
                    __m256 _r[8];
                    for( unsigned l=0; l<8; l++ )
                    {
                        // Load in result matrix diagonals
                        _r[l] = _mm256_set_ps( res[ii+7][jj+((l+7)%8)],
                                               res[ii+6][jj+((l+6)%8)],
                                               res[ii+5][jj+((l+5)%8)],
                                               res[ii+4][jj+((l+4)%8)],
                                               res[ii+3][jj+((l+3)%8)],
                                               res[ii+2][jj+((l+2)%8)],
                                               res[ii+1][jj+((l+1)%8)],
                                               res[ii+0][jj+((l+0)%8)]  );
                    }
                    for( unsigned kk=k; kk<k+block_depth && kk<a.cols(); kk++ )
                    {
                        __m256 _a, _b;
                        // Load in column vector from a and row vector from b
                        _a = _mm256_load_ps( &a[kk][ii] );
                        _b = _mm256_load_ps( &b[kk][jj] );

                        // The vector product yields a diagonal of the result
                        // matrix, rotate one vector each time to yield all diagonals
                        for( unsigned l=0; l<8; l++ )
                        {
                            __m256 _t;
                            //_t = _mm256_mul_ps( _a, _b );
                            //_r[l] = _mm256_add_ps( _r[l], _t );
                            _r[l] = _mm256_fmadd_ps( _a, _b, _r[l] );
                            _b = _mm256_permutevar8x32_ps( _b, _mm256_set_epi32(0,7,6,5,4,3,2,1));
                        }
                    }
                    __attribute__ ((aligned (32))) float out[8][8];

                    // Export diagonals
                    for( unsigned l=0; l<8; l++ )
                    {
                        _mm256_store_ps( out[l], _r[l] );
                    }

                    for( unsigned iii=0; iii<8; iii++ )
                    {
                        for( unsigned jjj=0; jjj<8; jjj++ )
                        {
                            res[ii+iii][jj+jjj] = out[(jjj-iii+8)%8][iii];
                        }
                    }
                }
            }
        }
    };

    thread_safe_queue<bar> queue;
    std::atomic<unsigned> done_count(0);
    std::vector<std::thread> thread_vec;

    auto thread_func = [&]( unsigned id )
    {
        bar b;
        while( queue.pop_front( b ) )
        {
            foo( b.i, b.j );
        }
        done_count++;
    };

    for( unsigned i=0; i<num_threads; i++ )
    {
        thread_vec.emplace_back( thread_func, i );
        thread_vec[i].detach();
    }

    for( unsigned i=0; i<res.rows(); i+=block_width )
    {
        for( unsigned j=0; j<res.cols(); j+=block_width )
        {
            queue.push_back( bar(i,j) );
        }
    }

    queue.write_done();

    while( done_count.load() < num_threads ) std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
}

std::ostream& operator<<( std::ostream& o, const __m256d& d )
{
    __attribute__ ((aligned (32))) double out[4];
    _mm256_store_pd( out, d );
    for( unsigned i=0; i<4; i++ )
    {
        std::cout << out[i] << " ";
    }
    std::cout << std::endl;
    return o;
}

void dmul5( const Matrix<double>& a, const Matrix<double>& b, Matrix<double>& res, unsigned num_threads, unsigned block_width, unsigned block_depth )
{
    struct bar
    {
        unsigned i, j;
        bar() : i(0), j(0) {}
        bar( unsigned _aI, unsigned _aJ ) : i(_aI), j( _aJ ) {}
    };

    assert( a.rows() == res.rows() );
    assert( b.cols() == res.cols() );
    assert( a.cols() == b.rows() );

    assert( a.get_major_dim() == false );
    assert( b.get_major_dim() == true );

    unsigned val_align = a.alignment();

    res.initialize( 0.0F );

    auto foo = [&]( unsigned i, unsigned j )
    {
        for( unsigned k=0; k<res.rows(); k+=block_depth )
        {
            for( unsigned ii=i; ii<i+block_width && ii<res.rows(); ii+=4 )
            {
                for( unsigned jj=j; jj<j+block_width && jj<res.cols(); jj+=4 )
                {
                    __m256d _r[4];
                    for( unsigned l=0; l<4; l++ )
                    {
                        // Load in result matrix diagonals
                        _r[l] = _mm256_set_pd( res[ii+3][jj+((l+3)%4)],
                                               res[ii+2][jj+((l+2)%4)],
                                               res[ii+1][jj+((l+1)%4)],
                                               res[ii+0][jj+((l+0)%4)]  );
                    }
                    for( unsigned kk=k; kk<k+block_depth && kk<a.cols(); kk++ )
                    {
                        __m256d _a, _b;
                        // Load in column vector from a and row vector from b
                        _a = _mm256_load_pd( &a[kk][ii] );
                        _b = _mm256_load_pd( &b[kk][jj] );

                        // The vector product yields a diagonal of the result
                        // matrix, rotate one vector each time to yield all diagonals
                        for( unsigned l=0; l<4; l++ )
                        {
                            __m256d _t;
                            //_t = _mm256_mul_pd( _a, _b );
                            //_r[l] = _mm256_add_pd( _r[l], _t );
                            _r[l] = _mm256_fmadd_pd( _a, _b, _r[l] );
                            _b = _mm256_permute4x64_pd( _b, _MM_SHUFFLE( 0, 3, 2, 1 ) );
                        }
                    }
                    __attribute__ ((aligned (32))) double out[4][4];

                    // Export diagonals
                    for( unsigned l=0; l<4; l++ )
                    {
                        _mm256_store_pd( out[l], _r[l] );
                    }

                    for( unsigned iii=0; iii<4; iii++ )
                    {
                        for( unsigned jjj=0; jjj<4; jjj++ )
                        {
                            res[ii+iii][jj+jjj] = out[(jjj-iii+4)%4][iii];
                        }
                    }
                }
            }
        }
    };

    thread_safe_queue<bar> queue;
    std::atomic<unsigned> done_count(0);
    std::vector<std::thread> thread_vec;

    auto thread_func = [&]( unsigned id )
    {
        bar b;
        while( queue.pop_front( b ) )
        {
            foo( b.i, b.j );
        }
        done_count++;
    };

    for( unsigned i=0; i<num_threads; i++ )
    {
        thread_vec.emplace_back( thread_func, i );
        thread_vec[i].detach();
    }

    for( unsigned i=0; i<res.rows(); i+=block_width )
    {
        for( unsigned j=0; j<res.cols(); j+=block_width )
        {
            queue.push_back( bar(i,j) );
        }
    }

    queue.write_done();

    while( done_count.load() < num_threads ) std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
}

std::ostream& operator<<( std::ostream& o, __m128i i )
{
    __attribute__ ((aligned (32))) uint16_t out[8];
    _mm_store_si128( (__m128i*)out, i );

    for( unsigned i=0; i<8; i++ )
    {
        std::cout << out[i] << " ";
    }
    std::cout << std::endl;
    return o;
}


void i16mul5( const Matrix<int16_t>& a, const Matrix<int16_t>& b, Matrix<int16_t>& res, unsigned num_threads, unsigned block_width, unsigned block_depth )
{
    struct bar
    {
        unsigned i, j;
        bar() : i(0), j(0) {}
        bar( unsigned _aI, unsigned _aJ ) : i(_aI), j( _aJ ) {}
    };

    assert( a.rows() == res.rows() );
    assert( b.cols() == res.cols() );
    assert( a.cols() == b.rows() );

    assert( a.get_major_dim() == false );
    assert( b.get_major_dim() == true );

    unsigned val_align = a.alignment();

    res.initialize( 0.0F );

    auto foo = [&]( unsigned i, unsigned j )
    {
        for( unsigned k=0; k<res.rows(); k+=block_depth )
        {
            for( unsigned ii=i; ii<i+block_width && ii<res.rows(); ii+=8 )
            {
                for( unsigned jj=j; jj<j+block_width && jj<res.cols(); jj+=8 )
                {
                    __m128i _r[16];
                    for( unsigned l=0; l<8; l++ )
                    {
                        // Load in result matrix diagonals
                        _r[l] = _mm_setr_epi16( res[ii+0 ][jj+((l+0 )%8)],
                                                res[ii+1 ][jj+((l+1 )%8)],
                                                res[ii+2 ][jj+((l+2 )%8)],
                                                res[ii+3 ][jj+((l+3 )%8)],
                                                res[ii+4 ][jj+((l+4 )%8)],
                                                res[ii+5 ][jj+((l+5 )%8)],
                                                res[ii+6 ][jj+((l+6 )%8)],
                                                res[ii+7 ][jj+((l+7 )%8)]  );
                    }
                    for( unsigned kk=k; kk<k+block_depth && kk<a.cols(); kk++ )
                    {
                        __m128i _a, _b;
                        // Load in column vector from a and row vector from b
                        _a = _mm_load_si128( (__m128i*)&a[kk][ii] );
                        _b = _mm_load_si128( (__m128i*)&b[kk][jj] );

                        // The vector product yields a diagonal of the result
                        // matrix, rotate one vector each time to yield all diagonals
                        for( unsigned l=0; l<8; l++ )
                        {
                            __m128i _t;
                            _t = _mm_mullo_epi16( _a, _b );
                            _r[l] = _mm_add_epi16( _r[l], _t );
                            __m128i mask = _mm_set_epi8( 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2 );
                            _b = _mm_shuffle_epi8( _b, mask );
                        }
                    }
                    __attribute__ ((aligned (32))) uint16_t out[8][8];

                    // Export diagonals
                    for( unsigned l=0; l<8; l++ )
                    {
                        _mm_store_si128( (__m128i*)out[l], _r[l] );
                    }

                    for( unsigned iii=0; iii<8; iii++ )
                    {
                        for( unsigned jjj=0; jjj<8; jjj++ )
                        {
                            res[ii+iii][jj+jjj] = out[(jjj-iii+8)%8][iii];
                        }
                    }
                }
            }
        }
    };

    thread_safe_queue<bar> queue;
    std::atomic<unsigned> done_count(0);
    std::vector<std::thread> thread_vec;

    auto thread_func = [&]( unsigned id )
    {
        bar b;
        while( queue.pop_front( b ) )
        {
            foo( b.i, b.j );
        }
        done_count++;
    };

    for( unsigned i=0; i<num_threads; i++ )
    {
        thread_vec.emplace_back( thread_func, i );
        thread_vec[i].detach();
    }

    for( unsigned i=0; i<res.rows(); i+=block_width )
    {
        for( unsigned j=0; j<res.cols(); j+=block_width )
        {
            queue.push_back( bar(i,j) );
        }
    }

    queue.write_done();

    while( done_count.load() < num_threads ) std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
}


void i32mul5( const Matrix<int32_t>& a, const Matrix<int32_t>& b, Matrix<int32_t>& res, unsigned num_threads, unsigned block_width, unsigned block_depth )
{
    struct bar
    {
        unsigned i, j;
        bar() : i(0), j(0) {}
        bar( unsigned _aI, unsigned _aJ ) : i(_aI), j( _aJ ) {}
    };

    assert( a.rows() == res.rows() );
    assert( b.cols() == res.cols() );
    assert( a.cols() == b.rows() );

    assert( a.get_major_dim() == false );
    assert( b.get_major_dim() == true );

    unsigned val_align = a.alignment();

    res.initialize( 0.0F );

    auto foo = [&]( unsigned i, unsigned j )
    {
        for( unsigned k=0; k<res.rows(); k+=block_depth )
        {
            for( unsigned ii=i; ii<i+block_width && ii<res.rows(); ii+=val_align )
            {
                for( unsigned jj=j; jj<j+block_width && jj<res.cols(); jj+=val_align )
                {
                    __m256i _r[8];
                    for( unsigned l=0; l<8; l++ )
                    {
                        // Load in result matrix diagonals
                        _r[l] = _mm256_setr_epi32( res[ii+0][jj+((l+0)%8)],
                                                   res[ii+1][jj+((l+1)%8)],
                                                   res[ii+2][jj+((l+2)%8)],
                                                   res[ii+3][jj+((l+3)%8)],
                                                   res[ii+4][jj+((l+4)%8)],
                                                   res[ii+5][jj+((l+5)%8)],
                                                   res[ii+6][jj+((l+6)%8)],
                                                   res[ii+7][jj+((l+7)%8)]  );
                    }
                    for( unsigned kk=k; kk<k+block_depth && kk<a.cols(); kk++ )
                    {
                        __m256i _a, _b;
                        // Load in column vector from a and row vector from b
                        _a = _mm256_load_si256( (__m256i*)&a[kk][ii] );
                        _b = _mm256_load_si256( (__m256i*)&b[kk][jj] );

                        // The vector product yields a diagonal of the result
                        // matrix, rotate one vector each time to yield all diagonals
                        for( unsigned l=0; l<8; l++ )
                        {
                            __m256i _t;
                            _t = _mm256_mullo_epi32( _a, _b );
                            _r[l] = _mm256_add_epi32( _r[l], _t );
                            _b = _mm256_permutevar8x32_epi32( _b, _mm256_set_epi32(0,7,6,5,4,3,2,1));
                        }
                    }
                    __attribute__ ((aligned (32))) int32_t out[8][8];

                    // Export diagonals
                    for( unsigned l=0; l<8; l++ )
                    {
                        _mm256_store_si256( (__m256i*)out[l], _r[l] );
                    }

                    for( unsigned iii=0; iii<8; iii++ )
                    {
                        for( unsigned jjj=0; jjj<8; jjj++ )
                        {
                            res[ii+iii][jj+jjj] = out[(jjj-iii+8)%8][iii];
                        }
                    }
                }
            }
        }
    };

    thread_safe_queue<bar> queue;
    std::atomic<unsigned> done_count(0);
    std::vector<std::thread> thread_vec;

    auto thread_func = [&]( unsigned id )
    {
        bar b;
        while( queue.pop_front( b ) )
        {
            foo( b.i, b.j );
        }
        done_count++;
    };

    for( unsigned i=0; i<num_threads; i++ )
    {
        thread_vec.emplace_back( thread_func, i );
        thread_vec[i].detach();
    }

    for( unsigned i=0; i<res.rows(); i+=block_width )
    {
        for( unsigned j=0; j<res.cols(); j+=block_width )
        {
            queue.push_back( bar(i,j) );
        }
    }

    queue.write_done();

    while( done_count.load() < num_threads ) std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
}
