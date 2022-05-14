#include "distr_log_db/b_plus.hpp"
#include "distr_log_db/coloring.hpp"

#include <cassert>
#include <cstring>
#include <string>

/****************************************************************************
*                            LEAF NODE
****************************************************************************/

B_Tree::Leaf_Node::Leaf_Node( B_Tree* _aPar, uint32_t _aNodeId, bool exists )
    : _mDataModified( 0 ),
      _mNodeId( _aNodeId )
{
    _mPar = _aPar;
    _mInUse = 0;
    _mDirty = true;
    memset( &_mStored, 0, sizeof( _mStored ) );
    memset( &_mCurrent, 0, sizeof( _mCurrent ) );
    memset( &_mLog, 0, sizeof( _mLog ) );

    if( exists )
    {
        _mDirty = false;
        _mPar->fetch_node( this, _aNodeId );
        memcpy( &_mCurrent, &_mStored, sizeof( _mStored ) );

        for( uint32_t i=0; i<_mLog._mSize; i++ )
        {
            KeyValTxn& kvt = _mLog._mKVTs[i];
            if( _mPar->txn_state( kvt.t ) == TxnState_Committed )
            {
                _mCurrent.insert( kvt.k, kvt.v );
            }
        }
    }
}

B_Tree::Leaf_Node::~Leaf_Node()
{
    if( _mDirty )
    {
        persist();
    }
}

void B_Tree::Leaf_Node::persist()
{
    _mPar->store_node( this, _mNodeId );
    _mDirty = false;
}

void B_Tree::Leaf_Node::split( Node*& n )
{
    assert( _mCurrent._mSize == Leaf_Node_Order );
    assert( _mNodeId );
    _mInUse++;
    _mDirty = true;

    uint32_t id;
    Leaf_Node* ptr = _mPar->new_leaf_node( id );
    n = ptr;

    _mCurrent._mSize = Leaf_Node_Order / 2;
    ptr->_mCurrent._mSize = Leaf_Node_Order - _mCurrent._mSize;
    memcpy( &ptr->_mCurrent._mKVs[ 0 ], &_mCurrent._mKVs[ _mCurrent._mSize ], sizeof( KeyVal ) * ptr->_mCurrent._mSize );
    memset( &_mCurrent._mKVs[ _mCurrent._mSize ], 0, ptr->_mCurrent._mSize * sizeof( KeyVal ) );

    uint32_t l_idx = _mLog.index( ptr->_mCurrent._mKVs[ 0 ].k );

    ptr->_mLog._mSize = _mLog._mSize - l_idx;
    _mLog._mSize = l_idx;

    memcpy( &ptr->_mLog._mKVTs[ 0 ], &_mLog._mKVTs[ l_idx ], ptr->_mLog._mSize * sizeof( KeyValTxn ) );
    memset( &_mLog._mKVTs[ _mLog._mSize], 0, ptr->_mLog._mSize * sizeof( KeyValTxn ) );

    uint32_t s_idx = _mStored.index( ptr->_mCurrent._mKVs[ 0 ].k );

    ptr->_mStored._mSize = _mStored._mSize - s_idx;
    _mStored._mSize = s_idx;

    memcpy( &ptr->_mStored._mKVs[ 0 ], &_mStored._mKVs[ s_idx ], ptr->_mStored._mSize * sizeof( KeyVal ) );
    memset( &_mStored._mKVs[ _mStored._mSize], 0, ptr->_mStored._mSize * sizeof( KeyVal ) );

    _mInUse--;
};

bool B_Tree::Leaf_Node::find( const Key& k, Val& v ) const
{
    return _mCurrent.find( k, v );
}

