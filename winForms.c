/**

						     FAIRING SIZER
						   DATA FORMS WINDOW

/**
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

#include <stdio.h>
#include <math.h>

#include "winForms.h"
#include "resource.h"
#include "ogive.h"
#include "calcSurfaceArea.h"
#include "calcVolume.h"

#include "lua.h"
#include "lualib.h"
#include "luaxlib.h"


// Windows Synonym
#define swprintf _snwprintf

// Math Macros
#define fROUND(X) (double)floor(X+0.500001)
#define RADIUS(x) ((x) / 2.0)
#define PI (3.141592653589793)
#define DEG2RAD(X) (X * PI) / 180.0

// Boat Tail Sensitivity
#define ETA 0.01


// Child Window Geometries
#define CALCULATED_DATA_ORIGIN_X 125
#define CALCULATED_DATA_ORIGIN_Y 200

#define UNITS_COMBO_LABEL_X 5
#define UNITS_COMBO_LABEL_Y 10
#define UNITS_COMBO_LABEL_WIDTH 100
#define UNITS_COMBO_LABEL_HEIGHT 30

#define UNITS_COMBO_LBOX_X 5
#define UNITS_COMBO_LBOX_Y 30
#define UNITS_COMBO_LBOX_WIDTH  150
#define UNITS_COMBO_LBOX_HEIGHT 300

#define REFRESH_BUTTON_X 5
#define REFRESH_BUTTON_Y 60
#define REFRESH_BUTTON_WIDTH 120
#define REFRESH_BUTTON_HEIGHT 40

#define NASTRAN_BUTTON_X 5
#define NASTRAN_BUTTON_Y 105
#define NASTRAN_BUTTON_WIDTH 120
#define NASTRAN_BUTTON_HEIGHT 40

#define EDIT_CONTROL_TITLE_X 250
#define EDIT_CONTROL_TITLE_Y   5
#define EDIT_CONTROL_TITLE_WIDTH 200
#define EDIT_CONTROL_TITLE_HEIGHT 30

#define EDIT_CONTROL_LABEL_WIDTH  150
#define EDIT_CONTROL_LABEL_HEIGHT  30

#define EDIT_CONTROL_ENTRY_WIDTH  50
#define EDIT_CONTROL_ENTRY_HEIGHT 20


//    Child Window IDs
/** 
    Note:
    IDs 0-99 Edit Forms
        100  Recalc Button
	200  Units List Box
**/
#define REFRESH_BUTTON_ID 100
#define UNITS_COMBO_LBOX_ID 200
#define NASTRAN_BUTTON_ID 300

// Private Prototypes
static  double _getValue( int ID );
LRESULT CALLBACK EntryFormsProc( HWND, UINT, WPARAM, LPARAM );


/** --------------------------------------------------------------------------------------------------------------------

						      MODULE DATA

    ------------------------------------------------------------------------------------------------------------------- **/

// Lua Global State
static lua_State *L;
static int l_ogiveY( lua_State * );
static int l_getLength( lua_State * );
static int l_getOgiveLength( lua_State * );
static int l_getBaseDiameter( lua_State * );
static int l_getBarrelLength( lua_State * );
static int l_getNoseRadius( lua_State * );
static int l_getNoseCapStartingX( lua_State * );

// Window Data
static const TCHAR szClassName[] = TEXT( "Forms Window" );
static HWND hUnitsComboBox;


#define MAX_LABEL_LENGTH 32

enum GEOMETRY_TYPE {
  BASE_DIAMETER,
  HEIGHT,
  OGIVE_RADIUS,
  NOSE_RADIUS,   
  DYNAMIC_DIAMETER,
  SPLITLINE,
  BT_DIAMETER,
  BT_ANGLE 
};

/* Data Structure for Fairing Geometry */
struct editItems {
  
