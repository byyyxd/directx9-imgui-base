#pragma once

#include <d3dx9.h>
#include <d3d9.h>
#include "xorstr.hpp"
#include "singleton.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_internal.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

static LPDIRECT3D9 g_pD3D {};
static LPDIRECT3DDEVICE9 g_pd3dDevice {};
static D3DPRESENT_PARAMETERS g_d3dpp {};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

class menu : public singleton<menu>
{
private:
	void render_items( HWND hwnd, int width, int height );

	void set_position( int x, int y, int w, int h, bool b_center, HWND hwnd )
	{
		POINT point {};
		GetCursorPos( &point );

		auto flags = SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE;
		if ( x != 0 && y != 0 )
		{
			x = point.x - x;
			y = point.y - y;
			flags &= ~SWP_NOMOVE;
		}

		if ( w != 0 && h != 0 )
			flags &= ~SWP_NOSIZE;

		if ( b_center )
		{
			RECT rect {};
			if ( w != 0 && h != 0 )
			{
				rect.right = w;
				rect.bottom = h;
			}
			else
			{
				GetWindowRect( hwnd, &rect );
			}

			x = ( GetSystemMetrics( SM_CXSCREEN ) - rect.right ) / 2;
			y = ( GetSystemMetrics( SM_CYSCREEN ) - rect.bottom ) / 2;

			flags &= ~SWP_NOMOVE;
		}

		SetWindowPos( hwnd, nullptr, x, y, w, h, flags );
	}

	void get_mouse_offset( int &x, int &y, HWND hwnd )
	{
		POINT point {};
		RECT rect {};

		GetCursorPos( &point );
		GetWindowRect( hwnd, &rect );

		x = point.x - rect.left;
		y = point.y - rect.top;
	}

	bool create_device_d3d( HWND hWnd )
	{
		if ( ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) == NULL )
			return false;

		ZeroMemory( &g_d3dpp, sizeof( g_d3dpp ) );

		g_d3dpp.Windowed = TRUE;
		g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		g_d3dpp.EnableAutoDepthStencil = TRUE;
		g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

		if ( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice ) < 0 )
			return false;

		return true;
	}

	void cleanup_device_d3d( )
	{
		if ( g_pd3dDevice )
		{
			g_pd3dDevice->Release( );
			g_pd3dDevice = NULL;
		}

		if ( g_pD3D )
		{
			g_pD3D->Release( );
			g_pD3D = NULL;
		}
	}

	void reset_device( )
	{
		ImGui_ImplDX9_InvalidateDeviceObjects( );

		HRESULT hr = g_pd3dDevice->Reset( &g_d3dpp );
		if ( hr == D3DERR_INVALIDCALL )
			IM_ASSERT( 0 );

		ImGui_ImplDX9_CreateDeviceObjects( );
	}

	static LRESULT WINAPI wnd_proc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		if ( ImGui_ImplWin32_WndProcHandler( hWnd, msg, wParam, lParam ) )
			return true;

		switch ( msg )
		{
			case WM_SIZE:
				if ( g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED )
				{
					g_d3dpp.BackBufferWidth = LOWORD( lParam );
					g_d3dpp.BackBufferHeight = HIWORD( lParam );
					menu::get( ).reset_device( );
				}
				return 0;
			case WM_SYSCOMMAND:
				if ( ( wParam & 0xfff0 ) == SC_KEYMENU )
					return 0;
				break;
			case WM_DESTROY:
				PostQuitMessage( 0 );
				return 0;
		}

		return DefWindowProc( hWnd, msg, wParam, lParam );
	}


