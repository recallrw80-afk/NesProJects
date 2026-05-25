#include <iostream>
#include <Cartridge.h>

int main(int argc, char** argv)
{
    if ( argc < 2 )
    {
        std::cerr << "Usage: New <rom.nes> \n";
        return 1;
    }

    Cartridge cart;

    if ( !cart.load(argv[1]) )
    {
        std::cerr << "Failed to load ROM.\n";
        return 1;
    }
    #ifdef DEBUG
    cart.print_info();
    #endif
    return 0;
}