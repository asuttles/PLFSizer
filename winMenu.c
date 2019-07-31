/**
				      FAIRING SIZER

		              MAIN WINDOW MENU CALLBACKS

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

#include <windows.h>
#include "resource.h"


// 'About' Dialogue Callback
INT_PTR CALLBACK AboutDialogProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

  switch (msg) {

  case WM_COMMAND:

    switch (LOWORD(wParam)) {
      
    case IDOK:
    case IDCANCEL:
      EndDialog(hwnd, (INT_PTR) LOWORD(wParam));	    /* Use 'EndDialog', NOT 'DestroyWindow()' */
      return (INT_PTR) TRUE;
    }

  case WM_INITDIALOG:					    /* Processing Before Dialog Appears (Controls exist) */
    return (INT_PTR) TRUE;

  default:
    return (INT_PTR) FALSE;				    /* Return FALSE for unprocessed messages */
  }
}



/**
   Local Variables: **
   mode:c **
   comment-column:60 **
   fill-column:120 **
   compile-command:"gcc -D UNICODE -D _UNICODE -D _WIN32_IE=0x0300 -c winMenu.c -o winMenu.o" **
   End: **

**/
