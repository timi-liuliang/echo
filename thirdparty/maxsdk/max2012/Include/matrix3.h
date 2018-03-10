/**********************************************************************
 *<
   FILE: matrix3.h

   DESCRIPTION: Class definitions for Matrix3

   CREATED BY: Dan Silva

   HISTORY:

 *>   Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "GeomExport.h"
#include "maxheap.h"
#include "ioapi.h"
#include "point3.h"
#include "point4.h"

//Flags
#define POS_IDENT  1
#define ROT_IDENT  2
#define SCL_IDENT  4
#define MAT_IDENT (POS_IDENT|ROT_IDENT|SCL_IDENT)

typedef float MRow[3];

class Quat;
class AngAxis;

class Matrix3;

/*! \sa  Class Point3, <a href="ms-its:3dsmaxsdk.chm::/trans_matrix_representations.html">Matrix Representations of 3D Transformations</a>, Class Quat,  Class AngAxis, Structure AffineParts, Class BigMatrix.\n\n
\par Description:
This class implements a 4x3 3D transformation matrix object. Methods are
provided to zero the matrix, set it to the identity, compute its inverse, apply
incremental translation, rotation and scaling, and build new X, Y and Z
rotation matrices. Operators are provided for matrix addition, subtraction, and
multiplication. All methods are implemented by the system.\n\n
Note: In 3ds Max, all vectors are assumed to be row vectors. Under this
assumption, multiplication of a vector with a matrix can be written either way
(Matrix*Vector or Vector*Matrix), for ease of use, and the result is the same
-- the (row) vector transformed by the matrix.
\par Data Members:
private:\n\n
<b>float m[4][3];</b>\n\n
Matrix storage.\n\n
<b>DWORD flags;</b>\n\n
Matrix Identity Flags.\n\n
<b>POS_IDENT</b>\n\n
Indicates the translation row of the matrix is the identity.\n\n
<b>ROT_IDENT</b>\n\n
Indicates the rotation elements of the matrix are the identity.\n\n
<b>SCL_IDENT</b>\n\n
Indicates the scale elements of the matrix are the identity.\n\n
<b>MAT_IDENT</b>\n\n
Indicates the matrix is the identity matrix. This is equivalent to
<b>(POS_IDENT|ROT_IDENT|SCL_IDENT)</b>.\n\n
<b>static const Matrix3 Identity;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
An instance of an identity matrix.  */
class Matrix3: public MaxHeapOperators {
   friend Matrix3 GEOMEXPORT RotateXMatrix(float angle);   
   friend Matrix3 GEOMEXPORT RotateYMatrix(float angle); 
   friend Matrix3 GEOMEXPORT RotateZMatrix(float angle); 
   friend Matrix3 GEOMEXPORT TransMatrix(const Point3& p);
   friend Matrix3 GEOMEXPORT ScaleMatrix(const Point3& s);
   friend Matrix3 GEOMEXPORT RotateYPRMatrix(float Yaw, float Pitch, float Roll);
   friend Matrix3 GEOMEXPORT RotAngleAxisMatrix(Point3& axis, float angle);
   friend Matrix3 GEOMEXPORT Inverse(const Matrix3& M);
   friend Point3 GEOMEXPORT operator*(const Matrix3& A, const Point3& V);
   friend Point3 GEOMEXPORT operator*(const Point3& V, const Matrix3& A);
   friend Point3 GEOMEXPORT VectorTransform(const Matrix3& M, const Point3& V); 
   friend Matrix3 GEOMEXPORT XFormMat(const Matrix3& xm, const Matrix3& m);
    friend Point3 GEOMEXPORT VectorTransform(const Point3& V, const Matrix3& M);
   friend class Quat;
   float m[4][3];
   // Access i-th row as Point3 for read or assignment:
   Point3& operator[](int i) { return((Point3&)(*m[i]));  }
   DWORD flags;

public:
   /*! \remarks Returns a reference to the 'i-th' Point3 of the matrix. */
   const Point3& operator[](int i) const { return((Point3&)(*m[i])); }
   // if change any components directly via GetAddr, must call this
   /*! \remarks This clears the <b>MAT_IDENT</b> flag to indicate the matrix
   is not the identity. If any changes are made to components directly via
   <b>GetAddr()</b>, this method must be called. */
   void SetNotIdent() { flags &= ~MAT_IDENT; }
   /*! \remarks This sets the specified identity flag(s).
   \par Parameters:
   <b>DWORD f</b>\n\n
   Specifies the identity flag bit(s) to set. See Matrix Identity Flags above.
   */
   void SetIdentFlags(DWORD f) { flags &= ~MAT_IDENT; flags |= f; }
   /*! \remarks Returns the identity flags. */
   DWORD GetIdentFlags() const { return flags; }
   /*! \remarks Clears the specified identity flag(s). See Matrix Identity
   Flags above.
   \par Parameters:
   <b>DWORD f</b>\n\n
   Specifies the identity flag bit(s) to clear. */
   void ClearIdentFlag(DWORD f) { flags &= ~f; }
   /*! \remarks Returns TRUE if the matrix is the identity matrix (based on
   the flags); otherwise FALSE. */
   BOOL IsIdentity() const { return ((flags&MAT_IDENT)==MAT_IDENT); }
   /*! \remarks This method may be used to recompute the <b>*_IDENT</b> flags for this
   matrix. For instance, if you call a method, such as
   <b>INode::GetObjTMAfterWSM()</b>, and it returns a matrix, you cannot use
   the <b>IsIdentity()</b> method to check if the matrix is indeed the
   identity. This is because the flags that method checks are not initialized
   by the INode method. What you can do however is call this method first. This
   will validate the flags in the matrix so they accuratly reflect the
   properties of the matrix. If after calling this method, and then calling
   <b>IsIdentity()</b>, the proper result would be returned. */
   GEOMEXPORT void ValidateFlags(); // recomputes the IDENT flags

