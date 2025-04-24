#pragma once

#include <string>
#include <vector>

// Vendor
#include "json/json.hpp"

// Project
#include "Transaction.h"

// for convenience
using json = nlohmann::json;

// ----------------------------------------------------------------------------
class Block
{
 public:
   Block() = default;

   json        toJson() const;
   std::string toStringWithoutHash() const;
   json        toJsonWithoutHash() const;
   void        calculateHash();

   // ----------------------------------------------------------------------------
   static Block fromJson( const json& j )
   {
      Block b;
      j.at( "index" ).get_to( b.index );
      j.at( "prevHash" ).get_to( b.prevHash );
      j.at( "hash" ).get_to( b.hash );
      j.at( "nonce" ).get_to( b.nonce );
      j.at( "timestamp" ).get_to( b.timestamp );

      for( const auto& txJson : j.at( "transactions" ) )
      {
         Transaction tx = Transaction::fromJson( txJson );
         b.txs.push_back( tx );
      }

      return b;
   }

   int32_t                  index;
   std::string              prevHash;
   std::string              hash;
   std::vector<Transaction> txs;
   uint64_t                 nonce;
   std::string              timestamp;
};

void to_json( json& j, const Block& b );
void from_json( const json& j, Block& b );
