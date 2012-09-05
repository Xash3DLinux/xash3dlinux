/*
game.cpp -- executable to run Xash Engine
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

#ifdef _WIN32
#include <windows.h>
#else
#include "recdefs.h"
#include <stdlib.h>
#include <sys/types.h>
#include <dlfcn.h>

//X Windows
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#endif


#define GAME_PATH	"valve"	// default dir to start from

typedef void (*pfnChangeGame)( const char *progname );
typedef int (*pfnInit)( const char *progname, int bChangeGame, pfnChangeGame func );
typedef void (*pfnShutdown)( void );

pfnInit Host_Main;
pfnShutdown Host_Shutdown = NULL;
char szGameDir[128]; // safe place to keep gamedir
HINSTANCE	hEngine;

#ifndef _WIN32
int MessageBoxLinux( const char *lpText, const char *lpCaption )
{
	Widget top_wid, button;
	XtAppContext  app;

	int argc2 = 0;
	char **argv2 = NULL;

	top_wid = XtVaAppInitialize(&app, (char *)lpCaption, NULL, 0, &argc2, argv2, NULL, NULL);

	button = XmCreatePushButton(top_wid, (char *)lpText, NULL, 0);

	/* tell Xt to manage button */
	XtManageChild(button);

	XtRealizeWidget(top_wid); /* display widget hierarchy */
	XtAppMainLoop(app); /* enter processing loop */

	return 1;
}
#endif

void Sys_Error( const char *errorstring )
{
#ifdef _WIN32
	MessageBox( NULL, errorstring, "Xash Error", MB_OK|MB_SETFOREGROUND|MB_ICONSTOP );
#else
	MessageBoxLinux( errorstring, "Xash Error" );
#endif
	exit( 1 );
}

void Sys_LoadEngine( void )
{
#ifdef _DEDICATED
#ifdef _WIN32
	if(( hEngine = LoadLibrary( "dedicated.dll" )) == NULL )
	{
		Sys_Error( "Unable to load the dedicated.dll" );
	}

	if(( Host_Main = (pfnInit)GetProcAddress( hEngine, "Host_Main" )) == NULL )
	{
		Sys_Error( "xash.dll missed 'Host_Main' export" );
	}

	// this is non-fatal for us but change game will not working
	Host_Shutdown = (pfnShutdown)GetProcAddress( hEngine, "Host_Shutdown" );
#else
	if(( hEngine = dlopen( "dedicated.so", RTLD_NOW )) == NULL )
	{
		Sys_Error( "Unable to load the dedicated.so" );
	}

	if(( Host_Main = (pfnInit)dlsym( hEngine, "Host_Main" )) == NULL )
	{
		Sys_Error( "xash.dll missed 'Host_Main' export" );
	}

	// this is non-fatal for us but change game will not working
	Host_Shutdown = (pfnShutdown)dlsym( hEngine, "Host_Shutdown" );
#endif
#else
#ifdef _WIN32
	if(( hEngine = LoadLibrary( "xash.dll" )) == NULL )
	{
		Sys_Error( "Unable to load the xash.dll" );
	}

	if(( Host_Main = (pfnInit)GetProcAddress( hEngine, "Host_Main" )) == NULL )
	{
		Sys_Error( "xash.dll missed 'Host_Main' export" );
	}

	// this is non-fatal for us but change game will not working
	Host_Shutdown = (pfnShutdown)GetProcAddress( hEngine, "Host_Shutdown" );
#else
	if(( hEngine = dlopen( "xash.so", RTLD_NOW )) == NULL )
	{
		Sys_Error( "Unable to load the xash.so" );
	}

	if(( Host_Main = (pfnInit)dlsym( hEngine, "Host_Main" )) == NULL )
	{
		Sys_Error( "xash.dll missed 'Host_Main' export" );
	}

	// this is non-fatal for us but change game will not working
	Host_Shutdown = (pfnShutdown)dlsym( hEngine, "Host_Shutdown" );
#endif
#endif
}

void Sys_UnloadEngine( void )
{

	if( Host_Shutdown ) Host_Shutdown( );

#ifdef _WIN32
	if( hEngine ) FreeLibrary( hEngine );
#else
	if( hEngine ) dlclose( hEngine );
#endif
}

void Sys_ChangeGame( const char *progname )
{
	if( !progname || !progname[0] ) Sys_Error( "Sys_ChangeGame: NULL gamedir" );
	if( Host_Shutdown == NULL ) Sys_Error( "Sys_ChangeGame: missed 'Host_Shutdown' export\n" );
	strncpy( szGameDir, progname, sizeof( szGameDir ) - 1 );

	Sys_UnloadEngine ();
	Sys_LoadEngine ();
	
	Host_Main( szGameDir, TRUE, ( Host_Shutdown != NULL ) ? Sys_ChangeGame : NULL );
}

#ifdef _WIN32
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
#else
int main(int argc, char *argv[])
#endif
{
	Sys_LoadEngine();

	return Host_Main( GAME_PATH, FALSE, ( Host_Shutdown != NULL ) ? Sys_ChangeGame : NULL );
}