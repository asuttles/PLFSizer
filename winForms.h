#include <windows.h>
#include <Windowsx.h>
#include <stdio.h>
#include <commctrl.h>

HWND	CreateFormsWindow( HWND, int, int, int, int );

double	getBaseDiameter( void );
double	getHeight( void );
double	getOgiveRadius( void );
double	getNoseRadius( void );
double	GetBarrelLength( double, double, double, double );
double  getDynamicDiameter( void );
double	getPLVolumeOffset( void );
double  getLowerBarrelLength( void );
double  getBTDiameter( void );
double  getBTAngle( void );
double  getBTHeight( void );
double  getBTLength( void );
BOOL    isBoatTail( void );
TCHAR * getUnitsString( void );
