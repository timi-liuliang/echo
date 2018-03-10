/*		3DMath.h - the 3D math family of classes - vectors, rays, quat, matrices for MAXScript
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#pragma once

#include "..\ScripterExport.h"
#include "..\kernel\value.h"
#include "..\..\quat.h"
#include "..\..\ipoint3.h"

#pragma warning(push)
#pragma warning(disable:4100)

extern ScripterExport void _QuatToEuler(Quat& q, float* ang);
extern ScripterExport void _EulerToQuat(float* ang, Quat& q);

/* ------------------------ Point3Value ------------------------------ */

applyable_class_debug_ok (Point3Value)

class Point3Value : public Value
{
public:
	Point3		p;

	ENABLE_STACK_ALLOCATE(Point3Value);

 ScripterExport Point3Value(Point3 init_point);
 ScripterExport Point3Value(float x, float y, float z);
 ScripterExport Point3Value(Value* x, Value* y, Value* z);

				classof_methods(Point3Value, Value);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
#	define		is_point3(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Point3Value))

	static Value* make(Value**arg_list, int count);
	
	/* operations */

#include "..\macros\define_implementations.h"
#	include "..\protocols\vector.inl"
	use_generic  ( coerce,	"coerce");
	use_generic  ( copy,	"copy");
	use_generic  ( get,		"get");
	use_generic  ( put,		"put");

	/* built-in property accessors */

	def_property ( x );
	def_property ( y );
	def_property ( z );

	Point3		to_point3() { return p; }
	AColor		to_acolor() { return AColor (p.x / 255.0f, p.y / 255.0f, p.z / 255.0f); }
	Point2		to_point2() { return Point2 (p.x, p.y); }
	ScripterExport void		to_fpvalue(FPValue& v);
	COLORREF	to_colorref() { return RGB((int)p.x, (int)p.y, (int)p.z); }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

class ConstPoint3Value : public Point3Value
{
public:
 ScripterExport ConstPoint3Value(float x, float y, float z)
	 : Point3Value (x, y, z) { }

	void		collect() { delete this; }
	BOOL		is_const() { return TRUE; }

	Value* set_x(Value** arg_list, int count) { throw RuntimeError (_M("Constant vector, not settable")); return NULL; }
	Value* set_y(Value** arg_list, int count) { throw RuntimeError (_M("Constant vector, not settable")); return NULL; }
	Value* set_z(Value** arg_list, int count) { throw RuntimeError (_M("Constant vector, not settable")); return NULL; }
};

// The following function has been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.

inline  IPoint3 to_ipoint3(Value* val) { 
			Point3 p = val->to_point3(); 
			return IPoint3((int)p.x, (int)p.y, (int)p.z); }

// End of 3ds max 4.2 Extension

/* ------------------------ RayValue ------------------------------ */

applyable_class_debug_ok (RayValue)

class RayValue : public Value
{
public:
	Ray			r;

	ENABLE_STACK_ALLOCATE(RayValue);

 ScripterExport RayValue(Point3 init_origin, Point3 init_dir);
 ScripterExport RayValue(Ray init_ray);

				classof_methods (RayValue, Value);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	/* operations */
	
	use_generic  ( copy,	"copy");

	/* built-in property accessors */

	def_property ( pos );
	def_property_alias ( position, pos );
	def_property ( dir );

	Ray			to_ray() { return r; }
	ScripterExport void		to_fpvalue(FPValue& v);
#	define		is_ray(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(RayValue))

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ------------------------ QuatValue ------------------------------ */

applyable_class_debug_ok (QuatValue)

class QuatValue : public Value
{
public:
	Quat		q;

	ENABLE_STACK_ALLOCATE(QuatValue);

 ScripterExport QuatValue(const Quat& init_quat);
 ScripterExport QuatValue(float w, float x, float y, float z);
 ScripterExport QuatValue(Value* w, Value* x, Value* y, Value* z);
 ScripterExport QuatValue(Value* val);
 ScripterExport QuatValue(AngAxis& aa);
 ScripterExport QuatValue(float* angles);
 ScripterExport QuatValue(Matrix3& m);

				classof_methods (QuatValue, Value);
#	define		is_quat(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(QuatValue))
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	/* operations */

#include "..\macros\define_implementations.h"
#	include "..\protocols\quat.inl"
	use_generic  ( copy,	"copy");

	/* built-in property accessors */

