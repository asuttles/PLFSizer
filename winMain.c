/**

				      FAIRING SIZER

				       WINDOWS MAIN

Fairing Sizer - Show PLF Dimensions on Notebook Paper Sized Drawing
Copyright (C) 2019  Andrew C. Suttles

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Andrew Suttles - andrew.suttles@gmail.com
**/

#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "winMenu.h"
#include "winForms.h"
#include "winDraw.h"


static const TCHAR szClassName[] = TEXT( "Main Window" );


// COMMON CONTROLS
//INITCOMMONCONTROLSEX initialize_common_controls( void ) {

void initialize_common_controls( void ) {

  INITCOMMONCONTROLSEX icc;

  icc.dwSize = sizeof(icc);				    /* Size of common control struct */
  icc.dwICC = ICC_WIN95_CLASSES;                            /* Bit flags defining loaded controls */
  InitCommonControlsEx(&icc);
  
  //  return icc;
  return;
}


// PROCESS MESSAGES
LRESULT CALLBACK WndProc( HWND hwnd, 			    /* Window Handle */
			  UINT msg, 			    
			  WPARAM wParam,                    /* wParam = HIWORD(wParam) and LOWORD(wParam) */	
			  LPARAM lParam ) {		    /* HWND of control which sent message */            

  static int iScrollUnits;
  static int iVscrollPos = 0;
  static HWND hFormsWindow, hDrawWindow;

  switch( msg ) {

    // Handle Commands or System Commands
  case WM_SYSCOMMAND:				            /* Accelerator keystrokes selecting menu items */
  case WM_COMMAND: 					    /* Menu, Control message, Accelerator */
    switch( LOWORD( wParam ))  {
	  
    case ID_HELP_ABOUT: {

      /* Create Modeless dialog */
      int ret = DialogBox( GetModuleHandle(NULL),
  			   MAKEINTRESOURCE(IDD_ABOUTDIALOG), hwnd, AboutDialogProc);

      if (ret == -1) {
  	MessageBox(hwnd, TEXT( "Dialog failed!" ), TEXT( "Error" ),
  		   MB_OK | MB_ICONINFORMATION);
      }
    }

      break;

    case ID_FILE_EXIT:
      PostMessage( hwnd, WM_CLOSE, 0, 0 );
      break;

    default:
      return DefWindowProc( hwnd, msg, wParam, lParam );			    
    }
    
    break;
    

    // Creating Main Window and Sub-Windows...
  case WM_CREATE: {
    
    /* Startup Sound */
    PlaySound( TEXT ("SystemStart"), NULL, SND_ALIAS );

    /* Create Forms Window as Child Window Control */
    if (( hFormsWindow = CreateFormsWindow( hwnd, 
					    FORMS_WINDOW_HEIGHT,
					    FORMS_WINDOW_WIDTH,
					    FORMS_WINDOW_X,
					    FORMS_WINDOW_Y
					    )) == NULL ) {
    
      MessageBox( NULL, TEXT( "Forms Window Creation Failed!" ), TEXT( "Error!" ),
		  MB_ICONERROR | MB_OK );

      PostMessage( hwnd, WM_CLOSE, 0, 0 );
    }


    /* Create Drawing Window as Child Window Control */
    if (( hDrawWindow = CreateDrawingWindow( hwnd, 
					     DRAWING_WINDOW_HEIGHT,
					     DRAWING_WINDOW_WIDTH,
					     DRAWING_WINDOW_X,
					     DRAWING_WINDOW_Y
					     )) == NULL ) {
    
      MessageBox( NULL, TEXT( "Drawing Window Creation Failed!" ), TEXT( "Error!" ),
		  MB_ICONERROR | MB_OK );
      
      PostMessage( hwnd, WM_CLOSE, 0, 0 );
    }

    /* Set Scroll Bar Position */
    SetScrollRange( hwnd, SB_VERT, 0, MAIN_WINDOW_SCROLL_RANGE, FALSE );
    SetScrollPos(   hwnd, SB_VERT, iVscrollPos, TRUE );

    /* Define Scroll Units */
    RECT rDesktop;
    GetWindowRect( GetDesktopWindow(), &rDesktop );
    iScrollUnits = ( rDesktop.bottom - rDesktop.top ) / MAIN_WINDOW_SCROLL_RANGE;

  }
    break;


    // (Re)Paint Main Window 
  case WM_PAINT:
    InvalidateRect( hDrawWindow, NULL, TRUE );		    /* Force Drawing to Re-Draw/Update */
    return DefWindowProc( hwnd, msg, wParam, lParam );


    // Handle Mouse Scrolling
  case WM_VSCROLL: {
   
    RECT rChild;
    POINT pOrigin;
    
    int iOldVscrollPos = iVscrollPos;
 

    switch ( LOWORD( wParam )) {

    case SB_TOP:
      iVscrollPos = 0;
      break;

    case SB_BOTTOM:
      iVscrollPos = MAIN_WINDOW_SCROLL_RANGE;
      break;

    case SB_LINEUP:
      iVscrollPos -= 1;
      break ;
     
    case SB_LINEDOWN:
      iVscrollPos += 1;
      break ;
     
    case SB_PAGEUP:
      iVscrollPos -= 2;
      break ;
     
    case SB_PAGEDOWN:
      iVscrollPos += 2;
      break ;
     
    case SB_THUMBPOSITION:
      iVscrollPos = HIWORD( wParam );
      break ;
     
    default :
      break ;
    }

    /* Set Scroll Bar Position */
    iVscrollPos = ( iVscrollPos < 0 ) ? 0 : iVscrollPos;
    iVscrollPos = ( iVscrollPos > MAIN_WINDOW_SCROLL_RANGE ) ? MAIN_WINDOW_SCROLL_RANGE : iVscrollPos;


    /* Re-Draw All Windows, IFF New Scroll Position */
    if( iVscrollPos != GetScrollPos( hwnd, SB_VERT )) {

      SetScrollPos( hwnd, SB_VERT, iVscrollPos, TRUE );

      /* Scroll & Re-Paint Child Windows */
      pOrigin.x = pOrigin.y = 0;			    /* Main Window Client Origin */
      ClientToScreen( hwnd, &pOrigin );			    /* Convert to Screen Units */

      /* Forms Window */
      GetWindowRect( hFormsWindow, &rChild );
      MoveWindow( hFormsWindow, 
		  rChild.left - pOrigin.x,
		  ( rChild.top - pOrigin.y ) - ( iScrollUnits * ( iVscrollPos - iOldVscrollPos )),
		  rChild.right - rChild.left,
		  rChild.bottom - rChild.top,
		  TRUE );

      /* Drawing Window */
      GetWindowRect( hDrawWindow, &rChild );
      MoveWindow( hDrawWindow, 
		  rChild.left - pOrigin.x,
		  ( rChild.top - pOrigin.y ) - ( iScrollUnits * ( iVscrollPos - iOldVscrollPos )),
		  rChild.right - rChild.left,
		  rChild.bottom - rChild.top,
		  TRUE );

      InvalidateRect( hwnd, NULL, TRUE );
    }
  }
    break;


    // Process Mouse Scroll
  case WM_MOUSEWHEEL:

    if ( GET_WHEEL_DELTA_WPARAM( wParam ) > 0 )
      SendMessage( hwnd, WM_VSCROLL, SB_LINEUP, 0 );
    else
      SendMessage( hwnd, WM_VSCROLL, SB_LINEDOWN, 0 );
    
    break;

    // Don't Let WinMain Lose Focus
  /* case WM_KILLFOCUS: */
  /*   SetFocus( hwnd ); */
  /*   break; */

    // Gracefully close program...
  case WM_CLOSE:
    DestroyWindow( hwnd );
    break;
    
  case WM_DESTROY:
    PostQuitMessage( 0 );				    /* Post WM_QUIT to message loop */
    break;
    
  default:
    return DefWindowProc( hwnd, msg, wParam, lParam );			    
  }
  
  return 0;
}