   // CAUTION: if you change the matrix via this pointer, you MUST clear the
   // proper IDENT flags !!!
   /*! \remarks Returns the address of this Matrix3.\n\n
   The Matrix3 class keeps flags indicating identity for rotation, scale,
   position, and the matrix as a whole, and thus the direct access via the []
   operator is restricted to prevent developers from modifying the matrix
   without updating the flags. This method, GetAddr(), still lets you get at
   the matrix itself and then you can use the [] operator on the result. Note:
   If you change the matrix via this pointer, you MUST clear the proper IDENT
   flags!\n\n
   Also Note: <b>typedef float MRow[3];</b>
   \return  The address of the Matrix3. */
   MRow* GetAddr() { return (MRow *)(m); }   
   /*! \remarks Returns the address of this Matrix3.\n\n
   The Matrix3 class keeps flags indicating identity for rotation, scale,
   position, and the matrix as a whole, and thus the direct access via the []
   operator is restricted to prevent developers from modifying the matrix
   without updating the flags. This method, GetAddr(), still lets you get at
   the matrix itself and then you can use the [] operator on the result. Note:
   If you change the matrix via this pointer, you MUST clear the proper IDENT
   flags!\n\n
   Also Note: <b>typedef float MRow[3];</b> */
   const MRow* GetAddr() const { return (MRow *)(m); }

   // Constructors
   /*! \remarks Constructor. Note that no initialization is done. Use
   <b>Zero()</b> or <b>Identity()</b>, or the constructors below. */
   Matrix3(){ flags = 0; }  // NO INITIALIZATION done in this constructor!!             
   /*! \remarks Constructor. If TRUE is passed to the method the matrix is set
   to the identity.
   \par Parameters:
   <b>BOOL init</b>\n\n
   Specifies if the Matrix3 should be initialized to the identity. */
   Matrix3(BOOL /*init*/) {flags=0; IdentityMatrix();}
	/*! \remarks Constructor. The matrix is initialized to <b>fp</b>.
	\par Parameters:
	<b>float (*fp)[3]</b>\n\n
	Specifies the initial values for the matrix. */
   GEOMEXPORT Matrix3(float (*fp)[3]);
    /*! \remarks Constructor. Initializes the matrix with the row data passed and validates
    the matrix flags.
    \par Parameters:
    <b>const Point3\& U</b>\n\n
    The data for row 0.\n\n
    <b>const Point3\& V</b>\n\n
    The data for row 1.\n\n
    <b>const Point3\& N</b>\n\n
    The data for row 2.\n\n
    <b>const Point3\& T</b>\n\n
    The data for row 3. */
    Matrix3(const Point3& U, const Point3& V, const Point3& N, const Point3& T) {
        flags = 0; SetRow(0, U); SetRow(1, V); SetRow(2, N); SetRow(3, T); 
        ValidateFlags(); }