void B_Tree::Leaf_Node::insert( const Key& k, const Val& v, Txn t )
{
    _mDirty = true;
    _mCurrent.insert( k, v );
    if( _mLog._mSize == Log_Size )
    {
        for( uint32_t i=0; i<_mLog._mSize; i++ )
        {
            KeyValTxn& kvt = _mLog._mKVTs[ i ];
            switch( _mPar->txn_state( kvt.t ) )
            {
            case TxnState_Aborted:
                _mLog.remove( kvt.k );
                i--;
                break;
            case TxnState_Committed:
                _mDataModified = true;
                _mStored.insert( kvt.k, kvt.v );
                _mLog.remove( kvt.k );
                i--;
                break;
            case TxnState_Current:
                // Transaction is still current, we need to keep this in the log
                break;
            case TxnState_Invalid:
            default:
                assert( 0 );
                break;
            }
        }
    }
    _mLog.insert( k, v, t );
}

void B_Tree::Leaf_Node::print( size_t depth ) const
{
    std::string s( depth, ' ' );
    std::cout << s << "Leaf_Node" << std::endl;
    std::cout << s << "   ID: " << _mNodeId << std::endl;
    std::cout << s << "   Stored:" << std::endl;
    std::cout << s << "      Size: " << _mStored._mSize << std::endl;
    for( size_t i=0; i<_mStored._mSize; i++ )
    {
        switch( state( _mStored._mKVs[ i ].k ) )
        {
        case TxnState_Aborted:
            std::cout << Color::Yellow;
            break;
        case TxnState_Current:
            std::cout << Color::Blue;
            break;
        case TxnState_Committed:
            std::cout << Color::Green;
            break;
        case TxnState_Invalid:
            break;
        }
        std::cout << s << "      " << std::hex << std::setw(8) << std::setfill( '0' ) << _mStored._mKVs[ i ].k
                                   << "   \"" << (char*)_mStored._mKVs[ i ].v.val << "\"" << Color::Reset << std::endl;
    }
    std::cout << s << "   Current:" << std::endl;
    std::cout << s << "      Size: " << _mCurrent._mSize << std::endl;
    for( size_t i=0; i<_mCurrent._mSize; i++ )
    {
        switch( state( _mCurrent._mKVs[ i ].k ) )
        {
        case TxnState_Aborted:
            std::cout << Color::Yellow;
            break;
        case TxnState_Current:
            std::cout << Color::Blue;
            break;
        case TxnState_Committed:
            std::cout << Color::Green;
            break;
        case TxnState_Invalid:
            break;
        }
        std::cout << s << "      " << std::hex << std::setw(8) << std::setfill( '0' ) << _mCurrent._mKVs[ i ].k
                                   << "   \"" << (char*)_mCurrent._mKVs[ i ].v.val << "\"" << Color::Reset << std::endl;
    }
    std::cout << s << "   Log:" << std::endl;
    std::cout << s << "      Size: " << _mLog._mSize << std::endl;
    for( size_t i=0; i<_mLog._mSize; i++ )
    {
        switch( _mPar->txn_state( _mLog._mKVTs[ i ].t ) )
        {
        case TxnState_Aborted:
            std::cout << Color::Yellow;
            break;
        case TxnState_Current:
            std::cout << Color::Blue;
            break;
        case TxnState_Committed:
            std::cout << Color::Green;
            break;
        case TxnState_Invalid:
            break;
        }
        std::cout << s << "      " << std::hex << std::setw(8) << std::setfill( '0' ) << _mLog._mKVTs[ i ].k
                                   << "   \"" << (char*)_mLog._mKVTs[ i ].v.val << "\" " << _mLog._mKVTs[ i ].t << Color::Reset << std::endl;
    }
}

B_Tree::TxnState B_Tree::Leaf_Node::state( Key k ) const
{
    uint32_t idx = _mLog.index( k );
    if( _mLog._mKVTs[ idx ].k != k )
    {
        return TxnState_Invalid;
    }
    return _mPar->txn_state( _mLog._mKVTs[ idx ].t );
}

/****************************************************************************
*                                 DATA
****************************************************************************/

