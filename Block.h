#pragma once

#include <string>
#include <vector>

// Vendor
#include "json/json.hpp"

// Project
#include "Transaction.h"

// for convenience
using json = nlohmann::json;

// UTXO structure
struct UTXO
{
   std::string txid;          // Transaction ID
   int         outputIndex;   // Output index in transaction
   double      amount;        // Amount in tokens
   std::string address;       // Owner address
};

// ----------------------------------------------------------------------------
class Block
{
 public:
   Block() = default;

   // ----------------------------------------------------------------------------
   json         toJson() const;
   std::string  toStringWithoutHash() const;
   std::string  toString() const;
   json         toJsonWithoutHash() const;
   static Block fromJson( const json& j );

   // ----------------------------------------------------------------------------
   std::string calculateHash() const;

   int32_t                  index;
   std::string              prevHash;
   std::string              hash;
   std::vector<Transaction> txs;
   uint64_t                 nonce;
   std::string              timestamp;
};

void to_json( json& j, const Block& b );
void from_json( const json& j, Block& b );
