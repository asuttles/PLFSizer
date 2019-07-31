/**

						     FAIRING SIZER

						  DRAW SYSTEM GRAPHIC

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
#include <stdio.h>					    /* DEBUG */

#include "ogive.h"
#include "winForms.h"
#include "calcSurfaceArea.h"
#include "calcVolume.h"
#include "resource.h"

// Windows Compiler Synonym
#define swprintf _snwprintf

// Dimensioning Constants
#define ARROW_LEN 10
#define ARROW_HGT 7

#define DIM_LINE_OFFSET 20

// Minimum Width of Drawing for Decorations
#define DECORATION_THRESHOLD 500

// Integration Constant
#define OGIVE_NUM 1000

// Math Macros
#define ROUND(X) ((int)(X+0.5))
#define MAX(X,Y) ((X) > (Y)) ? X : Y

// ---------------------------------------------------------------------------------------------------------------------
//						      MODULE DATA
// ---------------------------------------------------------------------------------------------------------------------

// Define diameter dimension types
enum diameter_t { OD, ID, NOSE };

// Screen size and pixels per unit
static int  xClient, yClient, DRAWING_OFFSET, ppu;

// Window Name
static const TCHAR szClassName[] = TEXT( "Drawing Window" );

// Window Handles
static WNDCLASSEX dwc;
static HWND hDrawWindow;

// NASA Logo Bitmap handle
static HBITMAP hBM1, hBM2;

// Drawing Pens
static HPEN pen, centerLinePen, dashPen, dimPen;

// Popup Menu
static HMENU hMenu;

// Graphics Device Context
static HDC hDC;

// ---------------------------------------------------------------------------------------------------------------------
//						PAINT FAIRING IN DRAWING WINDOW
// ---------------------------------------------------------------------------------------------------------------------

// Dimensioning Helper Functions

static void drawLeftArrowHead( int x, int y ) {

  MoveToEx( hDC, x, y, NULL );
  LineTo(   hDC, x+ARROW_LEN, y+ARROW_HGT );
  MoveToEx( hDC, x, y, NULL );
  LineTo(   hDC, x+ARROW_LEN, y-ARROW_HGT );  
  MoveToEx( hDC, x, y, NULL );
}

static void drawRightArrowHead( int x, int y ) {

  MoveToEx( hDC, x, y, NULL );
  LineTo(   hDC, x-ARROW_LEN, y+ARROW_HGT );
  MoveToEx( hDC, x, y, NULL );
  LineTo(   hDC, x-ARROW_LEN, y-ARROW_HGT );  
  MoveToEx( hDC, x, y, NULL );
}

static void drawUpArrowHead( int x, int y ) {
  
  MoveToEx( hDC, x, y, NULL );
  LineTo(   hDC, x-ARROW_HGT, y+ARROW_LEN );
  MoveToEx( hDC, x, y, NULL );
  LineTo(   hDC, x+ARROW_HGT, y+ARROW_LEN );  
  MoveToEx( hDC, x, y, NULL );
}

static void drawDownArrowHead( int x, int y ) {
  
  MoveToEx( hDC, x, y, NULL );
  LineTo(   hDC, x-ARROW_HGT, y-ARROW_LEN );
  MoveToEx( hDC, x, y, NULL );
  LineTo(   hDC, x+ARROW_HGT, y-ARROW_LEN );  
  MoveToEx( hDC, x, y, NULL );
}


// Draw Length Dimensions
static void drawLengthDimension( int x1, int y1, 	    /* Dimension pt 1 */
				 int x2, int y2, 	    /* Dimension pt 2 */
				 int level, 		    /* Dimension Stack Depth */
				 double length ) {	    /* Text label */
  

  int dimLineY = (int)(( (float)level * 0.07 ) * yClient ); /* Y_coord for Top of vertical Dim Lines */
  //int horLineY = dimLineY + (int)(( MAX(y1,y2) - dimLineY ) * 0.05 );
  int horLineY = (int)(1.1*dimLineY);			    /* Y_coord for Horiz. Line with Dim Text */

  wchar_t buffer[8];					    /* Buffer for Dimension Text */

  /* Align Dimension Text */
  UINT oldAlignment = SetTextAlign( hDC, TA_BOTTOM | TA_CENTER );

  /* Create Dimension Pen */
  SelectObject( hDC, dimPen );

  /* Draw Dimension Lines and Text */
  MoveToEx( hDC, x1, y1-DIM_LINE_OFFSET, NULL );	    /* Left vertical Dim Line */
  LineTo( hDC, x1, dimLineY );

  MoveToEx( hDC, x2, y2-DIM_LINE_OFFSET, NULL );	    /* Right vertical Dim Line */
  LineTo( hDC, x2, dimLineY );
   
  MoveToEx( hDC, x1, 2*dimLineY, NULL );		    /* Draw Horiz Line for dim */
  drawLeftArrowHead( x1, horLineY );
  LineTo( hDC, x2, horLineY );
  drawRightArrowHead( x2, horLineY );

  swprintf( buffer, 7,					    /* Draw Dimension Label */
	    TEXT( "%7.2f" ), length );
  TextOut( hDC, (x1+x2)/2, horLineY, buffer, 7 );


  SelectObject( hDC, pen );				    /* Restore Old Drawing Pen */
  SetTextAlign( hDC, oldAlignment );			    /* Restore Old Text Alignment */
}


