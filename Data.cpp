#include <iostream>
#include <openssl/sha.h>
#include <sstream>

#include "Data.h"

// ----------------------------------------------------------------------------
// JSON serialization for Transaction //
void to_json( json& j, const Transaction& b )
{
   j = json{ { "sender", b.sender },
             { "receiver", b.receiver },
             { "amount", b.amount },
             { "timestamp", b.timestamp } };
}

void from_json( const json& j, Transaction& b )
{
   j.at( "sender" ).get_to( b.sender );
   j.at( "receiver" ).get_to( b.receiver );
   j.at( "amount" ).get_to( b.amount );
   j.at( "timestamp" ).get_to( b.timestamp );
}

// ----------------------------------------------------------------------------
// JSON serialization for BLOCK //
void to_json( json& j, const Block& b )
{
   j[ "index" ]        = b.index;
   j[ "prevHash" ]     = b.prevHash;
   j[ "hash" ]         = b.hash;
   j[ "transactions" ] = b.txs;
   j[ "nonce" ]        = b.nonce;
   j[ "timestamp" ]    = b.timestamp;
}

// ----------------------------------------------------------------------------
void from_json(const json& j, Block& b)
{
   j.at( "index" ).get_to( b.index );
   j.at( "prevHash" ).get_to( b.prevHash );
   j.at( "transactions" ).get_to( b.txs );
   j.at( "nonce" ).get_to( b.nonce );
   j.at( "timestamp" ).get_to( b.timestamp );
}

// ----------------------------------------------------------------------------
json Block::toJsonWithoutHash() const
{
   json j;
   j[ "index" ]        = index;
   j[ "prevHash" ]     = prevHash;
   //j[ "hash" ]         = hash;
   j[ "transactions" ] = txs;
   j[ "nonce" ]        = nonce;
   j[ "timestamp" ]    = timestamp;
   return j;
}

// ----------------------------------------------------------------------------
json Block::toJson() const
{
   json j;
   j[ "index" ]        = index;
   j[ "prevHash" ]     = prevHash;
   j[ "hash" ]         = hash;
   j[ "transactions" ] = txs;
   j[ "nonce" ]        = nonce;
   j[ "timestamp" ]    = timestamp;
   return j;
}

// Transaction //
// ----------------------------------------------------------------------------
json Transaction::toJson() const
{
   return { { "sender", sender },
            { "receiver", receiver },
            { "amount", amount },
            { "timestamp", timestamp } };
}

// ----------------------------------------------------------------------------
std::string Block::toStringWithoutHash() const
{
   return toJsonWithoutHash().dump();
}

// ----------------------------------------------------------------------------
void Block::calculateHash()
{
   auto data = toStringWithoutHash();

   unsigned char hash_[ SHA256_DIGEST_LENGTH ];
   SHA256( ( unsigned char* ) data.c_str(), data.size(), hash_ );

   std::stringstream ss;
   for ( int i = 0; i < SHA256_DIGEST_LENGTH; i++ )
   {
      ss << std::hex << std::setw( 2 ) << std::setfill( '0' )
         << ( int ) hash_[ i ];
   }

   hash = ss.str();
}
