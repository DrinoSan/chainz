#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Client.h"

// ----------------------------------------------------------------------------
std::string readPrivateKeyFromFile( const std::string& filePath )
{
   std::ifstream file( filePath );
   if ( !file.is_open() )
   {
      throw std::runtime_error( "Failed to open private key file: " +
                                filePath );
   }

   std::string privateKey;
   std::getline( file, privateKey );
   file.close();
   return privateKey;
}

// ----------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
   if ( argc < 9 )
   {
      std::cerr << "Usage: ./client -sender <sender> -receiver <receiver> "
                   "-amount <amount> -fee <fee> -keyfile <keyfile>"
                << std::endl;
      return 1;
   }

   std::string sender, receiver, keyfile;
   double      amount = 0.0, fee = 0.0;

   for ( int i = 1; i < argc; i += 2 )
   {
      std::string arg = argv[ i ];
      if ( arg == "-sender" )
      {
         sender = argv[ i + 1 ];
      }
      else if ( arg == "-receiver" )
      {
         receiver = argv[ i + 1 ];
      }
      else if ( arg == "-amount" )
      {
         amount = std::stod( argv[ i + 1 ] );
      }
      else if ( arg == "-fee" )
      {
         fee = std::stod( argv[ i + 1 ] );
      }
      else if ( arg == "-keyfile" )
      {
         keyfile = argv[ i + 1 ];
      }
      else
      {
         std::cerr << "Unknown argument: " << arg << std::endl;
         return 1;
      }
   }

   try
   {
      std::string privateKey = readPrivateKeyFromFile( keyfile );
      Transaction tx = Client::createTransaction( sender, receiver, amount, fee,
                                                  privateKey );

      if ( !tx.txid.empty() )
      {
         std::cout << "Transaction created successfully:\n"
                   << tx.toJson().dump( 4 ) << std::endl;

         std::vector<std::string> peers{ "localhost:8080" };
         Client::broadcastTransaction( peers, tx );
      }
      else
      {
         std::cout << "No Transaction created\n";
      }
   }
   catch ( const std::exception& e )
   {
      std::cerr << "Error: " << e.what() << std::endl;
      return 1;
   }

   return 0;
}