    /*! \remarks Initializes the matrix with the row data passed and validates the matrix
    flags.
    \par Parameters:
    <b>const Point3\& U</b>\n\n
    The data for row 0.\n\n
    <b>const Point3\& V</b>\n\n
    The data for row 1.\n\n
    <b>const Point3\& N</b>\n\n
    The data for row 2.\n\n
    <b>const Point3\& T</b>\n\n
    The data for row 3.
    \return  A reference to this matrix. */
    Matrix3& Set(const Point3& U, const Point3& V, const Point3& N, const Point3& T) {
        flags = 0; SetRow(0, U); SetRow(1, V); SetRow(2, N); SetRow(3, T);
        ValidateFlags(); return *this; }

    // Data member
    static const Matrix3 Identity;

    // Comparison operators
    /*! \remarks Compares the elements of this matrix and the one specified element by
    element for exact equality. Returns nonzero if they are equal; otherwise
    zero.
    \par Parameters:
    <b>const Matrix3\& M</b>\n\n
    The matrix to compare against. */
    GEOMEXPORT int operator==(const Matrix3& M) const;
	 /*! \remarks	 Compares the elements of this matrix and the one specified element by element
	 for equality within the specified tolerance <b>epsilon</b>. Returns nonzero if
	 they are 'equal'; otherwise zero.
	 \par Parameters:
	 <b>const Matrix3\& M</b>\n\n
	 The matrix to compare against.\n\n
	 <b>float epsilon = 1E-6f</b>\n\n
	 The tolerance for comparison. If the values in the matrix are within this value
	 (<b>+ epsilon</b> or <b>- epsilon</b>) they are considered equal. */
    GEOMEXPORT int Equals(const Matrix3& M, float epsilon = 1E-6f) const;

   // Assignment operators
   /*! \remarks Subtracts a <b>Matrix3</b> from this <b>Matrix3</b>. */
   GEOMEXPORT Matrix3& operator-=( const Matrix3& M);
   /*! \remarks Adds a <b>Matrix3</b> to this <b>Matrix3</b>. */
   GEOMEXPORT Matrix3& operator+=( const Matrix3& M); 
   /*! \remarks Multiplies this Matrix3 by the specified Matrix3 (<b>*this =
   (*this)*M;</b>). */
   GEOMEXPORT Matrix3& operator*=( const Matrix3& M);     // Matrix multiplication
   GEOMEXPORT Matrix3& operator*=( float a);

   // Operations on matrix
   /*! \remarks Set this matrix to the Identity Matrix. */
   GEOMEXPORT void IdentityMatrix();       // Make into the Identity Matrix
   /*! \remarks This method sets all elements of the matrix to 0.0f */
   GEOMEXPORT void Zero();     // set all elements to 0
   
   /*! \remarks Returns the specified row of this matrix.
   \par Parameters:
   <b>int i</b>\n\n
   Specifies the row to retrieve. */
   Point3 GetRow(int i) const { return (*this)[i]; }  
   /*! \remarks Sets the specified row of this matrix to the specified values.
   \par Parameters:
   <b>int i</b>\n\n
   Specifies the row to set.\n\n
   <b>Point3 p</b>\n\n
   The values to set. */
   GEOMEXPORT void SetRow(int i, Point3 p);

   /*! \remarks Returns the 'i-th' column of this matrix.
   \par Parameters:
   <b>int i</b>\n\n
   Specifies the column to get (0-2). */
   GEOMEXPORT Point4 GetColumn(int i) const;
   /*! \remarks Sets the 'i-th' column of this matrix to the specified values.
   \par Parameters:
   <b>int i</b>\n\n
   Specifies the column to set (0-2).\n\n
   <b>Point4 col</b>\n\n
   The values to set. */
   GEOMEXPORT void SetColumn(int i, Point4 col);
   /*! \remarks Returns the upper three entries in the specified column.
   \par Parameters:
   <b>int i</b>\n\n
   Specifies the partial column to get (0-2). */
   GEOMEXPORT Point3 GetColumn3(int i) const;