// Draw Diameter Dimensions
static void drawDiameterDimensions( int x1, int y1, 
				    int x2, int y2, 
				    int type, double diameter ) {

  int x1_1, x1_2, xd;					    /* Beg/End Points for Dimension Lines */
  wchar_t buffer[9];					    /* Buffer for Dimension Text */

  /* Select Dimension Pen */
  SelectObject( hDC, dimPen );

  /* Align Dimension Text */
  UINT oldAlignment;

  switch (type) {					    /* Coords for Dimension Lines */

  case OD:

    oldAlignment = SetTextAlign( hDC, TA_CENTER | TA_BASELINE );
    
    x1_1 = x1 + DIM_LINE_OFFSET;
    x1_2 = x1_1 + (int)(0.95 * DRAWING_OFFSET);
    xd   = x1_1 + (int)( 0.9 * ( x1_2 - x1_1));
    //xd   = x1_1 + (int)(0.9 * (x1_2 - x1_1));
    break;

  case ID:
    
    oldAlignment = SetTextAlign( hDC, TA_CENTER | TA_BASELINE  );

    x1_1 = x1 + DIM_LINE_OFFSET;
    x1_2 = (int)(0.90 * xClient);     
    xd   = x1_1 + (int)(0.9 * (x1_2 - x1_1));
    break;

  case NOSE:

    oldAlignment = SetTextAlign( hDC, TA_CENTER | TA_BASELINE );
    
    x1_1 = x1 - DIM_LINE_OFFSET;
    x1_2 = (int)(0.5 * DRAWING_OFFSET);
    xd   = x1_1 - (int)(0.9 * (x1_1 - x1_2));
    break;
  }


  // Draw Lines and Text
  MoveToEx( hDC, x1_1, y1, NULL ); 			    /* Top Dimension Line */
  LineTo(   hDC, x1_2, y1 );

  MoveToEx( hDC, x1_1, y2, NULL ); 			    /* Bot Dimension Line */
  LineTo(   hDC, x1_2, y2 );

  MoveToEx( hDC, xd, y1, NULL );			    /* Draw Vert. Dimension Line */
  drawUpArrowHead( xd, y1 );
  LineTo(   hDC, xd, y2 );
  drawDownArrowHead( xd, y2 );

  swprintf( buffer, 8, 					    /* Draw Dimension Label */
	    TEXT( "%lc%7.2f" ), 0x00D8, diameter );
  TextOutW( hDC, xd, (y2+y1)/2, buffer, 8 );

  
  SelectObject( hDC, pen );				    /* Restore Old Drawing Pen */
  SetTextAlign( hDC, oldAlignment );			    /* Restore Old Text Alignment */
}


// Write Title on Drawing
static void writeDrawingTitle( int x, int y, 
			       double Db, 
			       double L, 
			       double Ro, 
			       double Rn, 
			       double Vp, 
			       double As ) {

  wchar_t buffer[64];
  TCHAR * unitString = getUnitsString();

  /* Get Default Text Metrics for System Font */
  TEXTMETRIC tm;
  int cyChar;						    /* Text spacing */

  GetTextMetrics( hDC, &tm );
  cyChar = tm.tmHeight + tm.tmExternalLeading;

  /* Write Title */
  swprintf( buffer, 64, TEXT( "%lc%.1f%s x %.1f%s TANGENT OGIVE PAYLOAD FAIRING" ), 0x00D8, Db, unitString, L, unitString );
  TextOutW( hDC, x, y, buffer, wcslen( buffer ));
  y += (1.25 * cyChar);

  /* Write Radii */
  swprintf( buffer, 64, TEXT( "Ogive Radius: %.2f%s, Nose Radius: %.2f %s" ), Ro, unitString, Rn, unitString );
  TextOutW( hDC, x, y, buffer, wcslen( buffer ));
  y += cyChar;
  
  /* Write Surface Area */
  swprintf( buffer, 64, TEXT( "Surface Area: %.0f sq-%s" ), As, unitString );
  TextOutW( hDC, x, y, buffer, wcslen( buffer ));
  y += cyChar;
  
  /* Write Payload Volume */
  swprintf( buffer, 64, TEXT( "Payload Volume: %.0f cu-%s" ), Vp, unitString );
  TextOutW( hDC, x, y, buffer, wcslen( buffer ));

  /* Dimension Note */
  swprintf( buffer, 64, TEXT( "NOTE: All Dimensions are in \'%s\' Unless Otherwise Specified" ), unitString );
  TextOutW( hDC, 15, yClient-15-cyChar, buffer, wcslen( buffer ));

  return;
}


