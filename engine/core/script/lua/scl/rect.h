#pragma once

#ifdef max
#undef max
#endif
namespace scl	{

////////////////////////////////////
//	rect:
//		x	y	z
//	在space中x y 为地面，z为垂直方向
//  在screen中x y 为以左上角作为(0,0)开始算的坐标，z无效
////////////////////////////////////
class rect
{
public:
	int	left;
	int	top;
	int	right;
	int	bottom;
	rect();
	rect(const int _left, const int _top, const int _right, const int _bottom);

	int		width			() const { return right - left; }
	int		height			() const { return bottom - top; }
	void	set_position	(const int _left, const int _top) { left = _left; top = _top; }
	void	set_width		(const int _width) { right = left + _width; }
	void	set_height		(const int _height) { bottom = top + _height; }
	void	set_size		(const int _width, const int _height) { set_width(_width); set_height(_height); }
	void	cut				(const rect& bound);	//根据bound截取rectf的一部分
	bool	contains		(const int x, const int y);
};

class rectf
{
public:
	float	left;
	float	top;
	float	right;
	float	bottom;

	rectf();
	rectf(const int   _left, const int   _top, const int   _right, const int   _bottom);
	rectf(const float _left, const float _top, const float _right, const float _bottom);

	void	set				(const int _left, const int _top, const int _right, const int _bottom);
	void	set				(const float _left, const float _top, const float _right, const float _bottom);

	//properties
	bool	is_empty		() const;
	float	width			() const { return right - left; }
	float	height			() const { return bottom - top; }
	void	set_position	(const float _left, const float _top) { left = _left; top = _top; }
	void	set_width		(const float _width) { right = left + _width; }
	void	set_height		(const float _height) { bottom = top + _height; }
	void	set_size		(const float _width, const float _height) { set_width(_width); set_height(_height); }
	bool	contains		(const float x, const float y);

	//根据bound截取rectf的一部分
	void	cut				(const rectf& bound);

	//根据bound截取rectf的一部分，同时输出对应纹理坐标的截取结果，保存在textureRect中
	void	cut_with_texture(const rectf& bound, rectf& textureRect);

	//移动rect，基准点为rect左上角，原来的(left, top)移动到(newLeft, newTop)
	void	move_to			(const float newLeft, const float newTop);
	void	move_to			(const int newLeft, const int newTop) { move_to(static_cast<float>(newLeft), static_cast<float>(newTop)); }
	void	move			(const float	d_left, const float d_top);
	void	move			(const int		d_left, const int	d_top) { move(static_cast<float>(d_left), static_cast<float>(d_top)); }

	//rect向外膨胀，如果x，y小于0，则为收缩
	void	inflate			(const float x, const float	y) { inflate(x, x, y, y); }
	void 	inflate			(const int   x,	const int   y) { inflate(x, x, y, y); }
	void 	inflate			(const int   _left,	const int   _right, const int	_top, const int		_bottom);
	void 	inflate			(const float _left,	const float _right, const float _top, const float	_bottom);
	void 	shrink			(const int   x,	const int   y) { shrink(x, x, y, y); }
	void 	shrink			(const float x, const float	y) { shrink(x, x, y, y); }
	void 	shrink			(const int   _left,	const int   _right, const int	_top, const int		_bottom) { inflate(-_left, -_right, -_top, -_bottom); }
	void 	shrink			(const float _left,	const float _right, const float _top, const float	_bottom) { inflate(-_left, -_right, -_top, -_bottom); }
	void 	shrink_ratio	(const float _left,	const float _right, const float _top, const float	_bottom) { shrink(static_cast<int>(width() * _left), static_cast<int>(width()*_right), static_cast<int>(height()*_top), static_cast<int>(height()*_bottom)); }
	void 	shrink_ratio	(const float _all) { shrink_ratio(_all, _all, _all, _all); }
	void	try_contain		(const scl::rectf& r);

	//将rect放入到世界坐标系中
	void	to_world		(const int deviceWidth, const int deviceHeight);

	static const rectf& zero();
	static const rectf& empty() { return zero(); }
	static const rectf& max();

private:
	void	_point_to_world	(float& x, float& y, const int deviceWidth, const int deviceHeight);
};

class point2f
{
public:
	float x; 
	float y;

	point2f() : x(0), y(0) {}
	point2f(const float _x, const float _y) : x(_x), y(_y) {}
	void set(const float _x, const float _y) { x = _x; y = _y; }
};

class quadf
{
public:
	point2f points[4];

	quadf() {}
	quadf(const point2f& lefttop, const point2f& righttop, const point2f& leftbottom, const point2f& rightbottom) 
	{
		points[0] = lefttop;
		points[1] = righttop;
		points[2] = leftbottom;
		points[3] = rightbottom;
	}

	quadf(const rectf& r)
	{
		//		0--1
		//		|  |
		//		2--3
		points[0].set(r.left, r.top);
		points[1].set(r.right, r.top);
		points[2].set(r.left, r.bottom);
		points[3].set(r.right, r.bottom);
	}

	void invert()
	{
		quadf old = *this;
		points[0] = old.points[2];
		points[1] = old.points[3];
		points[2] = old.points[0];
		points[3] = old.points[1];
	}
};

} //namespace scl

