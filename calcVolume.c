
/**
						     FAIRING SIZER

		                             Calculate the Payload Volume

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

#include <math.h>
#include <stdio.h>

#include "winForms.h"
#include "ogive.h"

#define PI (3.141592653589793)
#define SQUARE(x) ((x) * (x))
#define SIMPSON_SEGMENTS 1000


/**

Payload Fairing Regional Volumes

      Volumes for:
           - Nose Cap:   getNoseCapVolume
	   - Ogive:      getOgiveVolume
	   - Barrel:     getBarrelVolume
 **/

// Volume of a Sperical Cap 
// V = ( PI*h / 6 ) * (3a^2 + h^2)
static double getNoseCapVolume( double ogive_radius,
				double nose_radius,
				double base_diameter ) { 

  /* Radius at Base of Spherical Cap */
  double  a = GetOgiveSphereTangencyPointY( ogive_radius, nose_radius, base_diameter );

  /* Height of Sperical Cap */
  double Xo = GetNoseRadiusCenterPointX(    ogive_radius, nose_radius, base_diameter );
  double Xt = GetOgiveSphereTangencyPointX( ogive_radius, nose_radius, base_diameter );

  double h  = nose_radius - (Xo - Xt); 

  return ( PI * h / 6.0 ) * (( 3.0 * SQUARE(a) ) + SQUARE(h) );
}


// Barrel Volume
// V = PI*r^2*h
static double getBarrelVolume( double ogive_radius,
			       double nose_radius,
			       double base_diameter,
			       double height,
			       double offset ) {

  double h = GetBarrelLength( ogive_radius, nose_radius, base_diameter, height );
  double r = ( base_diameter / 2.0 ) - offset;

  return PI * SQUARE( r ) * h;
}




// Integrate to Get Ogive Volume
// PI * Integrate (X1 to X2) y^2 * dx
static double getOgiveVolume( double ro,                    /* Radius Ogive  */
			      double rn,		    /* Radius Nose   */
			      double d,			    /* Base Diameter */
			      double o ) {                  /* Offset for LOC */

  int i;
  double X_i, y;
  double sum = 0;

  /* Get integration bounds */
  double X1 = GetOgiveSphereTangencyPointX( ro, rn, d );
  double X2  = GetOgiveLength( ro, d );


  /* SIMPSON INTEGRATION */


  /* Split interval into segments */
  double h = ( X2 - X1 ) / SIMPSON_SEGMENTS;

  /* Calc f(x0) */
  y = ogiveY( ro, d, X1 );
  sum += (y > o) ? SQUARE( y-o ) : 0.0;

  /* Perform Simpson Integration over middle segments - calc f(x_i) */
  for ( i=1; i<( SIMPSON_SEGMENTS - 1); i++ ) {

    X_i = X1 + ( h * i );
    y = ogiveY( ro, d, X_i );
    sum += ((i%2==0) ? 4.0 : 2.0) * ((y > o) ? SQUARE( y-o ) : 0.0);
  }

  /* Calc f(xn) */
  y = ogiveY( ro, d, X2 );
  sum += (y > o) ? SQUARE( y-o ) : 0.0;

  /* Return Volume Calc */
  return PI * (( (X2 - X1) / (3.0 * SIMPSON_SEGMENTS) ) * sum );
}			    




/* Public Functions: */
/*                 Get Fairing Total Volume */

double GetFairingVolume( double ogive_radius,
			 double nose_radius,
			 double base_diameter,
			 double height ) {

  return getNoseCapVolume( ogive_radius, nose_radius, base_diameter ) + 
    getOgiveVolume( ogive_radius, nose_radius, base_diameter, 0.0 ) +
    getBarrelVolume( ogive_radius, nose_radius, base_diameter, height, 0.0 );
}


/*                 Get Payload Volume */
double GetPayloadVolume( double ogive_radius,
			 double nose_radius,
			 double base_diameter,
			 double height,
			 double offset ) {

  return  getOgiveVolume( ogive_radius, nose_radius, base_diameter, offset ) +
    getBarrelVolume( ogive_radius, nose_radius, base_diameter, height, offset );
}


/*                 Get a Barrel Volume */
double GetBarrelVolume( double len, double dia ) {

  return PI * SQUARE(( dia / 2.0 )) * len;
}




/**
   Local Variables: **
   mode:c **
   comment-column:60 **
   fill-column: 120 **
   compile-command:"gcc -c calcVolume.c -o calcVolume.o" **
   End: **
**/
