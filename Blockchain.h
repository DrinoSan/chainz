#pragma once

#include <vector>

#include "Data.h"

// ----------------------------------------------------------------------------
class Blockchain
{
 public:
   Blockchain();
   
   void addTransaction( const Transaction& tx );
   void mineBlock();
   void mineBlock(Block& block, int difficulty);
   bool isValidPoW(const std::string& hash, int difficulty) const;
   bool isChainValid() const;
   std::string calculateHash( const Block& block ) const;
   std::string getCurrentTime() const;

   std::string toString() const;

   std::vector<Block>       chain;
 private:
   std::vector<Transaction> pendingTxs;
};
