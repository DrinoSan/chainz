#include <vector>

#include "Transaction.h"
#include "UTXO.h"
#include "json/json.hpp"

// for convenience
using json = nlohmann::json;

class Client
{
 public:
   // ----------------------------------------------------------------------------
   static Transaction createTransaction( const std::string& senderAddr,
                                         const std::string& receiverAddr,
                                         double amount, double fee,
                                         const std::string& privateKey );

   static void broadcastTransaction( const std::vector<std::string>& peers,
                                     const Transaction&              tx );

 private:
   // ----------------------------------------------------------------------------
   static std::vector<utxo::UTXO> getUtxos( const std::string& address );
};
