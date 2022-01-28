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
    inline T at( unsigned row, unsigned col ) const;
    /**
     * @brief Returns reference to selected cell
     * 
     * @param row 
     * @param col 
     * @return float& 
     */
    inline T& at( unsigned row, unsigned col );
    /**
     * @brief Returns pointer to major vector pointer at index. Based on
     * storage method, this will either return a row or a column vector
     * 
     * @param idx 
     * @return float& 
     */
    inline T*& operator[]( unsigned idx ) { return _mMajorDimVectPtr[ idx ]; }
    /**
     * @brief Returns pointer to major vector pointer at index. Based on
     * storage method, this will either return a row or a column vector
     * 
     * @param idx 
     * @return float& 
     */
    inline const T* operator[]( unsigned idx ) const { return _mMajorDimVectPtr[ idx ]; }
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

bool operator==( const Matrix<float>& a, const Matrix<float>& b );
bool operator==( const Matrix<double>& a, const Matrix<double>& b );
bool operator==( const Matrix<int16_t>& a, const Matrix<int16_t>& b );
bool operator==( const Matrix<int32_t>& a, const Matrix<int32_t>& b );

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
 * @param a 
 * @param b 
 * @param res 
 */
void fmul2( const Matrix<float>& a, const Matrix<float>& b, Matrix<float>& res );

/**
 * @brief 
 * 
 * @param a 
 * @param b 
 * @param res 
 * @param block_width 
 */
void fmul3( const Matrix<float>& a, const Matrix<float>& b, Matrix<float>& res, unsigned block_width=344  );

/**
 * @brief 
 * 
 * @param a 
 * @param b 
 * @param res 
 * @param block_width 
 */
void fmul4( const Matrix<float>& a, const Matrix<float>& b, Matrix<float>& res, unsigned block_width=344  );

/**
 * @brief 
 * 
 * @param a 
 * @param b 
 * @param res 
 * @param num_threads 
 * @param block_width 
 */
void fmul5( const Matrix<float>& a, const Matrix<float>& b, Matrix<float>& res, unsigned num_threads, unsigned block_width=200, unsigned block_depth=200 );

/**
 * @brief 
 * 
 * @param a 
 * @param b 
 * @param res 
 * @param num_threads 
 * @param block_width 
 */
void dmul5( const Matrix<double>& a, const Matrix<double>& b, Matrix<double>& res, unsigned num_threads, unsigned block_width=200, unsigned block_depth=200 );

/**
 * @brief 
 * 
 * @param a 
 * @param b 
 * @param res 
 * @param num_threads 
 * @param block_width 
 */
void i16mul5( const Matrix<int16_t>& a, const Matrix<int16_t>& b, Matrix<int16_t>& res, unsigned num_threads, unsigned block_width=200, unsigned block_depth=200 );

/**
 * @brief 
 * 
 * @param a 
 * @param b 
 * @param res 
 * @param num_threads 
 * @param block_width 
 */
void i32mul5( const Matrix<int32_t>& a, const Matrix<int32_t>& b, Matrix<int32_t>& res, unsigned num_threads, unsigned block_width=200, unsigned block_depth=200 );


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
inline T Matrix<T>::at( unsigned row, unsigned col ) const
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
inline T& Matrix<T>::at( unsigned row, unsigned col )
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

    res.initialize( 0.0F );
    for( unsigned i=0; i<res.rows(); i++ )
    {
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

    res.initialize( 0.0F );
    for( unsigned i=0; i<res.rows(); i+=block_width )
    {
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