  int  ID;
  BOOL getsFocus;
  HWND hControl;
  WNDPROC process;
  TCHAR label[MAX_LABEL_LENGTH];
  TCHAR entry[MAX_LABEL_LENGTH];
  int   X;
  int   Y;
}
//    id                FOCUS?                                                                          X   Y
  static sizeData[] = {
    { BASE_DIAMETER,    TRUE,  (HWND)NULL, (WNDPROC) NULL, TEXT( "Barrel Diameter" ),  TEXT( "27.6" ), 180, 30 },
    { HEIGHT,           FALSE, (HWND)NULL, (WNDPROC) NULL, TEXT( "Height" ),           TEXT( "119.3" ),180, 52 },
    { OGIVE_RADIUS,     FALSE, (HWND)NULL, (WNDPROC) NULL, TEXT( "Ogive Radius" ),     TEXT( "71.6" ), 180, 74 },
    { NOSE_RADIUS,      FALSE, (HWND)NULL, (WNDPROC) NULL, TEXT( "Nose Radius" ),      TEXT( "4.58" ), 180, 96 },
    { DYNAMIC_DIAMETER, FALSE, (HWND)NULL, (WNDPROC) NULL, TEXT( "PL Diameter" ),      TEXT( "24.6" ), 375, 30 },
    { SPLITLINE,        FALSE, (HWND)NULL, (WNDPROC) NULL, TEXT( "Low Barrel Len" ),   TEXT( "56.6" ), 375, 52 },
    { BT_DIAMETER,      FALSE, (HWND)NULL, (WNDPROC) NULL, TEXT( "BT Diameter" ),      TEXT( "27.6" ), 375, 74 },
    { BT_ANGLE,         FALSE, (HWND)NULL, (WNDPROC) NULL, TEXT( "BT Angle" ),         TEXT( "24.0" ), 375, 96 } };


static int lenSizeData = sizeof( sizeData ) / sizeof( struct editItems );


/* Data Structure for Units */

enum UNITS_TYPE {
  FEET=0,
  METERS
};


struct unitItems {

  int ID;
  TCHAR label[MAX_LABEL_LENGTH];
  TCHAR lenString[3];
  BOOL isDefault;
} 

  static unitsData[] = {
    { FEET,   TEXT( "English (lb-ft)" ), TEXT( "ft" ), TRUE  },
    { METERS, TEXT( "Metric (N-m)" ),    TEXT( "m" ),  FALSE } };
    
static int lenUnitsData = sizeof( unitsData ) / sizeof( struct unitItems );


/** --------------------------------------------------------------------------------------------------------------------

							DRAWING

    ------------------------------------------------------------------------------------------------------------------- **/


// Draw the List Controls
static void DrawUnitsCombo( HWND hWnd, 
			    HINSTANCE hInstance ) {

  int i;

  /* Draw the List Box Title */
  CreateWindowEx( 0,
		  TEXT( "STATIC" ),
		  TEXT( "Units Type" ),
		  WS_CHILD | WS_VISIBLE | SS_LEFT,
		  UNITS_COMBO_LABEL_X, UNITS_COMBO_LABEL_Y, 
		  UNITS_COMBO_LABEL_WIDTH, UNITS_COMBO_LABEL_HEIGHT,
		  (HWND)hWnd, (HMENU)NULL, hInstance, NULL );

  /* Draw the List Box */
  hUnitsComboBox = CreateWindowEx( 0, 
				 TEXT( "COMBOBOX" ),
				 TEXT(""),
				 WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL,
				 UNITS_COMBO_LBOX_X, UNITS_COMBO_LBOX_Y, 
				 UNITS_COMBO_LBOX_WIDTH, UNITS_COMBO_LBOX_HEIGHT,
				 (HWND)hWnd, (HMENU)UNITS_COMBO_LBOX_ID, hInstance, NULL );

  if( !hUnitsComboBox ) {
    MessageBox( NULL, 
		TEXT( "Could not create the Units combo box" ),
		TEXT( "Failed Control Creation" ),
		MB_OK);
    return;
  }

  /* Add choices to drop list */
  for( i=0; i<lenUnitsData; i++ ) {

    SendMessage( hUnitsComboBox, (UINT)CB_INSERTSTRING, 
		 (WPARAM)unitsData[i].ID, (LPARAM)unitsData[i].label );

    /* Set Default Choice */
    if ( unitsData[i].isDefault ) {
      SendMessage( hUnitsComboBox, (UINT)CB_SETCURSEL,
		   (WPARAM)unitsData[i].ID, (LPARAM)0 );
    }
  }

  return;
}

