#pragma once

#include "pixel_format.h"

namespace Echo
{
	struct PixelBox
	{
		ui32			left;
		ui32			top;
		ui32			right;
		ui32			bottom;
		ui32			front;
		ui32			back;
		void*			data; 		// The data pointer 

		// The pixel format 
		PixelFormat		pixFmt;

		/** Number of elements between the leftmost pixel of one row and the left
		pixel of the next. This value must always be equal to getWidth() (consecutive)
		for compressed formats.
		*/
		ui32			rowPitch;

		/** Number of elements between the top left pixel of one (depth) slice and
		the top left pixel of the next. This can be a negative value. Must be a multiple of
		rowPitch. This value must always be equal to getWidth()*getHeight() (consecutive)
		for compressed formats.
		*/
		ui32			slicePitch;

	public:
		PixelBox();
		PixelBox(ui32 width, ui32 height, ui32 depth, PixelFormat pixFormat, void* pPixData = NULL);

		~PixelBox();

	public:
		// Get the width of this box
		inline ui32			getWidth() const { return right - left; }
		// Get the height of this box
		inline ui32			getHeight() const { return bottom - top; }
		// Get the depth of this box
		inline ui32			getDepth() const { return back - front; }

		// Return true if the other box is a part of this one
		inline bool			contains(const PixelBox& def) const
		{
			return (def.left >= left && def.top >= top && def.front >= front &&
				def.right <= right && def.bottom <= bottom && def.back <= back);
		}

		/** Set the rowPitch and slicePitch so that the buffer is laid out consecutive
		in memory.
		*/
		void				setConsecutive();

		/**	Get the number of elements between one past the rightmost pixel of
		one row and the leftmost pixel of the next row. (IE this is zero if rows
		are consecutive).
		*/
		ui32				getRowSkip() const;

		/** Get the number of elements between one past the right bottom pixel of
		one slice and the left top pixel of the next slice. (IE this is zero if slices
		are consecutive).
		*/
		ui32				getSliceSkip() const;

		/** Return whether this buffer is laid out consecutive in memory (ie the pitches
		are equal to the dimensions)
		*/
		bool				isConsecutive() const;

		/** Return the size (in bytes) this image would take if it was
		laid out consecutive in memory
		*/
		ui32				getConsecutiveSize() const;

		/** Return a subvolume of this PixelBox.
		@param def	Defines the bounds of the subregion to return
		@return	A pixel box describing the region and the data in it
		@remarks	This function does not copy any data, it just returns
		a PixelBox object with a data pointer pointing somewhere inside
		the data of object.
		@throws	Exception(ERR_INVALIDPARAMS) if def is not fully contained
		*/
		bool				getSubVolume(PixelBox& out, const PixelBox& def) const;

		/**
		* Get color value from a certain location in the PixelBox. The z coordinate
		* is only valid for cubemaps and volume textures. This uses the first (largest)
		* mipmap.
		*/
		Color				getColor(ui32 x, ui32 y, ui32 z) const;

		/**
		* Set colour value at a certain location in the PixelBox. The z coordinate
		* is only valid for cubemaps and volume textures. This uses the first (largest)
		* mipmap.
		*/
		void				setColor(const Color& color, ui32 x, ui32 y, ui32 z);
	};
}