   // zero the translation part;
   /*! \remarks This method zeros the translation portion of this matrix. */
   GEOMEXPORT void NoTrans();
   // null out the rotation part;
   /*! \remarks This method zeros the rotation portion of this matrix. */
   GEOMEXPORT void NoRot();
   // null out the scale part;
   /*! \remarks The method zeros the scale portion of this matrix without
   orthogonalization. If the matrix was sheared (skewed) then the method is not
   able to remove scale component completely. Use Orthogonalize() method first,
   and then NoScale() to remove scale component entirely. Read SCL_IDENT flag
   to check if NoScale() method was enough to make the matrix to be orthogonal
   (with perpendicular axes of unit length).Prototype:\n\n
   <b>void Orthogonalize();</b> \remarks This is an "unbiased"
   orthogonalization of this matrix. The algorithm seems to take a maximum of 4
   iterations to converge. An orthogonal matrix has an axis system where each
   axis is 90 degrees from the others (it's not skewed). */
   GEOMEXPORT void NoScale();
   
   // This is an "unbiased" orthogonalization
   // It seems to take a maximum of 4 iterations to converge.
   GEOMEXPORT void Orthogonalize();

   // Access the translation row
   /*! \remarks Sets the translation row of this matrix to the specified
   values. The <b>POS_IDENT</b> flag is cleared.
   \par Parameters:
   <b>const Point3 p</b>\n\n
   Specifies the values for the translation row. */
   void SetTrans(const Point3 p) { (*this)[3] = p;  flags &= ~POS_IDENT; }
   /*! \remarks Sets the specified component of the translation row of this
   matrix to the specified value. The <b>POS_IDENT</b> flag is cleared.
   \par Parameters:
   <b>int i</b>\n\n
   Specifies the component of the translation row of this matrix to set.\n\n
   <b>float v</b>\n\n
   The value to set. */
   void SetTrans(int i, float v) { (*this)[3][i] = v; flags &= ~POS_IDENT; }
	/*! \remarks Returns the translation row of this matrix.
	\return  The translation row of this matrix. */
   const Point3& GetTrans() const { return (*this)[3]; }
   
   // Apply Incremental transformations to this matrix
   // Equivalent to multiplying on the RIGHT by transform
   /*! \remarks Apply an incremental translation transformation to this
   matrix. This is equivalent to multiplying on the RIGHT by the transform.
   \par Parameters:
   <b>const Point3\& p</b>\n\n
   Specifies the translation. */
   GEOMEXPORT void Translate(const Point3& p);
   /*! \remarks Apply an incremental X rotation transformation to this matrix.
   This is equivalent to multiplying on the RIGHT by the transform.
   \par Parameters:
   <b>float angle</b>\n\n
   Specifies the X rotation in radians. */
   GEOMEXPORT void RotateX(float angle);  
   /*! \remarks Apply an incremental Y rotation transformation to this matrix.
   This is equivalent to multiplying on the RIGHT by the transform.
   \par Parameters:
   <b>float angle</b>\n\n
   Specifies the Y rotation in radians. */
   GEOMEXPORT void RotateY(float angle);
   /*! \remarks Apply an incremental Z rotation transformation to this matrix.
   This is equivalent to multiplying on the RIGHT by the transform.
   \par Parameters:
   <b>float angle</b>\n\n
   Specifies the Z rotation in radians. */
   GEOMEXPORT void RotateZ(float angle);
   // if trans = FALSE the translation component is unaffected:
   /*! \remarks Apply an incremental scaling transformation to this matrix.
   This is equivalent to multiplying on the RIGHT by the transform.
   \par Parameters:
   <b>const Point3\& s</b>\n\n
   The scale values.\n\n
   <b>BOOL trans = FALSE</b>\n\n
   If set to <b>TRUE</b>, the translation component is scaled. If <b>trans</b>
   = <b>FALSE</b> the translation component is unaffected. When 3ds Max was
   originally written there was a bug in the code for this method where the
   translation portion of the matrix was not being scaled. This meant that when
   a matrix was scaled the bottom row was not scaled. Thus it would always
   scale about the local origin of the object, but it would scale the world
   axes. When this bug was discovered, dependencies existed in the code upon
   this bug. Thus it could not simply be fixed because it would break the
   existing code that depended upon it working the incorrect way. To correct
   this the <b>trans</b> parameter was added. If this is set to <b>TRUE</b>,
   the translation component will be scaled correctly. The existing plug-ins
   don't use this parameter, it defaults to <b>FALSE</b>, and the code behaves
   the old way. */
   GEOMEXPORT void Scale(const Point3& s, BOOL trans = FALSE);