// Draw the Re-calculate Button
static void DrawRecalcButton( HWND hWnd, HINSTANCE hInstance ) {

  if( !CreateWindowEx( 0,
		       TEXT( "BUTTON" ),
		       TEXT( "REFRESH" ),
		       WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		       REFRESH_BUTTON_X, REFRESH_BUTTON_Y, 
		       REFRESH_BUTTON_WIDTH, REFRESH_BUTTON_HEIGHT,
		       (HWND)hWnd, (HMENU)REFRESH_BUTTON_ID, hInstance, NULL )) {
  

    MessageBox( NULL, TEXT( "Could not create REFRESH button" ),
		TEXT( "Failed Control Creation" ), MB_OK);
    return;
  }
}
 

// Draw the Nastran Button
static void DrawNastranButton( HWND hWnd, HINSTANCE hInstance ) {

  if( !CreateWindowEx( 0,
		       TEXT( "BUTTON" ),
		       TEXT( "NASTRAN" ),
		       WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		       NASTRAN_BUTTON_X, NASTRAN_BUTTON_Y, 
		       NASTRAN_BUTTON_WIDTH, NASTRAN_BUTTON_HEIGHT,
		       (HWND)hWnd, (HMENU)NASTRAN_BUTTON_ID, hInstance, NULL )) {
  

    MessageBox( NULL, TEXT( "Could not create NASTRAN button" ),
		TEXT( "Failed Control Creation" ), MB_OK);
    return;
  }
}
 


// Draw the Static Title Over Edit Forms
static void DrawEditControlsTitle( HWND hWnd, 
				   HINSTANCE hInstance ) {

  /* Section Title */
  CreateWindowEx( 0,
		  TEXT( "STATIC" ), TEXT( "PLF Geometry Information" ),
		  WS_CHILD | WS_VISIBLE | SS_CENTER,
		  EDIT_CONTROL_TITLE_X, EDIT_CONTROL_TITLE_Y, 
		  EDIT_CONTROL_TITLE_WIDTH, EDIT_CONTROL_TITLE_HEIGHT,
		  (HWND)hWnd, (HMENU)NULL, hInstance, NULL );

  return;
}

// Draw the Edit Controls
static void DrawEditControls( HWND hWnd, 
			      HINSTANCE hInstance ) { 
  int i;
  HWND handle;

  for ( i=0; i<lenSizeData; i++ ) {

    CreateWindowEx( 0,					    /* Label */
		    TEXT( "STATIC" ),
		    sizeData[i].label,
		    WS_CHILD | WS_VISIBLE | SS_LEFT,
		    sizeData[i].X, sizeData[i].Y, 
		    EDIT_CONTROL_LABEL_WIDTH, EDIT_CONTROL_LABEL_HEIGHT,
		    (HWND)hWnd, (HMENU)NULL, hInstance, NULL );

  handle =
    CreateWindowEx( 0,					    /* Edit Box */
		    TEXT( "EDIT" ),
		    sizeData[i].entry,
		    WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		    sizeData[i].X + 120, sizeData[i].Y, 
		    EDIT_CONTROL_ENTRY_WIDTH, EDIT_CONTROL_ENTRY_HEIGHT,
		    (HWND)hWnd, (HMENU)sizeData[i].ID, hInstance, NULL );

  if ( sizeData[i].getsFocus == TRUE ) {
    SetFocus( handle );
  }

  /* Pass Windows Handle Back to Data Structure */
  sizeData[i].hControl = handle;

  /* Save Original Windows Process for Child Window */
  sizeData[i].process = (WNDPROC) SetWindowLong( handle, GWL_WNDPROC, (LONG) EntryFormsProc );
  }

  return;
}


