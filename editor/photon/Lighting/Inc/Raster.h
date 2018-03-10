/*=============================================================================
	Raster.h: Triangle rasterization code
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/


#pragma once

namespace Lightmass
{

//
//	FTriangleRasterizer - A generic 2d triangle rasterizer which linearly interpolates vertex parameters and calls a virtual function for each pixel.
//

template<class RasterPolicyType> class FTriangleRasterizer : public RasterPolicyType
{
public:

	typedef typename RasterPolicyType::InterpolantType InterpolantType;

	void DrawTriangle(const InterpolantType& I0,const InterpolantType& I1,const InterpolantType& I2,const FVector2D& P0,const FVector2D& P1,const FVector2D& P2,UBOOL BackFacing)
	{
		InterpolantType	Interpolants[3] = { I0, I1, I2 };
		FVector2D		Points[3] = { P0, P1, P2 };

		// Find the top point.

		if(Points[1].Y < Points[0].Y && Points[1].Y <= Points[2].Y)
		{
			Exchange(Points[0],Points[1]);
			Exchange(Interpolants[0],Interpolants[1]);
		}
		else if(Points[2].Y < Points[0].Y && Points[2].Y <= Points[1].Y)
		{
			Exchange(Points[0],Points[2]);
			Exchange(Interpolants[0],Interpolants[2]);
		}

		// Find the bottom point.

		if(Points[1].Y > Points[2].Y)
		{
			Exchange(Points[2],Points[1]);
			Exchange(Interpolants[2],Interpolants[1]);
		}

		// Calculate the edge gradients.

		FLOAT			TopMinDiffX = (Points[1].X - Points[0].X) / (Points[1].Y - Points[0].Y),
						TopMaxDiffX = (Points[2].X - Points[0].X) / (Points[2].Y - Points[0].Y);
		InterpolantType	TopMinDiffInterpolant = (Interpolants[1] - Interpolants[0]) / (Points[1].Y - Points[0].Y),
						TopMaxDiffInterpolant = (Interpolants[2] - Interpolants[0]) / (Points[2].Y - Points[0].Y);

		FLOAT			BottomMinDiffX = (Points[2].X - Points[1].X) / (Points[2].Y - Points[1].Y),
						BottomMaxDiffX = (Points[2].X - Points[0].X) / (Points[2].Y - Points[0].Y);
		InterpolantType	BottomMinDiffInterpolant = (Interpolants[2] - Interpolants[1]) / (Points[2].Y - Points[1].Y),
						BottomMaxDiffInterpolant = (Interpolants[2] - Interpolants[0]) / (Points[2].Y - Points[0].Y);

		DrawTriangleTrapezoid(
			Interpolants[0],
			TopMinDiffInterpolant,
			Interpolants[0],
			TopMaxDiffInterpolant,
			Points[0].X,
			TopMinDiffX,
			Points[0].X,
			TopMaxDiffX,
			Points[0].Y,
			Points[1].Y,
			BackFacing
			);

		DrawTriangleTrapezoid(
			Interpolants[1],
			BottomMinDiffInterpolant,
			Interpolants[0] + TopMaxDiffInterpolant * (Points[1].Y - Points[0].Y),
			BottomMaxDiffInterpolant,
			Points[1].X,
			BottomMinDiffX,
			Points[0].X + TopMaxDiffX * (Points[1].Y - Points[0].Y),
			BottomMaxDiffX,
			Points[1].Y,
			Points[2].Y,
			BackFacing
			);
	}

	FTriangleRasterizer(const RasterPolicyType& InRasterPolicy): RasterPolicyType(InRasterPolicy) {}

private:

	void DrawTriangleTrapezoid(
		const InterpolantType& TopMinInterpolant,
		const InterpolantType& DeltaMinInterpolant,
		const InterpolantType& TopMaxInterpolant,
		const InterpolantType& DeltaMaxInterpolant,
		FLOAT TopMinX,
		FLOAT DeltaMinX,
		FLOAT TopMaxX,
		FLOAT DeltaMaxX,
		FLOAT MinY,
		FLOAT MaxY,
		UBOOL BackFacing
		)
	{
		INT	IntMinY = Clamp(appCeil(MinY),RasterPolicyType::GetMinY(),RasterPolicyType::GetMaxY() + 1),
			IntMaxY = Clamp(appCeil(MaxY),RasterPolicyType::GetMinY(),RasterPolicyType::GetMaxY() + 1);

		for(INT IntY = IntMinY;IntY < IntMaxY;IntY++)
		{
			FLOAT			Y = IntY - MinY,
							MinX = TopMinX + DeltaMinX * Y,
							MaxX = TopMaxX + DeltaMaxX * Y;
			InterpolantType	MinInterpolant = TopMinInterpolant + DeltaMinInterpolant * Y,
							MaxInterpolant = TopMaxInterpolant + DeltaMaxInterpolant * Y;

			if(MinX > MaxX)
			{
				Exchange(MinX,MaxX);
				Exchange(MinInterpolant,MaxInterpolant);
			}

			if(MaxX > MinX)
			{
				INT				IntMinX = Clamp(appCeil(MinX),RasterPolicyType::GetMinX(),RasterPolicyType::GetMaxX() + 1),
								IntMaxX = Clamp(appCeil(MaxX),RasterPolicyType::GetMinX(),RasterPolicyType::GetMaxX() + 1);
				InterpolantType	DeltaInterpolant = (MaxInterpolant - MinInterpolant) / (MaxX - MinX);

				for(INT X = IntMinX;X < IntMaxX;X++)
				{
					RasterPolicyType::ProcessPixel(X,IntY,MinInterpolant + DeltaInterpolant * (X - MinX),BackFacing);
				}
			}
		}
	}
};



}
