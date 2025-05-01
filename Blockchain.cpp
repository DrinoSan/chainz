#include <algorithm>
#include <ctime>
#include <iostream>
#include <memory>
#include <openssl/sha.h>
#include <sstream>
#include <vector>

#include "Blockchain.h"
#include "Node.h"

// ----------------------------------------------------------------------------
Blockchain::Blockchain()
{
   Block genesis;
   genesis.index     = 0;
   genesis.prevHash  = "Mojo";
   genesis.timestamp = getCurrentTime();
   genesis.hash      = genesis.calculateHash();

   chain.push_back( genesis );
}

// ----------------------------------------------------------------------------
void Blockchain::addTransaction( const Transaction& tx )
{
   pendingTxs.push_back( tx );
}

// ----------------------------------------------------------------------------
bool Blockchain::isChainValid() const
{
   for ( size_t i = 1; i < chain.size(); i++ )
   {
      const Block& current = chain[ i ];
      const Block& prev    = chain[ i - 1 ];

      if ( current.hash != current.calculateHash() )
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
      block.hash = block.calculateHash();
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
      json j = block;
      ss << j.dump( 4 ) << "\n" << std::endl;
   }

   return ss.str();
}

// ----------------------------------------------------------------------------
bool Blockchain::addBlock( const Block& block )
{
   int32_t rewardCount{};
   double  totalFees;
   for ( const auto& tx : block.txs )
   {
      if ( tx.isReward )
      {
         ++rewardCount;
         if ( rewardCount > 1 || tx.amount != 10 || tx.sender != "network" )
         {
            std::cout << "RewardCount: "<<rewardCount << ::std::endl;
            std::cout << "tx.amount: "<<tx.amount << ::std::endl;
            std::cout << "tx.sender: "<<tx.sender << ::std::endl;
            std::cerr << "Invalid isReward transaction identified, block will "
                         "not be added"
                      << std::endl;
            std::cerr << "Invalid number of rewardCount, block seems fishy"
                      << std::endl;

            return false;
         }
      }
      else
      {
         if ( !isValidTransaction( tx ) )
         {
            std::cerr
                << "Invalid regular transaction found, block will not be added"
                << std::endl;
            return false;
         }

         // Validate regular transaction
         double inputSum = 0;
         for ( const auto& input : tx.inputs )
         {
            auto it =
                std::find_if( utxoSet.begin(), utxoSet.end(),
                              [ & ]( const UTXO& u ) {
                                 return u.txid == input.txid &&
                                        u.outputIndex == input.outputIndex;
                              } );

            if ( it == utxoSet.end() )
            {
               return false;   // UTXO not found
            }

            inputSum += it->amount;
         }

         double outputSum = 0;
         for ( const auto& output : tx.outputs )
         {
            outputSum += output.amount;
         }

         if ( inputSum < outputSum )
         {
            return false;   // Invalid amounts
         }

         totalFees += ( inputSum - outputSum );
      }
   }

   // Update UTXO set
   for ( const auto& tx : block.txs )
   {
      // Remove consumed UTXOs
      for ( const auto& input : tx.inputs )
      {
         auto it = std::find_if( utxoSet.begin(), utxoSet.end(),
                                 [ & ]( const UTXO& u ) {
                                    return u.txid == input.txid &&
                                           u.outputIndex == input.outputIndex;
                                 } );

         if ( it != utxoSet.end() )
         {
            utxoSet.erase( it );
         }
      }

      // Add new UTXOs
      for ( size_t i = 0; i < tx.outputs.size(); ++i )
      {
         utxoSet.push_back( { tx.txid, static_cast<int>( i ),
                              tx.outputs[ i ].amount,
                              tx.outputs[ i ].address } );
      }
   }

   // Update mempool
   std::vector<Transaction> newPendingTxs;
   for ( const auto& mempoolTx : pendingTxs )
   {
      bool included = false;
      for ( const auto& blockTx : block.txs )
      {
         if ( mempoolTx.txid == blockTx.txid )
         {
            included = true;
            break;
         }
      }

      if ( !included )
      {
         newPendingTxs.push_back( mempoolTx );
      }
   }

   pendingTxs = std::move( newPendingTxs );

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

   // checkHash
   if ( block.hash != block.calculateHash() )
   {
      std::cerr << "Invalid Hash" << std::endl;
      return false;
   }

   chain.push_back( block );
   return true;
}

// ----------------------------------------------------------------------------
bool Blockchain::isTransactionDuplicate( const Transaction& tx ) const
{
   auto it = std::find_if( pendingTxs.begin(), pendingTxs.end(),
                           [ & ]( const auto& pendingTx )
                           { return tx == pendingTx; } );

   return it != pendingTxs.end();
}

