#include "b_plus.hpp"

#include <cassert>
#include <cstring>
#include <string>

B_Tree::Tree_Node::Tree_Node( B_Tree* _aPar, uint32_t _aNodeId, bool exists )
{
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

B_Tree::Tree_Node::~Tree_Node()
{
    _mPar->store_node( this, _mNodeId );
}

void B_Tree::Tree_Node::split( Node*& n )
{
    assert( _mSize == Tree_Node_Order );
    uint32_t id;
    Tree_Node*& ptr = _mPar->new_tree_node( id );
    n = ptr;

    _mSize = Leaf_Node_Order / 2;
    ptr->_mSize = Leaf_Node_Order - _mSize;

    memcpy( &ptr->_mChildNodes[ 0 ], &_mChildNodes[ _mSize ], sizeof( uint32_t ) * ptr->_mSize );
    memcpy( &ptr->_mKeys[ 0 ], &_mKeys[ _mSize ], sizeof( Key ) * ( ptr->_mSize - 1 ) );
}

B_Tree::Node* B_Tree::Tree_Node::find( const Key& k )
{
    uint32_t idx = index( k );
    return _mPar->unswizzle( _mChildNodes[ idx ] );
}

bool B_Tree::Tree_Node::find( const Key& k, Val& v )
{
    uint32_t idx = index( k );
    return _mPar->unswizzle( _mChildNodes[idx] )->find( k, v );
}

void B_Tree::Tree_Node::insert( const Key& k, const Val& v )
{
    uint32_t i;

    Node* ptr = find( k );

    if( ptr->size() == ptr->max_size() )
    {
        Node* n;
        ptr->split( n );

        //ptr->print();
        //n->print();

        insert( n );
        insert( k, v );
    }
    else
    {
        ptr->insert( k, v );
    }
}

void B_Tree::Tree_Node::insert( const Node* v )
{
    assert( _mSize != Tree_Node_Order );
    size_t split_index = index( v->largest() );

    if( v->largest() > _mPar->unswizzle( _mChildNodes[split_index] )->largest() )
    {
        split_index++;
    }

    {
        uint32_t* temp_nodes = new uint32_t[_mSize-split_index];

        memcpy( temp_nodes, &_mChildNodes[split_index], ( _mSize-split_index ) * sizeof( uint32_t ) );
        memcpy( &_mChildNodes[split_index+1], temp_nodes, ( _mSize-split_index ) * sizeof( uint32_t ) );
        delete [] temp_nodes;
        _mChildNodes[ split_index ] = v->node_id();
    }

    if( split_index )
    {
        Key* temp_keys = new Key[ _mSize - split_index ];

        memcpy( temp_keys, &_mKeys[ split_index - 1 ], ( _mSize - split_index ) * sizeof( Key ) );
        memcpy( &_mKeys[ split_index ], temp_keys, ( _mSize - split_index ) * sizeof( Key ) );
        delete [] temp_keys;
        _mKeys[ split_index - 1 ] = _mPar->unswizzle( _mChildNodes[ split_index ] )->smallest();
    }
    else
    {
        Key* temp_keys = new Key[ _mSize - 1 ];

        memcpy( temp_keys, &_mKeys[ 0 ], _mSize - 1 );
        memcpy( &_mKeys[ 1 ], temp_keys, _mSize - 1 );
        delete [] temp_keys;
        _mKeys[ 0 ] = _mPar->unswizzle( _mChildNodes[ 1 ] )->smallest();
    }

    _mSize++;
}

void B_Tree::Tree_Node::print( size_t depth ) const
{
    std::string s( depth, ' ' );
    std::cout << s << "Tree_Node" << std::endl;
    std::cout << s << "   ID: " << _mNodeId << std::endl;
    std::cout << s << "   Size: " << _mSize << std::endl;
    for( size_t i=0; i<_mSize; i++ )
    {
        _mPar->unswizzle( _mChildNodes[i] )->print( depth + 3 );
        if( i<_mSize-1 )
        {
            std::cout << s << "   Key" << std::endl;
            std::cout << s << "      " << std::hex << _mKeys[i] << std::endl;
        }
    }
}

uint32_t B_Tree::Tree_Node::index( Key k, uint32_t start, uint32_t end ) const
{
    if( start == end )
    {
        assert( k < _mKeys[start] );
        return start;
    }

    int mid = start + (end - start) / 2;

    if( k < _mKeys[ mid ] )
    {
        return index( k, start, mid );
    }
    else
    {
        return index( k, mid + 1, end );
    }
}

uint32_t B_Tree::Tree_Node::index( Key k ) const
{
    assert( _mSize );

    if( _mSize == 1 ) return 0;

    if( k >= _mKeys[ _mSize - 2 ] ) return _mSize - 1;

    return index( k, 0, _mSize - 2 );
}
