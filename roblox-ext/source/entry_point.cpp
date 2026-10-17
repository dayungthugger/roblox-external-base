#include <framework/entities/entities.hpp>
#include <framework/globals.hpp>
#include <overlay/overlay.hpp>

int main( )
{
    if ( !g_overlay.init( ) )
        return 1;

    datamodel_t::tick( );
    cache::tick( );

    while ( g_overlay.begin_frame( ) )
    {
        g_overlay.end_frame( );
    }

    cache::stop( );
    datamodel_t::stop( );
    g_overlay.shutdown( );

    return 0;
}