// Draw the Volume Split
static void writeVolumeSplit( double L, 
			      int X, 
			      double Vp ) {

  wchar_t buffer[64];

  double  Dd = getDynamicDiameter();
  double  Vl = Vp - GetBarrelVolume( L, Dd ); 
  
  // Drawing Text Anchor Point
  int xl = (( xClient - DRAWING_OFFSET ) + X ) / 2;
  int xu = DRAWING_OFFSET + ( 2 * ( X - DRAWING_OFFSET ) / 3);
  int y  = ( yClient / 2 ) + (( Dd / 4 ) * ppu );
  

  UINT oldTextAlignment = SetTextAlign( hDC, TA_CENTER );


  /* Lower Fairing */
  TextOutW( hDC, xl, y, buffer, 
	    swprintf( buffer, 64, TEXT( "Lower Fairing Volume: %.0f cu-%s" ), 
		      Vl, getUnitsString() ));

  /* Upper Fairing */
  TextOutW( hDC, xu, y, buffer, 
	    swprintf( buffer, 64, TEXT( "Upper Fairing Volume: %.0f cu-%s" ), 
		      Vp-Vl, getUnitsString() ));

  SetTextAlign( hDC, oldTextAlignment );

  return;
}

// Draw the NASA Logo
static void drawNASALogo() {

  if ( yClient > DECORATION_THRESHOLD ) {
  
    BITMAP bm;						    /* BITMAP info struct */
  
    /* Set up a memory DC for bitmap, compatible with window DC */
    HDC hdcMem = CreateCompatibleDC( hDC );

    /* Select the bitmap into the memory DC */
    HBITMAP hBMOld = SelectObject( hdcMem, hBM1 );
  
    /* Read Information about bitmap into bm struct */
    GetObject( hBM1, sizeof( bm ), &bm );

    /* Copy image from memory DC to Window DC */
    int cx = 50;
    int cy = yClient - 50 - bm.bmHeight;
    BitBlt( hDC, cx, cy, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY );

    /* Restore memory DC */
    SelectObject( hdcMem, hBMOld );
    DeleteDC(hdcMem);
  
  }
  return;
}


// Draw the SLS Logo
static void drawSLSLogo() {

  if ( yClient > DECORATION_THRESHOLD ) {  

    BITMAP bm;						    /* BITMAP info struct */
  
    /* Set up a memory DC for bitmap, compatible with window DC */
    HDC hdcMem = CreateCompatibleDC( hDC );

    /* Select the bitmap into the memory DC */
    HBITMAP hBMOld = SelectObject( hdcMem, hBM2 );
  
    /* Read Information about bitmap into bm struct */
    GetObject( hBM2, sizeof( bm ), &bm );

    /* Copy image from memory DC to Window DC */
    int cx = 175;
    int cy = yClient - 50 - ( bm.bmHeight / 2 );
    //BitBlt( hDC, cx, cy, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY );
    StretchBlt( hDC,   cx, cy, bm.bmWidth / 2, bm.bmHeight / 2, 
		hdcMem, 0, 0,  bm.bmWidth,     bm.bmHeight, SRCCOPY ); 

    /* Restore memory DC */
    SelectObject( hdcMem, hBMOld );
    DeleteDC(hdcMem);
  
  }
  return;
}

