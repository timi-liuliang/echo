/*=============================================================================
	LMCollision.h: Collision definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

/**
 *	Line Check With Triangle
 *	Algorithm based on "Fast, Minimum Storage Ray/Triangle Intersection"
 *	Returns TRUE if the line segment does hit the triangle
 */
FORCEINLINE UBOOL LineCheckWithTriangle(
	FCheckResult& Result,
	const FVector4& V1,
	const FVector4& V2,
	const FVector4& V3,
	const FVector4& Start,
	const FVector4& End,
	const FVector4& Direction
	)
{
	FVector4	Edge1 = V3 - V1,
		Edge2 = V2 - V1,
		P = Direction ^ Edge2;
	FLOAT	Determinant = Edge1 | P;

	if(Determinant < DELTA)
	{
		return FALSE;
	}

	FVector4	T = Start - V1;
	FLOAT	U = T | P;

	if(U < 0.0f || U > Determinant)
	{
		return FALSE;
	}

	FVector4	Q = T ^ Edge1;
	FLOAT	V = Direction | Q;

	if(V < 0.0f || U + V > Determinant)
	{
		return FALSE;
	}

	FLOAT	Time = (Edge2 | Q) / Determinant;

	if(Time < 0.0f || Time > Result.Time)
	{
		return FALSE;
	}

	Result.Normal = ((V3-V2)^(V2-V1)).SafeNormal();
	Result.Time = ((V1 - Start)|Result.Normal) / (Result.Normal|Direction);

	return TRUE;
}

/**
 * Determines whether a line intersects a box.
 */
