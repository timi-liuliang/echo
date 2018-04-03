#pragma once

#define SCL_MAX_FRAME 62

#ifdef _WIN32
struct _EXCEPTION_POINTERS;
#endif

namespace scl {

//	[1]			0x00B513DA	:	MyClass::Func	d:\src\myclass.cpp	(13)	  0x0F4C
//	 |				|				|					|			 |			|
//frame_index	address			function_name		file_name	  line_num	function_offset
enum FORMAT
{
	FRAME_INDEX		=	0x01,	//帧号
	ADDRESS			=	0x02,	//地址值
	FUNCTION_NAME	=	0x04,	//函数名
	FILE_NAME		=	0x08,	//文件名
	LINE_NUM		=	0x10,	//行数
	FUNCTION_OFFSET	=	0x20,	//offset in the current function (only avalible under linux)
};

#ifdef _WIN32
unsigned long except_handler(_EXCEPTION_POINTERS* lpEP);
#endif  

#if defined(linux) || defined(__APPLE__) || defined(__ANDROID__) || defined(__GNUC__)
void	print_stack_to_file();
#endif

//input param	: stack / maxframe
int		backtrace	(void** stack, int maxframe, int skip = 0);

//input param	: stack / framecount
//output param	: output / outputlen
int		print_stack	(void* const* stack, const int framecount, char* output, const int outputlen, const int format = 0xFFFFFFFF, const char* const line_sperator = "\n");

//example:
//		void* stack[62] = { NULL };
//		int c = scl::backtrace(stack, 62);	
//		char s[1024] = { 0 };
//		scl::print_stack(stack, c, s, 1024);
//		printf("%s\n", s)

} //namespace scl

