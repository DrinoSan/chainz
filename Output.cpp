#include "Output.h"

// ----------------------------------------------------------------------------
// Forward declarations of JSON serialization functions
void to_json( json& j, const Output& o )
{
   j = json{ { "address", o.address },
             { "amount", o.amount } };
}

// ----------------------------------------------------------------------------
void from_json( const json& j, Output& o )
{
   j.at( "address" ).get_to( o.address );
   j.at( "amount" ).get_to( o.amount );
}

