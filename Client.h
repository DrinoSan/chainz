#include "Transaction.h"

class Client
{
 public:
   static Transaction createTransaction( const std::string& senderAddr,
                                         const std::string& receiverAddr,
                                         double amount, double fee,
                                         const std::string& privateKey );
};