// Update Payload Volume, etc. Information
static void UpdateVolumeInformation( HDC hdc, int cxChar, int cyChar ) {

  wchar_t buffer[24];  
  int i;

  int x = CALCULATED_DATA_ORIGIN_X;
  int y = CALCULATED_DATA_ORIGIN_Y;

  double Ro = getOgiveRadius();
  double Rn = getNoseRadius();
  double Db = getBaseDiameter();
  double H  = getHeight();
  double O  = getPLVolumeOffset();

  double Vp = GetPayloadVolume( Ro, Rn, Db, H, O );
  double Vb = GetBarrelVolume( getLowerBarrelLength(), getDynamicDiameter() );

  wchar_t * frmt0 = TEXT( "%8.0f %s%s" );
  wchar_t * frmt1 = TEXT( "%8.1f %s%s" );


struct {
  TCHAR * szLabel;
  double metric;
  TCHAR * szUnitPrefix;
  wchar_t * format;
} sysmetrics [] = {
  
  { TEXT( "Surface Area"),       GetFairingSurfaceArea( Ro, Rn, Db, H ), TEXT( "sq-"),  frmt0 },
  { TEXT( "Tot OML Volume"),     GetFairingVolume( Ro, Rn, Db, H ),      TEXT( "cu-"),  frmt0 },
  { TEXT( "Payload Volume"),     Vp,                                     TEXT( "cu-"),  frmt0 },
  { TEXT( "   -Lower Barrel"),   Vb,                                     TEXT( "cu-"),  frmt0 },
  { TEXT( "   -Upper Fairing"),  Vp - Vb,                                TEXT( "cu-"),  frmt0 },
  { TEXT( "Nose+Ogive Length" ), GetLengthUpperFairing( Ro, Rn, Db ),    TEXT( "    "), frmt1 },
  { TEXT( "Barrel Length"),      GetBarrelLength( Ro, Rn, Db, H ),       TEXT( "    "), frmt1 },
};

 SetBkMode( hdc, TRANSPARENT );				    /* Text background same as window */

  /* Print Fairing Information */
  for (i = 0 ; i < ( (int)(sizeof sysmetrics / sizeof sysmetrics [0]) ); i++ ) {

    /* Print Label */
    TextOut( hdc, x, y + cyChar * i, sysmetrics[i].szLabel, 
	     lstrlen( sysmetrics[i].szLabel ));
    SetTextAlign( hdc, TA_RIGHT | TA_TOP) ;

    /* Print Metric */
    TextOut( hdc, x + 30 * cxChar, y + cyChar * i, buffer,
	     swprintf( buffer, 15, sysmetrics[i].format, sysmetrics[i].metric, 
		       sysmetrics[i].szUnitPrefix, getUnitsString()));
    SetTextAlign( hdc, TA_LEFT | TA_TOP );
  }

  SetBkMode( hdc, OPAQUE );				    /* Re-set Default */

  return;
}
		 


// Draw the Data Forms in Forms Window
static void InitializeForms(HWND hWnd) {

  HINSTANCE hInstance = GetModuleHandle(NULL);

  /* Draw Overall Status Information */
  DrawUnitsCombo( hWnd, hInstance );

  /* Draw Re-calculate Button */
  DrawRecalcButton( hWnd, hInstance );

  /* Draw the Nastran Button */
  DrawNastranButton( hWnd, hInstance );

  /* Draw Size Information Static Label */
  DrawEditControlsTitle( hWnd, hInstance );

  /* Draw Entry Forms for Fairing Geometry */
  DrawEditControls( hWnd, hInstance );

  /* Draw Fairing Volume/Size Information */
  /* 'UpdateVolumeInformation' draws calculated information in Forms Window */

  return;
}