   // Apply Incremental transformations to this matrix
   // Equivalent to multiplying on the LEFT by transform 
   /*! \remarks Apply an incremental translation transformation to this
   matrix. This is equivalent to multiplying on the LEFT by the transform.
   \par Parameters:
   <b>const Point3\& p</b>\n\n
   Specifies the translation distance. */
   GEOMEXPORT void PreTranslate(const Point3& p);
   /*! \remarks Apply an incremental X rotation transformation to this matrix.
   This is equivalent to multiplying on the LEFT by the transform.
   \par Parameters:
   <b>float angle</b>\n\n
   Specifies the X rotation in radians. */
   GEOMEXPORT void PreRotateX(float angle);  
   /*! \remarks Apply an incremental Y rotation transformation to this matrix.
   This is equivalent to multiplying on the LEFT by the transform.
   \par Parameters:
   <b>float angle</b>\n\n
   Specifies the Y rotation in radians. */
   GEOMEXPORT void PreRotateY(float angle);
   /*! \remarks Apply an incremental Z rotation transformation to this matrix.
   This is equivalent to multiplying on the LEFT by the transform.
   \par Parameters:
   <b>float angle</b>\n\n
   Specifies the Z rotation in radians. */
   GEOMEXPORT void PreRotateZ(float angle);
   // if trans = FALSE the translation component is unaffected:
   /*! \remarks Apply an incremental scaling transformation to this matrix.
   This is equivalent to multiplying on the LEFT by the transform.
   \par Parameters:
   <b>const Point3\& s</b>\n\n
   The scale values.\n\n
   <b>BOOL trans = FALSE</b>\n\n
   If trans = FALSE the translation component is unaffected. */
   GEOMEXPORT void PreScale(const Point3& s, BOOL trans = FALSE);

    // set matrix as described
    /*! \remarks Sets this matrix to the identity and the translation components to the
    specified values.
    \par Parameters:
    <b>const Point3\& p</b>\n\n
    The translation values to store. */
    GEOMEXPORT void SetTranslate(const Point3& p);     // makes translation matrix
    /*! \remarks Sets this matrix to the identity and the rotation components to the
    specified X rotation.
    \par Parameters:
    <b>float angle</b>\n\n
    The angle for X rotation (in radians). */
    GEOMEXPORT void SetRotateX(float angle);           // makes rotation matrix
    /*! \remarks Sets this matrix to the identity and the rotation components to the
    specified Y rotation.
    \par Parameters:
    <b>float angle</b>\n\n
    The angle for Y rotation (in radians). */
    GEOMEXPORT void SetRotateY(float angle);
    /*! \remarks Sets this matrix to the identity and the rotation components to the
    specified Z rotation.
    \par Parameters:
    <b>float angle</b>\n\n
    The angle for Z rotation (in radians). */
    GEOMEXPORT void SetRotateZ(float angle);
    /*! \remarks Sets the rotation components of the matrix as specified by the quaternion.
    The translation and scale components will match the identity matrix.
    \par Parameters:
    <b>const Quat\& q</b>\n\n
    Specifies the rotation to use for the matrix. */
    GEOMEXPORT void SetRotate(const Quat& q);
    /*! \remarks Sets the rotation components of the matrix as specified by the AngAxis. The
    translation and scale components will match the identity matrix.
    \par Parameters:
    <b>const AngAxis\& aa</b>\n\n
    Specifies the rotation to use for the matrix. */
    GEOMEXPORT void SetRotate(const AngAxis& aa);
    /*! \remarks Sets the rotation components of this matrix using yaw, pitch and roll
    angles. There are many different conventions for specifying a rotation by
    means of three Euler angles. This function uses the convention of rotating
    around the world Z axis, then the X axis, then the Y axis; the three
    arguments are given in the order Y, X, Z.\n\n
    This one is equivalent to:\n\n
     M.IdentityMatrix();\n\n
     M.RotateZ(roll);\n\n
     M.RotateX(pitch);\n\n
     M.RotateY(yaw);\n\n
    --Which presupposes Y is vertical, X is sideways, Z is forward
    \par Parameters:
    <b>float yaw</b>\n\n
    The yaw angle in radians.\n\n
    <b>float pitch</b>\n\n
    The pitch angle in radians.\n\n
    <b>float roll</b>\n\n
    The roll angle in radians. */
    GEOMEXPORT void SetRotate(float yaw, float pitch, float roll);
	 /*! \remarks	 Sets the rotation portion of the matrix to the rotation specified by the angle
	 and axis and sets the translation portion to zeros.
	 \par Parameters:
	 <b>const Point3\& axis</b>\n\n
	 The axis of rotation.\n\n
	 <b>float angle</b>\n\n
	 The angle of rotation about the axis in radians. */
    GEOMEXPORT void SetAngleAxis(const Point3& axis, float angle);
    /*! \remarks Sets the scale components of this matrix to the specified values. The other
    components to this matrix will match the identity.
    \par Parameters:
    <b>const Point3\& s</b>\n\n
    The scale factors for the matrix. */
    GEOMEXPORT void SetScale(const Point3& s);          // makes scale matrix
    /*! \remarks This creates a matrix describing a viewpoint which is at the 'from'
    location, looking toward the 'to' location; the viewpoint is tilted so that
    the 'up' vector points to the top of the view.
    \par Parameters:
    <b>const Point3\& from</b>\n\n
    This specifies the viewpoint source location.\n\n
    <b>const Point3\& to</b>\n\n
    This vector specifies the direction of view.\n\n
    <b>const Point3\& up</b>\n\n
    This vector points to the top of the view. */
    GEOMEXPORT void SetFromToUp(const Point3& from, const Point3& to, const Point3& up);
    /*! \remarks This method inverts this matrix. An inverted matrix, when multiplied by the
    original, yields the identity. */
    GEOMEXPORT void Invert();                          // in place
      
