#pragma once

#include <string>

#include "json/json.hpp"

// for convenience
using json = nlohmann::json;

// ----------------------------------------------------------------------------
// Input structure
struct Input
{
   std::string txid;   // Reference to UTXO
   int         outputIndex;
   double      amount;
   std::string signature;   // Simplified (in practice, cryptographic signature)
};

// ----------------------------------------------------------------------------
// Forward declarations of JSON serialization functions
void to_json( json& j, const Input& i );
void from_json( const json& j, Input& i );
