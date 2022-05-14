#include "distr_log_db/b_plus.hpp"

#include "distr_log_db/tracer.hpp"

#include <fstream>
#include <cstring>

int main()
{
    B_Tree::Val v;

    std::cout << std::hex;
    {
        // Instantiate b tree, with reset TRUE
        B_Tree b( "foo.dtb", true );

        // Add 7 random initial values
        for( size_t i=0; i<7; i++ )
        {
            B_Tree::Key k = ( rand() << 16 ) | ( rand() & 0xffff ) & 0xffffffff;

            snprintf( (char*)v.val, sizeof( v.val ), "0x%08x", k );

            B_Tree::Txn t = b.new_txn();
            b.insert( k, v, t );
            b.txn_commit( t );
        }
    }

    // Reopen b tree to verify storage to file system works properly
    {
        B_Tree b( "foo.dtb" );
        b.print();
        std::cout << std::endl;
    }

    {
        // Reopen b tree
        B_Tree b( "foo.dtb" );

        // Create new transaction
        B_Tree::Txn t = b.new_txn();

        // Add value to b tree as a transaction
        B_Tree::Key k = ( rand() << 16 ) | ( rand() & 0xffff ) & 0xffffffff;
        snprintf( (char*)v.val, sizeof( v.val ), "0x%08x", k );
        b.insert( k, v, t );

        b.print();

        // Call b tree destructor to emulate system crash before transaction
        // can be completed.
    }

    {
        // Reopen b tree to verify previous insertion did not persist
        B_Tree b( "foo.dtb" );
        b.print();
        std::cout << std::endl;
    }

    {
        // Reopen b tree
        B_Tree b( "foo.dtb" );

        // Create a second transaction
        B_Tree::Txn t = b.new_txn();

        // Add value to b tree as a transaction
        B_Tree::Key k = ( rand() << 16 ) | ( rand() & 0xffff ) & 0xffffffff;
        snprintf( (char*)v.val, sizeof( v.val ), "0x%08x", k );
        b.insert( k, v, t );

        // Commit transaction to ensure value persists
        b.txn_commit( t );
    }

    {
        // Reopen tree to verify previous insertion persisted
        B_Tree b( "foo.dtb" );
        b.print();
        std::cout << std::endl;
    }

    {
        B_Tree b( "foo.dtb" );

        for( size_t i=0; i<300; i++ )
        {
            B_Tree::Key k = ( rand() << 16 ) | ( rand() & 0xffff ) & 0xffffffff;

            snprintf( (char*)v.val, sizeof( v.val ), "0x%08x", k );

            B_Tree::Txn t = b.new_txn();
            b.insert( k, v, t );
            b.txn_commit( t );
        }
    }

    {
        // Reopen tree to verify previous insertion persisted
        B_Tree b( "foo.dtb" );
        b.print();
        std::cout << std::endl;
    }
}