    //binary operators
   /*! \remarks Perform matrix multiplication. */
   GEOMEXPORT Matrix3 operator*(const Matrix3&) const;
   /*! \remarks Perform matrix addition. */
   GEOMEXPORT Matrix3 operator+(const Matrix3&) const;
   /*! \remarks Perform matrix subtraction. */
   GEOMEXPORT Matrix3 operator-(const Matrix3&) const;

    /*! \remarks Returns the specified point transformed by this matrix.
    \par Parameters:
    <b>const Point3\& p</b>\n\n
    The point to transform by this matrix. */
    GEOMEXPORT Point3 PointTransform(const Point3& p) const;
    /*! \remarks Returns the specified vector transformed by this matrix.
    \par Parameters:
    <b>const Point3\& p</b>\n\n
    The vector to transform by this matrix. */
    GEOMEXPORT Point3 VectorTransform(const Point3& p) const;
	 /*! \remarks	 Transforms the specified list of points with this matrix.
	 \par Parameters:
	 <b>Point3 *array</b>\n\n
	 The array of points to transform with this matrix.\n\n
	 <b>int n</b>\n\n
	 The number of points in the array.\n\n
	 <b>int stride = sizeof(Point3)</b>\n\n
	 The size of the increment used when moving to the next point. If you wish to
	 transform an array of data objects which contain x, y, and z coordinates in
	 order (such as a <b>Point4</b>, or a structure containing a <b>Point3</b> as a
	 member) you can specify a 'stride' value (for instance
	 <b>sizeof(data_object)</b>). */
    GEOMEXPORT void TransformPoints(Point3 *array, int n,
                int stride = sizeof(Point3));
	 /*! \remarks	 Transforms the specified list of points with this matrix and stores the
	 resulting transformed points in the storage passed.
	 \par Parameters:
	 <b>const Point3 *array</b>\n\n
	 The array of points to transform (the source).\n\n
	 <b>Point3 *to</b>\n\n
	 The array to store the transformed points (the destination).\n\n
	 <b>int n</b>\n\n
	 The number of points in the source array.\n\n
	 <b>int stride = sizeof(Point3)</b>\n\n
	 <b>int strideTo = sizeof(Point3)</b>\n\n
	 The size increment used when moving to the next storage location. */
    GEOMEXPORT void TransformPoints(const Point3 *array, Point3 *to, int n,
                int stride = sizeof(Point3), int strideTo = sizeof(Point3));
	 /*! \remarks	 Transforms the specified list of vectors with this matrix.
	 \par Parameters:
	 <b>Point3 *array</b>\n\n
	 The array of vectors to transform with this matrix.\n\n
	 <b>int n</b>\n\n
	 The number of vectors in the array.\n\n
	 <b>int stride = sizeof(Point3)</b>\n\n
	 The size of the increment used when moving to the next vector. If you wish to
	 transform an array of data objects which contain x, y, and z coordinates in
	 order (such as a <b>Point4</b>, or a structure containing a <b>Point3</b> as a
	 member) you can specify a 'stride' value (for instance
	 <b>sizeof(data_object)</b>). */
    GEOMEXPORT void TransformVectors(Point3 *array, int n,
                int stride = sizeof(Point3));
	 /*! \remarks	 Transforms the specified list of vectors with this matrix and stores the
	 resulting transformed vectors in the storage passed.
	 \par Parameters:
	 <b>const Point3 *array</b>\n\n
	 The array of vectors to transform (the source).\n\n
	 <b>Point3 *to</b>\n\n
	 The array to store the transformed vectors (the destination).\n\n
	 <b>int n</b>\n\n
	 The number of vectors in the source array.\n\n
	 <b>int stride = sizeof(Point3)</b>\n\n
	 The size increment used when moving to the next source location.\n\n
	 <b>int strideTo = sizeof(Point3)</b>\n\n
	 The size increment used when moving to the next storage location. */
    GEOMEXPORT void TransformVectors(const Point3 *array, Point3 *to, int n,
                int stride = sizeof(Point3), int strideTo = sizeof(Point3));

