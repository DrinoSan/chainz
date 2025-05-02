#include <chrono>

#include "Client.h"

// Vendor
#include "vendor/Server.h"

// ----------------------------------------------------------------------------
void Client::broadcastTransaction( const std::vector<std::string>& peers,
                                   const Transaction&              tx )
{
   for ( const auto& peer : peers )
   {
      httplib::Client cli( peer.c_str() );

      json j = tx;
      cli.Post( "/tx", j.dump(), "application/json" );
   }
}

// ----------------------------------------------------------------------------
Transaction Client::createTransaction( const std::string& senderAddr,
                                       const std::string& receiverAddr,
                                       double amount, double fee,
                                       const std::string& privateKey )
{
   // Retrieve utxoSet
   Transaction tx;
   auto        availableUtxos = getUtxos( senderAddr );

   if ( availableUtxos.empty() )
   {
      return tx;
   }

   auto needed = amount + fee;

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
   tx.timestamp = std::to_string( milliseconds );

   return tx;
}

// ----------------------------------------------------------------------------
std::vector<utxo::UTXO> Client::getUtxos( const std::string& address )
{
   std::string     endpoint{ "/utxo/" + address };
   httplib::Client cli( "localhost:8080" );

   std::vector<utxo::UTXO> utxoResult{};
   if ( auto res = cli.Get( endpoint ) )
   {
      auto status = res->status;
      if ( status != httplib::StatusCode::OK_200 )
      {
         std::cout << "Got some error: " << status << std::endl;
      }

      auto body = res->body;

      json bodyJson = json::parse( body );

      for ( auto& [ key, val ] : bodyJson.items() )
      {
         utxoResult.push_back( val );
      }

      std::cout << "Retrieved UTXO set: " << bodyJson.dump( 4 ) << std::endl;
   }
   else
   {
      std::cout << "Seems like no peer is alive\n";
   }

   return utxoResult;
}

// ----------------------------------------------------------------------------
void Client::showUTXOs( const std::vector<std::string>& peers )
{
   std::string     endpoint{ "/utxo" };
   httplib::Client cli( "localhost:8080" );

   if ( auto res = cli.Get( endpoint ) )
   {
      auto body = res->body;

      json bodyJson = json::parse( body );
      std::cout << body << std::endl;
   }
}
