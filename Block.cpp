#include <iostream>
#include <openssl/sha.h>
#include <sstream>

#include "Block.h"

// ----------------------------------------------------------------------------
// JSON serialization for BLOCK //
void to_json( json& j, const Block& b )
{
   auto timestampSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                               b.timestamp.time_since_epoch() )
                               .count();

   j[ "index" ]        = b.index;
   j[ "prevHash" ]     = b.prevHash;
   j[ "hash" ]         = b.hash;
   j[ "transactions" ] = b.txs;
   j[ "nonce" ]        = b.nonce;
   j[ "difficulty" ]   = b.difficulty;
   j[ "timestamp" ]    = timestampSeconds;
}

// ----------------------------------------------------------------------------
void from_json( const json& j, Block& b )
{
   j.at( "index" ).get_to( b.index );
   j.at( "prevHash" ).get_to( b.prevHash );
   j.at( "hash" ).get_to( b.hash );
   j.at( "transactions" ).get_to( b.txs );
   j.at( "nonce" ).get_to( b.nonce );
   j.at( "difficulty" ).get_to( b.difficulty );

   std::int64_t timestampSeconds;
   j.at( "timestamp" ).get_to( timestampSeconds );
   b.timestamp = std::chrono::system_clock::time_point(
       std::chrono::seconds( timestampSeconds ) );
}

// ----------------------------------------------------------------------------
json Block::toJson() const
{
   json j;
   j[ "index" ]    = index;
   j[ "prevHash" ] = prevHash;
   // j[ "hash" ]         = hash;
   j[ "transactions" ] = txs;
   j[ "nonce" ]        = nonce;
   j[ "difficulty" ]   = difficulty;

   auto timestampSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                               timestamp.time_since_epoch() )
                               .count();
   j[ "timestamp" ] = timestampSeconds;
   return j;
}

// ----------------------------------------------------------------------------
std::string Block::toString() const
{
   return toJson().dump();
}

// ----------------------------------------------------------------------------
std::string Block::calculateHash() const
{
   std::stringstream ss;
   ss << index << prevHash;
   auto timestamp_seconds = std::chrono::duration_cast<std::chrono::seconds>(
                                timestamp.time_since_epoch() )
                                .count();
   ss << timestamp_seconds;
   for ( const auto& tx : txs )
   {
      json j = tx;
      ss << j.dump();
   }
   ss << nonce;

   unsigned char hash[ SHA256_DIGEST_LENGTH ];
   SHA256( reinterpret_cast<const unsigned char*>( ss.str().c_str() ),
           ss.str().length(), hash );

   std::stringstream hashHex;
   for ( int i = 0; i < SHA256_DIGEST_LENGTH; i++ )
   {
      hashHex << std::hex << std::setw( 2 ) << std::setfill( '0' )
              << static_cast<int>( hash[ i ] );
   }

   return hashHex.str();

   // auto data = toString();

   // unsigned char hash_[ SHA256_DIGEST_LENGTH ];
   // SHA256( ( unsigned char* ) data.c_str(), data.size(), hash_ );

   // std::stringstream ss;
   // for ( int i = 0; i < SHA256_DIGEST_LENGTH; i++ )
   //{
   //    ss << std::hex << std::setw( 2 ) << std::setfill( '0' )
   //       << ( int ) hash_[ i ];
   // }

   // return ss.str();
}

// ----------------------------------------------------------------------------
Block Block::fromJson( const json& j )
{
   Block b;
   j.at( "index" ).get_to( b.index );
   j.at( "prevhash" ).get_to( b.prevHash );
   j.at( "hash" ).get_to( b.hash );
   j.at( "nonce" ).get_to( b.nonce );

   std::int64_t timestampSeconds;
   j.at( "timestamp" ).get_to( timestampSeconds );
   b.timestamp = std::chrono::system_clock::time_point(
       std::chrono::seconds( timestampSeconds ) );

   for ( const auto& txjson : j.at( "transactions" ) )
   {
      Transaction tx = Transaction::fromJson( txjson );
      b.txs.push_back( tx );
   }

   return b;
}