inline UBOOL LineBoxIntersection
(
	const FBox&		Box,
	const FVector4&	Start,
	const FVector4&	End,
	const FVector4&	Direction,
	const FVector4&	OneOverDirection
)
{
	FVector4	Time;
	UBOOL	bStartIsOutside = FALSE;

	if(Start.X < Box.Min.X)
	{
		bStartIsOutside = TRUE;
		if(End.X >= Box.Min.X)
		{
			Time.X = (Box.Min.X - Start.X) * OneOverDirection.X;
		}
		else
		{
			return FALSE;
		}
	}
	else if(Start.X > Box.Max.X)
	{
		bStartIsOutside = TRUE;
		if(End.X <= Box.Max.X)
		{
			Time.X = (Box.Max.X - Start.X) * OneOverDirection.X;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		Time.X = 0.0f;
	}

	if(Start.Y < Box.Min.Y)
	{
		bStartIsOutside = TRUE;
		if(End.Y >= Box.Min.Y)
		{
			Time.Y = (Box.Min.Y - Start.Y) * OneOverDirection.Y;
		}
		else
		{
			return FALSE;
		}
	}
	else if(Start.Y > Box.Max.Y)
	{
		bStartIsOutside = TRUE;
		if(End.Y <= Box.Max.Y)
		{
			Time.Y = (Box.Max.Y - Start.Y) * OneOverDirection.Y;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		Time.Y = 0.0f;
	}

	if(Start.Z < Box.Min.Z)
	{
		bStartIsOutside = TRUE;
		if(End.Z >= Box.Min.Z)
		{
			Time.Z = (Box.Min.Z - Start.Z) * OneOverDirection.Z;
		}
		else
		{
			return FALSE;
		}
	}
	else if(Start.Z > Box.Max.Z)
	{
		bStartIsOutside = TRUE;
		if(End.Z <= Box.Max.Z)
		{
			Time.Z = (Box.Max.Z - Start.Z) * OneOverDirection.Z;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		Time.Z = 0.0f;
	}

	if(bStartIsOutside)
	{
		const FLOAT	MaxTime = Max(Time.X,Max(Time.Y,Time.Z));

		if(MaxTime >= 0.0f && MaxTime <= 1.0f)
		{
			const FVector4 Hit = Start + Direction * MaxTime;
			const FLOAT BOX_SIDE_THRESHOLD = 0.1f;
			if(	Hit.X > Box.Min.X - BOX_SIDE_THRESHOLD && Hit.X < Box.Max.X + BOX_SIDE_THRESHOLD &&
				Hit.Y > Box.Min.Y - BOX_SIDE_THRESHOLD && Hit.Y < Box.Max.Y + BOX_SIDE_THRESHOLD &&
				Hit.Z > Box.Min.Z - BOX_SIDE_THRESHOLD && Hit.Z < Box.Max.Z + BOX_SIDE_THRESHOLD)
			{
				return TRUE;
			}
		}

		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

//
//	LineCheckWithBox
//

static UBOOL LineCheckWithBox
(
	const FVector4& BoxCenter,
	const FVector4& BoxRadii,
	const FVector4& Start,
	const FVector4&	Direction,
	const FVector4&	OneOverDirection
)
{
	//const FVector* boxPlanes = &Box.Min;
	
	FLOAT tf, tb;
	FLOAT tnear = 0.f;
	FLOAT tfar = 1.f;
	
	FVector4 LocalStart = Start - BoxCenter;

	// X //
	// First - see if ray is parallel to slab.
	if(Direction.X != 0.f)
	{
		// If not, find the time it hits the front and back planes of slab.
		tf = - (LocalStart.X * OneOverDirection.X) - BoxRadii.X * Abs(OneOverDirection.X);
		tb = - (LocalStart.X * OneOverDirection.X) + BoxRadii.X * Abs(OneOverDirection.X);

		if(tf > tnear)
			tnear = tf;

		if(tb < tfar)
			tfar = tb;

		if(tfar < tnear)
			return 0;
	}
	else
	{
		// If it is parallel, early return if start is outiside slab.
		if(!(Abs(LocalStart.X) <= BoxRadii.X))
		{
			return 0;
		}
	}

	// Y //
	if(Direction.Y != 0.f)
	{
		// If not, find the time it hits the front and back planes of slab.
		tf = - (LocalStart.Y * OneOverDirection.Y) - BoxRadii.Y * Abs(OneOverDirection.Y);
		tb = - (LocalStart.Y * OneOverDirection.Y) + BoxRadii.Y * Abs(OneOverDirection.Y);

		if(tf > tnear)
			tnear = tf;

		if(tb < tfar)
			tfar = tb;

		if(tfar < tnear)
			return 0;
	}
	else
	{
		if(!(Abs(LocalStart.Y) <= BoxRadii.Y))
			return 0;
	}

	// Z //
	if(Direction.Z != 0.f)
	{
		// If not, find the time it hits the front and back planes of slab.
		tf = - (LocalStart.Z * OneOverDirection.Z) - BoxRadii.Z * Abs(OneOverDirection.Z);
		tb = - (LocalStart.Z * OneOverDirection.Z) + BoxRadii.Z * Abs(OneOverDirection.Z);

		if(tf > tnear)
			tnear = tf;

		if(tb < tfar)
			tfar = tb;

		if(tfar < tnear)
			return 0;
	}
	else
	{
		if(!(Abs(LocalStart.Z) <= BoxRadii.Z))
		{
			return 0;
		}
	}

	// we hit!
	return 1;
}

static UBOOL ClipLineWithBox(const FBox& Box, const FVector4& Start, const FVector4& End, FVector4& IntersectedStart, FVector4& IntersectedEnd)
{
    IntersectedStart = Start;
    IntersectedEnd = End;

    FVector4 Dir;
    FLOAT TEdgeOfBox,TLineLength;
    UBOOL StartCulled,EndCulled;
   
    // Bound by neg X
    StartCulled = IntersectedStart.X < Box.Min.X;
    EndCulled = IntersectedEnd.X < Box.Min.X;
    if (StartCulled && EndCulled)
    {
        IntersectedStart = Start;
        IntersectedEnd = Start;
        return FALSE;
    }
    else if (StartCulled)
    {
        check(IntersectedEnd.X > IntersectedStart.X); // div by 0 should be impossible by check above

        Dir = IntersectedStart - IntersectedEnd;
        TEdgeOfBox = Box.Min.X - IntersectedEnd.X;
        TLineLength = IntersectedStart.X - IntersectedEnd.X;
        IntersectedStart = IntersectedEnd + Dir*(TEdgeOfBox/TLineLength);
    }
    else if (EndCulled)
    {
        check(IntersectedStart.X > IntersectedEnd.X); // div by 0 should be impossible by check above

        Dir = IntersectedEnd - IntersectedStart;
        TEdgeOfBox = Box.Min.X - IntersectedStart.X;
        TLineLength = IntersectedEnd.X - IntersectedStart.X;
        IntersectedEnd = IntersectedStart + Dir*(TEdgeOfBox/TLineLength);
    }

    // Bound by pos X
    StartCulled = IntersectedStart.X > Box.Max.X;
    EndCulled = IntersectedEnd.X > Box.Max.X;
    if (StartCulled && EndCulled)
    {
        IntersectedStart = Start;
        IntersectedEnd = Start;
        return FALSE;
    }
    else if (StartCulled)
    {
        check(IntersectedEnd.X < IntersectedStart.X); // div by 0 should be impossible by check above

        Dir = IntersectedStart - IntersectedEnd;
        TEdgeOfBox = Box.Max.X - IntersectedEnd.X;
        TLineLength = IntersectedStart.X - IntersectedEnd.X;
        IntersectedStart = IntersectedEnd + Dir*(TEdgeOfBox/TLineLength);
    }
    else if (EndCulled)
    {
        check(IntersectedStart.X < IntersectedEnd.X); // div by 0 should be impossible by check above

        Dir = IntersectedEnd - IntersectedStart;
        TEdgeOfBox = Box.Max.X - IntersectedStart.X;
        TLineLength = IntersectedEnd.X - IntersectedStart.X;
        IntersectedEnd = IntersectedStart + Dir*(TEdgeOfBox/TLineLength);
    }

    // Bound by neg Y
    StartCulled = IntersectedStart.Y < Box.Min.Y;
    EndCulled = IntersectedEnd.Y < Box.Min.Y;
    if (StartCulled && EndCulled)
    {
        IntersectedStart = Start;
        IntersectedEnd = Start;
        return FALSE;
    }
    else if (StartCulled)
    {
        check(IntersectedEnd.Y > IntersectedStart.Y); // div by 0 should be impossible by check above

        Dir = IntersectedStart - IntersectedEnd;
        TEdgeOfBox = Box.Min.Y - IntersectedEnd.Y;
        TLineLength = IntersectedStart.Y - IntersectedEnd.Y;
        IntersectedStart = IntersectedEnd + Dir*(TEdgeOfBox/TLineLength);
    }
    else if (EndCulled)
    {
        check(IntersectedStart.Y > IntersectedEnd.Y); // div by 0 should be impossible by check above

        Dir = IntersectedEnd - IntersectedStart;
        TEdgeOfBox = Box.Min.Y - IntersectedStart.Y;
        TLineLength = IntersectedEnd.Y - IntersectedStart.Y;
        IntersectedEnd = IntersectedStart + Dir*(TEdgeOfBox/TLineLength);
    }

    // Bound by pos Y
    StartCulled = IntersectedStart.Y > Box.Max.Y;
    EndCulled = IntersectedEnd.Y > Box.Max.Y;
    if (StartCulled && EndCulled)
    {
        IntersectedStart = Start;
        IntersectedEnd = Start;
        return FALSE;
    }
    else if (StartCulled)
    {
        check(IntersectedEnd.Y < IntersectedStart.Y); // div by 0 should be impossible by check above

        Dir = IntersectedStart - IntersectedEnd;
        TEdgeOfBox = Box.Max.Y - IntersectedEnd.Y;
        TLineLength = IntersectedStart.Y - IntersectedEnd.Y;
        IntersectedStart = IntersectedEnd + Dir*(TEdgeOfBox/TLineLength);
    }
    else if (EndCulled)
    {
        check(IntersectedStart.Y < IntersectedEnd.Y); // div by 0 should be impossible by check above

        Dir = IntersectedEnd - IntersectedStart;
        TEdgeOfBox = Box.Max.Y - IntersectedStart.Y;
        TLineLength = IntersectedEnd.Y - IntersectedStart.Y;
        IntersectedEnd = IntersectedStart + Dir*(TEdgeOfBox/TLineLength);
    }

    // Bound by neg Z
    StartCulled = IntersectedStart.Z < Box.Min.Z;
    EndCulled = IntersectedEnd.Z < Box.Min.Z;
    if (StartCulled && EndCulled)
    {
        IntersectedStart = Start;
        IntersectedEnd = Start;
        return FALSE;
    }
    else if (StartCulled)
    {
        check(IntersectedEnd.Z > IntersectedStart.Z); // div by 0 should be impossible by check above

        Dir = IntersectedStart - IntersectedEnd;
        TEdgeOfBox = Box.Min.Z - IntersectedEnd.Z;
        TLineLength = IntersectedStart.Z - IntersectedEnd.Z;
        IntersectedStart = IntersectedEnd + Dir*(TEdgeOfBox/TLineLength);
    }
    else if (EndCulled)
    {
        check(IntersectedStart.Z > IntersectedEnd.Z); // div by 0 should be impossible by check above

        Dir = IntersectedEnd - IntersectedStart;
        TEdgeOfBox = Box.Min.Z - IntersectedStart.Z;
        TLineLength = IntersectedEnd.Z - IntersectedStart.Z;
        IntersectedEnd = IntersectedStart + Dir*(TEdgeOfBox/TLineLength);
    }

    // Bound by pos Z
    StartCulled = IntersectedStart.Z > Box.Max.Z;
    EndCulled = IntersectedEnd.Z > Box.Max.Z;
    if (StartCulled && EndCulled)
    {
        IntersectedStart = Start;
        IntersectedEnd = Start;
        return FALSE;
    }
    else if (StartCulled)
    {
        check(IntersectedEnd.Z < IntersectedStart.Z); // div by 0 should be impossible by check above

        Dir = IntersectedStart - IntersectedEnd;
        TEdgeOfBox = Box.Max.Z - IntersectedEnd.Z;
        TLineLength = IntersectedStart.Z - IntersectedEnd.Z;
        IntersectedStart = IntersectedEnd + Dir*(TEdgeOfBox/TLineLength);
    }
    else if (EndCulled)
    {
        check(IntersectedStart.Z < IntersectedEnd.Z); // div by 0 should be impossible by check above

        Dir = IntersectedEnd - IntersectedStart;
        TEdgeOfBox = Box.Max.Z - IntersectedStart.Z;
        TLineLength = IntersectedEnd.Z - IntersectedStart.Z;
        IntersectedEnd = IntersectedStart + Dir*(TEdgeOfBox/TLineLength);
    }
    return TRUE;
}

} // namespace