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
   // ----------------------------------------------------------------------------
   static Transaction fromJson( const json& j );

   // ----------------------------------------------------------------------------
   json toJson() const;

   // ----------------------------------------------------------------------------
   bool operator==( const Transaction& other ) const;

 public:
   std::string sender;
   std::string receiver;
   double      amount;
   std::string timestamp;
};

// Forward declarations of JSON serialization functions
void to_json( json& j, const Transaction& t );
void from_json( const json& j, Transaction& t );