    // Property function
    /*! \remarks Retrieves the yaw, pitch and roll angles represented by the rotation in
    this matrix.
    \par Parameters:
    <b>float *yaw</b>\n\n
    The yaw rotation angle is stored here (in radians).\n\n
    <b>float *pitch</b>\n\n
    The pitch rotation angle is stored here (in radians).\n\n
    <b>float *roll</b>\n\n
    The roll rotation angle is stored here (in radians).
    \par Operators:
    */
    GEOMEXPORT void GetYawPitchRoll(float *yaw, float *pitch, float *roll);

    // I/O
   GEOMEXPORT IOResult Save(ISave* isave);
   GEOMEXPORT IOResult Load(ILoad* iload);

   /*! \remarks Returns the 'parity' of the matrix. Scaling one axis of the matrix 
	 negatively switches the 'parity'. However if you scale two axis the parity will flip back. 
	 Scaling three axis switches the parity again.\n\n
   When rendering a mesh, if you scale something along one axis, it turns
   'inside out'. That is the direction when the normals are reversed. This
   method may be used to detect that case and then reverse the normals. The 3ds
   Max renderer does this -- if this method returns TRUE it flips all the
   normals so it won't turn inside out.
	 */
   GEOMEXPORT BOOL Parity() const;
   };

// Build new matrices for transformations
/*! \remarks Builds a new matrix for use as a X rotation transformation.
\par Parameters:
<b>float angle</b>\n\n
Specifies the angle of rotation in radians.
\return  A new X rotation <b>Matrix3</b>. */
GEOMEXPORT Matrix3 RotateXMatrix(float angle);   
/*! \remarks Builds a new matrix for use as a Y rotation transformation.
\par Parameters:
<b>float angle</b>\n\n
Specifies the angle of rotation in radians.
\return  A new Y rotation <b>Matrix3</b>. */
GEOMEXPORT Matrix3 RotateYMatrix(float angle); 
/*! \remarks Builds a new matrix for use as a Z rotation transformation.
\par Parameters:
<b>float angle</b>\n\n
Specifies the angle of rotation in radians.
\return  A new Z rotation <b>Matrix3</b>. */
GEOMEXPORT Matrix3 RotateZMatrix(float angle); 
/*! \remarks Builds a new matrix for use as a translation transformation.
\par Parameters:
<b>const Point3\& p</b>\n\n
Specifies the translation values.
\return  A new translation <b>Matrix3</b>. */
GEOMEXPORT Matrix3 TransMatrix(const Point3& p);
/*! \remarks Builds a new matrix for use as a scale transformation.
\par Parameters:
<b>const Point3\& p</b>\n\n
Specifies the scale values.
\return  A new scale <b>Matrix3</b>. */
GEOMEXPORT Matrix3 ScaleMatrix(const Point3& s);
/*! \remarks Builds a new matrix for use as a rotation transformation by
specifying yaw, pitch and roll angles.\n\n
This definition will depend on what our coordinate system is. This one is
equivalent to:\n\n
<b>M.IdentityMatrix();</b>\n\n
<b>M.RotateZ(roll);</b>\n\n
<b>M.RotateX(pitch);</b>\n\n
<b>M.RotateY(yaw);</b>\n\n
Which presupposes Y is vertical, X is sideways, Z is forward
\par Parameters:
<b>float Yaw</b>\n\n
Specifies the yaw angle in radians.\n\n
<b>float Pitch</b>\n\n
Specifies the pitch angle in radians.\n\n
<b>float Roll</b>\n\n
Specifies the roll angle in radians.
\return  A new rotation <b>Matrix3</b>. */
GEOMEXPORT Matrix3 RotateYPRMatrix(float Yaw, float Pitch, float Roll);
/*! \remarks Builds a new matrix for use as a rotation transformation by
specifying an angle and axis.
\par Parameters:
<b>Point3\& axis</b>\n\n
Specifies the axis of rotation. Note that this angle is expected to be
normalized.\n\n
<b>float angle</b>\n\n
Specifies the angle of rotation. Note: The direction of the angle in this
method is opposite of that in <b>AngAxisFromQ()</b>.
\return  A new rotation Matrix3. */
GEOMEXPORT Matrix3 RotAngleAxisMatrix(Point3& axis, float angle);
 