bool B_Tree::Leaf_Node::Data::find( const Key& k, Val& v ) const
{
    uint32_t idx = index( k );
    if( idx < _mSize && _mKVs[ idx ].k == k )
    {
        v = _mKVs[ idx ].v;
        return true;
    }
    return false;
}

void B_Tree::Leaf_Node::Data::insert( const Key& k, const Val& v )
{
    assert( _mSize != Leaf_Node_Order );

    uint32_t idx = index( k );

    if( k == _mKVs[ idx ].k )
    {
        // Entry already exists
        _mKVs[ idx ].v = v;
        return;
    }

    memmove( &_mKVs[ idx + 1 ], &_mKVs[ idx ], ( _mSize - idx ) * sizeof( KeyVal ) );

    _mKVs[ idx ].k = k;
    _mKVs[ idx ].v = v;

    _mSize++;
}

uint32_t B_Tree::Leaf_Node::Data::index( Key k, uint32_t start, uint32_t end ) const
{
    if( start == end ) return start;

    uint32_t mid = ( start + end ) / 2;

    if( k <= _mKVs[ mid ].k )
    {
        return index( k, start, mid );
    }
    else
    {
        return index( k, mid + 1, end );
    }
}

uint32_t B_Tree::Leaf_Node::Data::index( Key k ) const
{
    if( !_mSize ) return 0;
    // If target lies beyond the max element, than the index of strictly smaller
    // value than target should be (end - 1)
    if( k > _mKVs[ _mSize - 1].k ) return _mSize;

    return index( k, 0, _mSize -1 );
}

void B_Tree::Leaf_Node::Data::remove( Key k )
{
    uint32_t idx = index( k );

    if( k != _mKVs[ idx ].k )
    {
        // Entry doesn't exist
        return;
    }

    memmove( &_mKVs[ idx ], &_mKVs[ idx + 1 ], ( _mSize - idx ) * sizeof( KeyVal ) );
    memset( &_mKVs[ _mSize - 1 ], 0, sizeof( KeyVal ) );

    _mSize--;
}

/****************************************************************************
*                                 LOG
****************************************************************************/

void B_Tree::Leaf_Node::Log::insert( const Key& k, const Val& v, Txn t )
{
    assert( _mSize != Log_Size );

    uint32_t idx = index( k );

    if( k == _mKVTs[ idx ].k && t == _mKVTs[ idx ].t )
    {
        _mKVTs[ idx ].v = v;
        return;
    }

    // We cannot add a log to the same element for different transactions
    assert( k != _mKVTs[ idx ].k );

    memmove( &_mKVTs[ idx + 1 ], &_mKVTs[ idx ], ( _mSize - idx ) * sizeof( KeyValTxn ) );

    _mKVTs[ idx ].k = k;
    _mKVTs[ idx ].v = v;
    _mKVTs[ idx ].t = t;

    _mSize++;
}

uint32_t B_Tree::Leaf_Node::Log::index( Key k, uint32_t start, uint32_t end ) const
{
    if( start == end ) return start;

    uint32_t mid = ( start + end ) / 2;

    if( k <= _mKVTs[ mid ].k )
    {
        return index( k, start, mid );
    }
    else
    {
        return index( k, mid + 1, end );
    }
}

uint32_t B_Tree::Leaf_Node::Log::index( Key k ) const
{
    if( !_mSize ) return 0;
    // If target lies beyond the max element, than the index of strictly smaller
    // value than target should be (end - 1)
    if( k > _mKVTs[ _mSize - 1 ].k ) return _mSize;

    return index( k, 0, _mSize - 1 );
}

void B_Tree::Leaf_Node::Log::remove( Key k )
{
    uint32_t idx = index( k );

    if( k != _mKVTs[ idx ].k )
    {
        // Entry doesn't exist
        return;
    }

    memmove( &_mKVTs[ idx ], &_mKVTs[ idx + 1 ], ( _mSize - idx ) * sizeof( KeyValTxn ) );
    memset( &_mKVTs[ _mSize - 1 ], 0, sizeof( KeyValTxn ) );

    _mSize--;
}