// CREATE MAIN WINDOW CLASS
BOOL CreateMainWindowClass( HINSTANCE hInstance ) {

  WNDCLASSEX wc;

  wc.cbSize        = sizeof( WNDCLASSEX );                  /* Size of wc structure */
  wc.style         = 0;					    /* Class Style */
  wc.lpfnWndProc   = WndProc;				    /* Pointer to Window Procedure */
  wc.cbClsExtra    = 0;					    /* Extra Data Memory Allocation (per Class) */
  wc.cbWndExtra    = 0;					    /* Extra Data Memory Allocation (per Window) */
  wc.hInstance     = hInstance;				    /* Handle to app instance - received from WinMain */
                                                            /* Icon/Cursor Resources */
  wc.hIcon         = (HICON) LoadIcon( hInstance, MAKEINTRESOURCE(IDI_ROCKETICON) );
  wc.hIconSm       = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, LR_SHARED);
  wc.hCursor       = (HCURSOR) LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground = (HBRUSH)( COLOR_WINDOW+2 );	    /* Window Background Color */
  wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MAINMENU);	    /* Name of Menu Resource */
  wc.lpszClassName = szClassName;			    /* Name for this Window Class */


  // Register the Window Class
  if ( !RegisterClassEx( &wc) ) {
    MessageBox( NULL, TEXT( "Failed to Register Main Window Class!" ), TEXT( "Error!" ),
		MB_ICONERROR | MB_OK );
    return FALSE;
  }

  return TRUE;
}