/** --------------------------------------------------------------------------------------------------------------------

						    UNIT CONVERSION

    ------------------------------------------------------------------------------------------------------------------- **/

/**

   Convert Each Edit Control by Scale Factor of 'x'

 **/

static void convertUnits( double x ) {

  int i;
  TCHAR buffer[32];

  for (i=0; i<lenSizeData; i++ ) {

    swprintf( buffer, 31, TEXT( "%.3f" ), ( _getValue( sizeData[i].ID ) * x ));
    Edit_SetText( sizeData[i].hControl, buffer );
  }

  return;
}

/** --------------------------------------------------------------------------------------------------------------------

						PROCESS WINDOWS MESSAGES

    ------------------------------------------------------------------------------------------------------------------- **/


// PROCESS FORMS WINDOW MESSAGE TRAFFIC
LRESULT CALLBACK WinFormsProc( HWND hwnd,		    /* Window Handle */
			       UINT msg,		    /* Message */
			       WPARAM wParam,		    /* wParam = HIWORD(wParam) and LOWORD(wParam) */	
			       LPARAM lParam ) {	    /* HWND of control which sent message */            

  static int  cxChar, cyChar;

  switch( msg ) {

  case WM_CREATE: {

    /* Create Forms */
    InitializeForms( hwnd );

    /* Get Font Info for Drawing Volume Data */
    HDC hdc = GetDC( hwnd );
    TEXTMETRIC tm;

    GetTextMetrics( hdc, &tm );
    cxChar = tm.tmAveCharWidth;
    cyChar = tm.tmHeight + tm.tmExternalLeading;

    ReleaseDC( hwnd, hdc );

    /* Create Lua State */
    L = luaL_newstate();
    luaL_openlibs( L );

    /* Read lua functions into global state */
    luaL_dofile( L, "nast.lua" );

    /* Push C Functions Into Lua Global State */
    lua_pushcfunction( L, l_ogiveY );
    lua_setglobal( L, "ogiveY" );

    lua_pushcfunction( L, l_getLength );
    lua_setglobal( L, "getLength" );

    lua_pushcfunction( L, l_getOgiveLength );
    lua_setglobal( L, "getOgiveLength" );

    lua_pushcfunction( L, l_getBaseDiameter );
    lua_setglobal( L, "getBaseDiameter" );

    lua_pushcfunction( L, l_getBarrelLength );
    lua_setglobal( L, "getBarrelLength" );

    lua_pushcfunction( L, l_getNoseRadius );
    lua_setglobal( L, "getNoseRadius" );

    lua_pushcfunction( L, l_getNoseCapStartingX );
    lua_setglobal( L, "getNoseCapXt" );

    return 0;
  }

  case WM_PAINT: {					    /* Validate Client Area */

    InvalidateRect( hwnd, NULL, TRUE );                      
    
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint( hwnd, &ps );
    UpdateVolumeInformation( hdc, cxChar, cyChar );
    EndPaint( hwnd, &ps );
   
    return 0;
  }

  case WM_COMMAND:
    switch( HIWORD( wParam )) {
      
    case BN_CLICKED: {
      
      if ( LOWORD( wParam ) == REFRESH_BUTTON_ID ) {	    /* Refresh Drawing */
	SendMessage( GetParent( hwnd ), WM_PAINT, 0, 0 );
      }
      else if ( LOWORD( wParam ) == NASTRAN_BUTTON_ID ) {   /* Create NASTRAN Deck */

	/* Call func with 2 arg and 0 return */
	lua_getglobal(  L, "write_deck" );
	lua_call( L, 0, 0 );
	MessageBeep(0);
      }
      else {
	return DefWindowProc( hwnd, msg, wParam, lParam );
      }

      return 0;
    }

    case EN_CHANGE: {

      int i;
      int length = sizeof( sizeData ) / sizeof( struct editItems );

      for ( i=0; i<length; i++ ) {

	if ( (HWND)lParam == (HWND)sizeData[i].hControl ) {
	  InvalidateRect( hwnd, NULL, TRUE );                      
	  return 0;
	}
      }
    }

    case CBN_SELCHANGE: {				    /* Swap Units in Calculations */


      if ( LOWORD( wParam ) == UNITS_COMBO_LBOX_ID ) {

	convertUnits(( SendMessage( hUnitsComboBox, CB_GETCURSEL, 0, 0 ) ==  FEET ) ? 3.28084 : 0.3048 ); 
	return 0;
      }
      else {

	return DefWindowProc( hwnd, msg, wParam, lParam );
      }
    }
      
    default:
      return DefWindowProc( hwnd, msg, wParam, lParam );      
    }

    
  case WM_CLOSE:
    lua_close(L);
    return 0;

  default:
    return DefWindowProc( hwnd, msg, wParam, lParam );			    
  }

  return 0;
}



