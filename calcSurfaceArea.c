/**
						     FAIRING SIZER

					       Calculate Surface Area

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
#define RADIUS(x) ((x) / 2.0)
#define SQUARE(x) ((x) * (x))
#define SIMPSON_SEGMENTS 1000

/**

Payload Fairing Regional Surface Areas

    Surface Areas for:
           - Nose Cap:   getNoseCapSurfaceArea
	   - Ogive:      getOgiveSurfaceArea
	   - Barrel:     getBarrelSurfaceArea
	   - BoatTail:   getBoatTailSurfaceArea
 **/

// Surface Area of Sperical Cap 
// A = 2PI x r x h
static double getNoseCapSurfaceArea( double ogive_radius,
				     double nose_radius,
				     double base_diameter ) { 

  /**
     Xo - Xt = Dist from cap center point to X coord of tangency w/ogive
  **/
  double Xo = GetNoseRadiusCenterPointX(    ogive_radius, nose_radius, base_diameter );
  double Xt = GetOgiveSphereTangencyPointX( ogive_radius, nose_radius, base_diameter );

  return 2.0 * PI * nose_radius * (Xo - Xt);
}

// Ogive Integrand for Independent Dimension X
static double getOgiveIntegrand( double r, 			    /* Ogive Radius */
				 double d,			    /* Base Diameter */
				 double x ) {

  /**
     Surface Area Integrad:
         [ Y * sqrt( 1 + y'^2) ]dx
  **/

  return ogiveY( r, d, x ) * sqrt( 1 + SQUARE( ogiveYPrime( r, d, x )));
}


// Integreate Ogive Surface Area
static double integrateOgiveSurfaceArea( double r,		    /* Ogive Radius */
					 double d,		    /* Base Diameter */
					 double X1, 
					 double X2 ) {

  /* Integration Constants */
  int i;
  double X_i, term;
  double sum = 0;

  /* Split interval into segments */
  double h = ( X2 - X1 ) / SIMPSON_SEGMENTS;

  /* Calc f(x0) */
  sum += getOgiveIntegrand( r, d, X1 );

  /* Perform Simpson Integration over middle segments - calc f(x_i) */
  for ( i=1; i<( SIMPSON_SEGMENTS - 1); i++ ) {

    X_i = X1 + ( h * i );
    sum += ((i%2==0) ? 4.0 : 2.0) * getOgiveIntegrand( r, d, X_i );
  }

  /* Calc f(xn) */
  sum += getOgiveIntegrand( r, d, X2 );


  return ((X2 - X1) / (3.0 * SIMPSON_SEGMENTS)) * sum;
}


// Integrate to Get Ogive Surface Area
static double getOgiveSurfaceArea( double ogive_radius,
				   double nose_radius,
				   double base_diameter ) {

  double Xt = GetOgiveSphereTangencyPointX( ogive_radius,
					    nose_radius, base_diameter );

  /* Tangency pt w/barrel */
  double L  = GetOgiveLength( ogive_radius, base_diameter ); 

  return 2.0 * PI * integrateOgiveSurfaceArea( ogive_radius, base_diameter, Xt, L );
}			    


// Barrel surface area
static double getBarrelSurfaceArea( double ogive_radius,
				    double nose_radius,
				    double base_diameter,
				    double height ) {

  return 2.0 * PI * (base_diameter / 2.0) * 
    GetBarrelLength( ogive_radius, nose_radius, base_diameter, height );
}


// Boat Tail (Frustrum) Surface Area

static double getBoatTailSurfaceArea( double barrel_diameter,
				      double boat_tail_diameter,
				      double boat_tail_height ) {

  double Dr = RADIUS( barrel_diameter ) - RADIUS( boat_tail_diameter );

  printf( "The Dr is: %f\tthe BT Height is: %f\n", Dr, boat_tail_height );
  fflush( NULL );

  return sqrt( SQUARE( Dr ) + SQUARE( boat_tail_height ));
}


/* Public Function: */
/*                 Get Fairing Total Surface Area */

double GetFairingSurfaceArea( double ogive_radius,
			      double nose_radius,
			      double base_diameter,
			      double height ) {

  double Ac  = getNoseCapSurfaceArea( ogive_radius, nose_radius, base_diameter );
  double Ao  = getOgiveSurfaceArea( ogive_radius, nose_radius, base_diameter );
  double Ab  = getBarrelSurfaceArea( ogive_radius, nose_radius, base_diameter, height );
  double At  = ( !isBoatTail() ) ? 0.0 : 
    getBoatTailSurfaceArea( getBaseDiameter(), getBTDiameter(), getBTHeight() );


  return Ac + Ao + Ab + At;
}



/**
   Local Variables: **
   mode:c **
   comment-column:60 **
   fill-column: 120 **
   compile-command:"gcc -c calcSurfaceArea.c -o calcSurfaceArea.o" **
   End: **
**/
