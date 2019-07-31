-- Write a NASTRAN BDF for Payload Fairing

-- Fairing Sizer - Show PLF Dimensions on Notebook Paper Sized Drawing
-- Copyright (C) 2019  Andrew C. Suttles
-- 
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
-- 
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
-- 
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.
-- 
-- Andrew Suttles - andrew.suttles@gmail.com


local _deg2rad = math.pi / 180.0	  

-- GLOBALS for GEOMETRY

local _L			-- Overall Length
local _Lo			-- Length Ogive Section
local _Lb			-- Length Barrel Section

local _Db			-- Diameter Barrel

local _Rn			-- Nose Radius
local _Xt			-- Nose Xt (Tangency)

local _S 			-- Grid Spacing


-- ------------------------------------------------------------------------------
--			   Helper Functions
-- ------------------------------------------------------------------------------

local function write_blank_line(f,n)
   for i = 1,n do
      f:write( "$\n" )
   end
end

local function write_ruler(f)
   f:write( "$--01--][--02--][--03--][--04--][--05--][--06--][--07--][--08--][--09--][--10--]\n" )
end

local function write_cord2c(f)
   f:write( "CORD2C         1               0       0       0       0       0       1\n" )
   f:write( "               1       0       0\n" )
end

local function write_grid_description(f)

   write_blank_line(f,1)
   f:write( "$  GRID IDs:  XXYYYZZZ\n" )
   f:write( "$             XX       = Section ID( Barrel=10, Ogive=20, Nose=30\n" )
   f:write( "$                YYY    = Ring Number for this section ( < 500  )\n" )
   f:write( "$                   ZZZ = Grid Number for this ring (0 - 359 )\n" ) 

end

-- Write a circumferential ring of grids
-- @Out: None
-- @Params:
--     f    = file handle
--     elID = fairing segment ID
--     i    = ring number
--     z    = z Coordinate
--     R    = R Coordinate
local function write_grid_ring( f, elID, i, z, R )

   --Grid ID    Section ID           Ring ID
   local id = ( 1000000 * elID ) + ( 1000 * i )

   for j = 0,359 do		       -- 360 degrees

      f:write( string.format( "GRID    %-8d%8d%8.2f%8.5f%8.2f\n", 
			      id + j, 1, R, ( _deg2rad * j), z ))
   end
end

-- ------------------------------------------------------------------------------
--			   Write Barrel BDF
-- ------------------------------------------------------------------------------

local function write_barrel_grids( f ) 
   

   local Rb = ( _Db / 2.0 )		  -- radius of barrel diameter
   _S        = Rb * _deg2rad		  -- dist between grids
                                          -- calc num vertical rings 
   local n  = math.min( math.floor( _Lb / _S ), 500 )

   -- Lower and Middle Rings
   for i = 0,n do
      local z = ( _Lb / n ) * i
      write_grid_ring( f, 10, i, z, Rb )
   end
   
   -- Top Ring
   write_grid_ring( f, 10, n+1, _Lb, Rb )
end


local function write_barrel_bdf()

   local f = io.open( "barrel.bdf", "w" ) -- NASTRAN bdf for barrel section

   write_grid_description(f)
   write_blank_line(f,1)
   write_ruler(f)
   write_barrel_grids( f )

   f:close()
end

-- ------------------------------------------------------------------------------
--			   Write Ogive BDF
-- ------------------------------------------------------------------------------

local function write_ogive_grids( f )

   local n  = math.min( math.floor( _Lo / _S ), 500 )

   -- Lower and Middle Rings
   for i = 0,n do
      local z = ( _Lo / n ) * i
      write_grid_ring( f, 20, i, _Lb + z, ogiveY( z / 12.0 ))
   end

   -- Top Ring
   write_grid_ring( f, 20, n+1, _Lb + _Lo, ogiveY( _Lo / 12.0 ))
end


local function write_ogive_bdf()

   local f = io.open( "ogive.bdf", "w" ) -- NASTRAN bdf for ogive section

   write_grid_description(f)
   write_blank_line(f,1)
   write_ruler(f)
   write_ogive_grids( f )

   f:close()
end


-- ------------------------------------------------------------------------------
--			   Write Ogive BDF
-- ------------------------------------------------------------------------------

local function write_nose_grids( f )

   local i = 0
   local z0 = _Lb + _Lo

   -- Lower and Middle Rings
   for x = _Xt,_Rn,_S do
      local z = z0 + (x - _Xt)
      local h = _Rn - x		-- dist from x to Rn
      write_grid_ring( f, 30, i, z, math.sqrt(( 2.0 * h * _Rn ) - ( h * h )))
      i = i + 1
   end

   -- Top Grid
   f:write( string.format( "GRID    %-8d%8d%8.2f%8.5f%8.2f\n", 
			   30000000 + (1000 * i),
			   1, 0.0, 0.0, _L ))
end

local function write_nose_bdf()

   local f = io.open( "nose.bdf", "w" ) -- NASTRAN bdf for ogive section

   write_grid_description(f)
   write_blank_line(f,1)
   write_ruler(f)
   write_nose_grids( f )

   f:close()

end

-- ------------------------------------------------------------------------------
--			      Write BDF
-- ------------------------------------------------------------------------------

local function set_global_state()

   _L  = getLength()
   _Lo = getOgiveLength()
   _Lb = getBarrelLength()
   _Db = getBaseDiameter()
   _Rn = getNoseRadius()
   _Xt = getNoseCapXt()

end

function write_deck()

   set_global_state()		-- Load Fairing Dimensions
   
   local f = io.open( "fairing.bdf", "w" ) -- Create NASTRAN bdf 

   f:write( "BEGIN BULK\n" )
   write_blank_line(f,2)

   -- Coord System
   write_cord2c(f)
   write_blank_line(f, 2)

   -- Barrel Section
   f:write( "INCLUDE 'barrel.bdf'\n" )
   write_barrel_bdf( Db, Lb )
   
   -- Ogive Section
   f:write( "INCLUDE 'ogive.bdf'\n" )
   write_ogive_bdf()

   -- Nose Cap Section
   f:write( "INCLUDE 'nose.bdf'\n" )
   write_nose_bdf()

   f:close()
end