// PROCESS ENTRY FORMS MESSAGE TRAFFIC
LRESULT CALLBACK EntryFormsProc( HWND hwnd,
				 UINT msg,
				 WPARAM wParam,
				 LPARAM lParam ) {	    /* HWND of control which sent message */            

  int id = GetWindowLong( hwnd, GWL_ID );

  switch( msg ) {

  case WM_KEYDOWN:
    
    switch( wParam ) {

    case VK_TAB: {

      /* Which Child Window Control Gets Focus? */
      if( GetKeyState( VK_SHIFT ) < 0 ) {		    /* SHIFT-TAB */
	id = (( id == 0 ) ? lenSizeData - 1 : id - 1 );
      }
      else {						    /* TAB */
	id = (( id == (lenSizeData - 1)) ? 0 : id + 1 );
      }

      /* Set Focus For New Child Window */
      SetFocus( sizeData[id].hControl );
      break;
    }

    case VK_NEXT:
      SendMessage( GetParent( GetParent( hwnd )), WM_VSCROLL, SB_PAGEDOWN, 0 );
      break;

    case VK_PRIOR:
      SendMessage( GetParent( GetParent( hwnd )), WM_VSCROLL, SB_PAGEUP, 0 );
      break;

    case VK_HOME:
      SendMessage( GetParent( GetParent( hwnd )), WM_VSCROLL, SB_TOP, 0 );
      break;

    case VK_END:
      SendMessage( GetParent( GetParent( hwnd )), WM_VSCROLL, SB_BOTTOM, 0 );
      break;
    }

    break;
    
  case WM_LBUTTONDOWN:
    SetFocus( sizeData[id].hControl );
    break;

  case WM_SETFOCUS:
    SendMessage( hwnd, EM_SETSEL, 0, -1 );
    break;
  }

  return CallWindowProc( sizeData[id].process, hwnd, msg, wParam, lParam );
}


/** --------------------------------------------------------------------------------------------------------------------

						     PUBLIC METHODS

    ------------------------------------------------------------------------------------------------------------------- **/