// Paint the Fairing on the Drawing Window
static void paintDrawingWindow() {

  /* Geometry Information */
  double H  = getHeight();
  double Ro = getOgiveRadius();
  double Rn = getNoseRadius();
  double Db = getBaseDiameter();
  double Ll = getLowerBarrelLength();

  double Vp = GetPayloadVolume( Ro, Rn, Db, H, getPLVolumeOffset() );


  /* Blunted Nose Geometric Info  */
  double Xo = GetNoseRadiusCenterPointX( Ro, Rn, Db );
  double Xt = GetOgiveSphereTangencyPointX( Ro, Rn, Db );
  double Yt = GetOgiveSphereTangencyPointY( Ro, Rn, Db );
  double Lo = GetOgiveLength( Ro, Db ) - Xt;	    
  double Lb = GetBarrelLength( Ro, Rn, Db, H);

  /* Drawing Regions */
  int X0 = DRAWING_OFFSET;				    /* Top of Nose */
  int X1 = X0 + ( ROUND( Rn - Xo + Xt ) * ppu );	    /* Nose/Ogive Tangency */
  int X2 = X1 + ( ROUND(Lo) * ppu );			    /* Ogive/Barrel Tangency */
  int X3 = X2 + ( ROUND(Lb) * ppu );			    /* Bottom of Fairing */
  int X4 = X3 - ( ROUND( ppu * Ll ));			    /* Split Line */

  int dyn_offset = ROUND( getPLVolumeOffset() * ppu );


  /* Drawing Information */
  int centerLine = yClient / 2;

  
  /* Drawing Paper Background */
  /* ------------------------------------------------------------------------- */
  SelectObject( hDC, pen );

  Rectangle( hDC, 0,0, xClient, yClient );
  Rectangle( hDC, 10,10, xClient-10, yClient-10);

  /* Draw NASA/SLS logos */
  /* ------------------------------------------------------------------------- */
  drawNASALogo();
  drawSLSLogo( );

  /* Draw a Center Line */
  /* ------------------------------------------------------------------------- */
  SelectObject( hDC, centerLinePen );

  MoveToEx( hDC, X0 - (DRAWING_OFFSET/4), centerLine, NULL );
  LineTo(   hDC, X3 + (DRAWING_OFFSET/4), centerLine ); 



  SelectObject( hDC, pen );				    /* Restore Old Pen */



  /* Draw the Nose Cap */
  /* ------------------------------------------------------------------------- */
  Arc( hDC, 
       /* Box Boundary */
       X0,						    /* Left boundary  */
       centerLine - ( ROUND(Rn) * ppu ),		    /* Top boundary   */
       X0 + ( 2 *     ROUND(Rn) * ppu ),		    /* Right boundary */
       centerLine + ( ROUND(Rn) * ppu ),		    /* Bottom boundary */
       /* Arc Start/End Pts */
       X1, centerLine - ( ROUND(Yt) * ppu ),		    /* X,Y Start */
       X1, centerLine + ( ROUND(Yt) * ppu ));		    /* X,Y End */


  /* Draw Top of Dynamic Volume */
  SelectObject( hDC, dashPen );
  MoveToEx( hDC, X1, centerLine - ( ROUND(Yt) * ppu ) + dyn_offset, NULL );
  LineTo(   hDC, X1, centerLine + ( ROUND(Yt) * ppu ) - dyn_offset );
  SelectObject( hDC, pen );

  /* Draw the Ogive Section */
  /* ------------------------------------------------------------------------- */
  int i;						    
  POINT pt[ OGIVE_NUM ];				    /* Points Array */
  double h = Lo / OGIVE_NUM;				    /* Interval size */

  for (i=0; i < OGIVE_NUM ; i++) {			    /* Draw Top */

    pt[i].x = X1 + ROUND( h * i * ppu );
    pt[i].y = centerLine - ROUND( ppu * ogiveY( Ro, Db,  Xt + (h*i) ));
  }
  Polyline( hDC, pt, OGIVE_NUM );			    

                                                            /* Draw Top Dynamic Offset */
  SelectObject( hDC, dashPen );
  for (i=0; i < OGIVE_NUM; i++ ) pt[i].y += dyn_offset;
  Polyline( hDC, pt, OGIVE_NUM );			    
  SelectObject( hDC, pen );

  for (i=0 ; i < OGIVE_NUM ; i++) {			    /* Draw Bottom */
    pt[i].y = centerLine + ROUND( ppu * ogiveY( Ro, Db, Xt + (h*i) ));
  }  
  Polyline( hDC, pt, OGIVE_NUM );
                                                            /* Draw Bot Dynamic Offset */
  SelectObject( hDC, dashPen );
  for (i=0; i < OGIVE_NUM; i++ ) pt[i].y -= dyn_offset;
  Polyline( hDC, pt, OGIVE_NUM );			    
  SelectObject( hDC, pen );


  /* Draw Barrel Section */
  /* ------------------------------------------------------------------------- */
  int Y1 = centerLine - ROUND(( Db / 2.0) * ppu );
  int Y2 = centerLine + ROUND(( Db / 2.0) * ppu );

  MoveToEx( hDC, X2, Y1, NULL );
  LineTo(   hDC, X3, Y1 );				    /* Top of Barrel */
  MoveToEx( hDC, X2, Y2, NULL );
  LineTo(   hDC, X3, Y2 );				    /* Bot of Barrel */
  LineTo(   hDC, X3, Y1 );

  /* Draw Dynamic Volume Offset */
  SelectObject( hDC, dashPen );
  MoveToEx( hDC, X2, Y1 + dyn_offset, NULL );		    /* Top of Barrel */
  LineTo(   hDC, X3, Y1 + dyn_offset );
  MoveToEx( hDC, X2, Y2 - dyn_offset, NULL );		    /* Bot of Barrel */
  LineTo(   hDC, X3, Y2 - dyn_offset );
  SelectObject( hDC, pen );

  /* Upper/Lower Fairing Split Line */
  if ( Ll > 1.0 ) {
    MoveToEx( hDC, X4, Y1, NULL );
    LineTo(   hDC, X4, Y2 );
  }

  // Decorate Drawing IFF there is Room 
  if ( yClient > DECORATION_THRESHOLD ) {

    /* Draw Dimensions */
    /* ------------------------------------------------------------------------- */

    drawLengthDimension( X0, centerLine, X3, Y1, 1, H );	    /* Overall Length */
    drawLengthDimension( X0, centerLine, X2, Y1, 2, H - Lb );  /* Nose to Barrel */
    drawLengthDimension( X2, Y1, X3, Y1, 2, Lb );		    /* Barrel Length */

    if ( Ll > 1.0 ) {
      drawLengthDimension( X4, Y1, X3, Y1, 3, Ll );	    /* Lower Barrel Length */
      writeVolumeSplit( Ll, X4, Vp );
    }

    /* Nose-Ogive i/f Diameter */
    drawDiameterDimensions( X1, centerLine - ( ROUND(Yt) * ppu ), 
			    X1, centerLine + ( ROUND(Yt) * ppu ), NOSE, Rn );
    drawDiameterDimensions( X3, Y1, X3, Y2, OD, Db );	    /* Base Diameter */

  

    /* Write Descriptive Text */
    /* ------------------------------------------------------------------------- */
    writeDrawingTitle( (int)1.6*xClient/2,  (Y2 + yClient)/2,
		       Db, H, Ro, Rn, Vp, 
		       GetFairingSurfaceArea( Ro, Rn, Db, H ));
  }

  return;
}


