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
         json            blockJson = block;
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

// ----------------------------------------------------------------------------
std::pair<int32_t, std::string> Node::getLongestChainHeight() const
{
   std::cout << "Retrieveng highest chain hight from peers" << std::endl;

   int32_t                         highestHeight{};
   std::pair<int32_t, std::string> peerWithHighestChain{ -420, "" };
   for ( const auto& peer : peers )
   {
      try
      {
         httplib::Client cli( peer.c_str() );
         auto            res = cli.Get( "/chain/height" );

         if ( res && res->status == 200 )
         {
            nlohmann::json j      = json::parse( res->body );
            int32_t        height = j[ "height" ].template get<int>();

            if ( height <= bc.chain.size() - 1 )
            {
               continue;
            }

            if ( height > highestHeight )
            {
               highestHeight               = height;
               peerWithHighestChain.first  = highestHeight;
               peerWithHighestChain.second = peer;
            }
         }
      }
      catch ( const std::exception& e )
      {
         std::cout << "Something went wrong with a peer maybe offline Peer: "
                   << peer << std::endl;
      }
   }

   return peerWithHighestChain;
}

// ----------------------------------------------------------------------------
std::vector<Block> Node::syncChain() const
{
   std::cout << "Checking if chain needs some syncing" << std::endl;

   // pair.first is only positiv if a higher chain is found
   std::pair<int32_t, std::string> peerWithHighestChain =
       getLongestChainHeight();

   std::vector<Block> newChain;
   if ( peerWithHighestChain.first <= 0 )
   {
      std::cout << "No sync needed, no longer chain was found\n";
      return newChain;
   }

   // Now we can load the whole chain and start updating utxos
   std::string peer = peerWithHighestChain.second;
   try
   {

      httplib::Client cli( peer );
      auto            res = cli.Get( "/chain" );

      if ( res && res->status == 200 )
      {
         nlohmann::json j = json::parse( res->body );

         for ( const auto& blockJson : j )
         {
            newChain.emplace_back( Block( blockJson ) );
         }
      }
   }
   catch ( const std::exception& e )
   {
      std::cout << "Something went wrong with a peer maybe offline Peer: "
                << peer << std::endl;
   }

   return newChain;
}
