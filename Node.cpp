#include "Node.h"

// ----------------------------------------------------------------------------
void Node::broadcastBlock( const Block& block ) const
{
   for ( const auto& peer : peers )
   {
      httplib::Client cli( peer.c_str() );
      cli.Post( "/block", block.toJson().dump(), "application/json" );
   }
}

// ----------------------------------------------------------------------------
void Node::broadcastTransaction( const Transaction& tx ) const
{
   for ( const auto& peer : peers )
   {
      httplib::Client cli( peer.c_str() );
      cli.Post( "/tx", tx.toJson().dump(), "application/json" );
   }
}

// ----------------------------------------------------------------------------
void Node::showChain() const
{
   for ( const auto& block : bc.chain )
   {
      std::cout << block.toJson().dump( 4 ) << std::endl;
   }
}