/*=============================================================================
	3DVisualizer.h: Helper class to visualize simple 3D geometry
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

/**
 * Helper class to visualize simple 3D geometry.
 */
class F3DVisualizer
{
public:
	/** Constructor. Initializes a D3D Device. */
	F3DVisualizer();
	
	/** Destructor. Destroys the D3D Device. */
	~F3DVisualizer();

	/**
	 * Adds a triangle to be visualized.
	 * @param P1	First vertex of the triangle
	 * @param P2	Second vertex of the triangle
	 * @param P3	Third vertex of the triangle
	 * @param Color	Color of the triangle
	 */
	void	AddTriangle( const FVector4& P1, const FVector4& P2, const FVector4& P3, FColor Color );

	/**
	 * Adds a triangle to be visualized.
	 * @param P1	First vertex of the line
	 * @param P2	Second vertex of the line
	 * @param Color	Color of the line
	 */
	void	AddLine( const FVector4& P1, const FVector4& P2, FColor Color );

	/** Returns the current number of triangles. */
	INT		NumTriangles() const;

	/** Returns the current number of lines. */
	INT		NumLines() const;

	/** Removes all geometry. */
	void	Clear();

	/**
	 * Exports all geometry into a D3D .x file into the current working folder.
	 * @param Filename	Desired filename (may include path)
	 * @param bShow		Whether the D3D .x file viewer should be invoked. If shown, we'll block until it has been closed.
	 */
	void	Export( const TCHAR* Filename, UBOOL bShow=FALSE );

private:
	struct FD3DWrapper*	D3D;
	struct FTriangle
	{
		FTriangle( const FVector4& P1, const FVector4& P2, const FVector4& P3, FColor InColor )
		:	Color( InColor )
		{
			Vertices[0] = P1;
			Vertices[1] = P2;
			Vertices[2] = P3;
		}
		FVector4	Vertices[3];
		FColor		Color;
	};
	struct FLine
	{
		FLine( const FVector4& P1, const FVector4& P2, FColor InColor )
		:	Color( InColor )
		{
			Vertices[0] = P1;
			Vertices[1] = P2;
		}
		FVector4	Vertices[2];
		FColor		Color;
	};
	TArray<FTriangle>	Triangles;
	TArray<FLine>		Lines;
};

}