	def_property ( w );
	def_property ( x );
	def_property ( y );
	def_property ( z );
	def_property ( angle );
	def_property ( axis );

	Quat		to_quat() { return q; }
	AngAxis		to_angaxis() { return AngAxis(q); }
	ScripterExport void		to_fpvalue(FPValue& v);

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ------------------------ AngleAxis ------------------------------ */

applyable_class_debug_ok (AngAxisValue)

class AngAxisValue : public Value
{
public:
	AngAxis		aa;

	ENABLE_STACK_ALLOCATE(AngAxisValue);

 ScripterExport AngAxisValue(const AngAxis& iaa);
 ScripterExport AngAxisValue(const Quat& q);
 ScripterExport AngAxisValue(const Matrix3& m);
 ScripterExport AngAxisValue(float* angles);
 ScripterExport AngAxisValue(float angle, Point3 axis);
 ScripterExport AngAxisValue(Value*);
 ScripterExport AngAxisValue(Value* angle, Value* axis);

				classof_methods (AngAxisValue, Value);
#	define		is_angaxis(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(AngAxisValue))
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	/* operations */
	
#include "..\macros\define_implementations.h"

	use_generic( coerce,	"coerce" );
	use_generic( eq,		"=");
	use_generic( ne,		"!=");
	use_generic( random,	"random");
	use_generic( copy,		"copy");

	/* built-in property accessors */

	def_property ( angle );
	def_property ( axis );
	def_property ( numrevs );

	AngAxis		to_angaxis() { return aa; }
	Quat		to_quat() { return Quat (aa); }
	ScripterExport void		to_fpvalue(FPValue& v);

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ------------------------ EulerAngles ------------------------------ */

applyable_class_debug_ok (EulerAnglesValue)

class EulerAnglesValue : public Value
{
public:
	float		angles[3];

	ENABLE_STACK_ALLOCATE(EulerAnglesValue);

 ScripterExport EulerAnglesValue(float ax, float ay, float az);
 ScripterExport EulerAnglesValue(const Quat&);
 ScripterExport EulerAnglesValue(const Matrix3&);
 ScripterExport EulerAnglesValue(const AngAxis&);

				classof_methods (EulerAnglesValue, Value);
#	define		is_eulerangles(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(EulerAnglesValue))
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	/* operations */
	
#include "..\macros\define_implementations.h"

	use_generic( coerce,	"coerce" );
	use_generic( eq,		"=");
	use_generic( ne,		"!=");
	use_generic( random,	"random");
	use_generic( copy,		"copy");

	/* built-in property accessors */

	def_property ( x );
	def_property ( y );
	def_property ( z );
	def_property ( x_rotation );
	def_property ( y_rotation );
	def_property ( z_rotation );

	AngAxis		to_angaxis() { return AngAxis (to_quat()); }
	Quat		to_quat() { Quat q; _EulerToQuat(angles, q); return Quat (q); }
	ScripterExport void		to_fpvalue(FPValue& v);
	float*		to_eulerangles() { return angles; }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ------------------------ Matrix ------------------------------ */

applyable_class_debug_ok (Matrix3Value)

class Matrix3Value : public Value
{
public:
	Matrix3		m;

	ENABLE_STACK_ALLOCATE(Matrix3Value);

 ScripterExport Matrix3Value(int i);
 ScripterExport Matrix3Value(const Matrix3& im);
 ScripterExport Matrix3Value(const Quat& q);
 ScripterExport Matrix3Value(const AngAxis& aa);
 ScripterExport Matrix3Value(float* angles);
 ScripterExport Matrix3Value(const Point3& row0, const Point3& row1, const Point3& row2, const Point3& row3);

				classof_methods (Matrix3Value, Value);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
#	define		is_matrix3(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Matrix3Value))

	/* operations */
	
#include "..\macros\define_implementations.h"
#	include "..\protocols\matrix.inl"
	use_generic( copy,		"copy");

	/* built-in property accessors */

	def_property ( row1 );
	def_property ( row2 );
	def_property ( row3 );
	def_property ( row4 );
	def_property ( translation );
	def_property ( pos );
	def_property ( rotation );
	def_property ( scale );