public:
	~menu( ) = default;
	menu( ) = default;

	bool render( int width, int height ) noexcept
	{

		WNDCLASSEX wc = {
			sizeof( WNDCLASSEX ),
			CS_CLASSDC,
			this->wnd_proc,
			0L,
			0L,
			GetModuleHandle( NULL ),
			NULL,
			NULL,
			NULL,
			NULL,
			"w",
			NULL
		};

		RegisterClassEx( &wc );

		auto hwnd = CreateWindow(
			wc.lpszClassName,
			"",
			WS_POPUP,
			100,
			100,
			width, height,
			NULL,
			NULL,
			wc.hInstance,
			NULL
		);

		if ( !create_device_d3d( hwnd ) )
		{
			cleanup_device_d3d( );
			UnregisterClass( wc.lpszClassName, wc.hInstance );
			return 1;
		}

		ShowWindow( hwnd, SW_SHOWDEFAULT );
		UpdateWindow( hwnd );

		ImGui::CreateContext( );

		auto &io = ImGui::GetIO( );
		const auto style = &ImGui::GetStyle( );

		const auto colors = style->Colors;

		io.Fonts->AddFontFromFileTTF( "C:\\Windows\\Fonts\\Verdana.ttf", 12.f );

		io.IniFilename = nullptr;

		ImGui_ImplWin32_Init( hwnd );
		ImGui_ImplDX9_Init( g_pd3dDevice );

		ImVec4 clear_color = ImVec4( 0.09f, 0.09f, 0.09f, 0.94f );

		bool done = false;
		while ( !done )
		{
			MSG msg {};
			while ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );

				if ( msg.message == WM_QUIT )
					done = true;
			}

			if ( done )
				break;

			ImGui_ImplDX9_NewFrame( );
			ImGui_ImplWin32_NewFrame( );
			ImGui::NewFrame( );

			style->WindowBorderSize = 1.f;
			style->WindowRounding = 0.f;
			style->WindowTitleAlign = ImVec2( 0.5, 0.5 );
			style->FrameRounding = 0.f;
			style->GrabRounding = 0.f;
			style->ChildRounding = 0.f;
			style->ItemSpacing = ImVec2( 5, 10 );
			style->FramePadding = ImVec2( 1, 1 );
			style->PopupRounding = 3.f;

			colors[ ImGuiCol_Border ] = ImColor( 51, 51, 51 );
			colors[ ImGuiCol_BorderShadow ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );

			colors[ ImGuiCol_TitleBg ] = ImVec4( 0.09f, 0.09f, 0.09f, 1.00f );
			colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.09f, 0.09f, 0.09f, 1.00f );
			colors[ ImGuiCol_TitleBgCollapsed ] = ImVec4( 0.09f, 0.09f, 0.09f, 1.00f );

			colors[ ImGuiCol_MenuBarBg ] = ImVec4( 0.08f, 0.08f, 0.08f, 0.00f );
			colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 0.15f, 0.15f, 0.15f, 0.00f );
			colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 0.10f, 0.10f, 0.10f, 0.00f );
			colors[ ImGuiCol_ScrollbarGrabHovered ] = ImVec4( 0.10f, 0.10f, 0.10f, 0.00f );
			colors[ ImGuiCol_ScrollbarGrabActive ] = ImVec4( 0.10f, 0.10f, 0.15f, 0.00f );
			colors[ ImGuiCol_CheckMark ] = ImColor( 0.9f, 0.24f, 0.24f );

			colors[ ImGuiCol_SliderGrab ] = ImColor( 0.9f, 0.24f, 0.24f );
			colors[ ImGuiCol_SliderGrabActive ] = ImColor( 0.9f, 0.24f, 0.24f );

			colors[ ImGuiCol_Button ] = ImColor( 20, 20, 20 );
			colors[ ImGuiCol_ButtonHovered ] = ImColor( 20, 20, 20 );
			colors[ ImGuiCol_ButtonActive ] = ImColor( 20, 20, 20 );

			colors[ ImGuiCol_Header ] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
			colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.13f, 0.13f, 0.13f, 1.00f );
			colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.12f, 0.12f, 0.12f, 1.00f );

			colors[ ImGuiCol_Separator ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.15f );
			colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.15f );
			colors[ ImGuiCol_SeparatorActive ] = ImVec4( 1.00f, 1.00f, 1.00f, 0.15f );

			colors[ ImGuiCol_ResizeGrip ] = ImVec4( 0.30f, 0.30f, 0.30f, 1.00f );
			colors[ ImGuiCol_ResizeGripActive ] = ImVec4( 0.09f, 0.09f, 0.09f, 1.00f );
			colors[ ImGuiCol_PlotLines ] = ImVec4( 0.40f, 0.12f, 0.99f, 0.90f );
			colors[ ImGuiCol_PlotLinesHovered ] = ImVec4( 0.40f, 0.12f, 0.99f, 0.90f );
			colors[ ImGuiCol_PlotHistogram ] = ImVec4( 0.40f, 0.12f, 0.99f, 0.90f );
			colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 0.40f, 0.12f, 0.99f, 0.90f );
			colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.09f, 0.09f, 0.09f, 1.00f );

			colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( 0.00f, 0.00f, 0.00f, 1.05f );
			colors[ ImGuiCol_DragDropTarget ] = ImVec4( 0.00f, 0.00f, 0.00f, 0.40f );

			colors[ ImGuiCol_WindowBg ] = ImColor( 28, 28, 28 );
			colors[ ImGuiCol_TitleBgActive ] = ImColor( 20, 20, 20 );
			colors[ ImGuiCol_FrameBg ] = ImColor( 45, 45, 45 );
			colors[ ImGuiCol_FrameBgHovered ] = ImColor( 45, 45, 45 );
			colors[ ImGuiCol_FrameBgActive ] = ImColor( 45, 45, 45 );
			colors[ ImGuiCol_ChildBg ] = ImColor( 23, 23, 23 );

			ImGui::StyleColorsDark( );

			render_items( hwnd, width, height );

			ImGui::EndFrame( );
			g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
			g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
			g_pd3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );
			D3DCOLOR clear_col_dx = D3DCOLOR_RGBA( ( int ) ( clear_color.x * clear_color.w * 255.0f ), ( int ) ( clear_color.y * clear_color.w * 255.0f ), ( int ) ( clear_color.z * clear_color.w * 255.0f ), ( int ) ( clear_color.w * 255.0f ) );
			g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0 );

			if ( g_pd3dDevice->BeginScene( ) >= 0 )
			{
				ImGui::Render( );
				ImGui_ImplDX9_RenderDrawData( ImGui::GetDrawData( ) );
				g_pd3dDevice->EndScene( );
			}

			HRESULT result = g_pd3dDevice->Present( NULL, NULL, NULL, NULL );

			if ( result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel( ) == D3DERR_DEVICENOTRESET )
				reset_device( );

		}

		ImGui_ImplDX9_Shutdown( );
		ImGui_ImplWin32_Shutdown( );
		ImGui::DestroyContext( );
		cleanup_device_d3d( );
		DestroyWindow( hwnd );
		UnregisterClass( wc.lpszClassName, wc.hInstance );

		return 0;
	}

};