// CREATE the DATA FORM WINDOW
HWND CreateFormsWindow( HWND hWndMain, 
			int height,
			int width,
			int X, int Y ) {


  HWND       hFormsWindow;
  WNDCLASSEX cwc;					    /* Child Window Class */

  // Define 'Forms' Window Class 
  ZeroMemory(&cwc, sizeof(cwc));

  cwc.cbSize        = sizeof( WNDCLASSEX );                 /* Size of wc structure */
  cwc.style         = 0;				    /* Class Style */
  cwc.lpfnWndProc   = (WNDPROC) WinFormsProc;		    /* Pointer to Window Procedure */
  cwc.cbClsExtra    = 0;				    /* Extra Data Memory Allocation (per Class) */
  cwc.cbWndExtra    = 0;				    /* Extra Data Memory Allocation (per Window) */
  cwc.hInstance     = GetModuleHandle(NULL);		    /* Handle to app instance - received from WinMain */
                                                            /* Icon/Cursor Resources */
  cwc.hIcon         = (HICON) NULL;
  cwc.hCursor       = (HCURSOR) LoadCursor( NULL, IDC_ARROW );
  cwc.hbrBackground = (HBRUSH)(COLOR_WINDOW);		    /* Window Background Color */
  cwc.lpszMenuName  = (LPCTSTR) NULL;			    /* Name of Menu Resource */
  cwc.lpszClassName = szClassName;                                                            
  cwc.hIconSm       = (HICON) NULL;			    /* Small 16x16 icon for taskbar */


  // Register the Forms Window Class
  if ( !RegisterClassEx( &cwc )) {

    MessageBox( NULL, TEXT( "Forms Window Registration Failed!" ), 
		TEXT( "Error!" ), MB_ICONERROR | MB_OK);
    return NULL;
  }

  hFormsWindow = 
    CreateWindowEx(
		   WS_EX_RIGHTSCROLLBAR | WS_EX_CLIENTEDGE | WS_TABSTOP,
		   szClassName,
		   TEXT( "Payload Fairing Characteristics" ),  /* Window Title */
		   WS_CHILD | WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS,
		   X,Y,					    /* X,Y Coords top-left corner */
		   width, height,
		   hWndMain,
		   (HMENU) NULL, 			    /* Menu Handle */
		   GetModuleHandle(NULL),		    /* App Instance Handle (from WinMain) */
		   NULL );				    /* Window Creation Data */


  if( hFormsWindow == NULL ) {

    MessageBox( NULL, TEXT( "Forms Window Creation Failed!" ), TEXT( "Error!" ),
		MB_ICONERROR | MB_OK );
    return NULL;
  }

  return hFormsWindow;
}

/** --------------------------------------------------------------------------------------------------------------------

					     C Functions Callable from Lua

    ------------------------------------------------------------------------------------------------------------------- **/

static int l_ogiveY( lua_State *L ) {

  double x  = luaL_checknumber( L, 1);			    /* Get ogive X-coordinate */

  double Ro = getOgiveRadius();
  double Db = getBaseDiameter();
  double Lo = GetOgiveLength( Ro, Db);


  lua_pushnumber( L, 12.0 * ogiveY( Ro, Db, Lo - x ));

  return 1;

}

static int l_getOgiveLength( lua_State *L ) {

  double Ro = getOgiveRadius();
  double Db = getBaseDiameter();
  double Rn = getNoseRadius();

  lua_pushnumber( L, 12.0 * ( GetOgiveLength( Ro, Db ) - GetOgiveSphereTangencyPointX( Ro, Rn, Db )));

  return 1;
}

static int l_getBaseDiameter( lua_State *L ) {
   
   lua_pushnumber( L, 12.0 * getBaseDiameter() );

   return 1;
 }

static int l_getBarrelLength( lua_State *L ) {

  lua_pushnumber( L, GetBarrelLength( getOgiveRadius(), getNoseRadius(),
				      getBaseDiameter(), getHeight() ) * 12.0 );

  return 1;
}

static int l_getNoseRadius( lua_State *L ) {

  lua_pushnumber( L, getNoseRadius() * 12.0 );

  return 1;
}

static int l_getLength( lua_State *L ) {
  
  lua_pushnumber( L, getHeight() * 12.0 );

  return 1;
}

static int l_getNoseCapStartingX( lua_State *L ) {

  double Ro = getOgiveRadius();
  double Rn = getNoseRadius();
  double Db = getBaseDiameter();

  double Xo = GetNoseRadiusCenterPointX(    Ro, Rn, Db );
  double Xt = GetOgiveSphereTangencyPointX( Ro, Rn, Db );

  lua_pushnumber( L, 12.0 * ( Xo - Xt ));

  return 1;
}

