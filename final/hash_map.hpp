#include <string>
#include <cstring>
#include <unordered_map>
#include <cassert>

template <typename Key, typename Val>
class Hash_Map
{
    public:
    struct Hash_Table_Data
    {
        bool exists;
        Key key;
        Val val;
    };

    Hash_Table_Data* _mMapTable;
    unsigned _mTableSize, _mCurrSize;

    Hash_Map( unsigned _aLength )
        : _mTableSize( _aLength ),
          _mCurrSize( 0 )
    {
        _mMapTable = new Hash_Table_Data[ _mTableSize ];
        memset( _mMapTable, 0, sizeof( Hash_Table_Data ) * _mTableSize );
    }

    unsigned calc_index( const Key& k )
    {
        return std::hash<Key>{}( k * ( k + 1 ) * ( k + 2 ) ) % _mTableSize;
    }
    Hash_Table_Data& find( const Key& k )
    {
        Hash_Table_Data* ptr = _mMapTable + calc_index( k );

        while( 1 )
        {
            if( ( ptr->exists && ptr->key == k ) || ( !ptr->exists ) )
            {
                return *ptr;
            }
            ptr++;
            if( ptr - _mMapTable >= _mTableSize )
            {
                ptr = _mMapTable;
            }
        }
    }
    void insert( const Key& k, const Val& v )
    {
        Hash_Table_Data& d = find( k );
        d.exists = true;
        d.key = k;
        d.val = v;
        _mCurrSize++;
    }
    Val& at( const Key& k )
    {
        return find( k ).val;
    }
    void clear()
    {
        memset( _mMapTable, 0, sizeof( Hash_Table_Data ) * _mTableSize );
        _mCurrSize = 0;
    }

    void remove( const Key& k )
    {
        Hash_Table_Data* ptr = &find( k );
        if( !ptr->exists )
        {
            std::cout << "error " << k << std::endl;
        }
        if( ptr->exists )
        {
            ptr->exists = false;
            delete ptr->val;
            _mCurrSize--;

            while( 1 )
            {
                ptr++;
                if( ptr - _mMapTable >= _mTableSize ) ptr = _mMapTable;

                if( ! ptr->exists )
                {
                    break;
                }

                Hash_Table_Data d;
                memcpy( &d, ptr, sizeof( Hash_Table_Data ) );
                ptr->exists = false;
                _mCurrSize--;
                insert( ptr->key, ptr->val );
            }
        }
    }
    void verify()
    {
        for( unsigned i=0; i<_mTableSize; i++ )
        {
            if( _mMapTable[ i ].exists )
            {
                unsigned index = calc_index( _mMapTable[ i ].key );
                Hash_Table_Data* ptr = &_mMapTable[ i ];
                while( 1 )
                {
                    assert( ptr->exists );
                    if( ( ptr - _mMapTable ) == index ) break;
                    ptr--;
                    if( ptr - _mMapTable < 0 ) ptr = _mMapTable + _mTableSize - 1;
                }
            }
        }
    }
    ~Hash_Map()
    {
        for( uint32_t i=0; i<_mTableSize; i++ )
        {
            if( _mMapTable[i].exists )
            {
                delete _mMapTable[i].val;
            }
        }
    }
};

#include <iostream>
#include <iomanip>
template <typename Key, typename Val>
std::ostream& operator<<( std::ostream& os, const Hash_Map<Key,Val>& map )
{
    os << "Hash Map:" << std::endl;
    for( size_t i=0; i<map._mTableSize; i++ )
    {
        os << "   " << std::setw( 4 ) << std::hex << i;
        if( map._mMapTable[ i ].exists )
        {
            os << " " << map._mMapTable[ i ].key << std::flush << " " << map._mMapTable[ i ].val->_mNodeId << " " << map._mMapTable[ i ].val->_mInUse;
        }
        os << std::endl;
    }
    os << std::endl;
    return os;
}
