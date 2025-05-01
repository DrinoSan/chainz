#pragma once

#include <string>
#include <vector>

#include "UTXO.h"
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
// Output structure
struct Output
{
   std::string address;   // Recipient
   double      amount;    // Amount
};

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
   std::string         timestamp;
   double              fee;
   bool                isReward = false;
};

// Forward declarations of JSON serialization functions
void to_json( json& j, const Transaction& t );
void from_json( const json& j, Transaction& t );
