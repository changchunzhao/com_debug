///////////////////////////////////////
// main.cpp

#include "stdafx.h"
#include "TabbedMDIApp.h"

CTabbedMDIApp *theApp_ptr;

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        // Start Win32++
        CTabbedMDIApp theApp;

		theApp_ptr=&theApp;
        // Run the application
        return theApp.Run();
    }
    
    // catch all unhandled CException types
    catch (const CException &e)
    {
        // Display the exception and quit
        MessageBox(NULL, e.GetText(), AtoT(e.what()), MB_ICONERROR);

        return -1;
    }
}
