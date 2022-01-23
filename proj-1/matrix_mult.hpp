#include <cstring>

template<typename T>
class Matrix
{
private:
    /**
     * @brief Value alignment. Minor dimension of matrix must be a multiple of this
     */
    unsigned _mValAlign = 32 / sizeof( T );
    /**
     * @brief True if matrix is stored in row major fashion
     */
    bool _mRowMajor;
    /**
     * @brief Number of rows in the matrix
     */
    unsigned _mNumRows;
    /**
     * @brief Number of columns in the matrix
     */
    unsigned _mNumCols;
    /**
     * @brief Number of rows allocated, not necessary all are used
     */
    unsigned _mRowsAlloc;
    /**
     * @brief Number of columns allocated, not necessary all are used
     */
    unsigned _mColsAlloc;
    /**
     * @brief Pointer to the beginning of the array
     */
    T* _mDataPtr;
    /**
     * @brief Array of pointers to minor dimension vectors. For example, in
     * a row major storage, this will point to the beginning of each of the
     * rows.
     */
    T** _mMajorDimVectPtr;
public:
    Matrix( unsigned _aRows, unsigned _aCols, bool _aRowMajor = true );
    Matrix( const Matrix<T>& m, bool _aRowMajor = true );
    void initialize( const T& _aInitial );
    ~Matrix();
    /**
     * @brief Returns reference to selected cell
     * 
     * @param row 
     * @param col 
     * @return float& 
     */
    inline float at( unsigned row, unsigned col ) const;
    /**
     * @brief Returns reference to selected cell
     * 
     * @param row 
     * @param col 
     * @return float& 
     */
    inline float& at( unsigned row, unsigned col );
    /**
     * @brief Returns pointer to major vector pointer at index. Based on
     * storage method, this will either return a row or a column vector
     * 
     * @param idx 
     * @return float& 
     */
    inline float*& operator[]( unsigned idx ) { return _mMajorDimVectPtr[ idx ]; }
    /**
     * @brief Returns pointer to major vector pointer at index. Based on
     * storage method, this will either return a row or a column vector
     * 
     * @param idx 
     * @return float& 
     */
    inline const float* operator[]( unsigned idx ) const { return _mMajorDimVectPtr[ idx ]; }
    /**
     * @brief Return rows in matrix
     * 
     * @return unsigned 
     */
    unsigned rows() const { return _mNumRows; }
    /**
     * @brief Return columns in matrix
     * 
     * @return unsigned 
     */
    unsigned cols() const { return _mNumCols; }
    /**
     * @brief Return allocated rows in matrix
     * 
     * @return unsigned 
     */
    unsigned rows_alloc() const { return _mNumRows; }
    /**
     * @brief Return allocated columns in matrix
     * 
     * @return unsigned 
     */
    unsigned cols_alloc() const { return _mNumCols; }
    /**
     * @brief Returns number of elements to align storage by
     * 
     * @return unsigned 
     */
    unsigned alignment() const { return _mValAlign; }
    /**
     * @brief Get the major dim object
     * 
     * @return true 
     * @return false 
     */
    bool get_major_dim() const { return _mRowMajor; }
    /**
     * @brief Set the major dim object
     * 
     * @param _aRowMajor 
     */
    void set_major_dim( bool _aRowMajor );
};

template<typename T>
bool operator==( const Matrix<T>& a, const Matrix<T>& b );

template<typename T>
Matrix<T> operator*( const Matrix<T>& a, const Matrix<T>& b );

template<typename T>
std::ostream& operator<<( std::ostream& out, const Matrix<T>& m );

/**
 * @brief 
 * 
 * @tparam T 
 * @param a 
 * @param b 
 * @param res 
 */
template<typename T>
void mul0( const Matrix<T>& a, const Matrix<T>& b, Matrix<T>& res );

/**
 * @brief 
 * 
 * @tparam T 
 * @param a 
 * @param b 
 * @param res 
 * @param block_width 
 */
template<typename T>
void mul1( const Matrix<T>& a, const Matrix<T>& b, Matrix<T>& res, unsigned block_width=64 );

/**
 * @brief 
 * 
 * @tparam T 
 * @param a 
 * @param b 
 * @param res 
 */
template<typename T>
void mul2( const Matrix<T>& a, const Matrix<T>& b, Matrix<T>& res );

