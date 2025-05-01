#pragma once

#include <string>

// Vendor
#include "json/json.hpp"

// for convenience
using json = nlohmann::json;

// UTXO structure
namespace utxo
{
struct UTXO
{
   std::string txid;          // Transaction ID
   int         outputIndex;   // Output index in transaction
   double      amount;        // Amount in tokens
   std::string address;       // Owner address
};

// ----------------------------------------------------------------------------
void to_json( json& j, const UTXO& u );

};   // namespace utxo

