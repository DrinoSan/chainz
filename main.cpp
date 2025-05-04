#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>

#include "Blockchain.h"
#include "Node.h"

std::mutex blockchainMutex;

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


   //if ( chain.isChainValid() )
   //{
   //   std::cout << "Blockchain is valid!" << std::endl;
   //}
   //else
   //{
   //   std::cout << "Blockchain is invalid!" << std::endl;
   //}

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

   Blockchain chain;
   std::string nodeAddress{ "NodePort:" + port };

   Node node( chain, host, portInt, peers );
   chain.setupChain();

   auto        threadMine = [ & ]()
   {
      while ( true )
      {
         {
            std::lock_guard<std::mutex> lock( blockchainMutex );
            chain.minePendingTransactions( nodeAddress );
         }
         std::this_thread::sleep_for( std::chrono::milliseconds( 5000 ) );
      }
   };
   std::thread thr( threadMine );

   node.svr.listen( host.c_str(), portInt );
   thr.join(); // Ensure the mining thread runs indefinitely
   return 0;
}
