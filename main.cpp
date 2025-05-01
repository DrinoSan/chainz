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

   //std::thread thr( threadMine );
   std::cout << chain.toString() << std::endl;
         chain.minePendingTransactions( nodeAddress );
         std::cout << "HERE\n";

   // chain.addTransaction(
   //     { "Alice", "Bob", 50.0, 0.5, chain.getCurrentTime() } );
   // chain.addTransaction( { "Alice", "Bob", 50.0, chain.getCurrentTime() } );
   // chain.addTransaction(
   //     { "Bob", "Charlie", 25.0, chain.getCurrentTime() } );

   // std::cout << "Mining new block...\n";
   // chain.mineBlock();

   // chain.addTransaction(
   //     { "Charlie", "Alice", 10.0, chain.getCurrentTime() } );
   // chain.addTransaction( { "Alice", "Bob", 5.0, chain.getCurrentTime() } );
   // chain.addTransaction( { "Bob", "Charlie", 2.0, chain.getCurrentTime() } );
   // chain.addTransaction(
   //     { "Charlie", "Alice", 1.0, chain.getCurrentTime() } );

   // chain.mineBlock();

   // chain.addTransaction( { "Alice", "Bob", 1.0, chain.getCurrentTime() } );
   // chain.mineBlock();

   // if ( chain.isChainValid() )
   //{
   //    std::cout << "Blockchain is valid!" << std::endl;
   // }
   // else
   //{
   //    std::cout << "Blockchain is invalid!" << std::endl;
   // }

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
