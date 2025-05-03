#include <iostream>

#include "Block.h"
#include "Transaction.h"

// ----------------------------------------------------------------------------
Transaction Transaction::fromJson( const json& j )
{
   Transaction tx;
   j.at( "sender" ).get_to( tx.sender );
   j.at( "receiver" ).get_to( tx.receiver );
   j.at( "amount" ).get_to( tx.amount );

   std::int64_t timestamp_seconds;
   j.at( "timestamp" ).get_to( timestamp_seconds );
   tx.timestamp = std::chrono::system_clock::time_point(
       std::chrono::seconds( timestamp_seconds ) );

   j.at( "inputs" ).get_to( tx.inputs );
   j.at( "outputs" ).get_to( tx.outputs );
   j.at( "isReward" ).get_to( tx.isReward );

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
   auto timestampSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                               timestamp.time_since_epoch() )
                               .count();

   return { { "sender", sender },
            { "receiver", receiver },
            { "amount", amount },
            { "timestamp", timestampSeconds } };
}

// ----------------------------------------------------------------------------
void to_json( json& j, const Transaction& t )
{
   auto timestampSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                               t.timestamp.time_since_epoch() )
                               .count();

   j = json{ { "sender", t.sender },     { "txid", t.txid },
             { "receiver", t.receiver }, { "amount", t.amount },
             { "fee", t.fee },           { "inputs", t.inputs },{ "isReward", t.isReward },
             { "outputs", t.outputs },   { "timestamp", timestampSeconds } };
}

// ----------------------------------------------------------------------------
void from_json( const json& j, Transaction& t )
{
   std::int64_t timestampSeconds;
   j.at( "timestamp" ).get_to( timestampSeconds );
   t.timestamp = std::chrono::system_clock::time_point(
       std::chrono::seconds( timestampSeconds ) );

   j.at( "sender" ).get_to( t.sender );
   j.at( "txid" ).get_to( t.txid );
   j.at( "receiver" ).get_to( t.receiver );
   j.at( "amount" ).get_to( t.amount );
   j.at( "inputs" ).get_to( t.inputs );
   j.at( "outputs" ).get_to( t.outputs );
   j.at( "isReward" ).get_to( t.isReward );
   j.at( "fee" ).get_to( t.fee );
}

// ----------------------------------------------------------------------------
Transaction Transaction::createTransaction(
    const std::string& senderAddr, const std::string& receiverAddr,
    double amount, double fee, const std::vector<utxo::UTXO>& availableUtxos,
    const std::string& privateKey )
{
   Transaction tx;
   auto        needed = amount + fee;

   // Get UTXO for senderAddr
   // Node needs to remove unspent utxos later on
   std::vector<utxo::UTXO> unspentUTXO;
   int32_t                 utxoAmountAccum{};
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

   auto now = std::chrono::system_clock::now();

   // Convert the current time to time since epoch
   auto duration = now.time_since_epoch();

   // Convert duration to milliseconds
   auto milliseconds =
       std::chrono::duration_cast<std::chrono::milliseconds>( duration )
           .count();

   tx.txid      = senderAddr + std::to_string( milliseconds );
   tx.sender    = senderAddr;
   tx.receiver  = receiverAddr;
   tx.fee       = fee;
   tx.amount    = amount;
   tx.isReward  = false;
   tx.timestamp = now;

   return tx;
}
