#include "distr_log_db/b_plus.hpp"

#include <cassert>
#include <cstring>
#include <string>

/**
 * @brief Construct a new b tree::b tree object. Attempts to load data from
 * database located in file system, if it fails it will create a new database
 * 
 * @param file_name 
 * @param reset If set to true, file will be overwritten with new database
 */
B_Tree::B_Tree( std::string file_name, bool reset )
    : _mLeafNodeMap( 8 ),
      _mTreeFile( file_name, std::ios::in | std::ios::out | std::ios::binary )
{
    memset( &_mHeader._mPage, 0, sizeof( Header::_mPage ) );

    if( _mTreeFile.fail() )
    {
        _mTreeFile.open( file_name, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc );
    }
    _mTreeFile.seekg( 0, std::ios_base::end );
    std::streampos pos = _mTreeFile.tellg();

    if( pos != File_Size || reset )
    {
        _mTreeFile.close();
        _mTreeFile.open( file_name, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc );
        _mTreeFile.seekg( File_Size - 1, std::ios_base::beg );
        _mTreeFile.write( "", 1 );
        _mTreeFile.flush();
    }

    _mTreeFile.seekg( 0, std::ios::beg );
    _mTreeFile.read( (char*)&_mHeader._mPage, sizeof( Header::_mPage ) );

    _mTreeNodes = new Tree_Node*[ Max_Num_Tree_Nodes ];

    if( _mHeader._mMaxLeafNodes != Max_Num_Leaf_Nodes ||
        _mHeader._mMaxTreeNodes != Max_Num_Tree_Nodes ||
        !_mHeader._mNumLeafNodes ||
        !_mHeader._mNumTreeNodes                         )
    {
        memset( &_mHeader._mPage, 0, sizeof( Header::_mPage ) );
        memset( &_mCurrTxns._mPage, 0, sizeof( Transactions::_mPage ) );
        memset( &_mAbortTxns._mPage, 0, sizeof( Transactions::_mPage ) );

        _mHeader._mMaxLeafNodes = Max_Num_Leaf_Nodes;
        _mHeader._mMaxTreeNodes = Max_Num_Tree_Nodes;
        _mHeader._mNumLeafNodes = 0;
        _mHeader._mNumTreeNodes = 0;

        Tree_Node* temp_root = new_tree_node( _mHeader._mRootId );

        uint32_t leaf_node_id;
        Leaf_Node* temp_leaf = new_leaf_node( leaf_node_id );

        _mRoot()->_mChildNodes[0] = temp_leaf->_mNodeId;
        _mRoot()->_mSize = 1;
    }
    else
    {
        for( uint32_t i=0; i<_mCurrTxns._mNumTransactions; i++ )
        {
            add_txn( _mAbortTxns, _mCurrTxns._mTransactions[i] );
            memset( &_mCurrTxns._mPage, 0, sizeof( _mCurrTxns._mPage ) );
            sync_header_txns();
        }

        for( uint32_t i=0; i<_mHeader._mNumTreeNodes; i++ )
        {
            _mTreeNodes[ i ] = new Tree_Node( this, i, true );
        }
    }
}

/**
 * @brief Destroy the B_Tree object
 * 
 */
B_Tree::~B_Tree()
{
    _mTreeFile.seekp( 0, std::ios::beg );
    _mTreeFile.write( (char*)&_mHeader._mPage, sizeof( Header::_mPage ) );
    _mTreeFile.sync();

    for( uint32_t i=0; i<_mHeader._mNumTreeNodes; i++ )
    {
        delete _mTreeNodes[ i ];
    }
    delete [] _mTreeNodes;
}

/**
 * @brief Insert key value pair into database with current transaction number
 * 
 * @param k 
 * @param v 
 * @param t 
 */
void B_Tree::insert( const Key& k, const Val& v, Txn t )
{
    if( _mRoot()->_mSize == Tree_Node_Order )
    {
        Node* a;
        Node* b;

        a = _mRoot();
        a->split( b );

        Tree_Node* temp_root =  new_tree_node( _mHeader._mRootId );
        sync_header_txns();

        temp_root->_mChildNodes[0] = a->node_id();
        temp_root->_mChildNodes[1] = b->node_id();
        temp_root->_mSize = 2;

        temp_root->_mKeys[0] = unswizzle( _mRoot()->_mChildNodes[1] )->smallest();
    }

    _mRoot()->insert( k, v, t );
}

/**
 * @brief Prints the B_Tree
 * 
 */
void B_Tree::print()
{
    std::cout << "B_Tree" << std::endl;
    ( (Tree_Node*)unswizzle( _mHeader._mRootId ) )->print();
}