// ---------------------------------------------------------------------------------------------------------------------
//						    COPY Drawing to
//						   Windows Clipboard
// ---------------------------------------------------------------------------------------------------------------------

static void copyToClipboard( HWND hwnd ) {

  // Set up Clipboard

  /* Open Clipboard */
  if( !OpenClipboard(NULL) ) {
    MessageBox( hwnd, TEXT( "Cannot open MS Windows clipboard" ), TEXT( "Failed" ), MB_OK );
    return;
  }

  /* Empty the Clipboard */
  if( !EmptyClipboard() ) {
    MessageBox( hwnd, TEXT( "Cannot Empty MS Windows clipboard" ), TEXT( "Failed" ), MB_OK );
    
    CloseClipboard();
    return;
  }

  // Copy Client Area to DDB in Memory

  /* Handle for DC Client Area of Window */
  HDC hdcWin = GetDC( hwnd );
  HDC hdcMem = CreateCompatibleDC( hdcWin );

  /* Create a DDB in Memory (Compatible with Window) */
  HBITMAP hBitmap = CreateCompatibleBitmap( hdcWin, xClient, yClient );

  /* Associate Bitmap with Memory DC */
  SelectObject( hdcMem, hBitmap );

  /* Bit Block Transfer into our Compatible Memory DC */
  if( !BitBlt(hdcMem, 0, 0, xClient, yClient, hdcWin, 0, 0, SRCCOPY )) {

    MessageBox( hwnd, TEXT( "BitBlt has failed for Clipboard creation" ), TEXT( "Failed" ), MB_OK );
  }

  else {

    /* Copy Data to Clipboard */
    SetClipboardData( CF_BITMAP, hBitmap );
  }


  // Clean Up

  CloseClipboard();

  DeleteObject( hBitmap );
  ReleaseDC( hwnd, hdcWin );
  DeleteDC(  hdcMem );


  return;
}

// ---------------------------------------------------------------------------------------------------------------------
//						    SAVE Drawing as
//					    Device Independant Bitmap (DIB)
// ---------------------------------------------------------------------------------------------------------------------

