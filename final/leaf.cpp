#include "b_plus.hpp"

#include <cassert>
#include <cstring>
#include <string>

B_Tree::Leaf_Node::Leaf_Node( B_Tree* _aPar, uint32_t _aNodeId, bool exists )
{
    _mInUse = 0;
    _mPar = _aPar;
    if( exists )
    {
        _mPar->fetch_node( this, _aNodeId );
    }
    else
    {
        _mSize = 0;
        _mNodeId = _aNodeId;
    }
}

B_Tree::Leaf_Node::~Leaf_Node()
{
    _mPar->store_node( this, _mNodeId );
}

void B_Tree::Leaf_Node::split( Node*& n )
{
    assert( _mSize == Leaf_Node_Order );
    assert( _mNodeId );
    _mInUse++;

    uint32_t id;
    Leaf_Node*& ptr = _mPar->new_leaf_node( id );
    ptr = new Leaf_Node( _mPar, id );
    n = ptr;

    _mSize = Leaf_Node_Order / 2;

    ptr->_mSize = Leaf_Node_Order - _mSize;
    memcpy( &ptr->_mKeys[0], &_mKeys[_mSize], sizeof( Key ) * ptr->_mSize );
    memcpy( &ptr->_mVals[0], &_mVals[_mSize], sizeof( Val ) * ptr->_mSize );
    _mInUse--;
};

bool B_Tree::Leaf_Node::find( const Key& k, Val& v )
{
    uint32_t idx = index( k );
    if( idx != 0xffffffff )
    {
        memcpy( &v, &_mVals[ idx ], sizeof( Val ) );
    }
    return idx != 0xffffffff;
}

void B_Tree::Leaf_Node::insert( const Key& k, const Val& v )
{
    assert( _mSize != Leaf_Node_Order );

    size_t split_index = _mSize;
    for( size_t i=0; i<_mSize; i++ )
    {
        if( k == _mKeys[i] )
        {
            memcpy( &_mVals[i], &v, sizeof( Val ) );
            return;
        }
        if( k < _mKeys[i] )
        {
            split_index = i;
            break;
        }
    }

    Key* temp_keys = new Key[_mSize-split_index];
    Val* temp_vals = new Val[_mSize-split_index];

    memcpy( temp_keys, &_mKeys[split_index], ( _mSize-split_index ) * sizeof( Key ) );
    memcpy( &_mKeys[split_index+1], temp_keys, ( _mSize-split_index ) * sizeof( Key ) );

    memcpy( temp_vals, &_mVals[split_index], ( _mSize-split_index ) * sizeof( Val ) );
    memcpy( &_mVals[split_index+1], temp_vals, ( _mSize-split_index ) * sizeof( Val ) );

    delete [] temp_keys;
    delete [] temp_vals;

    memcpy( &_mKeys[split_index], &k, sizeof( Key ) );
    memcpy( &_mVals[split_index], &v, sizeof( Val ) );

    _mSize++;
}

void B_Tree::Leaf_Node::print( size_t depth ) const
{
    std::string s( depth, ' ' );
    std::cout << s << "Leaf_Node" << std::endl;
    std::cout << s << "   ID: " << _mNodeId << std::endl;
    std::cout << s << "   Size: " << _mSize << std::endl;
    for( size_t i=0; i<_mSize; i++ )
    {
        std::cout << s << "   " << std::hex << _mKeys[i] << std::endl;
    }
}

uint32_t B_Tree::Leaf_Node::index( Key k, uint32_t start, uint32_t end ) const
{
    if( start > end ) return 0xffffffff;
    if( start == end ) return _mKeys[ start ] == k ? start : 0xffffffff;

    uint32_t mid = ( start + end ) / 2;

    if( k < _mKeys[mid] )
    {
        return index( k, start, mid );
    }
    else
    {
        return index( k, mid + 1, end );
    }
}

uint32_t B_Tree::Leaf_Node::index( Key k ) const
{
    assert( _mSize );
    // If target lies beyond the max element, than the index of strictly smaller
    // value than target should be (end - 1)
    if( k > _mKeys[_mSize - 1] ) return 0xffffffff;

    return index( k, 0, _mSize -1 );
}


