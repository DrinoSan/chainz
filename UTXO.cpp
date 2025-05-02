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

// ----------------------------------------------------------------------------
void from_json( const json& j, UTXO& u )
{
   j.at( "txid" ).get_to( u.txid );
   j.at( "outputIndex" ).get_to( u.outputIndex );
   j.at( "amount" ).get_to( u.amount );
   j.at( "address" ).get_to( u.address );
}
};   // namespace utxo
