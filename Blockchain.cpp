#include <ctime>
#include <iostream>
#include <memory>
#include <openssl/sha.h>
#include <sstream>

#include "Blockchain.h"

// ----------------------------------------------------------------------------
Blockchain::Blockchain()
{
   Block genesis;
   genesis.index     = 0;
   genesis.prevHash  = "0";
   genesis.timestamp = getCurrentTime();
   genesis.calculateHash();

   chain.push_back( genesis );
}

// ----------------------------------------------------------------------------
void Blockchain::addTransaction( const Transaction& tx )
{
   pendingTxs.push_back( tx );
}

// ----------------------------------------------------------------------------
std::string Blockchain::calculateHash( const Block& block ) const
{
   auto blockString = block.toStringWithoutHash();

   unsigned char hash[ SHA256_DIGEST_LENGTH ];
   SHA256( ( unsigned char* ) blockString.c_str(), blockString.size(), hash );

   std::stringstream hexStream;
   for ( int i = 0; i < SHA256_DIGEST_LENGTH; i++ )
   {
      hexStream << std::hex << std::setw( 2 ) << std::setfill( '0' )
                << ( int ) hash[ i ];
   }
   std::cout << "----hash: " << hexStream.str() << std::endl;
   return hexStream.str();
}

// ----------------------------------------------------------------------------
bool Blockchain::isChainValid() const
{
   for ( size_t i = 1; i < chain.size(); i++ )
   {
      const Block& current = chain[ i ];
      const Block& prev    = chain[ i - 1 ];

      if ( current.hash != calculateHash( current ) )
      {
         return false;
      }

      if ( current.prevHash != prev.hash )
      {
         return false;
      }

      if ( !isValidPoW( current.hash, 4 ) )
      {
         return false;
      }
   }

   return true;
}

// ----------------------------------------------------------------------------
bool Blockchain::isValidPoW( const std::string& hash, int difficulty ) const
{
   return hash.substr( 0, difficulty ) == std::string( difficulty, '0' );
}

// ----------------------------------------------------------------------------
void Blockchain::mineBlock( Block& block, int difficulty )
{
   block.nonce = 0;
   while ( true )
   {
      block.calculateHash();
      if ( isValidPoW( block.hash, difficulty ) )
      {
         break;
      }

      block.nonce++;
   }
}

// ----------------------------------------------------------------------------
void Blockchain::mineBlock()
{
   Block newBlock;
   newBlock.index     = chain.size();
   newBlock.prevHash  = chain.back().hash;
   newBlock.txs       = std::move( pendingTxs );
   newBlock.timestamp = getCurrentTime();
   pendingTxs.clear();

   mineBlock( newBlock, 4 );

   chain.push_back( newBlock );
   std::cout << "Block mined: " << newBlock.hash << std::endl;
}

// ----------------------------------------------------------------------------
std::string Blockchain::getCurrentTime() const
{
   std::time_t now = std::time( nullptr );
   return std::ctime( &now );
}

// ----------------------------------------------------------------------------
std::string Blockchain::toString() const
{
   std::stringstream ss;
   for ( const auto& block : chain )
   {
      ss << block.toJson().dump() << std::endl;
   }

   return ss.str();
}