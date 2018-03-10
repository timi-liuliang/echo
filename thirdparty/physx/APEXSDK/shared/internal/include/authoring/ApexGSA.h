/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#ifndef APEX_GSA_H
#define APEX_GSA_H


#include "PsShare.h"
#include "NxRenderMeshAsset.h"
#include "ApexCSGMath.h"

#ifndef WITHOUT_APEX_AUTHORING

namespace ApexCSG
{
namespace GSA
{

/*** Compact implementation of the void simplex algorithm for D = 3 ***/

typedef Real real;

/*
	VS3D_Halfspace_Set - abstract representation of a set of half-spaces

	The implementation of farthest_halfspace should return the half-space "most below" the given point.  That is, the
	half-space from the set to which the point has the least signed distance.  The plane returned is the boundary of the
	half-space found, and represented as a vector in projective coordinates (the coefficients of the plane equation).

	plane = the returned half-space boundary
	point = the input point
	returns the signed distance from the point to the plane
*/
struct VS3D_Halfspace_Set
{
	virtual	real	farthest_halfspace(real plane[4], const real point[3]) = 0;
};


// Simple types and operations for internal calculations
struct Vec3 { real x, y, z; };	// 3-vector 
struct Plane3 { Vec3 n; real d; };	// Representation of a plane in 3 dimensions; points p in the plane satisfy (p|n) + d = 0

inline Vec3	vec3(real x, real y, real z)				{ Vec3 r; r.x = x; r.y = y; r.z = z; return r; }	// vector builder
inline Vec3	operator + (const Vec3& a, const Vec3& b)	{ return vec3(a.x+b.x, a.y+b.y, a.z+b.z); }			// vector addition
inline Vec3	operator * (real s, const Vec3& v)			{ return vec3(s*v.x, s*v.y, s*v.z); }				// scalar multiplication
inline real	operator | (const Vec3& a, const Vec3& b)	{ return a.x*b.x + a.y*b.y + a.z*b.z; }				// dot product
inline Vec3 operator ^ (const Vec3& a, const Vec3& b)	{ return vec3(a.y*b.z - b.y*a.z, a.z*b.x - b.z*a.x, a.x*b.y - b.x*a.y); }	// cross product

// More accurate perpendicular
inline Vec3	perp(const Vec3& a, const Vec3& b)
{
	const Vec3 c = a^b;	// Cross-product gives perpendicular
	const real c2 = c|c;
	return c2 != 0 ? c + ((real)1/c2)*((a|c)*(c^b) + (b|c)*(a^c)) : c;	// Improvement to (a b)^T(c) = (0)
}

// Returns index of the extremal element in a three-element set {e0, e1, e2} based upon comparisons c_ij. The extremal index m is such that c_mn is true, or e_m == e_n, for all n.
inline int	ext_index(int c_10, int c_21, int c_20)	{ return c_10<<c_21|(c_21&c_20)<<1; }

// Returns index (0, 1, or 2) of minimum argument
inline int	index_of_min(real x0, real x1, real x2)	{ return ext_index((int)(x1 < x0), (int)(x2 < x1), (int)(x2 < x0)); }

// Compare fractions with positive deominators.  Returns a_num*sqrt(a_rden2) > b_num*sqrt(b_rden2)
inline bool frac_gt(real a_num, real a_rden2, real b_num, real b_rden2)
{
	const bool a_num_neg = a_num < 0;
	const bool b_num_neg = b_num < 0;
	return a_num_neg != b_num_neg ? b_num_neg : ((a_num*a_num*a_rden2 > b_num*b_num*b_rden2) != a_num_neg);
}

// Returns index (0, 1, or 2) of maximum fraction with positive deominators
inline int	index_of_max_frac(real x0_num, real x0_rden2, real x1_num, real x1_rden2, real x2_num, real x2_rden2)
{
	return ext_index((int)frac_gt(x1_num, x1_rden2, x0_num, x0_rden2), (int)frac_gt(x2_num, x2_rden2, x1_num, x1_rden2), (int)frac_gt(x2_num, x2_rden2, x0_num, x0_rden2));
}


// Update function for vs3d_test
static bool vs3d_update(Vec3& p, Plane3 S[4], int& plane_count, real eps2)
{
	// h plane is the last plane
	const Plane3& h = S[plane_count-1];

	// Initialize p by projecting the origin onto the h plane
	p = -h.d*h.n;

	if (plane_count == 1) return true;	// Handle plane_count == 1 specially (optimization; this could be commented out)

	// Start with origin in the h plane (homogeneous coordinates)
	Vec3 r = {0, 0, 1};

	// Create basis in the h plane
	const int min_i = index_of_min(h.n.x*h.n.x, h.n.y*h.n.y, h.n.z*h.n.z);
	const Vec3 y = h.n^vec3((real)(min_i == 0), (real)(min_i == 1), (real)(min_i == 2));
	const Vec3 x = y^h.n;

	// Create plane equations in the h plane.  These will not be normalized in general.
	int N = 0;			// Plane count in h subspace
	Vec3 R[3];			// Planes in h subspace
	real recip_n2[3];	// Plane normal vector reciprocal lengths squared
	int index[3];		// Keep track of original plane indices
	for (int i = 0; i < plane_count-1; ++i)
	{
		const Vec3& vi = S[i].n;
		const real cos_theta = h.n|vi;
		R[N] = vec3(x|vi, y|vi, S[i].d - h.d*cos_theta);
		index[N] = i;
		const real n2 = R[N].x*R[N].x + R[N].y*R[N].y;
		if (n2 >= eps2)
		{
			const real lin_norm = (real)1.5-(real)0.5*n2;	// 1st-order approximation to 1/sqrt(n2) expanded about n2 = 1
			R[N] = lin_norm*R[N];	// We don't need normalized plane equations, but rescaling (even with an approximate normalization) gives better numerical behavior
			recip_n2[N] = (real)1/(lin_norm*lin_norm*n2);
			++N;	// Keep this plane
		}
		else if (cos_theta < 0) return false;	// Parallel cases are redundant and rejected, anti-parallel cases are 1D voids
	}

	// Now work with the N-sized R array of half-spaces in the h plane
	int max_d_index;	// Used within switch statement
	switch (N)
	{
	case 1:
		if (R[0].z < 0) N = 0;	// S[0] is redundant, eliminate it
		else r = r + (-R[0].z*recip_n2[0])*vec3(R[0].x, R[0].y, 0);
		break;
	case 2: two_planes:
		max_d_index = (int)frac_gt(R[1].z, recip_n2[1], R[0].z, recip_n2[0]);
		if (R[max_d_index].z < 0) N = 0;	// S[0] and S[1] are redundant, eliminate them
		else
		{
			r = r + (-R[max_d_index].z*recip_n2[max_d_index])*vec3(R[max_d_index].x, R[max_d_index].y, 0);
			if ((r|R[max_d_index^1]) < 0)
			{
				index[0] = index[max_d_index];
				N = 1;	// S[max_d_index^1] is redundant, eliminate it
			}
			else
			{
				// Find intersection of R[0] and R[1]
				r = perp(R[0], R[1]);
				if (r.z*r.z*recip_n2[0]*recip_n2[1] < eps2) return false;	// 2D void found
				// Set r to the intersection of R[0] and R[1] and keep both
				r = ((real)1/r.z)*r;
			}
		}
		break;
	case 3:
		max_d_index = index_of_max_frac(R[0].z, recip_n2[0], R[1].z, recip_n2[1], R[2].z, recip_n2[2]);
		if (R[max_d_index].z < 0) N = 0;	// S[0], S[1], and S[2] are redundant, eliminate them
		else
		{
			const Vec3 row_x = {R[0].x, R[1].x, R[2].x};
			const Vec3 row_y = {R[0].y, R[1].y, R[2].y};
			const Vec3 row_w = {R[0].z, R[1].z, R[2].z};
			const Vec3 cof_w = perp(row_x, row_y);
			const bool detR_pos = (row_w|cof_w) > 0;
			const int nrw_sgn0 = cof_w.x*cof_w.x*recip_n2[1]*recip_n2[2] < eps2 ? 0 : (((int)((cof_w.x > 0) == detR_pos)<<1)-1);
			const int nrw_sgn1 = cof_w.y*cof_w.y*recip_n2[2]*recip_n2[0] < eps2 ? 0 : (((int)((cof_w.y > 0) == detR_pos)<<1)-1);
			const int nrw_sgn2 = cof_w.z*cof_w.z*recip_n2[0]*recip_n2[1] < eps2 ? 0 : (((int)((cof_w.z > 0) == detR_pos)<<1)-1);

			if ((nrw_sgn0|nrw_sgn1|nrw_sgn2) >= 0) return false;	// 3D void found

			const int positive_width_count = ((nrw_sgn0>>1)&1) + ((nrw_sgn1>>1)&1) + ((nrw_sgn2>>1)&1);
			if (positive_width_count == 1)
			{
				// A single positive width results from a redundant plane.  Eliminate it and peform N = 2 calculation.
				const int pos_width_index = ((nrw_sgn1 >> 1) & 1) | (nrw_sgn2 & 2);	// Calculates which index corresponds to a positive-width side
				R[pos_width_index] = R[2];
				recip_n2[pos_width_index] = recip_n2[2];
				index[pos_width_index] = index[2];
				N = 2;
				goto two_planes;
			}

			// Project origin onto max-d plane
			r = r + (-R[max_d_index].z*recip_n2[max_d_index])*vec3(R[max_d_index].x, R[max_d_index].y, 0);
			N = 1;	// Unless we use a vertex in the loop below
			const int index_max = index[max_d_index];

			// The number of finite widths should be >= 2.  If not, it should be 0, but in any case it implies three parallel lines in the plane, which we should not have here.
			// If we do have three parallel lines (# of finite widths < 2), we've picked the line corresponding to the half-plane farthest from the origin, which is correct.
			const int finite_width_count = (nrw_sgn0&1) + (nrw_sgn1&1) + (nrw_sgn2&1);
			if (finite_width_count >= 2)
			{
				const int i_remaining[2] = {(1<<max_d_index)&3, (3>>max_d_index)^1};	// = {(max_d_index+1)%3, (max_d_index+2)%3}
				const int i_select = (int)frac_gt(R[i_remaining[1]].z, recip_n2[i_remaining[1]], R[i_remaining[0]].z, recip_n2[i_remaining[0]]);	// Select the greater of the remaining two indices
				for (int i = 0; i < 2; ++i)
				{
					const int j = i_remaining[i_select^i];	// i = 0 => the next-greatest, i = 1 => the least
					if ((r|R[j]) >= 0)
					{
						r = perp(R[max_d_index], R[j]);
						r = ((real)1/r.z)*r;
						index[1] = index[j];
						N = 2;
						break;
					}
				}
			}

			index[0] = index_max;
		}
		break;
	}

	// Transform r back to 3D space
	p = p + r.x*x + r.y*y; 

	// Pack S array with kept planes
	Plane3 new_S[2];
	for (int i = 0; i < N; ++i) new_S[i] = S[index[i]];
	for (int i = 0; i < N; ++i) S[i] = new_S[i];
	S[N] = h;
	plane_count = N+1;

	return true;
}


// Performs the VS algorithm for D = 3
inline int vs3d_test(VS3D_Halfspace_Set& halfspace_set)
{
	// Tolerance for 3D void simplex algorithm
	const real eps = (real)8/(sizeof(real) == 4 ? (1L<<23) : (1LL<<52));
	const real eps2 = eps*eps;	// Commonly use epsilon squared

	// Maximum allowed iterations of main loop.  If exceeded, error code is returned
	const int max_iteration_count = 50;

	Plane3 S[4];			// Up to 4 planes
	int plane_count = 0;	// Number of valid planes
	Vec3 p = {0, 0, 0};		// Test point, initialized to origin

	// Track maximum distance from origin, which ideally only increases
	real p2 = 0;
	real p2_max = 0;

	int result = -1;	// Default result, changed to valid result if found in loop below

	for (int i = 0; i < max_iteration_count; ++i)	// Iterate until a stopping condition is met or the maximum number of iterations is reached
	{
		Plane3& plane = S[plane_count++];
		const real delta = halfspace_set.farthest_halfspace(&plane.n.x, &p.x);
		if (delta <= 0 || delta*delta <= eps2*(p2+1))
		{
			result = 1;	// Intersection found
			break;
		}
		if (!vs3d_update(p, S, plane_count, eps2))
		{
			result = 0;	// Void simplex found
			break;
		}
		p2 = p|p;
		if (p2*(1+2*eps) < p2_max)
		{
			result = 1;	// Monotonic increase violated
			break;
		}
		p2_max = p2;
	}

	PX_ASSERT(result >= 0);

	return result;
}


/*
	Utility class derived from GSA::ConvexShape, to handle common implementations

	PlaneIterator must have:
		1) a constructor which takes an object of type IteratorInitValues (either by value or refrence) in its constructor,
		2) a valid() method which returns a bool (true iff the plane() function can return a valid plane, see below),
		3) an inc() method to advance to the next plane, and
		4) a plane() method which returns a plane of type ApexCSG::Plane, either by value or reference (the plane will be copied).
*/
template<class PlaneIterator, class IteratorInitValues>
class StaticConvexPolyhedron : public VS3D_Halfspace_Set
{
public:
	virtual	Real farthest_halfspace(Real plane[4], const Real point[3])
	{
		Plane& halfspace = *(Plane*)plane;
		halfspace = Plane(Dir((Real)0), -(Real)1);
		Real greatest_s = -MAX_REAL;

		for (PlaneIterator it(m_initValues); it.valid(); it.inc())
		{
			const Plane test = it.plane();
			const Real s = point[0]*test[0] + point[1]*test[1] + point[2]*test[2] + test[3];
			if (s > greatest_s)
			{
				greatest_s = s;
				halfspace = test;
			}
		}

		// Return results
		return greatest_s;
	}

protected:
	IteratorInitValues	m_initValues;
};

};	// namespace GSA
};	// namespace ApexCSG

#endif	// #ifndef WITHOUT_APEX_AUTHORING

#endif // #ifndef APEX_GSA_H
