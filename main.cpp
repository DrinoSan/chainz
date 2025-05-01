#include <chrono>
#include <iostream>
#include <thread>

#include "Blockchain.h"
#include "Node.h"

int main( int argc, char* argv[] )
{
   std::string port;
   if ( argc < 2 )
   {
      port = "8080";
   }
   else
   {
      port = argv[ 1 ];
   }

   Blockchain chain;

   std::string nodeAddress{ "NodePort:" + port };
   auto        threadMine = [ & ]()
   {
      while ( true )
      {
         chain.minePendingTransactions( nodeAddress );
         std::this_thread::sleep_for( std::chrono::milliseconds( 5000 ) );
      }
   };

   // std::thread thr( threadMine );
   std::cout << chain.toString() << std::endl;
   chain.minePendingTransactions( nodeAddress );

   std::string nodeSignature = nodeAddress + "_SIG";
   ///////////////
   auto tx = Transaction::createTransaction( nodeAddress, "AliceAddr", 5, 1,
                                             chain.utxoSet, nodeSignature );
   chain.addTransaction( tx );
   std::cout << "Mining new block...\n";
   chain.minePendingTransactions( nodeAddress );
   ///////////////


   sleep(5);
   ///////////////
   auto tx4 = Transaction::createTransaction( nodeAddress, "AliceAddr4", 5, 1,
                                              chain.utxoSet, nodeSignature );
   chain.addTransaction( tx4 );
   std::cout << "Mining new block...\n";
   chain.minePendingTransactions( nodeAddress );
   ///////////////

   if ( chain.isChainValid() )
   {
      std::cout << "Blockchain is valid!" << std::endl;
   }
   else
   {
      std::cout << "Blockchain is invalid!" << std::endl;
   }

   std::cout << chain.toString() << std::endl;

   std::string              host = "localhost";
   std::vector<std::string> peersDummy{ "8080", "8081", "8082" };
   std::vector<std::string> peers;

   // This should be moved into a config or request from a remote server
   // which keeps track of peers
   int32_t portInt = std::stoi( port );
   std::cout << "Running on Port: " << portInt << std::endl;
   for ( auto& peer : peersDummy )
   {
      if ( peer == port )
         continue;

      peers.push_back( "localhost:" + peer );
      std::cout << "Added peer localhost:" + peer << std::endl;
   }

   Node node( chain, host, portInt, peers );
   return 0;
}
