
#include "includes.hpp"
#include "menu.hpp"

int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd )
{

	if ( !menu::get( ).render( 500, 350 ) )
	{
		MessageBoxA( nullptr, "Failed to create directx9 device!", nullptr, 0 );
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}