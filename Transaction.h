#pragma once

#include <string>
#include <vector>

#include "UTXO.h"
#include "json/json.hpp"
#include "Input.h"
#include "Output.h"

// for convenience
using json = nlohmann::json;

// ----------------------------------------------------------------------------
class Transaction
{
 public:
   // ----------------------------------------------------------------------------
   static Transaction fromJson( const json& j );
   json               toJson() const;

   // ----------------------------------------------------------------------------
   bool operator==( const Transaction& other ) const;

   // ----------------------------------------------------------------------------
   static Transaction
   createTransaction( const std::string& senderAddr,
                      const std::string& receiverAddr, double amount,
                      double fee, const std::vector<utxo::UTXO>& availableUtxos,
                      const std::string& privateKey );

 public:
   std::string         txid;
   std::vector<Input>  inputs;    // For regular txs
   std::vector<Output> outputs;   // For regular txs
   std::string         sender;
   std::string         receiver;
   double              amount;
   std::chrono::system_clock::time_point timestamp;
   double              fee;
   bool                isReward = false;
};

// Forward declarations of JSON serialization functions
void to_json( json& j, const Transaction& t );
void from_json( const json& j, Transaction& t );
