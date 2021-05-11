
#include "menu.hpp"
#include "includes.hpp"

void menu::render_items( HWND hwnd, int width, int height )
{
	static auto mouse_offset_x = 0, mouse_offset_y = 0;

	ImGui::SetNextWindowSize( ImVec2( static_cast< float >( width ), static_cast< float >( height ) ), ImGuiCond_Always );
	ImGui::SetNextWindowPos( ImVec2( 0, 0 ), ImGuiCond_Always );

	if ( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
		get_mouse_offset( mouse_offset_x, mouse_offset_y, hwnd );

	ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );

	ImGui::Begin( "##begin", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings );
	{

		if ( mouse_offset_y >= 0 && mouse_offset_y <= ImGui::GetTextLineHeight( ) + ImGui::GetStyle( ).FramePadding.y * 5.f && ImGui::IsMouseDragging( ImGuiMouseButton_Left ) )
			set_position( mouse_offset_x, mouse_offset_y, width, height, false, hwnd );

		


	}

	ImGui::PopStyleVar( );
	ImGui::End( );
}