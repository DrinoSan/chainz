#pragma once

#include <chrono>
#include <mutex>
#include <utility>
#include <vector>

#include "Block.h"
#include "Transaction.h"

// ----------------------------------------------------------------------------
class Blockchain
{
 public:
   Blockchain();

   int32_t getEpochDifficulty() const;
   int32_t calculateExpectedDifficulty() const;
   int32_t adjustDifficulty() const;
   bool    addTransaction( const Transaction& tx );
   bool    addBlock( const Block& block );
   void    setupChain();
   Block   createGenesisBlock();
   // bool    addToMempool( const Transaction& tx );

   // Calbacks for node
   std::function<void( const Block& )> broadcastBlockCallback;
   void
   setBroadcastBlockCallback( std::function<void( const Block& )> callback );

   // callback to sync chain
   std::function<std::vector<Block>()> syncChainCallback;
   void                  setSyncChainCallback( std::function<std::vector<Block>()> callback );

   // callback to get height of chains from peers
   std::function<std::pair<int32_t, std::string>()>
        getHighestChainHeightCallback;
   void setGetHighestChainHeightCallback(
       std::function<std::pair<int32_t, std::string>()> callback );

   // Needed for blocks
   std::chrono::system_clock::time_point getCurrentTime() const;

   // Json shit
   json        toJson() const;
   std::string toString() const;

   bool isChainValid() const;
   bool isChainValid( const std::vector<Block>& chain ) const;
   bool minePendingTransactions( std::string& minerAddress );

   std::vector<utxo::UTXO>
   getUTXOsForAddress( const std::string& address ) const;

 public:
   std::vector<utxo::UTXO> utxoSet;
   std::vector<Block>      chain;

 private:
   // Methods for checking
   bool isValidTransaction( const Transaction& tx ) const;
   bool isValidPoW( const std::string& hash, int difficulty ) const;
   bool isTransactionDuplicate( const Transaction& tx ) const;
   void mineBlock();
   void mineBlock( Block& block, int difficulty );
   void recomputeUTXOSet();
   std::vector<Transaction> selectTransactions( size_t max );

   bool               saveChain( const std::string& fileName ) const;
   bool appendBlockToJson( const std::string& fileName ) const;;
   std::vector<Block> loadChain( const std::string& fileName ) const;

   std::mutex pendingTxsMutex;

 private:
   std::vector<Transaction> pendingTxs;       // TODO rename to memPool
   int                      difficulty = 4;   // Initial difficulty
};
