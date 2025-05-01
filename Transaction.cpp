#include <iostream>

#include "Transaction.h"
#include "Block.h"

// ----------------------------------------------------------------------------
Transaction Transaction::fromJson( const json& j )
{
   Transaction tx;
   j.at( "sender" ).get_to( tx.sender );
   j.at( "receiver" ).get_to( tx.receiver );
   j.at( "amount" ).get_to( tx.amount );
   j.at( "timestamp" ).get_to( tx.timestamp );

   return tx;
}

// ----------------------------------------------------------------------------
bool Transaction::operator==( const Transaction& other ) const
{
   return ( sender == other.sender && receiver == other.receiver &&
            amount == other.amount && timestamp == other.timestamp );
}

// Transaction //
// JSON serialization for Transaction //
// ----------------------------------------------------------------------------
json Transaction::toJson() const
{
   return { { "sender", sender },
            { "receiver", receiver },
            { "amount", amount },
            { "timestamp", timestamp } };
}

// ----------------------------------------------------------------------------
void to_json( json& j, const Transaction& b )
{
   j = json{ { "sender", b.sender },
             { "receiver", b.receiver },
             { "amount", b.amount },
             { "timestamp", b.timestamp } };
}

// ----------------------------------------------------------------------------
void from_json( const json& j, Transaction& b )
{
   j.at( "sender" ).get_to( b.sender );
   j.at( "receiver" ).get_to( b.receiver );
   j.at( "amount" ).get_to( b.amount );
   j.at( "timestamp" ).get_to( b.timestamp );
}

// ----------------------------------------------------------------------------
Transaction Transaction::createTransaction(
    const std::string& senderAddr, const std::string& receiverAddr,
    double amount, double fee, const std::vector<UTXO>& availableUtxos,
    const std::string& privateKey )
{
   Transaction tx;
   auto        needed = amount + fee;

   // Get UTXO for senderAddr
   // Node needs to remove unspent utxos later on
   std::vector<UTXO> unspentUTXO;
   int32_t           utxoAmountAccum{};
   for ( const auto& utxo : availableUtxos )
   {
      if ( utxo.address == senderAddr )
      {
         unspentUTXO.push_back( utxo );
         utxoAmountAccum += utxo.amount;

         tx.inputs.push_back(
             { utxo.txid, utxo.outputIndex, utxo.amount, privateKey } );
      }
   }

   if ( utxoAmountAccum < needed )
   {
      std::cout << "Accumulated: " << utxoAmountAccum << std::endl;
      std::cout << "Needed: " << needed << std::endl;
      throw std::invalid_argument( "Unsuficient funds!!" );
   }

   auto change = utxoAmountAccum - ( amount + fee );
   if ( change > 0 )
   {
      tx.outputs.push_back( { senderAddr, change } );
   }

   tx.outputs.push_back( { receiverAddr, amount } );

   std::time_t now = std::time( nullptr );
   tx.txid         = senderAddr + std::ctime( &now );
   tx.sender       = senderAddr;
   tx.receiver     = receiverAddr;
   tx.amount       = amount;
   tx.isReward     = false;
   tx.timestamp    = std::ctime( &now );

   return tx;
}
