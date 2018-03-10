/*=============================================================================
	Texture.h: Texture definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{
	/** Texture formats used by FTexture2D */
	enum ETexture2DFormats
	{
		TF_UNKNOWN,
		TF_ARGB8,
		TF_ARGB16F
	};

	/** A 2D texture */
	class FTexture2D
	{
	protected:
		/** Texture dimensions */
		INT SizeX;
		INT SizeY;
		/** Format of texture which indicates how to interpret Data */
		ETexture2DFormats Format;
		/** The size of each element in the data array */
		INT ElementSize;
		/** Mip 0 texture data */
		BYTE* Data;

	public:
		FTexture2D() :
			  SizeX(0)
			, SizeY(0)
			, Format(TF_UNKNOWN)
			, ElementSize(0)
			, Data(NULL)
		{
		}

		FTexture2D(ETexture2DFormats InFormat, INT InSizeX, INT InSizeY) :
			  SizeX(InSizeX)
			, SizeY(InSizeY)
			, Format(InFormat)
			, ElementSize(0)
			, Data(NULL)
		{
			Init(InFormat, InSizeX, InSizeY);
		}

		virtual ~FTexture2D()
		{
			appFree(Data);
		}

		/** Accessors */
		INT GetSizeX() const { return SizeX; }
		INT GetSizeY() const { return SizeY; }
		BYTE* GetData() { return Data; }

		void Init(ETexture2DFormats InFormat, INT InSizeX, INT InSizeY)
		{
			check(InSizeX > 0 && InSizeY > 0);
			SizeX = InSizeX;
			SizeY = InSizeY;
			Format = InFormat;

			// Only supporting these formats
			check(InFormat == TF_ARGB8 || InFormat == TF_ARGB16F);

			switch (InFormat)
			{
			case TF_ARGB8:		ElementSize = sizeof(FColor);			break;
			case TF_ARGB16F:	ElementSize = sizeof(FFloat16Color);	break;
			}

			Data = (BYTE*)(appMalloc(ElementSize * SizeX * SizeY));
			appMemzero(Data, ElementSize * SizeX * SizeY);
		}

		inline BYTE* SampleRawPtr(const FVector2D& UV) const
		{
			// Wrapped addressing (uses appFloor and not appFractional, as appFractional causes the
			// following mapping:
			// .4 -> .4
			// -1.4 -> .4
			// (.4 - 1 = -.6) -> .6
			//
			// we need:
			// .4 -> .4
			// -1.4 -> .6
			// (.4 - 1 = -.6) -> .4
			//
			// because when you subtract 1 from a UV it needs to have the exact same fractional part
 			const INT X = Clamp(appTrunc((UV.X - appFloor(UV.X)) * SizeX), 0, SizeX - 1);
 			const INT Y = Clamp(appTrunc((UV.Y - appFloor(UV.Y)) * SizeY), 0, SizeY - 1);
			// Byte index into Data
			const INT DataIndex = Y * SizeX * ElementSize + X * ElementSize;
			return &Data[DataIndex];
		}

		inline FLinearColor Sample(const FVector2D& UV) const
		{
			BYTE* RawPtr = SampleRawPtr( UV );

			// Only supporting these formats
			checkSlow(Format == TF_ARGB8 || Format == TF_ARGB16F);

			if (Format == TF_ARGB16F)
			{
				// Lookup and convert to FP32, no filtering
				return FLinearColor(*(FFloat16Color*)RawPtr);
			}
			// Lookup and convert linear space and FP32, no filtering
			return FLinearColor(*(FColor*)RawPtr);
		}

		inline FVector4 SampleNormal(const FVector2D& UV) const
		{
			BYTE* RawPtr = SampleRawPtr( UV );

			// Only supporting these formats
			checkSlow(Format == TF_ARGB16F);
			FFloat16Color* Float16Color = (FFloat16Color*)RawPtr;

			FVector4 Normal( Float16Color->R.GetFloat(), Float16Color->G.GetFloat(), Float16Color->B.GetFloat(), 0.0f );
			return Normal;
		}
	};

} //namespace Lightmass
