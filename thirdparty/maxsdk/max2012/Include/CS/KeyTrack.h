
// This file should really be called vector.h, since it defines the internally used
// class, vector, as well as the path_property classes.  SDK users should not need this file,
// except to compile any programs which include iMoFlow.h.  Only the motion flow snippet class 
// references these classes in a function call which SDK users will probably not use.  
// This file is part of an old file structure which is being kept in place only for the sake of 
// allowing older programs using the CS SDK to run.

#pragma once

#include "..\maxheap.h"
#include "BipExp.h"
#include "Tracks.h"
#include "..\point4.h"
/*
//This is all in bipexp.h now
#define BIPSLAVE_CONTROL_CLASS_ID Class_ID(0x9154,0)
// this is the class for the center of mass, biped root controller ("Bip01")
#define BIPBODY_CONTROL_CLASS_ID  Class_ID(0x9156,0) 
// this is the class for the biped footstep controller ("Bip01 Footsteps")
#define FOOTPRINT_CLASS_ID Class_ID(0x3011,0)
#define SKELOBJ_CLASS_ID Class_ID(0x9125, 0)
#define BIPED_CLASS_ID Class_ID(0x9155, 0)
*/

// This class is used internally
class vector: public MaxHeapOperators {
public: 
	float x;
	float y;
	float z;
	float w;

	//constructors
	vector() 
		: x(0.0f),y(0.0f),z(0.0f),w(1.0f) {} // Should be twice as fast now
	vector(float X, float Y, float Z) 
		: x(X), y(Y), z(Z), w(1.0f) {} // Should be twice as fast now
	vector (float xx, float yy, float zz, float ww) 
		: x(xx), y(yy), z(zz), w(ww) { }
	vector(Point4 p) 
		: x(p.x), y(p.y), z(p.z), w(p.w) { }

	// Binary operators
	inline  vector operator-(const vector&) const;
	inline  vector operator+(const vector&) const; 
};

inline vector vector::operator-(const vector& b) const {
	return(vector(x-b.x,y-b.y,z-b.z));
	}

inline vector vector::operator+(const vector& b) const {
	return(vector(x+b.x,y+b.y,z+b.z));
	}

inline vector operator*(float f, const vector& a) {
	return(vector(a.x*f, a.y*f, a.z*f, a.w)); //MG was a.w*f
	}

inline Point4 VtoP4(const vector& v) { return Point4(v.x, v.y, v.z, v.w); }

typedef Point3 vector2D;


// This class is used internally
class path_property: public MaxHeapOperators
{
public:
	float val;
	int   time;
	float distance;
	path_property(){
		val = 0.0;
		time = 0;
		distance = 0;
	}

};

// This class is used internally
class path_properties: public MaxHeapOperators
{ 
      	public:
			float  turn_angle;
			float  line_distance;
			float  path_distance;
			float  average_speed;
			float speed_change;
			vector direction;
			path_property min_speed;
			path_property max_speed;
			path_property stop;
			path_properties(){
				turn_angle = 0.0;
				line_distance    = 0.0;
				path_distance = 0.0;
				average_speed = 0.0;
				speed_change  = 1.0;
				min_speed.val     = 1000000.0;
			}

		 path_properties& operator=(const path_properties& T);

};


