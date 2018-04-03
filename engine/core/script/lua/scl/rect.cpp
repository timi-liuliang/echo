#include "scl/rect.h"
#include "scl/math.h"

namespace scl	{

////////////////////////////////////
// rect
////////////////////////////////////
rect::rect() : left(0), top(0), right(0), bottom(0)
{

}

rect::rect(const int leftParam, const int topParam, const int rightParam, const int bottomParam) : 
left(leftParam), top(topParam), right(rightParam), bottom(bottomParam)
{

}

void rect::cut(const rect& bound)
{
	if (left < bound.left)
		left = bound.left;

	if (top < bound.top)
		top = bound.top;

	if (right > bound.right)
		right = bound.right;

	if (bottom > bound.bottom)
		bottom = bound.bottom;
}

bool rect::contains(const int x, const int y)
{
	return x >= left &&
		x <= right &&
		y >= top &&
		y <= bottom;
}

////////////////////////////////////
// rectf (rect of float)
////////////////////////////////////
rectf::rectf() : left(0), top(0), right(0), bottom(0)
{

}


rectf::rectf(const float leftParam, const float topParam, const float rightParam, const float bottomParam) : 
	left	(leftParam), 
	top		(topParam), 
	right	(rightParam), 
	bottom	(bottomParam)
{

}

rectf::rectf(const int leftParam, const int topParam, const int rightParam, const int bottomParam) : 
	left	(static_cast<float>(leftParam)), 
	top		(static_cast<float>(topParam)), 
	right	(static_cast<float>(rightParam)), 
	bottom	(static_cast<float>(bottomParam))
{

}


void rectf::to_world(const int deviceWidth, const int deviceHeight)
{
	_point_to_world(left,		top,	deviceWidth, deviceHeight);
	_point_to_world(right,	bottom, deviceWidth, deviceHeight);
}

void rectf::_point_to_world(float& x, float& y, const int deviceWidth, const int deviceHeight)
{
	////////////////////////////////////
	//一个point(x, y)从screen坐标系变换到world坐标系
	////////////////////////////////////

	//screen坐标系y轴向下，所以先要变换为y轴向上的坐标系，才能和world坐标系变换
	const float revertScreenX = x;
	const float revertScreenY = -y;

	//从screen变换到world的transform值
	const float transformX = -deviceWidth / 2.0f;
	const float transformY = deviceHeight / 2.0f;

	//获取位置坐标在world中的坐标值
	x = revertScreenX + transformX;
	y = revertScreenY + transformY;
}

void rectf::inflate(const int   left,	const int   right, const int top, const int bottom)
{
	this->left		-= left;
	this->right		+= right;
	this->top		-= top;
	this->bottom		+= bottom;
}

void rectf::inflate(const float left,	const float right, const float top, const float bottom)
{
	this->left		-= left;
	this->right		+= right;
	this->top		-= top;
	this->bottom		+= bottom;
}

void rectf::move_to(const float newLeft, const float newTop)
{
	float leftShift = newLeft - left;
	float topShift	= newTop - top;
	left	= newLeft;
	top		= newTop;
	right	+= leftShift;
	bottom	+= topShift;
}

bool rectf::is_empty() const
{
	return (left >= right) || (top >= bottom);
}

void rectf::cut_with_texture(const rectf& bound, rectf& textureRect)
{
	const float oriWidth	= right - left;
	const float oriHeight	= bottom - top;
	if (oriWidth < 0 || oriHeight < 0)
		return;

	//纹理坐标是从0到1的，所以这里要计算变化的比率
	float leftChangeRatio = 0;
	if (left < bound.left)
	{
		leftChangeRatio = (bound.left - left) / oriWidth ;
		left = bound.left;
	}

	float topChangeRatio = 0;
	if (top < bound.top)
	{
		topChangeRatio = (bound.top - top) / oriHeight;
		top = bound.top;
	}

	float rightChangeRatio = 0;
	if (right > bound.right)
	{
		rightChangeRatio = (right - bound.right) / oriWidth;
		right = bound.right;
	}

	float bottomChangeRatio = 0;
	if (bottom > bound.bottom)
	{
		bottomChangeRatio = (bottom - bound.bottom) / oriHeight;
		bottom = bound.bottom;
	}

	const float textureRectWidth	= textureRect.width();
	const float textureRectHeight	= textureRect.height();
	textureRect.left	+= textureRectWidth		* leftChangeRatio;
	textureRect.top		+= textureRectHeight	* topChangeRatio;
	textureRect.right	-= textureRectWidth		* rightChangeRatio;
	textureRect.bottom	-= textureRectHeight	* bottomChangeRatio;
}

void rectf::cut(const rectf& bound)
{
	if (left < bound.left)
		left = bound.left;

	if (top < bound.top)
		top = bound.top;

	if (right > bound.right)
		right = bound.right;

	if (bottom > bound.bottom)
		bottom = bound.bottom;
}

void rectf::set(const int _left, const int _top, const int _right, const int _bottom)
{
	left	=	static_cast<float>(_left); 
	top		=	static_cast<float>(_top); 
	right	=	static_cast<float>(_right);
	bottom	=	static_cast<float>(_bottom);
}

void rectf::set(const float _left, const float _top, const float _right, const float _bottom)
{
	left	=	_left; 
	top		=	_top; 
	right	=	_right;
	bottom	=	_bottom;
}


void rectf::move(const float d_left, const float d_top)
{
	left	+= d_left;
	top		+= d_top;
	right	+= d_left;
	bottom	+= d_top;
}

bool rectf::contains(const float x, const float y)
{
	return x >= left &&
		x <= right &&
		y >= top &&
		y <= bottom;
}

const rectf& rectf::zero()
{
	static const rectf s_zero(0, 0, 0, 0);
	return s_zero;
}

const rectf& rectf::max()
{
	static const rectf s_zero(static_cast<float>(MIN_INT), static_cast<float>(MIN_INT), static_cast<float>(MAX_INT), static_cast<float>(MAX_INT));
	return s_zero;
}

void rectf::try_contain(const scl::rectf& r)
{
	if (r.left < left)
		left = r.left;
	if (r.top < top)
		top = r.top;
	if (r.right > right)
		right = r.right;
	if (r.bottom > bottom)
		bottom = r.bottom;
}

} //namespace scl