/**
 * @brief 
 * 
 * @tparam T 
 * @param a 
 * @param b 
 * @param res 
 * @param block_width 
 */
template<typename T>
void mul3( const Matrix<T>& a, const Matrix<T>& b, Matrix<T>& res, unsigned block_width=344  );

/**
 * @brief 
 * 
 * @tparam T 
 * @param a 
 * @param b 
 * @param res 
 * @param block_width 
 */
template<typename T>
void mul4( const Matrix<T>& a, const Matrix<T>& b, Matrix<T>& res, unsigned block_width=344  );

/**
 * @brief 
 * 
 * @tparam T 
 * @param a 
 * @param b 
 * @param res 
 * @param num_threads 
 * @param block_width 
 */
template<typename T>
void mul5( const Matrix<T>& a, const Matrix<T>& b, Matrix<T>& res, unsigned num_threads, unsigned block_width=200, unsigned block_depth=200 );


/*************IMPLEMENTATION*****************/

template<typename T>
Matrix<T>::Matrix( unsigned _aRows, unsigned _aCols, bool _aRowMajor )
    : _mRowMajor( _aRowMajor ),
      _mNumRows( _aRows ),
      _mNumCols( _aCols ),
      _mRowsAlloc( ( ( _mNumRows + _mValAlign - 1 ) / _mValAlign ) * _mValAlign ),
      _mColsAlloc( ( ( _mNumCols + _mValAlign - 1 ) / _mValAlign ) * _mValAlign )
{
    _mDataPtr = new (std::align_val_t(32)) T[ _mRowsAlloc * _mColsAlloc ];
    if( _mRowMajor )
    {
        _mMajorDimVectPtr = new T*[ _mRowsAlloc ];
        for( unsigned i=0; i<_mRowsAlloc; i++ )
        {
            _mMajorDimVectPtr[i] = _mDataPtr + _mColsAlloc * i;
        }
    }
    else
    {
        _mMajorDimVectPtr = new T*[ _mColsAlloc ];
        for( unsigned i=0; i<_mColsAlloc; i++ )
        {
            _mMajorDimVectPtr[i] = _mDataPtr + _mRowsAlloc * i;
        }
    }
}

template<typename T>
Matrix<T>::Matrix( const Matrix<T>& m, bool _aRowMajor )
  : _mRowMajor( _aRowMajor ),
    _mNumRows( m.rows() ),
    _mNumCols( m.cols() ),
    _mRowsAlloc( ( ( _mNumRows + _mValAlign - 1 ) / _mValAlign ) * _mValAlign ),
    _mColsAlloc( ( ( _mNumCols + _mValAlign - 1 ) / _mValAlign ) * _mValAlign )
{
    _mDataPtr = new (std::align_val_t(32)) T[ _mRowsAlloc * _mColsAlloc ];
    if( _mRowMajor )
    {
        _mMajorDimVectPtr = new T*[ _mRowsAlloc ];
        for( unsigned i=0; i<_mRowsAlloc; i++ )
        {
            _mMajorDimVectPtr[i] = _mDataPtr + _mColsAlloc * i;
        }
    }
    else
    {
        _mMajorDimVectPtr = new T*[ _mColsAlloc ];
        for( unsigned i=0; i<_mColsAlloc; i++ )
        {
            _mMajorDimVectPtr[i] = _mDataPtr + _mRowsAlloc * i;
        }
    }

    for( unsigned i=0; i<_mRowsAlloc; i++ )
    {
        for( unsigned j=0; j<_mColsAlloc; j++ )
        {
            at( i, j ) = m.at( i, j );
        }
    }
}

template<typename T>
void Matrix<T>::initialize( const T& _aInitial )
{
    if( _mRowMajor )
    {
        for( unsigned i=0; i<_mRowsAlloc; i++ )
        {
            for( unsigned j=0; j<_mColsAlloc; j++ )
            {
                _mMajorDimVectPtr[i][j] = _aInitial;
            }
        }
    }
    else
    {
        for( unsigned i=0; i<_mColsAlloc; i++ )
        {
            for( unsigned j=0; j<_mRowsAlloc; j++ )
            {
                _mMajorDimVectPtr[i][j] = _aInitial;
            }
        }
    }
}

template<typename T>
Matrix<T>::~Matrix()
{
    delete [] _mDataPtr;
    delete [] _mMajorDimVectPtr;
}

