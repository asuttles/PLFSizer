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
#include<math.h>
#include<assert.h>
#include<stdio.h>

#define BASE_RADIUS ( base_diameter / 2.0 )
#define SQUARE(x) ((x) * (x))

#ifdef UNIT_TEST_ALL
#define UNIT_TEST_TANGENT_OGIVE
#endif



/**
**********************************************************************

			TANGENT OGIVE GEOMETRY

from: http://tmtpages.com/tech/tangent_ogive.htm

where:

       L = The length of the ogive nose from base to tip.
       D = The diameter of the base of the ogive.
       T = The diameter of the ogive tip.
       R = The radius of the curve of the ogive in inches.

**********************************************************************
 **/


/**
   TANGENT OGIVE RADIUS

   Return the tangent ogive RADIUS (R), given: L, D, T

**/
double tangent_ogive_radius( double L, double D, double T ) {

  assert( L > 0.0 && D > 0.0 && T > 0.0 ); 

  double delt	= D - T;
  double R1	= L * L;
  double R2	= R1 / delt;

  return R2 + ( delt / 4.0 );
}


/**
   TANGENT OGIVE LENGTH

   Return the tangent ogive LENGTH (L), given: D, T, R

**/
double tangent_ogive_length( double D, double T, double R ) {

  assert( D > 0.0 && T > 0.0 && R > 0.0 ); 

  double delt = D - T;
  double L1 = delt * delt;
  double L2 = L1 / 4.0;
  double L3 = R * delt - L2;

  return sqrt( L3 );
}


/**
   TANGENT OGIVE TIP DIAMETER

   Return the tangent ogive TIP (T) diameter, given: L, D, R

**/
double tangent_ogive_diameter_tip( double L, double D, double R ) {

  assert( L > 0.0 && D > 0.0 && R > 0.0 ); 

  double T1 = R*R - L*L;
  double T2 = sqrt( T1 * 4.0 );
  
  return D - (2.0 * R) + T2;
}


/**
   TANGENT OGIVE BASE DIAMETER

   Return the tangent ogive BASE (D) diameter, given: L, T, R

**/
double tangent_ogive_diameter_base( double L, double T, double R ) {

  assert( L > 0.0 && T > 0.0 && R > 0.0 ); 

  double D1 = R*R - L*L;
  double D2 = sqrt( D1 * 4.0 );
  
  return T + ( 2.0 * R  ) - D2;
}


/**
*************************************************************************************

		  SPHERICALLY BLUNTED TANGENT OGIVE

from: 
  http://en.wikipedia.org/wiki/Nose_cone_design#Spherically_blunted_tangent_ogive


*************************************************************************************
 **/

  /**
 
     GetOgiveLength

     The ogive length is the length from the point of tangency with
     barrel section, to the intersection of curve with center line.
     The point of intersection is forward of the tip of the blunting
     sphere.

   **/
double GetOgiveLength( double ogive_radius, 
		       double base_diameter ) {

  return sqrt(( 2.0 * BASE_RADIUS * ogive_radius ) - SQUARE( BASE_RADIUS ));
}

double GetNoseRadiusCenterPointX( double ogive_radius,
				  double nose_radius,
				  double base_diameter ) {

  double L  = GetOgiveLength( ogive_radius, base_diameter );
  double x1 = ogive_radius - nose_radius;
  double x2 = ogive_radius - BASE_RADIUS;

  return (L - sqrt( ( x1 * x1 ) - ( x2 * x2 )));
}

double GetNoseRadiusCenterPointY( void ) {
  
  return 0.0;
}

double GetOgiveSphereTangencyPointY( double ogive_radius,
				     double nose_radius,
				     double base_diameter ) {
 
  return (( nose_radius * ( ogive_radius - BASE_RADIUS )) / 
	  ( ogive_radius - nose_radius ));
}

 
double GetOgiveSphereTangencyPointX( double ogive_radius,
				     double nose_radius,
				     double base_diameter ) { 

  /* Dist. from Ogive-to-CL intersection to blunted nose CP */
  double Xo = GetNoseRadiusCenterPointX( ogive_radius, 
					 nose_radius, 
					 base_diameter );

  double Yt = GetOgiveSphereTangencyPointY( ogive_radius,
					    nose_radius,
					    base_diameter );

  return Xo - sqrt( SQUARE( nose_radius )  - SQUARE( Yt ));
}

