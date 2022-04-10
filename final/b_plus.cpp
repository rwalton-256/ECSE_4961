#include "b_plus.hpp"

#include <cassert>
#include <cstring>
#include <string>

void B_Tree::Leaf_Node::split( Node*& n )
{
    assert( _mSize == Leaf_Node_Order );

    Leaf_Node* ptr = new Leaf_Node;
    n = ptr;

    _mSize = Leaf_Node_Order / 2;

    ptr->_mSize = Leaf_Node_Order - _mSize;
    memcpy( &ptr->_mKeys[0], &_mKeys[_mSize], sizeof( Key ) * ptr->_mSize );
    memcpy( &ptr->_mVals[0], &_mVals[_mSize], sizeof( Val ) * ptr->_mSize );
};

void B_Tree::Tree_Node::split( Node*& n )
{
    assert( _mSize == Tree_Node_Order );

    Tree_Node* ptr = new Tree_Node;
    n = ptr;

    _mSize = Leaf_Node_Order / 2;
    ptr->_mSize = Leaf_Node_Order - _mSize;

    memcpy( &ptr->_mChildNodes[0], &_mChildNodes[_mSize], sizeof( Node*) * ptr->_mSize );

    rekey();
    ptr->rekey();
}

B_Tree::Node* B_Tree::Tree_Node::find( const Key& k )
{
    uint32_t idx = index( k );
    return _mChildNodes[ idx ];
    uint32_t i;
    if( !_mSize ) return nullptr;
    for( i=0; i<_mSize-1; i++ )
    {
        if( k < _mKeys[i] )
        {
            return _mChildNodes[i];
        }
    }
    return _mChildNodes[i];
}

bool B_Tree::Tree_Node::find( const Key& k, Val& v )
{
    uint32_t idx = index( k );

    return _mChildNodes[idx]->find( k, v );
    uint32_t i;
    if( !_mSize ) return false;
    for( i=0; i<_mSize-1; i++ )
    {
        if( k < _mKeys[i] )
        {
            return _mChildNodes[i]->find( k, v );
        }
    }
    return _mChildNodes[i]->find( k, v );
}

bool B_Tree::Leaf_Node::find( const Key& k, Val& v )
{
    uint32_t idx = index( k );
    if( idx != 0xffffffff )
    {
        memcpy( &v, &_mVals[ idx ], sizeof( Val ) );
    }
    return idx != 0xffffffff;
}

void B_Tree::insert( const Key& k, const Val& v )
{
    if( _mRoot->_mSize == Tree_Node_Order )
    {
        Node* a;
        Node* b;

        _mRoot->split( b );
        a = _mRoot;

        _mRoot = new Tree_Node;

        _mRoot->insert( a );
        _mRoot->insert( b );
    }

    _mRoot->insert( k, v );
}

void B_Tree::Tree_Node::insert( const Key& k, const Val& v )
{
    uint32_t i;

    Node* ptr = find( k );

    if( ptr->size() == ptr->max_size() )
    {
        Node* n;
        ptr->split( n );

        rekey();

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

    size_t split_index = 0;
    for( size_t i=0; i<_mSize; i++ )
    {
        // Comparison of the last nodes, key will not help comparison
        if( _mSize - 1 == i )
        {
            if( v->largest() > _mChildNodes[i]->largest() )
            {
                split_index = _mSize;
            }
            else
            {
                split_index = _mSize - 1;
            }
            break;
        }
        if( v->largest() < _mChildNodes[i]->largest() )
        {
            split_index = i;
            break;
        }
    }

    Node** temp_nodes = new Node*[_mSize-split_index];

    memcpy( temp_nodes, &_mChildNodes[split_index], ( _mSize-split_index ) * sizeof( Node* ) );
    memcpy( &_mChildNodes[split_index+1], temp_nodes, ( _mSize-split_index ) * sizeof( Node* ) );
    memcpy( &_mChildNodes[split_index], &v, sizeof( Node* ) );

    delete [] temp_nodes;

    _mSize++;
    rekey();
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

void B_Tree::print() const
{
    std::cout << "B_Tree" << std::endl;
    _mRoot->print();
}

void B_Tree::Tree_Node::print( size_t depth ) const
{
    std::string s( depth, ' ' );
    std::cout << s << "Tree_Node" << std::endl;
    std::cout << s << "   Size: " << _mSize << std::endl;
    for( size_t i=0; i<_mSize; i++ )
    {
        _mChildNodes[i]->print( depth + 3 );
        if( i<_mSize-1 )
        {
            std::cout << s << "   Key" << std::endl;
            std::cout << s << "      " << std::hex << _mKeys[i] << std::endl;
        }
    }
}

void B_Tree::Leaf_Node::print( size_t depth ) const
{
    std::string s( depth, ' ' );
    std::cout << s << "Leaf_Node" << std::endl;
    std::cout << s << "   Size: " << _mSize << std::endl;
    for( size_t i=0; i<_mSize; i++ )
    {
        std::cout << s << "   " << std::hex << _mKeys[i] << std::endl;
    }
}

void B_Tree::Tree_Node::rekey()
{
    if( !_mSize ) return;
    for( size_t i=0; i<_mSize-1; i++ )
    {
        _mKeys[i] = _mChildNodes[i+1]->smallest();
    }
}

B_Tree::B_Tree()
{
    _mRoot = new Tree_Node;

    Leaf_Node* temp_leaf = new Leaf_Node;

    _mRoot->_mChildNodes[0] = temp_leaf;
    _mRoot->_mSize = 1;

    temp_leaf->_mSize = 0;
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