/** --------------------------------------------------------------------------------------------------------------------

						     RETRIEVE DATA
							 PUBLIC

    ------------------------------------------------------------------------------------------------------------------- **/


// Get Numeric Value From Froms Window Edit Box
static double _getValue( int ID ) {

  TCHAR buffer[256];

  /* Find the correct data item */
  int i=0;
  while (( sizeData[i].ID != ID ) &&	 
	 ( i < lenSizeData )) {

    i++;
  }

  /* Didn't find the item */
  if ( i == lenSizeData ) {
    MessageBox( (HWND)NULL,
		TEXT( "Cannot find Data Item. Returning 0.0" ), TEXT( "Error" ),
		MB_OK | MB_ICONINFORMATION );
    
    return 0.0;
  }
  
  /* Process text */
  GetWindowText( sizeData[i].hControl, buffer, 255 );
  
  return _wtof( buffer );
}


// Fairing Base Diameter
double getBaseDiameter( void ) {
  
  return _getValue( BASE_DIAMETER );
}

// Fairing Height
double getHeight( void ) {
  
  return _getValue( HEIGHT );
}

// Fairing Ogive Radius
double getOgiveRadius( void ) {
  
  return _getValue( OGIVE_RADIUS );
}

// Fairing Nose Radius
double getNoseRadius( void ) {
  
  return _getValue( NOSE_RADIUS );
}

// Get diameter of dynamic volume
double  getDynamicDiameter( void ) {

  return _getValue( DYNAMIC_DIAMETER );
}

// Get PL Volume Offset
double getPLVolumeOffset( void ) {

  return (( _getValue( BASE_DIAMETER ) - _getValue( DYNAMIC_DIAMETER )) / 2.0 );
}

// Get Boattail Diameter
double getBTAngle( void ) {

  return _getValue( BT_ANGLE );
}

// Get Boattail Diameter
double getBTDiameter( void ) {

  return _getValue( BT_DIAMETER );
}

// Get Boattail Height and Length (hypotenuse)
double getBTHeight( void ) {

  return (( RADIUS( _getValue( BASE_DIAMETER )) - RADIUS( _getValue( BT_DIAMETER ))) /
	  tan( DEG2RAD(_getValue( BT_ANGLE ))));
}

double getBTLength( void ) {

  return (( RADIUS( _getValue( BASE_DIAMETER )) - RADIUS( _getValue( BT_DIAMETER ))) /
	  sin( DEG2RAD( _getValue( BT_ANGLE ))));
}

BOOL isBoatTail( void ) {


  if( ( _getValue( BT_ANGLE )   < ETA ) ||
      ( _getValue( BT_DIAMETER) < ETA ) ||
      ( _getValue( BASE_DIAMETER ) - _getValue( BT_DIAMETER ) < ETA )) {
    return FALSE;
  }

  return TRUE;
}


// Calculate Barrel Length
double GetBarrelLength( double ogive_radius,
			double nose_radius,
			double base_diameter,
			double height ) {

  return height - GetLengthUpperFairing( ogive_radius,
					 nose_radius,
					 base_diameter );
}

// Calculate Lower Barrel Length
double  getLowerBarrelLength( void ) {

  return _getValue( SPLITLINE );
}


// Get Units String
TCHAR * getUnitsString( void ) {

  int iIndex = SendMessage( hUnitsComboBox, CB_GETCURSEL, 0, 0 );

  return unitsData[iIndex].lenString;
}



/**
   Local Variables: **
   mode:c **
   comment-column:60 **
   fill-column: 120 **
   compile-command:"gcc -D UNICODE -D _UNICODE -D _WIN32_IE=0x0300 -c winForms.c -o winForms.o" **
   End: **
**/
