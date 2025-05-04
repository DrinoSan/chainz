#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <vector>

#include "Block.h"
#include "Blockchain.h"

// Vendor
#include "vendor/Server.h"

class Node
{
 public:
   Node( Blockchain& bc_, std::string& host, int32_t port,
         const std::vector<std::string>& peers_ )
       : bc{ bc_ }, peers{ std::move( peers_ ) }
   {
      // I want to keep blockchain to not keep track of the peers and
      // communication stuff currentyl via callbacks
      bc.setBroadcastBlockCallback( [ this ]( const Block& block )
                                    { this->broadcastBlock( block ); } );

      bc.setSyncChainCallback( [ this ]() { return this->syncChain(); } );

      bc.setGetHighestChainHeightCallback(
          [ this ]() { return this->getLongestChainHeight(); } );

      // Adding transaction
      svr.Post( "/tx",
                [ & ]( const auto& req, auto& res )
                {
                   try
                   {
                      auto        txJson = json::parse( req.body );
                      Transaction tx     = txJson;

                      std::cout << "Received transaction: " << txJson.dump( 4 )
                                << std::endl;

                      // Mempool holds pending transactions
                      if ( bc.addTransaction( tx ) )
                      {
                         std::cout << "Transaction added to mempool\n";
                         broadcastTransaction( tx );

                         res.set_content( "OK", "text/plain" );
                      }
                      else
                      {
                         std::cout << "Transaction rejected\n";
                         res.set_content( "Transaction Duplicate",
                                          "text/plain" );
                      }
                   }
                   catch ( const std::exception& e )
                   {
                      std::cerr << "Error processing transaction: " << e.what()
                                << std::endl;
                      res.status = 400;
                      res.set_content( "INVALID JSON", "text/plain" );
                   }
                } );

      // Adding block
      svr.Post( "/block",
                [ this ]( const httplib::Request& req, httplib::Response& res )
                {
                   try
                   {
                      json  blockJson = json::parse( req.body );
                      Block block     = blockJson;
                      if ( bc.addBlock( block ) )
                      {
                         std::cout << "Block received via Node\n";
                         broadcastBlock( block );
                         res.set_content( "OK", "text/plain" );
                      }
                      else
                      {
                         res.status = 400;
                         res.set_content( "Invalid block", "text/plain" );
                      }
                   }
                   catch ( const std::exception& e )
                   {
                      std::cout << "ERROR: " << e.what() << std::endl;
                      res.status = 400;
                      res.set_content( "Invalid JSON", "text/plain" );
                   }
                } );

      svr.Get( "/chain",
               [ this ]( const httplib::Request&, httplib::Response& res )
               {
                  json j = bc.toJson();
                  res.set_content( j.dump( 4 ), "application/json" );
               } );

      // Needed to check if chain sync is needed
      svr.Get( "/chain/height",
               [ & ]( const httplib::Request&, httplib::Response& res )
               {
                  nlohmann::json j;
                  j[ "height" ] =
                      bc.chain.size() -
                      1;   // Latest block index, because index starts at 0 and
                           // height in this case actually is the index.
                  res.set_content( j.dump(), "application/json" );
               } );

      // Needed for Transaction creation in client
      svr.Get( "/utxo",
               [ this ]( const httplib::Request& req, httplib::Response& res )
               {
                  nlohmann::json j = nlohmann::json::array();
                  for ( const auto& u : bc.utxoSet )
                  {
                     nlohmann::json uj;
                     utxo::to_json( uj, u );
                     j.push_back( uj );
                  }

                  res.set_content( j.dump( 4 ), "application/json" );
               } );

      svr.Get( "/utxo/:address",
               [ this ]( const httplib::Request& req, httplib::Response& res )
               {
                  auto userAddress = req.path_params.at( "address" );

                  nlohmann::json j = nlohmann::json::array();
                  for ( const auto& u : bc.utxoSet )
                  {
                     std::cout << "-------- " << u.txid << " " << u.outputIndex
                               << " " << u.amount << " " << u.address
                               << std::endl;
                     if ( u.address == userAddress )
                     {
                        nlohmann::json uj;
                        utxo::to_json( uj, u );
                        j.push_back( uj );
                     }
                  }

                  res.set_content( j.dump( 4 ), "application/json" );
               } );

   }

   // Used via callback
   void               broadcastBlock( const Block& block ) const;
   void               broadcastTransaction( const Transaction& tx ) const;
   std::vector<Block> syncChain() const;

   // Function will return positive integer if a peer has a longer chain than
   // its own
   /// @param int32_t chain hight will be positive if hight is higher than its
   /// own chain
   /// @param std::string name of peer
   std::pair<int32_t, std::string> getLongestChainHeight() const;

   httplib::Server          svr;
 private:
   Blockchain&              bc;
   std::vector<std::string> peers;
};
