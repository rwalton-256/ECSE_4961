#include <string>
#include <cstring>
#include <unordered_map>

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

    Hash_Table_Data& find( const Key& k )
    {
        unsigned index = std::hash<std::string>{}( k ) % _mTableSize;

        Hash_Table_Data* ptr = _mMapTable + index;

        while( 1 )
        {
            if( ( ptr->exists && ptr->key == k ) || ( !ptr->exists ) )
            {
                return *ptr;
            }
            ptr++;
            if( ptr - _mMapTable >= _mTableSize )
            {
                ptr == _mMapTable;
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
};