template<typename T>
inline float Matrix<T>::at( unsigned row, unsigned col ) const
{
    if( _mRowMajor )
    {
        return _mMajorDimVectPtr[row][col];
    }
    else
    {
        return _mMajorDimVectPtr[col][row];
    }
}

template<typename T>
inline float& Matrix<T>::at( unsigned row, unsigned col )
{
    if( _mRowMajor )
    {
        return _mMajorDimVectPtr[row][col];
    }
    else
    {
        return _mMajorDimVectPtr[col][row];
    }
}

template<typename T>
void Matrix<T>::set_major_dim( bool _aRowMajor )
{
    if( _aRowMajor == _mRowMajor )
    {
        return;
    }

    T* temp_data_ptr;
    T** temp_major_dim_vect_ptr;

    temp_data_ptr = new (std::align_val_t(32)) T[ _mRowsAlloc * _mColsAlloc ];
    if( _aRowMajor )
    {
        temp_major_dim_vect_ptr = new T*[ _mRowsAlloc ];
        for( unsigned i=0; i<_mRowsAlloc; i++ )
        {
            temp_major_dim_vect_ptr[i] = temp_data_ptr + _mColsAlloc * i;
        }

        for( unsigned i=0; i<_mRowsAlloc; i++ )
        {
            for( unsigned j=0; j<_mColsAlloc; j++ )
            {
                temp_major_dim_vect_ptr[i][j] = _mMajorDimVectPtr[j][i];
            }
        }
    }
    else
    {
        temp_major_dim_vect_ptr = new T*[ _mColsAlloc ];
        for( unsigned i=0; i<_mColsAlloc; i++ )
        {
            temp_major_dim_vect_ptr[i] = temp_data_ptr + _mRowsAlloc * i;
        }

        for( unsigned i=0; i<_mRowsAlloc; i++ )
        {
            for( unsigned j=0; j<_mColsAlloc; j++ )
            {
                temp_major_dim_vect_ptr[j][i] = _mMajorDimVectPtr[i][j];
            }
        }
    }

    delete [] _mDataPtr;
    delete [] _mMajorDimVectPtr;

    _mDataPtr = temp_data_ptr;
    _mMajorDimVectPtr = temp_major_dim_vect_ptr;
    _mRowMajor = _aRowMajor;
}

template<typename T>
bool operator==( const Matrix<T>& a, const Matrix<T>& b )
{
    if( a.rows() != b.rows() || a.cols() != b.cols() )
        return false;
    for( unsigned i=0; i<a.rows(); i++ )
    {
        for( unsigned j=0; j<b.cols(); j++ )
        {
            if( std::abs( a.at( i, j ) - b.at( i, j ) ) > std::abs( a.at( i, j ) ) / 1000000.0F )
            {
                std::cout << i << " " << j << std::endl;
                return false;
            }
        }
    }
    return true;
}

template<typename T>
Matrix<T> operator*( const Matrix<T>& a, const Matrix<T>& b )
{
    assert( a.rows() && a.cols() && b.rows() && b.cols() );
    assert( a.cols() == b.rows() );

    Matrix<T> r( a.rows(), b.cols() );

    Matrix<T> A( a );
    Matrix<T> B( b );

    A.set_major_dim( false );
    B.set_major_dim( true );

    mul5<T>( A, B, r, 6 );
    return r;
}

template<typename T>
std::ostream& operator<<( std::ostream& out, const Matrix<T>& m )
{
    out << "Matrix(" << m.rows() << "x" << m.cols() << "):" << std::endl;
    for( unsigned i=0; i<m.rows(); i++ )
    {
        for( unsigned j=0; j<m.cols(); j++ )
        {
            out << "[" << std::setw(7) << m.at(i,j) << "]" << std::flush;
        }
        out << std::endl;
    }
    return out;
}

