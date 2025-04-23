#pragma once

#include <string>
#include <vector>

#include "json/json.hpp"

// for convenience
using json = nlohmann::json;


// ----------------------------------------------------------------------------
class Transaction
{
 public:
   Transaction() = default;

   json toJson() const;

   std::string sender;
   std::string receiver;
   double      amount;
   std::string timestamp;
};

// ----------------------------------------------------------------------------
class Block
{
 public:
   Block() = default;

   //json toJson() const;
   json toJson() const;
   std::string toStringWithoutHash() const;
   json toJsonWithoutHash() const;
   void calculateHash();

   int32_t                  index;
   std::string              prevHash;
   std::string              hash;
   std::vector<Transaction> txs;
   uint64_t                 nonce;
   std::string              timestamp;
};

// Forward declarations of JSON serialization functions
void to_json(json& j, const Transaction& t);
void from_json(const json& j, Transaction& t);
void to_json(json& j, const Block& b);
void from_json(const json& j, Block& b);