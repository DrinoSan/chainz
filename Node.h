#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "Block.h"
#include "Blockchain.h"
#include "vendor/Server.h"

class Node
{
 public:
   Node( Blockchain& bc_, std::string& host, int32_t port,
         const std::vector<std::string>& peers_ )
       : bc{ bc_ }, peers{ std::move( peers_ ) }
   {
      // Adding transaction
      svr.Post( "/tx",
                [ & ]( const auto& req, auto& res )
                {
                   try
                   {
                      auto        txJson = json::parse( req.body );
                      Transaction tx     = Transaction::fromJson( txJson );

                      // Mempool holds pending transactions
                      if ( bc.addToMempool( tx ) )
                      {
                         std::cout << "Got Req: " << txJson.dump() << "\n";

                         broadcastTransaction( tx );

                         res.set_content( "OK", "text/plain" );
                      }
                      else
                      {
                         res.set_content( "Transaction Duplicate",
                                          "text/plain" );
                      }
                   }
                   catch ( const std::exception& e )
                   {
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
                      Block block     = Block::fromJson( blockJson );
                      if ( bc.addBlock( block ) )
                      {
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

      // Needed for Transaction creation in client
      svr.Get( "/utxo",
               [ this ]( const httplib::Request&, httplib::Response& res )
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

      svr.listen( host.c_str(), port );
   }

   void broadcastBlock( const Block& block ) const;
   void broadcastTransaction( const Transaction& tx ) const;

 private:
   Blockchain&              bc;
   httplib::Server          svr;
   std::vector<std::string> peers;
};