static void saveAsDIB( HWND hwnd ) {

  // Copy Client Area to DDB Memory Bitmap
  // -------------------------------------

  /* Handle for DC Client Area of Window */
  HDC hdcWin = GetDC( hwnd );
  HDC hdcMem = CreateCompatibleDC( hdcWin );

  /* Create a DDB in Memory (Compatible with Window) */
  HBITMAP hBitmap = CreateCompatibleBitmap( hdcWin, xClient, yClient );

  /* Associate Bitmap with Memory DC */
  SelectObject( hdcMem, hBitmap );

  /* Bit Block Transfer into our Compatible Memory DC */
  if( !BitBlt(hdcMem, 0, 0, xClient, yClient, hdcWin, 0, 0, SRCCOPY )) {

    MessageBox( hwnd, TEXT( "BitBlt has failed" ), TEXT( "Failed" ), MB_OK );
    goto done;
  }


  // Create DIB Info Header
  // -----------------------

  /* Get bitmap properties */
  BITMAP bmap;						    /* Info about bitmap */
  GetObject( hBitmap, sizeof( BITMAP ), &bmap );

  BITMAPINFOHEADER   bi;
     
  /* Define Bitmap Info Header */
  bi.biSize = sizeof( BITMAPINFOHEADER );    
  bi.biWidth = bmap.bmWidth;    
  bi.biHeight = bmap.bmHeight;  
  bi.biPlanes = 1;    
  bi.biBitCount = 32;    
  bi.biCompression = BI_RGB;    			    /* Uncompressed Format */
  bi.biSizeImage = 0;  
  bi.biXPelsPerMeter = 0;    
  bi.biYPelsPerMeter = 0;    
  bi.biClrUsed = 0;    
  bi.biClrImportant = 0;


  // Convert DDB to DIB
  // ------------------

  /* Calculate size of DIB bitmap pixels */
  DWORD bmSize = bmap.bmWidth * bmap.bmHeight * 4;	    /* 4 bytes per pixel {R,G,B,0} */

  /* Allocate memory for the pixels */
  char *lpBitmap;
  if (( lpBitmap = (char *)HeapAlloc( GetProcessHeap(), 0, bmSize )) == NULL ) {
    MessageBox( hwnd, TEXT( "Cannot allocate heap space for DIB" ), TEXT( "Failed" ), MB_OK );
    goto done;
  }

  /* Copy bits from DDB to DIB */
  GetDIBits( hdcMem, hBitmap, 0, (UINT)bmap.bmHeight, 
	     lpBitmap, (BITMAPINFO *)&bi, DIB_RGB_COLORS );
  
  
  // Write DIB to File
  // -----------------

  BITMAPFILEHEADER   bmfHeader;    

  /* Open the DIB File */
  HANDLE hFile = CreateFile( TEXT( "plf.bmp" ), GENERIC_WRITE, 0, NULL,
			     CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);   
    
  /* Offset to where the pixels begin */
  bmfHeader.bfOffBits = (DWORD)sizeof( BITMAPFILEHEADER ) + (DWORD)sizeof( BITMAPINFOHEADER ); 
    
  /* File Size = Headers + Pixels */
  bmfHeader.bfSize = bmSize + sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER );
     
  /* Define file type */
  bmfHeader.bfType = 0x4D42; 				    /* BM */
 
  DWORD dwBytesWritten = 0;
  WriteFile( hFile, (LPSTR)&bmfHeader, sizeof( BITMAPFILEHEADER ), &dwBytesWritten, NULL);
  WriteFile( hFile, (LPSTR)&bi,        sizeof( BITMAPINFOHEADER ), &dwBytesWritten, NULL);
  WriteFile( hFile, (LPSTR)lpBitmap,   bmSize,                     &dwBytesWritten, NULL);

  // --------
  // Clean up
  // --------

  HeapFree( GetProcessHeap(), 0, lpBitmap );		    /* Unallocate from heap */
  CloseHandle(hFile);					    /* Close file */

 done:
    DeleteObject( hBitmap );
    ReleaseDC( hwnd, hdcWin );
    DeleteDC(  hdcMem );
    
    return;
}


// ---------------------------------------------------------------------------------------------------------------------
//						     PRINT Drawing
// ---------------------------------------------------------------------------------------------------------------------

static void _print_it( DOCINFO *pDI ) {

    StartDoc( hDC, pDI );
    StartPage( hDC );

    paintDrawingWindow();			    /* The Magic */

    EndPage( hDC );
    EndDoc( hDC );

  return;
}

