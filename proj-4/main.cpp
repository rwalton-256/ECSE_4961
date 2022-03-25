#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cstring>
#include <iomanip>
#include <vector>
#include "hash_map.hpp"
#include "tracer.hpp"

class Column_Data
{
    public:
    // Maps column data string to 32-bit unique value
    Hash_Map<std::string, unsigned> encode_map;
    // Maps 32-bit unique value (index) to column string
    std::vector<std::string> encoded_entries;
    // Maps 32-bit unique value (index) to number of occurences
    std::vector<unsigned> data_count;
    // List of all column data from top to bottom in encoded form
    std::vector<unsigned> encoded_order;

    Column_Data() : encode_map( 250000 ) {}

    // Bring in data from file with raw data
    void load_from_raw( std::istream& is )
    {
        encoded_entries.clear();
        encoded_order.clear();
        encode_map.clear();
        unsigned code = 0;
        while( !is.eof() )
        {
            std::string s;
            getline( is, s, '\n' );
            if( !s.length() ) continue;
            if( !encode_map.find( s ).exists )
            {
                encode_map.insert( s, code );
                encoded_entries.push_back( s );
                data_count.push_back( 1 );
                code++;
            }
            unsigned index = encode_map.at( s );
            encoded_order.push_back( index );
            data_count[index]++;
        }
    }
    // Write data to a file in encoded form
    void write_encoded( std::ostream& os )
    {
        unsigned code = encoded_entries.size();

        os << "dict:" << std::endl;
        for( unsigned i=0; i<code; i++ )
        {
            os << encoded_entries[i] << std::endl;
        }

        os << "data:" << std::endl;
        os.write( (char*)encoded_order.data(), encoded_order.size() * sizeof( unsigned ) );
        unsigned term = 0xffffffff;
        os.write( (char*)&term, sizeof(term) );
    }
    void load_from_encoded( std::istream& is )
    {
        encoded_entries.clear();
        encoded_order.clear();
        encode_map.clear();

        while( 1 )
        {
            std::string s;
            getline( is, s, '\n' );
            if( s == "dict:" ) continue;
            if( s == "data:" ) break;

            encode_map.insert( s, encoded_entries.size() );
            encoded_entries.push_back( s );
            data_count.push_back( 0 );
        }

        while( !is.eof() )
        {
            unsigned code;
            is.read( (char*)&code, sizeof(code) );
            if( code == 0xffffffff ) break;

            encoded_order.push_back( code );
            data_count[ code ]++;
        }
    }
    void write_decoded( std::ostream& os )
    {
        for( std::vector<unsigned>::iterator itr=encoded_order.begin(); itr!=encoded_order.end(); itr++ )
        {
            os << encoded_entries[*itr] << std::endl;
        }
    }
    unsigned query( const std::string& s )
    {
        Hash_Map<std::string,unsigned>::Hash_Table_Data t = encode_map.find( s );
        return t.exists ? data_count[t.val] : 0;
    }
};

int main()
{
    /*********************************************************
     * Encode Small Data
    *********************************************************/
    {
        Column_Data cd;
        std::ifstream in_file("Small-Size-Column.txt");
        std::ofstream out_file("Small-Size-Column-Encoded.txt");

        {
            Tracer t("Small-Large");
            cd.load_from_raw( in_file );
            cd.write_encoded( out_file );
        }
    }
    /*********************************************************
     * Encode Medium Data
    *********************************************************/
    {
        Column_Data cd;
        std::ifstream in_file("Medium-Size-Column.txt");
        std::ofstream out_file("Medium-Size-Column-Encoded.txt");

        {
            Tracer t("Medium-Large");
            cd.load_from_raw( in_file );
            cd.write_encoded( out_file );
        }
    }
    /*********************************************************
     * Encode Large Data
    *********************************************************/
    {
        Column_Data cd;
        std::ifstream in_file("Large-Size-Column.txt");
        std::ofstream out_file("Large-Size-Column-Encoded.txt");

        {
            Tracer t("Encode-Large");
            cd.load_from_raw( in_file );
            cd.write_encoded( out_file );
        }
    }
    /*********************************************************
     * Load Small Data and Query
    *********************************************************/
    {
        Column_Data cd;
        std::ifstream encoded("Small-Size-Column-Encoded.txt");
        cd.load_from_encoded( encoded );
        {
            Tracer t("Query-Small-0");
            std::cout << "Found " << cd.query( "jhbmhu" ) << " instances!" << std::endl;
        }
        {
            Tracer t("Query-Small-1");
            std::cout << "Found " << cd.query( "oauiujvrsk" ) << " instances!" << std::endl;
        }
        {
            Tracer t("Query-Small-2");
            std::cout << "Found " << cd.query( "alksdjfasdfs" ) << " instances!" << std::endl;
        }
    }
    /*********************************************************
     * Load Medium Data and Query
    *********************************************************/
    {
        Column_Data cd;
        std::ifstream encoded("Medium-Size-Column-Encoded.txt");
        cd.load_from_encoded( encoded );
        {
            Tracer t("Query-Medium-0");
            std::cout << "Found " << cd.query( "zbuasihrp" ) << " instances!" << std::endl;
        }
        {
            Tracer t("Query-Medium-1");
            std::cout << "Found " << cd.query( "ynryjgfjmm" ) << " instances!" << std::endl;
        }
        {
            Tracer t("Query-Medium-2");
            std::cout << "Found " << cd.query( "aasdlkfljlskd" ) << " instances!" << std::endl;
        }
    }
    /*********************************************************
     * Load Large Data and Query
    *********************************************************/
    {
        Column_Data cd;
        std::ifstream encoded("Large-Size-Column-Encoded.txt");
        cd.load_from_encoded( encoded );
        {
            Tracer t("Query-Large-0");
            std::cout << "Found " << cd.query( "nunvhglmpt" ) << " instances!" << std::endl;
        }
        {
            Tracer t("Query-Large-1");
            std::cout << "Found " << cd.query( "zcza" ) << " instances!" << std::endl;
        }
        {
            Tracer t("Query-Large-2");
            std::cout << "Found " << cd.query( "adsfasdga" ) << " instances!" << std::endl;
        }
    }

    return 0;
}
