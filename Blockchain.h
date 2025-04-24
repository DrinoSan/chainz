#pragma once

#include <vector>

#include "Block.h"
#include "Transaction.h"

// ----------------------------------------------------------------------------
class Blockchain
{
 public:
   Blockchain();

   void        addTransaction( const Transaction& tx );
   void        mineBlock();
   void        mineBlock( Block& block, int difficulty );
   bool        addBlock( const Block& block );
   bool        addToMempool( const Transaction& tx );

   // Methods for checking
   bool        isValidTransaction( const Transaction& tx ) const;
   bool        isValidPoW( const std::string& hash, int difficulty ) const;
   bool        isChainValid() const;
   bool        isTransactionDuplicate( const Transaction& tx ) const;

   // Stuff I trust openssl
   std::string calculateHash( const Block& block ) const;

   // Needed for blocks
   std::string getCurrentTime() const;

   // Json shit
   json toJson() const;
   std::string toString() const;

   std::vector<Block> chain;

 private:
   std::vector<Transaction> pendingTxs;
};
