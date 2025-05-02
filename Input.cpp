#include "Input.h"

// ----------------------------------------------------------------------------
// Forward declarations of JSON serialization functions
void to_json( json& j, const Input& i )
{
   j = json{ { "txid", i.txid },
             { "outputIndex", i.outputIndex },
             { "amount", i.amount },
             { "signature", i.signature } };
}

// ----------------------------------------------------------------------------
void from_json( const json& j, Input& i )
{
   j.at( "txid" ).get_to( i.txid );
   j.at( "outputIndex" ).get_to( i.outputIndex );
   j.at( "amount" ).get_to( i.amount );
   j.at( "signature" ).get_to( i.signature );
}
