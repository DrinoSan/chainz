#include <iostream>

#include "Blockchain.h"

int main()
{
   Blockchain chain;

   chain.addTransaction( { "Alice", "Bob", 50.0, chain.getCurrentTime() } );
   chain.addTransaction( { "Bob", "Charlie", 25.0, chain.getCurrentTime() } );

   std::cout << "Mining new block..." << std::endl;
   chain.mineBlock();

   chain.addTransaction( { "Charlie", "Alice", 10.0, chain.getCurrentTime() } );
   chain.addTransaction( { "Alice", "Bob", 5.0, chain.getCurrentTime() } );
   chain.addTransaction( { "Bob", "Charlie", 2.0, chain.getCurrentTime() } );
   chain.addTransaction( { "Charlie", "Alice", 1.0, chain.getCurrentTime() } );

   chain.mineBlock();

   chain.addTransaction( { "Alice", "Bob", 1.0, chain.getCurrentTime() } );
   chain.mineBlock();

   if( chain.isChainValid() )
   {
      std::cout << "Blockchain is valid!" << std::endl;
   }
   else
   {
      std::cout << "Blockchain is invalid!" << std::endl;
   }

   std::cout << chain.toString() << std::endl;

   return 0;
}
