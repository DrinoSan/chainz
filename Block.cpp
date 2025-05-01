#include <iostream>
#include <openssl/sha.h>
#include <sstream>

#include "Block.h"

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
void from_json( const json& j, Block& b )
{
   j.at( "index" ).get_to( b.index );
   j.at( "prevHash" ).get_to( b.prevHash );
   j.at( "hash" ).get_to( b.hash );
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
   //j[ "hash" ]         = hash;
   j[ "transactions" ] = txs;
   j[ "nonce" ]        = nonce;
   j[ "timestamp" ]    = timestamp;
   return j;
}

// ----------------------------------------------------------------------------
std::string Block::toStringWithoutHash() const
{
   return toJsonWithoutHash().dump();
}

// ----------------------------------------------------------------------------
std::string Block::toString() const
{
   return toJson().dump();
}

// ----------------------------------------------------------------------------
std::string Block::calculateHash() const
{
   auto data = toString();

   unsigned char hash_[ SHA256_DIGEST_LENGTH ];
   SHA256( ( unsigned char* ) data.c_str(), data.size(), hash_ );

   std::stringstream ss;
   for ( int i = 0; i < SHA256_DIGEST_LENGTH; i++ )
   {
      ss << std::hex << std::setw( 2 ) << std::setfill( '0' )
         << ( int ) hash_[ i ];
   }

   return ss.str();
}

// ----------------------------------------------------------------------------
Block Block::fromJson( const json& j )
{
   Block b;
   j.at( "index" ).get_to( b.index );
   j.at( "prevhash" ).get_to( b.prevHash );
   j.at( "hash" ).get_to( b.hash );
   j.at( "nonce" ).get_to( b.nonce );
   j.at( "timestamp" ).get_to( b.timestamp );

   for ( const auto& txjson : j.at( "transactions" ) )
   {
      Transaction tx = Transaction::fromJson( txjson );
      b.txs.push_back( tx );
   }

   return b;
}
