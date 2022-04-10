#include <stdint.h>
#include <map>
#include <iostream>

class B_Tree
{
    public:

    static constexpr uint32_t Tree_Node_Order = 320;
    static constexpr uint32_t Leaf_Node_Order = 112;

    static constexpr uint32_t Max_Num_Tree_Nodes = 10000;
    static constexpr uint32_t Max_Num_Leaf_Nodes = 1000000;

    struct Val
    {
        uint8_t val[32];
    };
    typedef uint32_t Key;

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
    };

    struct Tree_Node : Node
    {
        union
        {
            struct
            {
                uint32_t _mSize;
                Node*    _mChildNodes[ Tree_Node_Order ];
                Key      _mKeys[ Tree_Node_Order - 1 ];
            };
            uint8_t _raw_[ 0x1000 ];
        };

        void split( Node*& b );
        bool find( const Key& k, Val& v );
        Node* find( const Key& k );
        void insert( const Key& k, const Val& v );
        void insert( const Node* v );
        Key largest() const { return _mChildNodes[ _mSize-1 ]->largest(); }
        Key smallest() const { return _mChildNodes[ 0 ]->smallest(); }
        uint32_t size() const { return _mSize; }
        uint32_t max_size() const { return Tree_Node_Order; };
        void print( size_t depth = 0 ) const;
        void rekey();
        uint32_t index( Key k, uint32_t start, uint32_t end ) const;
        uint32_t index( Key k ) const;
    };

    struct Leaf_Node : Node
    {
        union
        {
            struct
            {
                uint32_t _mSize;
                Key _mKeys[ Leaf_Node_Order ];
                Val _mVals[ Leaf_Node_Order ];
            };
            uint8_t _raw_[ 0x1000 ];
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
    };

    void insert( const Key& k, const Val& v );
    void print() const;
    bool find( const Key& k, Val& v ) { return _mRoot->find( k, v ); }

    // Member variables
    Tree_Node* _mRoot;

    //

    B_Tree();
};