static void printDrawing( HWND hwnd ) {
  
  // Save Screen Resolution
  int old_xClient = xClient;
  int old_yClient = yClient;
  int old_ppu     = ppu;
  HDC old_hdc     = hDC;

  // Initialize a DOCINFO
  DOCINFO di;
  ZeroMemory( &di, sizeof( DOCINFO ));
  
  di.cbSize = sizeof( DOCINFO );
  di.lpszDocName = TEXT( "Fairing Sizer" );
  di.lpszOutput = (LPCWSTR) NULL;
  di.lpszDatatype = (LPCWSTR) NULL;
  di.fwType = 0;

  // Initialize a PRINTDLG
  PRINTDLG pd; 

  ZeroMemory(&pd, sizeof(PRINTDLG)); 
  pd.lStructSize = sizeof(PRINTDLG); 
  pd.hwndOwner = hwnd;

  pd.hDevMode = NULL;
  pd.hDevNames = NULL;				    /* DEVNAMES Structure */
  pd.hDC = NULL;				    /* DC handle returned */
  pd.Flags = PD_RETURNDC | PD_USEDEVMODECOPIESANDCOLLATE;
  pd.nFromPage = 1;				    /* Default Page Settings */
  pd.nToPage = 1;
  pd.nMinPage = 1;
  pd.nMaxPage = 1;
  pd.nCopies = 1;
  pd.hInstance = GetModuleHandle(NULL);

  // Get User Printer
  if( PrintDlg(&pd) ) { 

    // Change to Landscape Mode
    DEVMODE* pDevMode = (DEVMODE*)GlobalLock( pd.hDevMode );
    pDevMode->dmFields |= DM_ORIENTATION;
    pDevMode->dmOrientation=DMORIENT_LANDSCAPE;
    ResetDC( pd.hDC, pDevMode );
    GlobalUnlock( pd.hDevMode );

    // Set resolution for landscaped printer
    hDC     = pd.hDC;
    xClient = GetDeviceCaps( hDC, HORZRES );
    yClient = GetDeviceCaps( hDC, VERTRES );
    DRAWING_OFFSET = xClient / 10;
    ppu     = ( xClient - ( 2 * DRAWING_OFFSET )) / ROUND( getHeight() );

    _print_it( &di );

    // Clean Up
    if( hDC ) DeleteDC( hDC );
    if( pd.hDevNames ) GlobalFree( pd.hDevNames );
    if( pd.hDevMode  ) GlobalFree( pd.hDevMode );

    // Restore screen resolution constants
    xClient = old_xClient;
    yClient = old_yClient;
    DRAWING_OFFSET = xClient / 10;
    ppu     = old_ppu;
    hDC     = old_hdc;
  }

  else {
    
    MessageBox( NULL, TEXT( "Print Dlg Closed or Failed!" ), TEXT( "Error" ),
		MB_OK | MB_ICONINFORMATION );
  }

  return;
}

// ---------------------------------------------------------------------------------------------------------------------
//					 PROCESS DRAWING WINDOW MESSAGE TRAFFIC
// ---------------------------------------------------------------------------------------------------------------------

LRESULT CALLBACK WinDrawProc( HWND hwnd,	/* Window Handle */
			      UINT msg,		/* Message */
			      WPARAM wParam,	/* wParam = HIWORD(wParam) and LOWORD(wParam) */	
			      LPARAM lParam ) {	/* HWND of control which sent message */            
  PAINTSTRUCT ps;
  
  switch( msg ) {

  case WM_CREATE: {

    /* Create Pens to Draw With */
    pen		    = CreatePen( PS_SOLID,   2, RGB(   0,   0,   0 ));
    centerLinePen   = CreatePen( PS_DASHDOT, 1, RGB(   0,  91, 153 ));
    dimPen          = CreatePen( PS_SOLID,   1, RGB( 255, 126,   0 ));
    dashPen         = CreatePen( PS_DASH,    1, RGB( 0,     0,   0 ));

    /* Create a Floating Popup Menu */
    hMenu = CreatePopupMenu();
    AppendMenu( hMenu, MF_POPUP, IDM_PRINT,                TEXT( "&Print" ));
    AppendMenu( hMenu, MF_POPUP, IDM_COPY,                 TEXT( "&Copy"  ));
    AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
    AppendMenu( hMenu, MF_POPUP, IDM_SAVE_BMP,             TEXT( "Save DI&B"  ));
    AppendMenu( hMenu, MF_POPUP, MF_GRAYED | IDM_SAVE_EMF, TEXT( "Save E&MF"  ));
  }

    /* Create NASA logo Bitmap */
    hBM1 = LoadImage( GetModuleHandle(NULL), 
		     MAKEINTRESOURCE( IDB_BITMAP1 ),
		     IMAGE_BITMAP,
		     0, 0,				    /* Default Size */
		     LR_DEFAULTCOLOR );

    if( hBM1 == NULL ) {
      MessageBox( hwnd, TEXT( "WARNING: Could not load NASA logo bitmap" ), 
		  TEXT( "Error" ), MB_OK | MB_ICONEXCLAMATION );
    }    

    /* Create SLS logo Bitmap */
    hBM2 = LoadImage( GetModuleHandle(NULL), 
		      MAKEINTRESOURCE( IDB_BITMAP2 ),
		      IMAGE_BITMAP,
		      0, 0,				    /* Default Size */
		      LR_DEFAULTCOLOR );

    if( hBM2 == NULL ) {
      MessageBox( hwnd, TEXT( "WARNING: Could not load NASA logo bitmap" ), 
		  TEXT( "Error" ), MB_OK | MB_ICONEXCLAMATION );
    }    

    return 0;
    
  case WM_RBUTTONUP: {
    
    POINT pt;

    pt.x = LOWORD( lParam );				    
    pt.y = HIWORD( lParam );

    ClientToScreen( hwnd, &pt );			    /* Coordinate Conversion */
    
    TrackPopupMenu( hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL );
  }
    return 0;

  case WM_SIZE:
    xClient = LOWORD (lParam);
    yClient = HIWORD (lParam);
    
    /* Calculate pixels per unit of measure */
    ppu     = ( xClient - ( 2 * DRAWING_OFFSET )) / ROUND( getHeight() );
    
    DRAWING_OFFSET = xClient / 10;
    
    return 0;

  case WM_PAINT: {

    /* Re-Calculate pixels per unit of measure */
    ppu     = ( xClient - ( 2 * DRAWING_OFFSET )) / ROUND( getHeight() );

    /* Erase Drawing Window */
    InvalidateRect( hwnd, NULL, TRUE );                      
    
    /** Re-Draw the Window
	BeginPaint  (Use in WM_PAINT messages)
	+ Erases Invalid Region of Client Area
	+ Fills in PS structure
	+ Validates Client Area
	+ -*- Retrieves Device Context -*-  **/
    hDC = BeginPaint( hDrawWindow, &ps );
    
    paintDrawingWindow();
    
    if ( ! EndPaint( hDrawWindow, &ps )) {
     
      MessageBox( NULL, TEXT( "Error Ending Paint Job in Drawing" ),
		  TEXT( "Failed Paint Drawing" ),
		  MB_ICONERROR | MB_ICONWARNING );
    }
   
    return 0;
  }


  case WM_COMMAND:
    switch( LOWORD( wParam )) {

    case IDM_PRINT:
      printDrawing( hwnd );
      return 0;

    case IDM_COPY:
      copyToClipboard( hwnd );
      MessageBeep(0);
      return 0;

    case IDM_SAVE_BMP:
      saveAsDIB( hwnd );
      MessageBeep(0);
      return 0;

    case IDM_SAVE_EMF:
      MessageBeep(0);
      return 0;

    default:
      return DefWindowProc( hwnd, msg, wParam, lParam );			    

    } // End WM_COMMAND 


  case WM_DESTROY:
    
    /* Remove Bitmap */
    DeleteObject( hBM1 );
    DeleteObject( hBM2 );
    return 0;
 
  default:
    return DefWindowProc( hwnd, msg, wParam, lParam );			    
  }
}

