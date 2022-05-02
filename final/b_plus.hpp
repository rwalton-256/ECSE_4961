#include <stdint.h>
#include <map>
#include <iostream>
#include <functional>
#include <fstream>
#include <string>
#include <mutex>
#include <atomic>

#include "hash_map.hpp"
#include "tracer.hpp"

class B_Tree
{
    public:

    static constexpr uint32_t Tree_Node_Order = 25;
    static constexpr uint32_t Leaf_Node_Order = 25;
    static constexpr uint32_t Log_Size = 15;

    static constexpr uint32_t Max_Num_Tree_Nodes = 10;
    static constexpr uint32_t Max_Num_Leaf_Nodes = 100;
    static constexpr uint32_t Max_Num_Txns = 100;

    struct Val
    {
        uint8_t val[32];
    };
    typedef uint32_t Key;

    struct Page
    {
        uint8_t _raw_[ 1024 ];
    };

    typedef uint32_t Transaction_ID;

    struct Node
    {
        virtual void split( Node*& b ) = 0;
        virtual bool find( const Key& k, Val& v ) = 0;
        virtual void insert( const Key& k, const Val& v, Transaction_ID t ) = 0;
        virtual Key largest() const = 0;
        virtual Key smallest() const = 0;
        virtual uint32_t size() const  = 0;
        virtual uint32_t max_size() const = 0;
        virtual void print( size_t depth = 0 ) const = 0;
        virtual uint32_t index( Key k ) const = 0;
        virtual uint32_t node_id() const = 0;

        B_Tree* _mPar;
        std::atomic<uint32_t> _mInUse;
        std::atomic<bool> _mDirty;
    };

    struct Leaf_Node : Node
    {
        union
        {
            struct
            {
                uint32_t _mSize;
                uint32_t _mNodeId;
                Key _mKeys[ Leaf_Node_Order ];
                Val _mVals[ Leaf_Node_Order ];
            };
            Page _mPage;
        };
        union
        {
            struct
            {
                uint32_t _mLogSize;
                Transaction_ID _mLogTxns[ Log_Size ];
                Key            _mLogKeys[ Log_Size ];
                Val            _mLogVals[ Log_Size ];
            };
            Page _mLogPage;
        };

        void split( Node*& b );
        bool find( const Key& k, Val& v );
        void insert( const Key& k, const Val& v, Transaction_ID t );
        Key largest() const { return _mKeys[ _mSize-1 ]; }
        Key smallest() const { return _mKeys[ 0 ]; }
        uint32_t size() const  { return _mSize; }
        uint32_t max_size() const { return Leaf_Node_Order; };
        void print( size_t depth = 0 ) const;
        uint32_t index( Key k, uint32_t start, uint32_t end ) const;
        uint32_t index( Key k ) const;
        uint32_t node_id() const { return _mNodeId; }

        Leaf_Node( B_Tree* _aPar, uint32_t _aNodeId, bool exists=false );
        ~Leaf_Node();
    };

    struct Tree_Node : Node
    {
        union
        {
            struct
            {
                uint32_t _mSize;
                uint32_t _mNodeId;
                uint32_t _mChildNodes[ Tree_Node_Order ];
                Key      _mKeys[ Tree_Node_Order - 1 ];
            };
            Page _mPage;
        };

        void split( Node*& b );
        bool find( const Key& k, Val& v );
        Node* find( const Key& k );
        void insert( const Key& k, const Val& v, Transaction_ID t );
        void insert( const Node* v );
        Key largest() const { return _mPar->unswizzle( _mChildNodes [ _mSize-1 ] )->largest(); }
        Key smallest() const { return _mPar->unswizzle( _mChildNodes[ 0 ] )->smallest(); }
        uint32_t size() const { return _mSize; }
        uint32_t max_size() const { return Tree_Node_Order; };
        void print( size_t depth = 0 ) const;
        void rekey();
        uint32_t index( Key k, uint32_t start, uint32_t end ) const;
        uint32_t index( Key k ) const;
        uint32_t node_id() const { return _mNodeId; }

        Tree_Node( B_Tree* _aPar, uint32_t _aNodeId, bool exists=false );
        ~Tree_Node();
    };

    struct Header
    {
        union
        {
            struct
            {
                uint32_t _mMaxTreeNodes;
                uint32_t _mMaxLeafNodes;
                uint32_t _mNumTreeNodes;
                uint32_t _mNumLeafNodes;
                uint32_t _mRootId;
                uint32_t _mRecentTransaction;
            };
            Page _mPage;
        };
    };

    struct Transactions
    {
        union
        {
            struct
            {
                uint32_t _mNumTransactions;
                Transaction_ID _mTransactions[100];
            };
            Page _mPage;
        };
    };

    void insert( const Key& k, const Val& v, Transaction_ID t );
    void print();
    bool find( const Key& k, Val& v ) { return unswizzle( _mHeader._mRootId )->find( k, v ); }

    // Member variables
    Tree_Node** _mTreeNodes;
    Header _mHeader;
    Transactions _mCurrTxns;
    Transactions _mAbortTxns;
    std::fstream _mTreeFile;
    Hash_Map<uint32_t, Leaf_Node*> _mLeafNodeMap;

    Node* unswizzle( uint32_t node_id );
    Tree_Node*& new_tree_node( uint32_t& node_id );
    Leaf_Node*& new_leaf_node( uint32_t& node_id );
    void clamp_size( float f );

    void store_node( Tree_Node* n, uint32_t idx );
    void store_node( Leaf_Node* n, uint32_t idx );
    void fetch_node( Tree_Node* n, uint32_t idx );
    void fetch_node( Leaf_Node* n, uint32_t idx );

    void remove_txn( Transactions& txns, Transaction_ID t );
    void add_txn( Transactions& txns, Transaction_ID t );

    void sync_header_txns();

    Transaction_ID new_txn();
    void txn_commit( Transaction_ID t );
    void txn_abort( Transaction_ID t );

    bool log_valid( Transaction_ID t );

    B_Tree( std::string file_name, bool reset=false );
    ~B_Tree();

    static constexpr std::streamoff Curr_Txns_Offset = 1 * sizeof( Header::_mPage );
    static constexpr std::streamoff Abort_Txns_Offset = Curr_Txns_Offset + sizeof( Transactions::_mPage );
    static constexpr std::streamoff Tree_Node_Offset = Abort_Txns_Offset + sizeof( Transactions::_mPage );
    static constexpr std::streamoff Leaf_Node_Offset = Tree_Node_Offset + Max_Num_Tree_Nodes * sizeof( Tree_Node::_mPage );
    static constexpr std::streamoff Leaf_Log_Offset = Leaf_Node_Offset + Max_Num_Leaf_Nodes * sizeof( Leaf_Node::_mPage );
    static constexpr std::streamoff File_Size = Leaf_Log_Offset + Max_Num_Leaf_Nodes * sizeof( Leaf_Node::_mLogPage );
};

std::ostream& operator<<( std::ostream& os, const B_Tree::Page& p );