/**
 * @brief Translates node identification number into usable pointer to
 * memory where node is stored. If node doesn't exist in memory, it will
 * be pulled in from the file system.
 * 
 * @param node_id Node Identification Number
 * @return B_Tree::Node* Pointer to node location in memory
 */
B_Tree::Node* B_Tree::unswizzle( uint32_t node_id )
{
    assert( node_id < Max_Num_Leaf_Nodes + Max_Num_Tree_Nodes );
    if( node_id < Max_Num_Tree_Nodes )
    {
        return _mTreeNodes[ node_id ];
    }
    else
    {
        Hash_Map<uint32_t,Leaf_Node*>::Hash_Table_Data data = _mLeafNodeMap.find( node_id );
        if( data.exists )
        {
            return data.val;
        }
        else
        {
            Leaf_Node* n = new Leaf_Node( this, node_id, true );
            n->_mInUse++;
            _mLeafNodeMap.insert( node_id, n );
            clamp_size( 0.5F );
            n->_mInUse--;
            return n;
        }
    }
}

/**
 * @brief Contructs a tree node with next available ID, and returns
 * a pointer to the new node
 * 
 * @param node_id Reference to identification number to be set with
 * new node id
 * @return B_Tree::Tree_Node* Pointer to the new node
 */
B_Tree::Tree_Node* B_Tree::new_tree_node( uint32_t& node_id )
{
    assert( _mHeader._mNumTreeNodes != Max_Num_Tree_Nodes );
    node_id = _mHeader._mNumTreeNodes;
    _mHeader._mNumTreeNodes++;
    sync_header_txns();
    _mTreeNodes[ node_id ] = new Tree_Node( this, node_id );
    return _mTreeNodes[node_id];
}

/**
 * @brief Contructs a leaf node with next available ID, and returns
 * a pointer to the new node. The node is stored in the leaf node
 * hash table.
 * 
 * @param node_id Reference to identification number to be set with
 * new node id
 * @return B_Tree::Leaf_Node* Pointer to the new node
 */
B_Tree::Leaf_Node* B_Tree::new_leaf_node( uint32_t& node_id )
{
    assert( _mHeader._mNumLeafNodes != Max_Num_Leaf_Nodes );
    node_id = _mHeader._mNumLeafNodes + Max_Num_Tree_Nodes;
    _mHeader._mNumLeafNodes++;
    sync_header_txns();
    Leaf_Node* n = new Leaf_Node( this, node_id );
    n->_mInUse++;
    _mLeafNodeMap.insert( node_id, n );
    clamp_size( 0.5F );
    n->_mInUse--;
    return _mLeafNodeMap.find( node_id ).val;
}

/**
 * @brief Prints B_Tree recursively to provided std::ostream
 * 
 * @param os 
 * @param p 
 * @return std::ostream& 
 */
std::ostream& operator<<( std::ostream& os, const B_Tree::Page& p )
{
    std::cout << "Page:";
    for( size_t i=0; i<sizeof( p._raw_ ); i+=4 )
    {
        if( !( i%16 ) ) std::cout << std::endl << std::hex << std::setw(4) << std::setfill('0') << i << " ";
        std::cout << std::hex << std::setw(8) << std::setfill('0') << *(uint32_t*)&p._raw_[i] << " ";
    }
    std::cout << std::endl;
    return os;
}

/**
 * @brief Writes appropriate data to file system
 * 
 * @param n Pointer to tree node
 * @param idx 
 */
void B_Tree::store_node( Tree_Node* n, uint32_t idx )
{
    _mTreeFile.seekp( Tree_Node_Offset + idx * sizeof( Tree_Node::_mPage ), std::ios::beg );
    _mTreeFile.write( (char*)&n->_mPage, sizeof( Tree_Node::_mPage ) );
    _mTreeFile.sync();
}

void B_Tree::store_node( Leaf_Node* n, uint32_t idx )
{
    assert( idx >= Max_Num_Tree_Nodes );
    if( n->_mDataModified )
    {
        _mTreeFile.seekp( Leaf_Node_Offset + ( idx - Max_Num_Tree_Nodes ) * ( sizeof( Leaf_Node::_mStored ) + sizeof( Leaf_Node::_mLog ) ), std::ios::beg );
        _mTreeFile.write( (char*)&n->_mStored, sizeof( Leaf_Node::_mStored ) );
        _mTreeFile.write( (char*)&n->_mLog, sizeof( Leaf_Node::_mLog ) );
    }
    else
    {
        _mTreeFile.seekp( Leaf_Node_Offset + ( idx - Max_Num_Tree_Nodes ) * ( sizeof( Leaf_Node::_mStored ) + sizeof( Leaf_Node::_mLog ) ) + sizeof( Leaf_Node::_mStored ), std::ios::beg );
        _mTreeFile.write( (char*)&n->_mLog, sizeof( Leaf_Node::_mLog ) );
    }

    _mTreeFile.sync();
}

