#include <nlohmann/json.hpp>
#include <string>
#include <vector>

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

                      std::cout << "Got Req: " << txJson.dump() << "\n";

                      // Mempool holds pending transactions
                      bc.addToMempool( tx );
                      broadcastTransaction( tx );

                      res.set_content( "OK", "text/plain" );
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

      svr.listen( host.c_str(), port );
   }

   void broadcastBlock( const Block& block ) const;
   void broadcastTransaction( const Transaction& tx ) const;

   void showChain() const;

 private:
   Blockchain&              bc;
   httplib::Server          svr;
   std::vector<std::string> peers;
};
