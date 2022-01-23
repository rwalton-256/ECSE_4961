#include <iostream>
#include <cassert>
#include <immintrin.h>
#include <new>
#include <iomanip>
#include "tracer.hpp"
#include "matrix_mult.hpp"



#if 0
template<typename T>
void mul4( const Matrix<T>& a, const Matrix<T>& b, Matrix<T>& res, unsigned block_width  )
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
                for( unsigned kk=k; kk<k+block_width && kk<a.cols(); kk+=val_align )
                {
                    __m256 _a, _b;
                    for( unsigned kkk=0; kkk<val_align && kk+kkk<a.cols(); kkk++ )
                    {
                        // Load in column vector from a and row vector from b
                        _a = _mm256_set_ps( a[ii+7][kk+kkk],
                                            a[ii+6][kk+kkk],
                                            a[ii+5][kk+kkk],
                                            a[ii+4][kk+kkk],
                                            a[ii+3][kk+kkk],
                                            a[ii+2][kk+kkk],
                                            a[ii+1][kk+kkk],
                                            a[ii  ][kk+kkk]  );
                        _b = _mm256_set_ps( b[kk+kkk][jj+7],
                                            b[kk+kkk][jj+6],
                                            b[kk+kkk][jj+5],
                                            b[kk+kkk][jj+4],
                                            b[kk+kkk][jj+3],
                                            b[kk+kkk][jj+2],
                                            b[kk+kkk][jj+1],
                                            b[kk+kkk][jj  ]  ); 
                        // The vector product yields a diagonal of the result
                        // matrix, rotate one vector each time to yield all diagonals

                        for( unsigned l=0; l<8; l++ )
                        {
                            _r[l] = _mm256_fmadd_ps( _a, _b, _r[l] );
                            _b = _mm256_permutevar8x32_ps( _b, _mm256_set_epi32(0,7,6,5,4,3,2,1));
                        }
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

template<typename T>
void mul5( const Matrix<T>& a, const Matrix<T>& b, Matrix<T>& res, unsigned block_width  )
{
    assert( a.rows() == res.rows() );
    assert( b.cols() == res.cols() );
    assert( a.cols() == b.rows() );

    unsigned val_align = a.alignment();

    res.initialize( 0.0F );
    for( unsigned i=0; i<res.rows(); i+=block_width )
    {
        for( unsigned j=0; j<res.cols(); j+=block_width )
        {
        }
    }
}

#endif