void B_Tree::fetch_node( Tree_Node* n, uint32_t idx )
{
    _mTreeFile.seekg( Tree_Node_Offset + idx * sizeof( Tree_Node::_mPage ), std::ios::beg );
    _mTreeFile.read( (char*)&n->_mPage, sizeof( Tree_Node::_mPage ) );
}

void B_Tree::fetch_node( Leaf_Node* n, uint32_t idx )
{
    assert( idx >= Max_Num_Tree_Nodes );
    _mTreeFile.seekg( Leaf_Node_Offset + ( idx - Max_Num_Tree_Nodes ) * ( sizeof( Leaf_Node::_mStored ) + sizeof( Leaf_Node::_mLog ) ), std::ios::beg );
    _mTreeFile.read( (char*)&n->_mStored, sizeof( Leaf_Node::_mStored ) );
    _mTreeFile.read( (char*)&n->_mLog, sizeof( Leaf_Node::_mLog ) );
}

void B_Tree::sync_header_txns()
{
    _mTreeFile.seekp( 0, std::ios::beg );
    _mTreeFile.write( (char*)&_mHeader._mPage, sizeof( Header::_mPage ) );

    _mTreeFile.seekp( Curr_Txns_Offset, std::ios::beg );
    _mTreeFile.write( (char*)&_mCurrTxns._mPage, sizeof( Transactions::_mPage ) );

    _mTreeFile.seekp( Abort_Txns_Offset, std::ios::beg );
    _mTreeFile.write( (char*)&_mAbortTxns._mPage, sizeof( Transactions::_mPage ) );

    _mTreeFile.sync();
}

void B_Tree::clamp_size( float f )
{
    assert( f > 0.0F && f < 1.0F );
    while( static_cast<float>( _mLeafNodeMap._mCurrSize ) / static_cast<float>( _mLeafNodeMap._mTableSize ) > f )
    {
        uint32_t i = rand() % _mLeafNodeMap._mTableSize;
        Hash_Map<uint32_t,Leaf_Node*>::Hash_Table_Data dat = _mLeafNodeMap._mMapTable[ i ];
        if( dat.exists && !_mLeafNodeMap._mMapTable[ i ].val->_mInUse )
        {
            _mLeafNodeMap.remove( dat.key );
        }
    }
}

void B_Tree::remove_txn( Transactions& txns, Txn t )
{
    for( uint32_t i=0; i<txns._mNumTransactions; i++ )
    {
        if( txns._mTransactions[ i ] == t )
        {
            Txn* txns_ptr = new Txn[ txns._mNumTransactions - i - 1 ];
            memcpy( txns_ptr, &txns._mTransactions[ i + 1 ], ( txns._mNumTransactions - i - 1 ) * sizeof( Txn ) );
            memcpy( &txns._mTransactions[ i ], txns_ptr, ( txns._mNumTransactions - i - 1 ) * sizeof( Txn ) );
            txns._mNumTransactions--;
            break;
        }
    }
}

void B_Tree::add_txn( Transactions& txns, Txn t )
{
    assert( txns._mNumTransactions != Max_Num_Txns );
    txns._mTransactions[ txns._mNumTransactions ] = t;
    txns._mNumTransactions++;
}

B_Tree::Txn B_Tree::new_txn()
{
    _mHeader._mRecentTransaction++;
    add_txn( _mCurrTxns, _mHeader._mRecentTransaction );
    sync_header_txns();
    return _mHeader._mRecentTransaction;
}

void B_Tree::txn_commit( Txn t )
{
    remove_txn( _mCurrTxns, t );
    sync_header_txns();
}

void B_Tree::txn_abort( Txn t )
{
    remove_txn( _mCurrTxns, t );
    add_txn( _mAbortTxns, t );
    sync_header_txns();
}

B_Tree::TxnState B_Tree::txn_state( Txn t )
{
    for( uint32_t i=0; i<_mAbortTxns._mNumTransactions; i++ )
    {
        if( _mAbortTxns._mTransactions[ i ] == t )
        {
            return TxnState_Aborted;
        }
    }
    for( uint32_t i=0; i<_mCurrTxns._mNumTransactions; i++ )
    {
        if( _mCurrTxns._mTransactions[ i ] == t )
        {
            return TxnState_Current;
        }
    }
    return TxnState_Committed;
}

B_Tree::Tree_Node* B_Tree::_mRoot()
{
    return (Tree_Node*)unswizzle( _mHeader._mRootId );
}