/*! \remarks Return the inverse of the matrix
\par Parameters:
<b>const Matrix3\& M</b>\n\n
The matrix to compute the inverse of. */
GEOMEXPORT Matrix3 Inverse(const Matrix3& M);  // return Inverse of matrix

// These two versions of transforming a point with a matrix do the same thing,
// regardless of the order of operands (linear algebra rules notwithstanding).
/*! \remarks These transform a <b>Point3</b> with a <b>Matrix3</b>. These two
versions of transforming a point with a matrix do the same thing, regardless of
the order of operands (linear algebra rules notwithstanding).
\par Parameters:
<b>const Matrix3\& A</b>\n\n
The matrix to transform the point with.
<b>const Point3\& V</b>\n\n
The point to transform.\n\n
\return  The transformed <b>Point3</b>. */
GEOMEXPORT Point3 operator*(const Matrix3& A, const Point3& V); // Transform Point with matrix
/*! \remarks These transform a <b>Point3</b> with a <b>Matrix3</b>. These two
versions of transforming a point with a matrix do the same thing, regardless of
the order of operands (linear algebra rules notwithstanding).
\par Parameters:
<b>const Point3\& V</b>\n\n
The point to transform.\n\n
<b>const Matrix3\& A</b>\n\n
The matrix to transform the point with.
\return  The transformed <b>Point3</b>. */
GEOMEXPORT Point3 operator*(const Point3& V, const Matrix3& A); // Transform Point with matrix

// ditto
/*! \remarks Transform the vector (Point3) with the specified matrix.
\par Parameters:
<b>const Matrix3\& A</b>\n\n
The matrix to transform the vector with.\n\n
<b>const Point3\& V</b>\n\n
The vector to transform.
\return  The transformed vector (as a Point3). */
GEOMEXPORT Point3 VectorTransform(const Matrix3& M, const Point3& V); 
GEOMEXPORT Point3 VectorTransform(const Point3& V, const Matrix3& M); 

// transform a plane: this only works if M is orthogonal
GEOMEXPORT Point4 TransformPlane(const Matrix3& M, const Point4& plin); 

// transformats matrix m so it is applied in the space of matrix xm:
//  returns xm*m*Inverse(xm)
/*! \remarks This method is used to build a matrix that constructs a
transformation in a particular space. For example, say you have a rotation you
want to apply, but you want to perform the rotation in another coordinate
system. To do this, you typically transform into the space of the coordinate
system, then apply the transformation, and then transform out of that
coordinate system. This method constructs a matrix that does just this. It
transformats matrix <b>m</b> so it is applied in the space of matrix <b>xm</b>.
It returns a <b>Matrix3</b> that is <b>xm*m*Inverse(xm)</b>.
\par Parameters:
<b>const Matrix3\& xm</b>\n\n
Specifies the coordinate system you want to work in.\n\n
<b>const Matrix3\& m</b>\n\n
Specifies the transformation matrix.
\return  Returns a <b>Matrix3</b> that is <b>xm*m*Inverse(xm)</b>. */
GEOMEXPORT Matrix3 XFormMat(const Matrix3& xm, const Matrix3& m);

