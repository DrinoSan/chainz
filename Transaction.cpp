#include "Transaction.h"

// ----------------------------------------------------------------------------
Transaction Transaction::fromJson( const json& j )
{
   Transaction tx;
   j.at( "sender" ).get_to( tx.sender );
   j.at( "receiver" ).get_to( tx.receiver );
   j.at( "amount" ).get_to( tx.amount );
   j.at( "timestamp" ).get_to( tx.timestamp );

   return tx;
}

// ----------------------------------------------------------------------------
bool Transaction::operator==( const Transaction& other ) const
{
   return ( sender == other.sender && receiver == other.receiver &&
            amount == other.amount && timestamp == other.timestamp );
}

// Transaction //
// JSON serialization for Transaction //
// ----------------------------------------------------------------------------
json Transaction::toJson() const
{
   return { { "sender", sender },
            { "receiver", receiver },
            { "amount", amount },
            { "timestamp", timestamp } };
}

// ----------------------------------------------------------------------------
void to_json( json& j, const Transaction& b )
{
   j = json{ { "sender", b.sender },
             { "receiver", b.receiver },
             { "amount", b.amount },
             { "timestamp", b.timestamp } };
}

// ----------------------------------------------------------------------------
void from_json( const json& j, Transaction& b )
{
   j.at( "sender" ).get_to( b.sender );
   j.at( "receiver" ).get_to( b.receiver );
   j.at( "amount" ).get_to( b.amount );
   j.at( "timestamp" ).get_to( b.timestamp );
}
