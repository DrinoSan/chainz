#include "Node.h"

// ----------------------------------------------------------------------------
void Node::broadcastBlock( const Block& block ) const
{
   std::cout << "Broadcasting Block\n";

   for ( const auto& peer : peers )
   {
      try
      {
         httplib::Client cli( peer.c_str() );
         json blockJson = block;
         cli.Post( "/block", blockJson.dump(), "application/json" );
      }
      catch ( const std::exception& e )
      {
         std::cout << "Something went wrong with a peer maybe offline Peer: "
                   << peer << std::endl;
      }
   }
}

// ----------------------------------------------------------------------------
void Node::broadcastTransaction( const Transaction& tx ) const
{
   std::cout << "Broadcasting tx\n";

   for ( const auto& peer : peers )
   {
      httplib::Client cli( peer.c_str() );

      json j = tx;
      cli.Post( "/tx", j.dump(), "application/json" );
   }
}
