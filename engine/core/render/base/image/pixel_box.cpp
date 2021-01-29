#include "pixel_box.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	PixelBox::PixelBox()
		: left(0)
		, top(0)
		, right(1)
		, bottom(1)
		, front(0)
		, back(1)
		, pixFmt(PF_UNKNOWN)
	{
	}

	PixelBox::PixelBox(ui32 width, ui32 height, ui32 depth, PixelFormat pixFormat, void* pPixData)
		: left(0)
		, top(0)
		, right(width)
		, bottom(height)
		, front(0)
		, back(depth)
		, data(pPixData)
		, pixFmt(pixFormat)
	{
		setConsecutive();
	}

	PixelBox::~PixelBox()
	{
	}

	void PixelBox::setConsecutive()
	{
		rowPitch = getWidth();
		slicePitch = getWidth() * getHeight();
	}

	ui32 PixelBox::getRowSkip() const
	{
		return (rowPitch - getWidth());
	}

	ui32 PixelBox::getSliceSkip() const
	{
		return (slicePitch - getHeight() * rowPitch);
	}

	bool PixelBox::isConsecutive() const
	{
		return (rowPitch == getWidth() && slicePitch == getWidth() * getHeight());
	}

	ui32 PixelBox::getConsecutiveSize() const
	{
		return PixelUtil::GetMemorySize(getWidth(), getHeight(), getDepth(), pixFmt);
	}

	bool PixelBox::getSubVolume(PixelBox& out, const PixelBox& def) const
	{
		if (PixelUtil::IsCompressed(pixFmt))
		{
			if (def.left == left && def.top == top && def.front == front &&
				def.right == right && def.bottom == bottom && def.back == back)
			{
				// Entire buffer is being queried
				out = *this;
				return true;
			}

			EchoLogError("Cannot return subvolume of compressed PixelBuffer");
			return false;
		}

		if (!contains(def))
		{
			EchoLogError("Bounds out of range");
			return false;
		}

		const ui32 pixSize = PixelUtil::GetPixelSize(pixFmt);

		// Calculate new data origin
		// Notice how we do not propagate left/top/front from the incoming box, since
		// the returned pointer is already offset
		PixelBox rval(def.getWidth(), def.getHeight(), def.getDepth(), pixFmt,
			((ui8*)data) + ((def.left - left) * pixSize) + ((def.top - top) * rowPitch * pixSize) + ((def.front - front) * slicePitch * pixSize));

		rval.rowPitch = rowPitch;
		rval.slicePitch = slicePitch;
		rval.pixFmt = pixFmt;

		out = rval;

		return true;
	}

	Color PixelBox::getColor(ui32 x, ui32 y, ui32 z) const
	{
		Color color;

		Byte pixelSize = PixelUtil::GetPixelSize(pixFmt);
		size_t pixelOffset = pixelSize * (z * slicePitch + y * rowPitch + x);
		PixelUtil::UnpackColor(color, pixFmt, (Byte*)data + pixelOffset);

		return color;
	}

	void PixelBox::setColor(const Color& color, ui32 x, ui32 y, ui32 z)
	{
		Byte pixelSize = PixelUtil::GetPixelSize(pixFmt);
		size_t pixelOffset = pixelSize * (z * slicePitch + y * rowPitch + x);
		PixelUtil::PackColor(color, pixFmt, (Byte*)data + pixelOffset);
	}
}