// ----------------------------------------------------------------------------
bool Blockchain::addToMempool( const Transaction& tx )
{
   if ( !isValidTransaction( tx ) )
   {
      return false;
   }

   pendingTxs.push_back( tx );
   return true;
}

// ----------------------------------------------------------------------------
bool Blockchain::isValidTransaction( const Transaction& tx ) const
{
   // TODO's
   // Add balance check ---->requires state tracking
   // Add signature verification
   if ( tx.sender.empty() || tx.receiver.empty() || tx.amount <= 0 ||
        isTransactionDuplicate( tx ) )
   {
      return false;
   }

   return true;
}

// ----------------------------------------------------------------------------
Block Blockchain::minePendingTransactions( std::string& minerAddress )
{
   Block block;
   block.index     = chain.size();
   block.prevHash  = chain.back().hash;
   block.timestamp = getCurrentTime();
   block.txs       = selectTransactions( 10 );

   // Node which successfully mines a block gets a isReward
   Transaction reward;
   reward.sender    = "network";
   reward.receiver  = minerAddress;
   reward.amount    = 10.0;
   reward.isReward  = true;
   reward.timestamp = getCurrentTime();
   reward.txid      = "reward_" + std::to_string( block.index );
   reward.outputs.push_back( { minerAddress, 10.0 } );
   block.txs.insert( block.txs.begin(), reward );

   mineBlock( block, 4 );

   if ( addBlock( block ) )
   {
      std::cout << "Block mined successfully!"
                << std::endl;   // We currently only have it locally
   }
   else
   {
      std::cout << "Block validation failed!" << std::endl;
   }

   return block;
}

// ----------------------------------------------------------------------------
std::vector<Transaction> Blockchain::selectTransactions( size_t max )
{
   // I guess locking is kinda important if stuff is distributed
   std::lock_guard<std::mutex> lock(pendingTxsMutex);

   // TODO, i want to add the blocks with highest fees which the node gets
   std::vector<Transaction> selected;
   if ( pendingTxs.size() < max )
   {
      max = pendingTxs.size();
   }

   std::vector<std::pair<int32_t, size_t>> txFeePairs;

   // I dont think this is good for performance
   // Sorting pendingTxs based on fee, which is calculated but total inputs minus total outputs for each transaction
   std::sort( pendingTxs.begin(), pendingTxs.end(),
              []( auto& tx1, auto& tx2 )
              {
                 auto inputAmountTx1 = std::accumulate(
                     tx1.inputs.begin(), tx1.inputs.end(), 0,
                     []( auto& i, auto& input ) { return input.amount + i; } );

                 auto outputAmountTx1 =
                     std::accumulate( tx1.outputs.begin(), tx1.outputs.end(), 0,
                                      []( auto& i, auto& output )
                                      { return output.amount + i; } );
                 auto feeTx1 = inputAmountTx1 - outputAmountTx1;

                 auto inputAmountTx2 = std::accumulate(
                     tx2.inputs.begin(), tx2.inputs.end(), 0,
                     []( auto& i, auto& input ) { return input.amount + i; } );

                 auto outputAmountTx2 =
                     std::accumulate( tx2.outputs.begin(), tx2.outputs.end(), 0,
                                      []( auto& i, auto& output )
                                      { return output.amount + i; } );
                 auto feeTx2 = inputAmountTx2 - outputAmountTx2;

                 return feeTx1 > feeTx2;
              } );

   for ( size_t i = 0; i < max; ++i )
   {
      if ( pendingTxs[ i ].isReward )
      {
         continue;
      }

      selected.push_back( pendingTxs[ i ] );
   }

   // I think i can erase all with rewarded true, first of all only the node
   // itself calls this function. For blocks received via broadcast we do all
   // checks in addBlock

   pendingTxs.erase( std::remove_if( pendingTxs.begin(), pendingTxs.end(),
                                     []( const auto& tx )
                                     { return tx.isReward == true; } ),
                     pendingTxs.end() );

   return selected;
}

// ----------------------------------------------------------------------------
json Blockchain::toJson() const
{
   json j;
   for ( const auto& block : chain )
   {
      j.push_back( block );
   }

   return j;
}

// ----------------------------------------------------------------------------
std::vector<UTXO>
Blockchain::getUTXOsForAddress( const std::string& address ) const
{
   std::vector<UTXO> utxoForAddress;
   for ( const auto& utxo : utxoSet )
   {
      if ( utxo.address == address )
      {
         utxoForAddress.push_back( utxo );
      }
   }

   return utxoForAddress;
}