// ---------------------------------------------------------------------------------------------------------------------
//						 CREATE DRAWING WINDOW
// ---------------------------------------------------------------------------------------------------------------------

HWND CreateDrawingWindow( HWND hWndMain, int height, int width, int X, int Y ) {

  // Define Drawing Window Class 
  ZeroMemory(&dwc, sizeof(dwc));
  dwc.cbSize        = sizeof( WNDCLASSEX );                 /* Size of wc structure */
  dwc.style         = 0;				    /* Class Style */
  dwc.lpfnWndProc   = (WNDPROC) WinDrawProc;		    /* Pointer to Window Procedure */
  dwc.cbClsExtra    = 0;				    /* Extra Data Memory Allocation (per Class) */
  dwc.cbWndExtra    = 0;				    /* Extra Data Memory Allocation (per Window) */
  dwc.hInstance     = GetModuleHandle(NULL);		    /* Handle to app instance - received from WinMain */
  dwc.hIcon         = (HICON) NULL;                         /* Icon/Cursor Resources */
  dwc.hCursor       = (HCURSOR) LoadCursor( NULL, IDC_ARROW );
  dwc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);		    /* Window Background Color - White for printing */
  dwc.lpszMenuName  = (LPCTSTR) NULL;			    /* Name of Menu Resource */
  dwc.lpszClassName = szClassName;                                                            
  dwc.hIconSm       = (HICON) NULL;			    /* Small 16x16 icon for taskbar */

  // Register the Drawing Window Class
  if ( !RegisterClassEx( &dwc )) {

    MessageBox( NULL, TEXT( "Drawing Window Registration Failed!" ), 
		TEXT( "Error!" ), MB_ICONERROR | MB_OK);
    return NULL;
  }

  // Create an Instance of the Drawing Window Class
  hDrawWindow = 
    CreateWindowEx(
		   WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE,
		   szClassName,
		   TEXT( "PLF OML Sketch" ),		    /* Window Title */
		   WS_CHILD | WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS,
		   X, Y,				    /* X,Y Coords top-left corner */
		   width, height,
		   hWndMain,
		   (HMENU) NULL, 			    /* Menu Handle */
		   GetModuleHandle(NULL),		    /* App Instance Handle (from WinMain) */
		   NULL );				    /* Window Creation Data */

  if( hDrawWindow == NULL ) {

    MessageBox( NULL, TEXT( "Drawing Window Creation Failed!" ), TEXT( "Error!" ),
		MB_ICONERROR | MB_OK );
    return NULL;
  }

  return hDrawWindow;
}


/**
   Local Variables: **
   mode:c **
   comment-column:60 **
   fill-column: 120 **
   compile-command:"gcc -D UNICODE -D _UNICODE -D _WIN32_IE=0x0300 -c winDraw.c -o winDraw.o" **
   End: **
**/