template<typename T>
void mul0( const Matrix<T>& a, const Matrix<T>& b, Matrix<T>& res )
{
    assert( a.rows() == res.rows() );
    assert( b.cols() == res.cols() );
    assert( a.cols() == b.rows() );

    assert( a.get_major_dim() == true );
    assert( b.get_major_dim() == true );

    Tracer t( "mul0" );

    res.initialize( 0.0F );
    for( unsigned i=0; i<res.rows(); i++ )
    {
        t.update_progress( static_cast<float>(i) / res.rows() );
        for( unsigned j=0; j<res.cols(); j++ )
        {
            for( unsigned k=0; k<a.cols(); k++ )
            {
                res[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

template<typename T>
void mul1( const Matrix<T>& a, const Matrix<T>& b, Matrix<T>& res, unsigned block_width )
{
    assert( a.rows() == res.rows() );
    assert( b.cols() == res.cols() );
    assert( a.cols() == b.rows() );

    assert( a.get_major_dim() == true );
    assert( b.get_major_dim() == true );

    Tracer t( "mul1" );

    res.initialize( 0.0F );
    for( unsigned i=0; i<res.rows(); i+=block_width )
    {
        t.update_progress( static_cast<float>(i) / res.rows() );
        for( unsigned j=0; j<res.cols(); j+=block_width )
        {
            for( unsigned k=0; k<a.cols(); k+=block_width )
            {
                for( unsigned ii=i; ii<i+block_width && ii < res.rows(); ii++ )
                {
                    for( unsigned jj=j; jj<j+block_width && jj < res.cols(); jj++ )
                    {
                        for(unsigned kk=k; kk<k+block_width && kk < a.cols(); kk++ )
                        {
                            res[ii][jj] += a[ii][kk] * b[kk][jj];
                        }
                    }
                }
            }
        }
    }
}

template<typename T>
void mul2( const Matrix<T>& a, const Matrix<T>& b, Matrix<T>& res )
{
    assert( a.rows() == res.rows() );
    assert( b.cols() == res.cols() );
    assert( a.cols() == b.rows() );

    assert( a.get_major_dim() == true );
    assert( b.get_major_dim() == true );

    Tracer t( "mul2" );

    unsigned val_align = a.alignment();

    res.initialize( 0.0F );
    for( unsigned i=0; i<res.rows(); i+=val_align )
    {
        t.update_progress( static_cast<float>(i) / res.rows() );
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

template<typename T>
void mul3( const Matrix<T>& a, const Matrix<T>& b, Matrix<T>& res, unsigned block_width  )
{
    assert( a.rows() == res.rows() );
    assert( b.cols() == res.cols() );
    assert( a.cols() == b.rows() );

    assert( a.get_major_dim() == true );
    assert( b.get_major_dim() == true );

    Tracer t( "mul3" );

    unsigned val_align = a.alignment();

    res.initialize( 0.0F );
    for( unsigned i=0; i<res.rows(); i+=block_width )
    {
        t.update_progress( static_cast<float>(i) / res.rows() );
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
    }
}

template<typename T>
void mul4( const Matrix<T>& a, const Matrix<T>& b, Matrix<T>& res, unsigned block_width )
{
    assert( a.rows() == res.rows() );
    assert( b.cols() == res.cols() );
    assert( a.cols() == b.rows() );

    assert( a.get_major_dim() == false );
    assert( b.get_major_dim() == true );

    Tracer t( "mul4" );

    unsigned val_align = a.alignment();

    res.initialize( 0.0F );

    for( unsigned i=0; i<res.rows(); i+=block_width )
    {
        t.update_progress( static_cast<float>(i) / res.rows() );
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
                        for( unsigned kk=k; kk<k+block_width && kk<a.cols(); kk+=val_align )
                        {
                            __m256 _a, _b;
                            for( unsigned kkk=0; kkk<val_align && kk+kkk<a.cols(); kkk++ )
                            {
                                // Load in column vector from a and row vector from b
                                _a = _mm256_load_ps( &a[kk+kkk][ii] );
                                _b = _mm256_load_ps( &b[kk+kkk][jj] );

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
    }
}

#include <vector>
#include <thread>
#include "thread_safe_queue.hpp"

template<typename T>
void mul5( const Matrix<T>& a, const Matrix<T>& b, Matrix<T>& res, unsigned num_threads, unsigned block_width, unsigned block_depth )
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

    Tracer t( "mul5" );

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
                    for( unsigned kk=k; kk<k+block_depth && kk<a.cols(); kk+=val_align )
                    {
                        __m256 _a, _b;
                        for( unsigned kkk=0; kkk<val_align && kk+kkk<a.cols(); kkk++ )
                        {
                            // Load in column vector from a and row vector from b
                            _a = _mm256_load_ps( &a[kk+kkk][ii] );
                            _b = _mm256_load_ps( &b[kk+kkk][jj] );

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
