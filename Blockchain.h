#pragma once

#include <chrono>
#include <mutex>
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
   void    mineBlock();
   void    mineBlock( Block& block, int difficulty );
   bool    addBlock( const Block& block );
   bool    addToMempool( const Transaction& tx );

   // Needed for blocks
   std::chrono::system_clock::time_point getCurrentTime() const;

   // Json shit
   json        toJson() const;
   std::string toString() const;

   bool isChainValid() const;
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
   std::vector<Transaction> selectTransactions( size_t max );

   std::mutex pendingTxsMutex;

 private:
   std::vector<Transaction> pendingTxs;       // TODO rename to memPool
   int                      difficulty = 4;   // Initial difficulty
};
