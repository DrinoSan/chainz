#include "Node.h"

// ----------------------------------------------------------------------------
void Node::broadcastBlock( const Block& block ) const
{
   std::cout << "Broadcasting Block\n";

   for ( const auto& peer : peers )
   {
      httplib::Client cli( peer.c_str() );
      cli.Post( "/block", block.toJson().dump(), "application/json" );
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
