#pragma once

#include <string>

#include "json/json.hpp"

// for convenience
using json = nlohmann::json;

// ----------------------------------------------------------------------------
// Output structure
struct Output
{
   std::string address;   // Recipient
   double      amount;    // Amount
};


// ----------------------------------------------------------------------------
// Forward declarations of JSON serialization functions
void to_json( json& j, const Output& o );
void from_json( const json& j, Output& o );

