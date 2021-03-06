/*
vgui_int.c - vgui dll interaction
Copyright (C) 2011 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "common.h"
#include "client.h"
#include "const.h"
#include "vgui_draw.h"
#include "vgui_main.h"
#include <tlhelp32.h>

CEnginePanel	*rootpanel = NULL;
CEngineSurface	*surface = NULL;
CEngineApp          *pApp = NULL;

SurfaceBase* CEnginePanel::getSurfaceBase( void )
{
	return surface;
}

App* CEnginePanel::getApp( void )
{
	return pApp;
}

void CEngineApp :: setCursorPos( int x, int y )
{
	POINT pt;

	pt.x = x;
	pt.y = y;

	ClientToScreen( (HWND)host.hWnd, &pt );

	::SetCursorPos( pt.x, pt.y );
}
	
void CEngineApp :: getCursorPos( int &x,int &y )
{
	POINT	pt;

	// find mouse movement
	::GetCursorPos( &pt );
	ScreenToClient((HWND)host.hWnd, &pt );

	x = pt.x;
	y = pt.y;
}

void VGui_RunFrame( void )
{
	PROCESSENTRY32	pe32;
	HANDLE		hSnapshot;

	host.force_draw_version = false;

	if(( hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 )) == INVALID_HANDLE_VALUE )
		return;

	pe32.dwSize = sizeof( PROCESSENTRY32 );

	if( !Process32First( hSnapshot, &pe32 ))
	{
		CloseHandle( hSnapshot );
		return;
	}

	do
	{
		if( Q_stristr( pe32.szExeFile, "fraps.exe" ))
		{
			host.force_draw_version = true;
			break;
		}
	} while( Process32Next( hSnapshot, &pe32 ));

	CloseHandle( hSnapshot );
}

void VGui_Startup( void )
{
	if( rootpanel )
	{
		rootpanel->setSize( menu.globals->scrWidth, menu.globals->scrHeight );
		return;
	}

	rootpanel = new CEnginePanel;
	rootpanel->setPaintBorderEnabled( false );
	rootpanel->setPaintBackgroundEnabled( false );
	rootpanel->setVisible( true );
	rootpanel->setCursor( new Cursor( Cursor::dc_none ));

	pApp = new CEngineApp;
	pApp->start();
	pApp->setMinimumTickMillisInterval( 0 );

	surface = new CEngineSurface( rootpanel );
	rootpanel->setSurfaceBaseTraverse( surface );

	ASSERT( rootpanel->getApp() != NULL );
	ASSERT( rootpanel->getSurfaceBase() != NULL );

	VGUI_DrawInit ();
}

void VGui_Shutdown( void )
{
	if( pApp ) pApp->stop();

	delete rootpanel;
	delete surface;
	delete pApp;

	rootpanel = NULL;
	surface = NULL;
	pApp = NULL;
}

void VGui_Paint( void )
{
	RECT	rect;

	if( cls.state != ca_active || !rootpanel )
		return;

	// setup the base panel to cover the screen
	Panel *pVPanel = surface->getEmbeddedPanel();
	if( !pVPanel ) return;

	host.input_enabled = rootpanel->isVisible();
	GetClientRect( host.hWnd, &rect );
	EnableScissor( true );

	if( cls.key_dest == key_game )
	{
		pApp->externalTick ();
	}

	pVPanel->setBounds( 0, 0, rect.right, rect.bottom );
	pVPanel->repaint();

	// paint everything 
	pVPanel->paintTraverse();

	EnableScissor( false );
}

void VGui_ViewportPaintBackground( int extents[4] )
{
//	Msg( "Vgui_ViewportPaintBackground( %i, %i, %i, %i )\n", extents[0], extents[1], extents[2], extents[3] );
}

void *VGui_GetPanel( void )
{
	return (void *)rootpanel;
}