#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <openssl/sha.h>
#include <sstream>
#include <vector>

#include "Blockchain.h"
#include "Node.h"
#include "UTXO.h"

// ----------------------------------------------------------------------------
Blockchain::Blockchain() {}

// ----------------------------------------------------------------------------
void Blockchain::setupChain()
{
   std::cout << "Setting up chain\n";

   // Make sure our callback is set
   std::vector<Block> newChain;
   if ( syncChainCallback )
   {
      std::vector<Block> chainFromPeer = syncChainCallback();
      auto               chainFromFile = loadChain( "chain.json" );

      if ( !chainFromPeer.empty() && ( chainFromPeer.size() - 1 > chainFromFile.size() - 1 ) )
      {
         std::cout << "Chain from peer was chosen to be the new chain\n";
         newChain = std::move( chainFromPeer );
      }
      else
      {
         newChain = std::move( chainFromFile );
         if ( !newChain.empty() )
         {
            std::cout << "Chain from file was choosen to be the new chain\n";
         }
      }
   }
   else
   {
      auto newChain = loadChain( "chain.json" );
   }

   // Making sure the chain is valid
   if ( newChain.size() > 1 && newChain[ 0 ].prevHash == "Mojo" &&
        isChainValid( newChain ) )
   {
      chain = std::move( newChain );
      recomputeUTXOSet();
      saveChain( "chain.json" );
   }
   else
   {
      // I will assume the genesis block is always the same
      std::cout << "No longer chain found in peers and no chain loaded from "
                   "file, creating new chain\n";
      chain.push_back( createGenesisBlock() );
   }
}

// ----------------------------------------------------------------------------
bool Blockchain::addTransaction( const Transaction& tx )
{
   if ( !isValidTransaction( tx ) )
   {
      std::cerr << "Invalid transaction: " << tx.toJson().dump( 4 )
                << std::endl;

      return false;
   }

   std::cout << "Adding transaction to mempool: " << tx.toJson().dump( 4 )
             << std::endl;

   pendingTxs.push_back( tx );
   return true;
}

// ----------------------------------------------------------------------------
bool Blockchain::isChainValid( const std::vector<Block>& chain ) const
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

      if ( !isValidPoW( current.hash, current.difficulty ) )
      {
         return false;
      }
   }

   return true;
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

      if ( !isValidPoW( current.hash, current.difficulty ) )
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
   block.difficulty = difficulty;
   block.nonce      = 0;
   while ( true )
   {
      block.hash = block.calculateHash();
      if ( isValidPoW( block.hash, block.difficulty ) )
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

   mineBlock( newBlock, difficulty );

   chain.push_back( newBlock );
   std::cout << "Block mined: " << newBlock.hash << std::endl;
}

