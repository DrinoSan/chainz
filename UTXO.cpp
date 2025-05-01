#include "UTXO.h"

namespace utxo
{
// ----------------------------------------------------------------------------
void to_json( json& j, const UTXO& u )
{
   j[ "txid" ]        = u.txid;
   j[ "outputIndex" ] = u.outputIndex;
   j[ "amount" ]      = u.amount;
   j[ "address" ]     = u.address;
}
};

