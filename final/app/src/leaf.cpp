#include "distr_log_db/b_plus.hpp"

#include <cassert>
#include <cstring>
#include <string>

B_Tree::Leaf_Node::Leaf_Node( B_Tree* _aPar, uint32_t _aNodeId, bool exists )
    : _mDataModified( 0 ),
      _mNodeId( _aNodeId )
{
    _mPar = _aPar;
    _mInUse = 0;
    _mDirty = false;
    memset( &_mStored, 0, sizeof( _mStored ) );
    memset( &_mCurrent, 0, sizeof( _mCurrent ) );
    memset( &_mLog, 0, sizeof( _mLog ) );

    if( exists )
    {
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
    snprintf( _mStored.foo, sizeof( _mStored.foo ), "\nLeaf Data: %02x\n", _mNodeId );
    snprintf( _mLog.foo, sizeof( _mStored.foo ), "\nLeaf Log: %02x\n", _mNodeId );
    _mPar->store_node( this, _mNodeId );
}

void B_Tree::Leaf_Node::split( Node*& n )
{
    assert( _mCurrent._mSize == Leaf_Node_Order );
    assert( _mNodeId );
    _mInUse++;

    uint32_t id;
    Leaf_Node*& ptr = _mPar->new_leaf_node( id );
    ptr = new Leaf_Node( _mPar, id );
    n = ptr;

    _mCurrent._mSize = Leaf_Node_Order / 2;
    ptr->_mCurrent._mSize = Leaf_Node_Order - _mCurrent._mSize;
    memcpy( &ptr->_mCurrent._mKVs[ 0 ], &_mCurrent._mKVs[ _mCurrent._mSize ], sizeof( KeyVal ) * ptr->_mCurrent._mSize );
    memset( &_mCurrent._mKVs[ _mCurrent._mSize ], 0, ptr->_mCurrent._mSize );

    // *********** IMPLEMENT LOG SPLITTING *****************

    _mInUse--;
};

bool B_Tree::Leaf_Node::Data::find( const Key& k, Val& v )
{
    uint32_t idx = index( k );
    if( idx < _mSize && _mKVs[ idx ].k == k )
    {
        v = _mKVs[ idx ].v;
        return true;
    }
    return false;
}

bool B_Tree::Leaf_Node::find( const Key& k, Val& v )
{
    return _mCurrent.find( k, v );
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

void B_Tree::Leaf_Node::insert( const Key& k, const Val& v, Txn t )
{
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
    std::cout << s << "   Size: " << _mCurrent._mSize << std::endl;
    for( size_t i=0; i<_mCurrent._mSize; i++ )
    {
        std::cout << s << "   " << std::hex << std::setw(8) << std::setfill( '0' ) << _mCurrent._mKVs[ i ].k
                                << "   \"" << (char*)_mCurrent._mKVs[ i ].v.val << "\"" << std::endl;
    }
    std::cout << s << "   LogSize: " << _mLog._mSize << std::endl;
    for( size_t i=0; i<_mLog._mSize; i++ )
    {
        std::cout << s << "   " << std::hex << std::setw(8) << std::setfill( '0' ) << _mLog._mKVTs[ i ].k
                                << "   \"" << (char*)_mLog._mKVTs[ i ].v.val << "\" " << _mLog._mKVTs[ i ].t << std::endl;
    }
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