// MAIN PROGRAM
int WINAPI WinMain( HINSTANCE hInstance,		    /* Handle for program instance */
		    HINSTANCE hPrevInstance,		    /* NULL - ignore in Win32 */
		    LPSTR     lpCmdLine,		    /* Command line args */
		    int       nCmdShow) {	      

  // Windows Handles
  HWND hWndMain;					    /* Main Window Handle */
  HACCEL hAccelerators;					    /* Keyboard Accelerator */
  HMENU hSysMenu;					    /* System Menu */

  // Message Structure
  MSG Msg;						    /* Message Structure */

  // Initialise common controls.
  //INITCOMMONCONTROLSEX icc = initialize_common_controls();
  initialize_common_controls();

  // Define 'Main' Window Class 
  if( !CreateMainWindowClass( hInstance )) {
    return -1;
  }
  
  // Create an Instance of the Main Window Class
  hWndMain = CreateWindowEx(
  			    WS_EX_CLIENTEDGE,		    /* 'Extended' Windows Style */
  			    szClassName,		    /* The class name we created above */
  			    TEXT("Payload Fairing Sizing"), /* Window Title */
  			    WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
  			    CW_USEDEFAULT,		    /* X,Y Coords top-left corner */
  			    CW_USEDEFAULT,
  			    MAIN_WINDOW_WIDTH,
  			    MAIN_WINDOW_HEIGHT,
  			    (HWND) NULL,		    /* Parent Window Handle */
  			    (HMENU) NULL,		    /* Menu Handle */
  			    hInstance, 			    /* App Instance Handle (from WinMain) */
  			    NULL );			    /* Window Creation Data */

  if( hWndMain == NULL ) {

    MessageBox( NULL, TEXT( "Main Window Creation Failed!" ), TEXT( "Error!" ),
		MB_ICONERROR | MB_OK );
    return -1;
  }

  // Load the Keyboard Accelerators
  hAccelerators = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDR_ACCELERATOR) );

  // Create the System Menu
  hSysMenu = GetSystemMenu( hWndMain, FALSE );
  InsertMenu( hSysMenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
  InsertMenu( hSysMenu, 6, MF_BYPOSITION, ID_HELP_ABOUT, TEXT("About"));

  // Show the Main Window
  ShowWindow( hWndMain, nCmdShow );
  UpdateWindow( hWndMain );		                    /* Send WM_PAINT Message */


  // The Message Loop (blocking call to message queue)
  while( GetMessage( &Msg, 				    /* Pointer to MSG structure */
		     NULL, 				    /* Handle to Window whose messages retrieved */
		                                            /* NULL = any in current thread */
		     0, 				    /* Filtering - key vs. mouse */
		     0) > 0) {				    /* Filter which message to retrieve */


    //    if ( !IsDialogMessage( hWndMain, &Msg )) {		    /* Convert dialog message into Window Message */

      TranslateMessage( &Msg );				   
      DispatchMessage(  &Msg );		   		    /* Call the Window Procedure for this Window */
    }
  //  }

  return Msg.wParam;
}


/**
   Local Variables: **
   mode:c **
   comment-column:60 **
   fill-column: 120 **
   compile-command:"gcc -D UNICODE -D _UNICODE -D _WIN32_IE=0x0300 -c winMain.c -o winMain.o" **
   End: **
**/
