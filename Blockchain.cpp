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

// ----------------------------------------------------------------------------
bool Blockchain::addBlock( const Block& block )
{
   // Index starts at 0 means the new block.index should be equal to the current
   // chain size
   if ( block.index != chain.size() )
   {
      std::cerr << "Invalid block index" << std::endl;
      return false;
   }

   if ( block.prevHash != chain.back().hash )
   {
      std::cerr << "Invalid previous hash" << std::endl;
      return false;
   }

   if ( !isValidPoW( block.hash, 4 ) )
   {
      std::cerr << "Invalid proof of work" << std::endl;
      return false;
   }

   chain.push_back( block );
   return true;
}

// ----------------------------------------------------------------------------
bool Blockchain::isTransactionDuplicate( const Transaction& tx ) const
{
   for ( const auto& pendingTx : pendingTxs )
   {
      if ( pendingTx == tx )
      {
         return true;
      }
   }

   return false;
}

// ----------------------------------------------------------------------------
bool Blockchain::addToMempool( const Transaction& tx )
{
   if ( !isValidTransaction( tx ) || !isTransactionDuplicate( tx ) )
   {
      return false;
   }

   pendingTxs.push_back( tx );
   return true;
}

// ----------------------------------------------------------------------------
bool Blockchain::isValidTransaction( const Transaction& tx ) const
{
   if ( tx.sender.empty() || tx.receiver.empty() || tx.amount <= 0 )
   {
      return true;
   }

   return false;
}

// ----------------------------------------------------------------------------
json Blockchain::toJson() const
{
   json j;
   for ( const auto& block : chain )
   {
      j.push_back( block.toJson() );
   }

   return j;
}