// ----------------------------------------------------------------------------
std::chrono::system_clock::time_point Blockchain::getCurrentTime() const
{
   return std::chrono::system_clock::now();
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
int32_t Blockchain::adjustDifficulty() const
{
   if ( chain.size() < 10 )
   {
      return getEpochDifficulty();
   }

   auto lastBlockTime    = chain.back().timestamp;
   auto tenBlocksAgoTime = chain[ chain.size() - 10 ].timestamp;
   auto duration         = lastBlockTime - tenBlocksAgoTime;

   auto seconds =
       std::chrono::duration_cast<std::chrono::seconds>( duration ).count();

   const int targetSeconds = 100;   // 10 seconds per block × 10 blocks
   int       newDifficulty = chain.back().difficulty;

   if ( seconds < targetSeconds )
   {
      newDifficulty++;
      std::cout << "Difficulty increased to " << difficulty << std::endl;
   }
   else if ( seconds > targetSeconds && difficulty > 1 )
   {
      newDifficulty--;
      std::cout << "Difficulty decreased to " << difficulty << std::endl;
   }

   return newDifficulty;
}

// ----------------------------------------------------------------------------
bool Blockchain::addBlock( const Block& block )
{
   int32_t rewardCount{};
   double  totalFees;

   int32_t expectedDifficulty = calculateExpectedDifficulty();
   if ( block.difficulty != expectedDifficulty )
   {
      std::cerr << "Invalid block difficulty: expected " << expectedDifficulty
                << ", got " << block.difficulty << std::endl;
      return false;
   }

   std::set<std::pair<std::string, int>> usedUTXOs;
   for ( const auto& tx : block.txs )
   {
      if ( tx.isReward )
      {
         ++rewardCount;
         if ( rewardCount > 1 || tx.amount != 10 || tx.sender != "network" )
         {
            std::cerr << "Invalid isReward transaction identified, block will "
                         "not be added"
                      << std::endl;
            std::cerr << "Invalid number of rewardCount, block seems fishy"
                      << std::endl;

            return false;
         }

         continue;
      }
      else
      {
         // Validate regular transaction
         double inputSum = 0;
         for ( const auto& input : tx.inputs )
         {
            auto utxoKey = std::make_pair( input.txid, input.outputIndex );
            if ( usedUTXOs.find( utxoKey ) != usedUTXOs.end() )
            {
               std::cerr
                   << "Double-spend attempt: UTXO already used in this block\n";

               return false;
            }

            auto it =
                std::find_if( utxoSet.begin(), utxoSet.end(),
                              [ & ]( const utxo::UTXO& u ) {
                                 return u.txid == input.txid &&
                                        u.outputIndex == input.outputIndex;
                              } );

            if ( it == utxoSet.end() )
            {
               std::cerr << "UTXO not found\n";
               return false;   // UTXO not found
            }

            usedUTXOs.insert( utxoKey );

            inputSum += it->amount;
         }

         double outputSum = 0;
         for ( const auto& output : tx.outputs )
         {
            outputSum += output.amount;
         }

         if ( inputSum < outputSum )
         {
            std::cerr << "Insufficient funds, inputSum: " << inputSum
                      << " outputSum: " << outputSum << std::endl;
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
                                 [ & ]( const utxo::UTXO& u ) {
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
      bool included    = false;
      bool conflicting = false;
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
         for ( const auto& input : mempoolTx.inputs )
         {
            auto utxoKey = std::make_pair( input.txid, input.outputIndex );
            if ( usedUTXOs.find( utxoKey ) != usedUTXOs.end() )
            {
               conflicting = true;
               break;
            }
         }
      }

      if ( !included && !conflicting )
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

   if ( !isValidPoW( block.hash, block.difficulty ) )
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

   // Now we cann add it to our json file, Bitcoin uses some kind of checkpoints
   // but thats for later TODO: check what checkpoints are
   saveChain( "chain.json" );

   return true;
}

// ----------------------------------------------------------------------------
bool Blockchain::isTransactionDuplicate( const Transaction& tx ) const
{
   auto it = std::find_if( pendingTxs.begin(), pendingTxs.end(),
                           [ & ]( const auto& pendingTx )
                           { return tx == pendingTx; } );

   if ( it != pendingTxs.end() )
   {
      std::cout << "Duplicate sender: " << it->sender
                << " receiver: " << it->receiver << " Amount: " << it->amount
                << std::endl;
   }
   return it != pendingTxs.end();
}

// ----------------------------------------------------------------------------
// bool Blockchain::addToMempool( const Transaction& tx )
//{
//   if ( !isValidTransaction( tx ) )
//   {
//      std::cerr << "Transaction failed validation: " << tx.toJson().dump( 4 )
//                << std::endl;
//      return false;
//   }
//
//   std::cout << "Adding transaction to mempool: " << tx.toJson().dump( 4 )
//             << std::endl;
//   pendingTxs.push_back( tx );
//   return true;
//}

// ----------------------------------------------------------------------------
bool Blockchain::isValidTransaction( const Transaction& tx ) const
{
   // TODO's
   // Add balance check ---->requires state tracking
   // Add signature verification
   if ( tx.sender.empty() || tx.receiver.empty() || tx.amount <= 0 ||
        isTransactionDuplicate( tx ) )
   {
      std::cout << "Invalid TX INFO sender: " << tx.sender
                << " receiver: " << tx.receiver << " amount: " << tx.amount
                << std::endl;
      return false;
   }

   // This is now checked in blockchain.addBlock and here to avoid dead
   // Transactions in pendingTxs
   // Check UTXO validity and balance
   double inputSum = 0.0;
   for ( const auto& input : tx.inputs )
   {
      // Verify UTXO exists in utxoSet
      auto it = std::find_if( utxoSet.begin(), utxoSet.end(),
                              [ & ]( const utxo::UTXO& u ) {
                                 return u.txid == input.txid &&
                                        u.outputIndex == input.outputIndex;
                              } );

      if ( it == utxoSet.end() )
      {
         std::cout << "Invalid TX: UTXO not found for input txid: "
                   << input.txid << " outputIndex: " << input.outputIndex
                   << std::endl;
         return false;
      }

      // Verify input amount matches UTXO amount
      if ( input.amount != it->amount )
      {
         std::cout << "Invalid TX: Input amount mismatch\n";
         return false;
      }

      inputSum += it->amount;

      // Check for double-spending in pendingTxs
      for ( const auto& pendingTx : pendingTxs )
      {
         for ( const auto& pendingInput : pendingTx.inputs )
         {
            if ( pendingInput.txid == input.txid &&
                 pendingInput.outputIndex == input.outputIndex )
            {
               std::cout << "Invalid TX: Double-spend attempt in mempool\n";
               return false;
            }
         }
      }
   }

   // Check balance (outputs + implicit fee)
   double outputSum = 0.0;
   for ( const auto& output : tx.outputs )
   {
      outputSum += output.amount;
   }

   if ( inputSum < outputSum )
   {
      std::cout << "Invalid TX: Insufficient funds (inputSum: " << inputSum
                << ", outputSum: " << outputSum << ")\n";
      return false;
   }

   return true;
}

// ----------------------------------------------------------------------------
bool Blockchain::minePendingTransactions( std::string& minerAddress )
{
   Block block;
   block.index      = chain.size();
   block.prevHash   = chain.back().hash;
   block.timestamp  = getCurrentTime();
   block.txs        = selectTransactions( 10 );
   block.difficulty = calculateExpectedDifficulty();

   double totalFees = 0.0;
   for ( const auto& tx : block.txs )
   {
      if ( !tx.isReward )
      {
         totalFees += tx.fee;
      }
   }

   auto now          = std::chrono::system_clock::now();
   auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                           now.time_since_epoch() )
                           .count();

   // Node which successfully mines a block gets a isReward
   Transaction reward;
   reward.sender    = "network";
   reward.receiver  = minerAddress;
   reward.amount    = 10.0;
   reward.isReward  = true;
   reward.fee       = 0;
   reward.timestamp = getCurrentTime();
   reward.txid      = "reward_blockIDX_" + std::to_string( block.index ) + "_" +
                 std::to_string( milliseconds );
   reward.outputs.push_back( { minerAddress, 10.0 + totalFees } );
   block.txs.insert( block.txs.begin(), reward );

   mineBlock( block, block.difficulty );

   if ( addBlock( block ) )
   {
      std::cout << "Block mined successfully!"
                << std::endl;   // We currently only have it locally

      // Checking if callback has been set;
      if ( broadcastBlockCallback )
      {
         broadcastBlockCallback( block );
      }
      else
      {
         std::cout
             << "Block will not be broadcasted, no callback set!!!!!!!!!\n";
      }

      return true;
   }
   else
   {
      std::cout << "Block validation failed!" << std::endl;
   }

   return false;
}

// ----------------------------------------------------------------------------
std::vector<Transaction> Blockchain::selectTransactions( size_t max )
{
   // I guess locking is kinda important if stuff is distributed
   std::lock_guard<std::mutex> lock( pendingTxsMutex );

   // TODO, i want to add the blocks with highest fees which the node gets
   std::vector<Transaction> selected;
   if ( pendingTxs.size() < max )
   {
      max = pendingTxs.size();
   }

   std::vector<std::pair<int32_t, size_t>> txFeePairs;

   // I dont think this is good for performance
   // Sorting pendingTxs based on fee, which is calculated but total inputs
   // minus total outputs for each transaction
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

   std::cout << "selected size: " << selected.size() << std::endl;
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
std::vector<utxo::UTXO>
Blockchain::getUTXOsForAddress( const std::string& address ) const
{
   std::vector<utxo::UTXO> utxoForAddress;
   for ( const auto& utxo : utxoSet )
   {
      if ( utxo.address == address )
      {
         utxoForAddress.push_back( utxo );
      }
   }

   return utxoForAddress;
}

// ----------------------------------------------------------------------------
int32_t Blockchain::calculateExpectedDifficulty() const
{
   if ( chain.size() % 10 == 0 && chain.size() >= 10 )
   {
      return adjustDifficulty();
   }

   return getEpochDifficulty();
}

// ----------------------------------------------------------------------------
int32_t Blockchain::getEpochDifficulty() const
{
   if ( chain.empty() )
   {
      return 4;   // Anfangs difficulty
   }

   return chain.back().difficulty;
}

// ----------------------------------------------------------------------------
void Blockchain::setBroadcastBlockCallback(
    std::function<void( const Block& )> callback )
{
   broadcastBlockCallback = std::move( callback );
}

// ----------------------------------------------------------------------------
void Blockchain::setSyncChainCallback(
    std::function<std::vector<Block>()> callback )
{
   syncChainCallback = std::move( callback );
}

// ----------------------------------------------------------------------------
void Blockchain::setGetHighestChainHeightCallback(
    std::function<std::pair<int32_t, std::string>()> callback )
{
   getHighestChainHeightCallback = std::move( callback );
}

// ----------------------------------------------------------------------------
void Blockchain::recomputeUTXOSet()
{
   utxoSet.clear();

   for ( const auto& block : chain )
   {
      for ( const auto& tx : block.txs )
      {
         // We adding all outputs of a transaction to the utxoSet as we learned
         // a output becomes a utxo :)
         for ( int32_t i = 0; i < tx.outputs.size(); ++i )
         {
            utxoSet.push_back( { tx.txid, i, tx.outputs[ i ].amount,
                                 tx.outputs[ i ].address } );
         }

         // We skip rewards, they have no inputs
         if ( tx.isReward )
         {
            continue;
         }

         // Trying to cleanup the utxoSet by removing all spent inputs
         for ( const auto& input : tx.inputs )
         {
            // Each input referenes only one utxo
            utxoSet.erase( std::remove_if( utxoSet.begin(), utxoSet.end(),
                                           [ & ]( const auto& utxo ) {
                                              return utxo.txid == input.txid &&
                                                     utxo.outputIndex ==
                                                         input.outputIndex;
                                           } ),
                           utxoSet.end() );
         }
      }
   }
}

// ----------------------------------------------------------------------------
bool Blockchain::saveChain( const std::string& fileName ) const
{
   json j = json::array();
   for ( const auto& block : chain )
   {
      j.push_back( block );
   }

   std::ofstream file( fileName );
   if ( !file.is_open() )
   {
      std::cerr << "Failed to open file: " << fileName << std::endl;
      return false;
   }

   file << std::setw( 4 ) << j << std::endl;
   file.close();

   return true;
}

// ----------------------------------------------------------------------------
bool Blockchain::appendBlockToJson( const std::string& fileName ) const
{
   // Check if the file exists
   bool fileExists = std::filesystem::exists( fileName );

   std::ofstream file;
   if ( fileExists )
   {
      file.open( fileName, std::ios::app );   // Open in append mode
   }
   else
   {
      file.open( fileName );   // Create new file
   }

   if ( !file.is_open() )
   {
      return false;
   }

   if ( !fileExists )
   {
      // If the file is new, write all blocks
      for ( const auto& block : chain )
      {
         json j = block;
         file << j.dump() << std::endl;
      }
   }
   else
   {
      // Find the last block index in the file
      int lastIndex = -1;
      {
         std::ifstream inFile( fileName );
         if ( inFile.is_open() )
         {
            std::string line;

            while ( std::getline( inFile, line ) )
            {
               if ( line.empty() ||
                    line.find_first_not_of( " \t\r\n" ) == std::string::npos )
               {
                  continue;
               }

               try
               {
                  json j = json::parse( line );
                  if ( j.contains( "index" ) )
                  {
                     lastIndex = j[ "index" ];
                  }
               }
               catch ( const std::exception& e )
               {
                  // Skip invalid lines
                  std::cerr << "Skipping invalid JSON line: " << e.what()
                            << std::endl;
               }
            }
            inFile.close();
         }
      }

      // Append only blocks with index greater than lastIndex
      for ( const auto& block : chain )
      {
         if ( block.index > lastIndex )
         {
            json j = block;
            file << j.dump() << std::endl;
         }
      }
   }

   return true;
}

// ----------------------------------------------------------------------------
std::vector<Block> Blockchain::loadChain( const std::string& fileName ) const
{
   std::vector<Block> loadedChain;

   // Check if the file exists
   if ( !std::filesystem::exists( fileName ) )
   {
      std::cerr << "File does not exist: " << fileName << std::endl;
      return loadedChain;   // Return empty chain
   }

   // Check if the file is empty
   if ( std::filesystem::file_size( fileName ) == 0 )
   {
      std::cerr << "File is empty: " << fileName << std::endl;
      return loadedChain;   // Return empty chain
   }

   // Open the file
   std::ifstream file( fileName );
   if ( !file.is_open() )
   {
      std::cerr << "Failed to open file: " << fileName << std::endl;
      return loadedChain;   // Return empty chain
   }

   try
   {
      // Parse the entire file as a JSON array
      json j;
      file >> j;
      file.close();

      // Ensure it's an array
      if ( !j.is_array() )
      {
         std::cerr << "File does not contain a JSON array: " << fileName
                   << std::endl;
         return loadedChain;
      }

      // Convert each JSON object to a Block
      for ( const auto& blockJson : j )
      {
         try
         {
            Block block =  blockJson;
            loadedChain.push_back( block );
         }
         catch ( const json::exception& e )
         {
            std::cerr << "Error parsing block: " << e.what() << std::endl;
         }
      }

      // Validate the loaded chain
      if ( !loadedChain.empty() && !isChainValid( loadedChain ) )
      {
         std::cerr << "Loaded chain is invalid" << std::endl;
         return std::vector<Block>{};
      }
   }
   catch ( const json::exception& e )
   {
      std::cerr << "Error parsing JSON file: " << e.what() << std::endl;
      file.close();
      return loadedChain;
   }

   return loadedChain;
}
//std::vector<Block> Blockchain::loadChain( const std::string& fileName ) const
//{
//   std::cout << "LOADING CHAIN\n";
//   std::ifstream file( fileName );
//   if ( !file.is_open() )
//   {
//      std::cout << "EMPTY\n";
//      return {};
//   }
//
//   json j;
//   std::cout << "BEFOR JSON CAST\n";
//   std::vector<Block> newChain{};
//   if ( std::filesystem::file_size( fileName ) == 0 )
//   {
//      std::cerr << "File is empty: " << fileName << std::endl;
//      return newChain;
//   }
//   file >> j;
//   std::string test;
//   file >> test;
//   std::cout << "TESTTT: " << test << std::endl;
//   std::cout << "CHAINNNNN: " << j.dump() << std::endl;
//
//   for ( const auto& blockJson : j )
//   {
//      std::cout << "BLOCK JSON: " << blockJson.dump( 4 ) << std::endl;
//      newChain.emplace_back( Block( blockJson ) );
//   }
//
//   std::cout << "Returning from loadChain chain has size: " << newChain.size()
//             << std::endl;
//   return newChain;
//}

// ----------------------------------------------------------------------------
Block Blockchain::createGenesisBlock()
{
   Block genesis;
   genesis.index      = 0;
   genesis.prevHash   = "Mojo";
   genesis.timestamp  = getCurrentTime();
   genesis.hash       = genesis.calculateHash();
   genesis.difficulty = 4;

   return genesis;
}
