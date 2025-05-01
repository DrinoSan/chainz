#pragma once

#include <vector>
#include <mutex>

#include "Block.h"
#include "Transaction.h"

// ----------------------------------------------------------------------------
class Blockchain
{
 public:
   Blockchain();

   void addTransaction( const Transaction& tx );
   void mineBlock();
   void mineBlock( Block& block, int difficulty );
   bool addBlock( const Block& block );
   bool addToMempool( const Transaction& tx );
   // Needed for blocks
   std::string getCurrentTime() const;

   // Json shit
   json        toJson() const;
   std::string toString() const;

   bool  isChainValid() const;
   Block minePendingTransactions( std::string& minerAddress );

   std::vector<UTXO> getUTXOsForAddress( const std::string& address ) const;

 public:
   std::vector<Block> chain;

 private:
   // Methods for checking
   bool isValidTransaction( const Transaction& tx ) const;
   bool isValidPoW( const std::string& hash, int difficulty ) const;
   bool isTransactionDuplicate( const Transaction& tx ) const;
   std::vector<Transaction> selectTransactions( size_t max );

   std::mutex pendingTxsMutex;

 private:
   std::vector<Transaction> pendingTxs;   // TODO rename to memPool
   std::vector<UTXO>        utxoSet;
};