double GetLengthUpperFairing( double ogive_radius,
			      double nose_radius,
			      double base_diameter ) {

  double L  = GetOgiveLength( ogive_radius, 
			      base_diameter );

  double Xo = GetNoseRadiusCenterPointX( ogive_radius, 
					 nose_radius, 
					 base_diameter );

  return L - Xo + nose_radius;
}

/**
**********************************************************************

Ogive: Y = F(x)

       Y = sqrt[ rho^2 - (L-x)^2 ] + R - rho

       where:
       rho = ogive radius
       L   = ogive length
       R   = base radius
       x   = independant variable

**********************************************************************
**/

double ogiveY( double ogive_radius, 
	       double base_diameter,
	       double X ) {

  double L = GetOgiveLength( ogive_radius, base_diameter );

  return sqrt( SQUARE( ogive_radius ) - SQUARE( L - X )) + 
    BASE_RADIUS - ogive_radius;
}


/**
**********************************************************************

Ogive: Y' = F(x)

       Y' = (L-x) / sqrt[ rho^2 - (L-x)^2 ]

       where:
       rho = ogive radius
       L   = ogive length
       x   = independant variable (dist from ogive-CL i/f)

**********************************************************************
**/

double ogiveYPrime( double ogive_radius,
		    double base_diameter,
		    double X ) {
  
  double L = GetOgiveLength( ogive_radius, base_diameter );

  return ( L - X ) / sqrt( SQUARE( ogive_radius ) - SQUARE( L - X ));
}
   

/**
**********************************************************************
			      UNIT TESTs
**********************************************************************
**/

#ifdef UNIT_TEST_TANGENT_OGIVE

#ifndef UNIT_TEST
#define UNIT_TEST
#endif

/* Test values from: http://tmtpages.com/tech/tangent_ogive.htm */
#define L 0.269
#define D 0.284
#define T 0.160
#define R 0.615

#define ETA 0.001		/* Ord of mag of sample values given in example */


void unit_test_tangent_ogive() {

  printf( "\nunit_test_tangent_ogive testing..." );

  // Basic Ogive Calculation Tests...

  /* Test 1: Ogive Length */
  assert( fabs( tangent_ogive_length( D, T, R ) - L ) < ETA );

  /* Test 2: Ogive Base Diameter */
  assert( fabs( tangent_ogive_diameter_base( L, T, R ) - D ) < ETA );

  /* Test 3: Ogive Tip Diameter  */
  assert( fabs( tangent_ogive_diameter_tip( L, D, R ) - T )  < ETA );

  /* Test 4: Ogive Radius  */
  assert( fabs( tangent_ogive_radius( L, D, T ) - R ) < ETA );

  // Blunted Ogive Calculation Tests...

  /* Test 5: Ogive Length */
  assert( fabs( GetOgiveLength( 71.6, 27.6 ) - 42.2577 ) < ETA );

  /* Test 6: Get Center Point of Fairing Nose Cap */
  assert( fabs( GetNoseRadiusCenterPointX( 71.6, 4.58, 27.6 ) - 8.3337 ) < ETA );

  /* Test 7: Get Ogive-Sphere Tangency Point Y */
  assert( fabs( GetOgiveSphereTangencyPointY( 71.6, 4.58, 27.6 ) - 3.9499 ) < ETA );

  /* Test 8: Get Ogive-Sphere Tangency Point X */
  assert( fabs( GetOgiveSphereTangencyPointX( 71.6, 4.58, 27.6 ) - 6.0154 ) < ETA );

  /* Test 9: Get Length of Upper Fairing */
  assert( fabs( GetLengthUpperFairing( 71.6, 4.58, 27.6 ) - 38.504 ) < ETA );  
  

  printf( "SUCCESS\n" );

  return;
}
#endif	/* UNIT_TEST_TANGENT_OGIVE */



#ifdef UNIT_TEST

int main() {

#ifdef UNIT_TEST_TANGENT_OGIVE
  unit_test_tangent_ogive();
#endif	/* UNIT_TEST_TANGENT_OGIVE */

  return 0;
}

#endif	/* UNIT_TEST */


/**
   Local Variables: **
   mode:c **
   comment-column:60 **
   fill-column: 120 **
   compile-command:"gcc -D UNIT_TEST_TANGENT_OGIVE ogive.c" **
   End: **
**/
