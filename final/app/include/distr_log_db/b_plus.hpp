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

    static constexpr uint32_t Tree_Node_Order = 10;
    static constexpr uint32_t Leaf_Node_Order = 10;
    static constexpr uint32_t Log_Size = 5;

    static constexpr uint32_t Max_Num_Tree_Nodes = 10;
    static constexpr uint32_t Max_Num_Leaf_Nodes = 100;
    static constexpr uint32_t Max_Num_Txns = 100;

    struct Val
    {
        uint8_t val[16];
        Val& operator=( const Val& a )
        {
            memcpy( this, &a, sizeof( Val ) );
            return *this;
        }
    };
    typedef uint32_t Key;
    typedef uint32_t Txn;

    struct KeyVal
    {
        Key k;
        Val v;
    };

    struct KeyValTxn
    {
        Key k;
        Val v;
        Txn t;
    };

    enum TxnState
    {
        TxnState_Invalid,
        TxnState_Current,
        TxnState_Aborted,
        TxnState_Committed,
    };

    struct Page
    {
        uint8_t _raw_[ 1024 ];
    };

    struct Node
    {
        virtual void split( Node*& b ) = 0;
        virtual bool find( const Key& k, Val& v ) const = 0;
        virtual void insert( const Key& k, const Val& v, Txn t ) = 0;
        virtual Key largest() const = 0;
        virtual Key smallest() const = 0;
        virtual uint32_t size() const  = 0;
        virtual uint32_t max_size() const = 0;
        virtual void print( size_t depth = 0 ) const = 0;
        virtual uint32_t node_id() const = 0;

        B_Tree* _mPar;
        std::atomic<uint32_t> _mInUse;
        std::atomic<bool> _mDirty;
    };

    struct Leaf_Node : Node
    {
        struct Data
        {
            union
            {
                struct
                {
                    char foo[24];
                    uint32_t _mSize;
                    KeyVal _mKVs[ Leaf_Node_Order ];
                };
                Page _mPage;
            };
            void insert( const Key& k, const Val& v );
            bool find( const Key& k, Val& v ) const;
            uint32_t index( Key k, uint32_t start, uint32_t end ) const;
            uint32_t index( Key k ) const;
            void remove( Key k );
        };
        struct Log
        {
            union
            {
                struct
                {
                    char foo[24];
                    uint32_t _mSize;
                    KeyValTxn _mKVTs[ Log_Size ];
                };
                Page _mPage;
            };
            void insert( const Key& k, const Val& v, Txn t );
            uint32_t index( Key k, uint32_t start, uint32_t end ) const;
            uint32_t index( Key k ) const;
            void remove( Key k );
        };

        Data _mStored, _mCurrent;
        Log _mLog;

        uint32_t _mNodeId;
        bool _mDataModified;

        void split( Node*& b );
        void shorten_log();
        void insert( const Key& k, const Val& v, Txn t );
        void persist();

        bool find( const Key& k, Val& v ) const;
        Key largest() const { return _mCurrent._mKVs[ _mCurrent._mSize-1 ].k; }
        Key smallest() const { return _mCurrent._mKVs[ 0 ].k; }
        uint32_t size() const  { return _mCurrent._mSize; }
        uint32_t max_size() const { return Leaf_Node_Order; };
        void print( size_t depth = 0 ) const;
        uint32_t node_id() const { return _mNodeId; }
        TxnState state( Key k ) const;

        Leaf_Node( B_Tree* _aPar, uint32_t _aNodeId, bool exists=false );
        ~Leaf_Node();
    };

    struct Tree_Node : Node
    {
        union
        {
            struct
            {
                char foo[24];
                uint32_t _mSize;
                uint32_t _mNodeId;
                uint32_t _mChildNodes[ Tree_Node_Order ];
                Key      _mKeys[ Tree_Node_Order - 1 ];
            };
            Page _mPage;
        };

        void split( Node*& b );
        bool find( const Key& k, Val& v ) const;
        Node* find( const Key& k ) const;
        void insert( const Key& k, const Val& v, Txn t );
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
                Txn _mTransactions[100];
            };
            Page _mPage;
        };
    };

    void insert( const Key& k, const Val& v, Txn t );
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
    Tree_Node* new_tree_node( uint32_t& node_id );
    Leaf_Node* new_leaf_node( uint32_t& node_id );
    void clamp_size( float f );

    void store_node( Tree_Node* n, uint32_t idx );
    void store_node( Leaf_Node* n, uint32_t idx );
    void fetch_node( Tree_Node* n, uint32_t idx );
    void fetch_node( Leaf_Node* n, uint32_t idx );

    void remove_txn( Transactions& txns, Txn t );
    void add_txn( Transactions& txns, Txn t );

    void sync_header_txns();

    Txn new_txn();
    void txn_commit( Txn t );
    void txn_abort( Txn t );

    TxnState txn_state( Txn t );

    Tree_Node* _mRoot();

    B_Tree( std::string file_name, bool reset=false );
    ~B_Tree();

    static constexpr std::streamoff Curr_Txns_Offset = 1 * sizeof( Header::_mPage );
    static constexpr std::streamoff Abort_Txns_Offset = Curr_Txns_Offset + sizeof( Transactions::_mPage );
    static constexpr std::streamoff Tree_Node_Offset = Abort_Txns_Offset + sizeof( Transactions::_mPage );
    static constexpr std::streamoff Leaf_Node_Offset = Tree_Node_Offset + Max_Num_Tree_Nodes * sizeof( Tree_Node::_mPage );
    static constexpr std::streamoff File_Size = Leaf_Node_Offset + Max_Num_Leaf_Nodes * ( sizeof( Leaf_Node::_mLog ) + sizeof( Leaf_Node::_mStored ) );
};

std::ostream& operator<<( std::ostream& os, const B_Tree::Page& p );
