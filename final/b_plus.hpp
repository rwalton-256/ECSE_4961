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

    static constexpr uint32_t Tree_Node_Order = 320;
    static constexpr uint32_t Leaf_Node_Order = 112;

    static constexpr uint32_t Max_Num_Tree_Nodes = 0x10000;
    static constexpr uint32_t Max_Num_Leaf_Nodes = 0x1000000;

    struct Val
    {
        uint8_t val[32];
    };
    typedef uint32_t Key;

    struct Page
    {
        uint8_t _raw_[ 4096 ];
    };

    struct Node
    {
        virtual void split( Node*& b ) = 0;
        virtual bool find( const Key& k, Val& v ) = 0;
        virtual void insert( const Key& k, const Val& v ) = 0;
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

        void split( Node*& b );
        bool find( const Key& k, Val& v );
        void insert( const Key& k, const Val& v );
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
        void insert( const Key& k, const Val& v );
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
            };
            Page _mPage;
        };
    };

    void insert( const Key& k, const Val& v );
    void print();
    bool find( const Key& k, Val& v ) { return unswizzle( _mHeader._mRootId )->find( k, v ); }

    // Member variables
    Tree_Node** _mTreeNodes;
    Header _mHeader;
    std::fstream _mTreeFile;
    Hash_Map<uint32_t, Leaf_Node*> _mLeafNodeMap;
    //

    Node* unswizzle( uint32_t node_id );
    Tree_Node*& new_tree_node( uint32_t& node_id );
    Leaf_Node*& new_leaf_node( uint32_t& node_id );
    void clamp_size( float f );

    Tracer _mTracer;

    void store_node( Tree_Node* n, uint32_t idx );
    void store_node( Leaf_Node* n, uint32_t idx );
    void fetch_node( Tree_Node* n, uint32_t idx );
    void fetch_node( Leaf_Node* n, uint32_t idx );

    B_Tree( std::string file_name );
    ~B_Tree();

    static constexpr std::streamoff Tree_Node_Offset = 1 * sizeof( Header::_mPage );
    static constexpr std::streamoff Leaf_Node_Offset = Tree_Node_Offset + Max_Num_Tree_Nodes * sizeof( Tree_Node::_mPage );
};

std::ostream& operator<<( std::ostream& os, const B_Tree::Page& p );