	use_generic( get,		"get");
	use_generic( put,		"put");


	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	Matrix3&	to_matrix3() { return m; }
	Quat		to_quat() { return Quat (m); }
	ScripterExport void		to_fpvalue(FPValue& v);

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ------------------------ Point2Value ------------------------------ */

applyable_class_debug_ok (Point2Value)

class Point2Value : public Value
{
public:
	Point2		p;

	ENABLE_STACK_ALLOCATE(Point2Value);

	ScripterExport Point2Value(Point2 ipoint);
	ScripterExport Point2Value(POINT ipoint);
	ScripterExport Point2Value(float x, float y);
	ScripterExport Point2Value(Value* x, Value* y);

				classof_methods(Point2Value, Value);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
#	define		is_point2(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Point2Value))

	static Value* make(Value**arg_list, int count);
	
	/* operations */

#include "..\macros\define_implementations.h"
	use_generic( plus,		"+" );
	use_generic( minus,		"-" );
	use_generic( times,		"*" );
	use_generic( div,		"/" );
	use_generic( uminus,	"u-");
	use_generic( eq,		"=");
	use_generic( ne,		"!=");
	use_generic( random,	"random");
	use_generic( length,	"length");
	use_generic( distance,	"distance");
	use_generic( normalize,	"normalize");
	use_generic( copy,		"copy");
	use_generic( get,		"get");
	use_generic( put,		"put");

	/* built-in property accessors */

	def_property ( x );
	def_property ( y );

	Point2		to_point2() { return p; }
	ScripterExport void		to_fpvalue(FPValue& v);

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

// The following function has been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.

inline  IPoint2 to_ipoint2(Value* val) { 
			Point2 p = val->to_point2(); 
			return IPoint2((int)p.x, (int)p.y); }

// End of 3ds max 4.2 Extension

/* ------------------------ Point4Value ------------------------------ */

applyable_class_debug_ok (Point4Value)

class Point4Value : public Value
{
public:
	Point4		p;

	ENABLE_STACK_ALLOCATE(Point4Value);

	ScripterExport Point4Value(Point4 init_point);
	ScripterExport Point4Value(float x, float y, float z, float w);
	ScripterExport Point4Value(Value* x, Value* y, Value* z, Value* w);

	classof_methods(Point4Value, Value);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
#	define		is_point4(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Point4Value))

	static Value* make(Value**arg_list, int count);

	/* operations */

#include "..\macros\define_implementations.h"
#	include "..\protocols\vector.inl"
	use_generic  ( coerce,	"coerce");
	use_generic  ( copy,	"copy");
	use_generic  ( get,		"get");
	use_generic  ( put,		"put");

	/* built-in property accessors */

	def_property ( x );
	def_property ( y );
	def_property ( z );
	def_property ( w );

	Point4		to_point4() { return p; }
	Point3		to_point3() { return Point3 (p.x, p.y, p.z); }
	AColor		to_acolor() { return AColor (p.x, p.y, p.z, p.w); }
	Point2		to_point2() { return Point2 (p.x, p.y); }
	ScripterExport void		to_fpvalue(FPValue& v);
	COLORREF	to_colorref() { return RGB((int)(p.x*255.f), (int)(p.y*255.f), (int)(p.z*255.f)); }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};


// The following class has been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.

// Wraps Box2 in SDK which is a sub-class of the RECT structure in windows
applyable_class_debug_ok (Box2Value)

class Box2Value : public Value
{

public:
	Box2 b;
						
	ScripterExport	Box2Value();
	ScripterExport	Box2Value(Box2 box);	
	ScripterExport	Box2Value(RECT rect);
	ScripterExport	Box2Value(IPoint2 ul, IPoint2 lr);
	ScripterExport	Box2Value(int x, int y, int w, int h);
	ScripterExport	Box2Value(Value* x, Value* y, Value* w, Value* h);

					classof_methods(Box2Value, Value);
	void			collect() { delete this; }
	ScripterExport	void	sprin1(CharStream* s);	
#	define			is_box2(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Box2Value))
	
	static Value*	make(Value**arg_list, int count);

	/* operations */

#include "..\macros\define_implementations.h"	
#	include "..\protocols\box.inl"
	
	/* built-in property accessors */

	def_property ( x );
	def_property ( y );
	def_property ( w );
	def_property ( h );
	def_property ( left );
	def_property ( top );
	def_property ( right );
	def_property ( bottom );
	def_property ( center );

	Box2&		to_box2() { return b; }
};

#pragma warning(pop)
// End of 3ds max 4.2 Extension

