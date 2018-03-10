/**********************************************************************
 *<
	FILE: expr.h

	DESCRIPTION: expression object include file.

	CREATED BY: Don Brittain

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxheap.h"
#include "export.h"
#include "strclass.h"
#include "Point3.h"
#include "TabTypes.h"

/*! \defgroup expressionTypes Expression Types */
//@{
#define SCALAR_EXPR		1	//!< A single floating point value.
/*! ans[0] = x, ans[1] = y, ans[2] = z. \n\n
The method getExprType() may be used to determine the type of the expression result. */
#define VECTOR_EXPR		3	//!< An array of floating point values.
//@}

/*! \defgroup expressionVariableTypes Expression Variable Types
\sa Class CallPoint3 */
//@{
#define SCALAR_VAR		SCALAR_EXPR	//!< A single floating point value.
/*! The x, y, z public data members of the Point3 are the values representing the 
vector. Vectors are specified in an expression by enclosing the three values in 
square brackets, i.e. "[]". For example, this is a unit vector parallel to the 
world Z axis: [0,0,1]. */
#define VECTOR_VAR		VECTOR_EXPR	//!< A Point3 value when passed to the eval() method.
//@}

class Expr;

typedef int (*ExprFunc)(Expr *e, float f);

class DllExport Inst: public MaxHeapOperators {
public:
	ExprFunc	func;
	float  		sVal;
};

class ExprVar : public MaxHeapOperators {
public:
	MSTR		name;
	int		type;
	int		regNum;
};


typedef Tab<Inst>    InstTab;
typedef Tab<ExprVar> ExprVarTab;

/*! \sa  Class Point3, \ref expressionTypes, \ref expressionVariableTypes, \ref expressionReturnCodes,
<a href="ms-its:3dsmaxsdk.chm::/ui_character_strings.html">Character Strings</a>.\n\n
\par Description:
This class may be used by developers to parse mathematical expressions. The
expression is created as a character string using a straightforward syntax.
Expressions consist of operators (+, -, *, /, etc.), literal constants (numbers
like 180, 2.718, etc.), variables (single floating point values or vector
(<b>Point3</b>) values), and functions (mathematical functions that take one
ore more arguments and return a result). The return value from the expression
may be a floating point value or a vector. There are many built in functions,
operators and constants available for use.\n\n
All methods of this class are implemented by the system.\n\n
Developers wishing to use these APIs should #include
<b>/MAXSDK/INCLUDE/EXPRLIB.H</b> and should link to
<b>/MAXSDK/LIB/EXPR.LIB</b>.\n\n
Sample code using these APIs is shown below, and is also available as part of
the expression controller in
<b>/MAXSDK/SAMPLES/CONTROLLERS/EXPRCTRL.CPP</b>.\n\n
Variables may be defined and used in expressions. Variable names are case
sensitive, and must begin with a letter of the alphabet, but may include
numbers. They may be any length. To create a named variable, you use the method
<b>defVar()</b>. This takes a name and returns a register number. Defining the
variable creates storage space in a list of variables maintained by the parser,
and the register number is used as an array index into the variable value
arrays passed into the expression evaluation method (<b>eval()</b>).\n\n
To use the variable in an expression just use its name. For example if you
define a variable named <b>radius</b>, you can use it in an expression like:
<b>2*pi*radius</b>. To give the variable a value, you define two arrays of
variables and pass them to the evaluation method (<b>eval()</b>). There is one
array for scalar variables, and one for vector variables. You pass these arrays
along with the number of variables in each list. See the sample code below for
an example.\n\n
The order of calling the methods of this class to evaluate an expression is as
follows:\n\n
Declare an expression instance (<b>Expr expr;</b>)\n\n
Define the expression (<b>char e1[] = "2*pi*radius";</b>).\n\n
Define any variables (<b>expr.defVar(SCALAR_VAR, _M("radius"));</b>)\n\n
Load the expression (<b>expr.load(e1);</b>)\n\n
Evaluate the expression (<b>expr.eval(...);</b>)\n\n
There are no restrictions on the use of white space in expressions -- it may be
used freely to make expressions more readable. In certain instances, white
space should be used to ensure non-ambiguous parsing. For example, the <b>x</b>
operator is used for to compute the cross product of two vectors. If a
developer has several vectors: <b>Vec</b>, <b>Axis</b> and <b>xAxis</b> and
wanted to compute the cross product, <b>VecxAxis</b> is ambiguous while <b>Vec
x Axis</b> is not.\n\n
All the necessary information to evaluate an expression is completely stored
within an expression object. For example, if you are passed a pointer to an
expression object for which some variables have been defined that you knew the
value of, you could get all the information you needed from the expression
object to completely evaluate the expression. This includes the expression
string, variable names, variable types, and variable register indices.\n\n
For complete documentation of the built in functions please refer to the 3ds
Max User's Guide under Using Expression Controllers. Below is an overview of
the operators, constants and functions that are available:
\par Expression Operators:
<b>Scalar Operators</b>\n\n
 Operator Use Meaning\n\n
 <b>+</b> p+q addition\n\n
 <b>-</b> p-q subtraction\n\n
 <b>-</b> -p additive inverse\n\n
 <b>*</b> p*q multiplication\n\n
 <b>/</b> p/q division\n\n
 <b>^</b> p^q power (p to the power of q)\n\n
 <b>**</b> p**q same as p^q\n\n
<b>Boolean Operators</b>\n\n
 <b>=</b> p=q equal to\n\n
 <b>\<</b> p\<q less than\n\n
 <b>\></b> p\>q greater than\n\n
 <b>\<=</b> p\<=q less than or equal to\n\n
 <b>\>=</b> p\>=q greater than or equal to\n\n
 <b>|</b> p|q logical OR\n\n
 <b>\&</b> p\&q logical AND\n\n
<b>Vector Operators</b>\n\n
 <b>+</b> V+W addition\n\n
 <b>-</b> V-W subtraction\n\n
 <b>*</b> p*V scalar multiplication\n\n
  V*p "\n\n
 <b>*</b> V*W dot product\n\n
 <b>x</b> VxW cross product\n\n
 <b>/</b> V/p scalar division\n\n
 <b>.</b> V.x first component (X)\n\n
 <b>.</b> V.y second component (Y)\n\n
 <b>.</b> V.z third component (Z)
\par Built-In Constants:
 <b>pi</b> 3.1415...\n\n
 <b>e</b> 2.7182...\n\n
 <b>TPS</b> 4800 (ticks per second)
\par Expression Functions:
<b>Trigonometric Functions</b>\n\n
The angles are specified and returned in degrees.\n\n
 <b>sin(p)</b> sine\n\n
 <b>cos(p)</b> cosine\n\n
 <b>tan(p)</b> tangent\n\n
 <b>asin(p)</b> arc sine\n\n
 <b>acos(p)</b> arc cosine\n\n
 <b>atan(p)</b> arc tangent\n\n
<b>Hyperbolic Functions</b>\n\n
 <b>sinh(p)</b> hyperbolic sine\n\n
 <b>cosh(p)</b> hyperbolic cosine\n\n
 <b>tanh(p)</b> hyperbolic tangent\n\n
<b>Conversion between Radians and Degrees</b>\n\n
 <b>radToDeg(p)</b> takes p in radians and returns the same angle in
degrees\n\n
 <b>degToRad(p)</b> takes p in degrees and returns the same angle in
radians\n\n
<b>Rounding Functions</b>\n\n
 <b>ceil(p)</b> smallest integer greater than or equal to p.\n\n
 <b>floor(p)</b> largest integer less than or equal to p.\n\n
<b>Standard Calculations</b>\n\n
 <b>ln(p)</b> natural (base e) logarithm\n\n
 <b>log(p)</b> common (base 10) logarithm\n\n
 <b>exp(p)</b> exponential function -- exp(e) = e^p\n\n
 <b>pow(p, q)</b> p to the power of q -- p^q\n\n
 <b>sqrt(p)</b> square root\n\n
 <b>abs(p)</b> absolute value\n\n
 <b>min(p, q)</b> minimum -- returns p or q depending on which is
smaller\n\n
 <b>max(p, q)</b> maximum -- returns p or q depending on which is
larger\n\n
 <b>mod(p, q)</b> remainder of p divided by q\n\n
<b>Conditional</b>\n\n
 <b>if (p, q, r)</b> works like the common spreadsheet "if" -- if p
is nonzero\n\n
  then "if" returns q, otherwise r.\n\n
<b>Vector Handling</b>\n\n
 <b>length(V)</b> the length of V\n\n
 <b>unit(V)</b> returns a unit vector in the same direction as V.\n\n
 <b>comp(V, I)</b> i-th component, where I=0, 1, or 2.\n\n
  comp([5,6,7],1) = 6\n\n
<b>Special Animation Functions</b>\n\n
 <b>noise(p, q, r)</b> 3D noise -- returns a randomly generated
position.\n\n
  p, q, and r are random values used as a seed.
\par Sample Code:
The following code shows how the expression parser can be used. This code
evaluates several expressions and displays the results in a dialog box. Both
scalar and vector variables are used. One expression contains an error to show
how error handling is done.\n\n
\code
void Utility::TestExpr()
{
// Declare an expression instance and variable storage
	Expr expr;
	float sRegs[2];                               // Must be at least getVarCount(SCALAR_VAR);
	Point3 vRegs[2];                              // Must be at least getVarCount(VECTOR_VAR);
	float ans[3];
	int status;

// Define a few expressions
	char e0[] = "2+2";
	char e1[] = "2.0 * pi * radius";
	char e2[] = "[1,1,0] + axis";
	char e3[] = "[sin(90.0), sin(radToDeg(0.5*pi)), axis.z]";
	char e4[] = "2+2*!@#$%";                      // Bad expression

// Define variables
	int radiusReg = expr.defVar(SCALAR_VAR, _M("radius"));
	int axisReg = expr.defVar(VECTOR_VAR, _M("axis"));
// Set the variable values
	sRegs[radiusReg] = 50.0f;
	vRegs[axisReg] = Point3(0.0f, 0.0f, 1.0f);
// Get the number of each we have defined so far
	int sCount = expr.getVarCount(SCALAR_VAR);
	int vCount = expr.getVarCount(VECTOR_VAR);

// Load and evaluate expression "e0"
	if (status = expr.load(e0))
		HandleLoadError(status, expr);
	else {
		status = expr.eval(ans, sCount, sRegs, vCount, vRegs);
		if (status != EXPR_NORMAL)
			HandleEvalError(status, expr);
		else
			DisplayExprResult(expr, ans);
	}
// Load and evaluate expression "e1"
	if (status = expr.load(e1))
		HandleLoadError(status, expr);
	else {
		status = expr.eval(ans, sCount, sRegs, vCount, vRegs);
		if (status != EXPR_NORMAL)
			HandleEvalError(status, expr);
		else
			DisplayExprResult(expr, ans);
	}
// Load and evaluate expression "e2"
	if (status = expr.load(e2))
		HandleLoadError(status, expr);
	else {
		status = expr.eval(ans, sCount, sRegs, vCount, vRegs);
		if (status != EXPR_NORMAL)
			HandleEvalError(status, expr);
		else
			DisplayExprResult(expr, ans);
	}
// Load and evaluate expression "e3"
	if (status = expr.load(e3))
		HandleLoadError(status, expr);
	else {
		status = expr.eval(ans, sCount, sRegs, vCount, vRegs);
		if (status != EXPR_NORMAL)
			HandleEvalError(status, expr);
		else
			DisplayExprResult(expr, ans);
	}
// Load and evaluate expression "e4"
	if (status = expr.load(e4))
		HandleLoadError(status, expr);
	else {
		status = expr.eval(ans, sCount, sRegs, vCount, vRegs);
		if (status != EXPR_NORMAL)
			HandleEvalError(status, expr);
		else
			DisplayExprResult(expr, ans);
	}
}


// Display the expression and the result
void Utility::DisplayExprResult(Expr expr, float *ans)
{
	MCHAR msg[128];

	if (expr.getExprType() == SCALAR_EXPR) {
		_stprintf(msg, _M("Answer to \"%s\" is %.1f"), expr.getExprStr(), *ans);
		Message(msg, _M("Expression Result"));
	}
	else {
		_stprintf(msg, _M("Answer to \"%s\" is [%.1f, %.1f, %.1f]"), expr.getExprStr(), ans[0], ans[1], ans[2]);
		Message(msg, _M("Expression Result"));
	}
}


// Display the load error message
void Utility::HandleLoadError(int status, Expr expr)
{
	MCHAR msg[128];

	if(status == EXPR_INST_OVERFLOW) {
		_stprintf(_M("Inst stack overflow: %s"), expr.getProgressStr());
		Message(msg, _M("Error"));
	}
	else if (status == EXPR_UNKNOWN_TOKEN) {
		_stprintf(msg, _M("Unknown token: %s"), expr.getProgressStr());
		Message(msg, _M("Error"));
	}
	else {
		_stprintf(msg, _M("Cannot parse \"%s\". Error begins at last char of: %s"),
		expr.getExprStr(), expr.getProgressStr());
		Message(msg, _M("Error"));
	}
}

// Display the evaluation error message
void Utility::HandleEvalError(int status, Expr expr)
{
	MCHAR msg[128];

	_stprintf(msg, _M("Can't parse expression \"%s\""), expr.getExprStr());
	Message(msg, _M("Error"));
}

// Display the specified message and title in a dialog box
void Utility::Message(MCHAR *msg, MCHAR *title)
{
	MessageBox(ip->GetMAXHWnd(), (LPCMSTR) msg, (LPCMSTR) title, MB_ICONINFORMATION|MB_OK);
}
\endcode */
class Expr: public MaxHeapOperators {
public:
	/*! \remarks Constructor. Internal data structures are initialized as
	empty. */
	Expr()	{ sValStk = vValStk = instStk = nextScalar = nextVector = 0; }
	/*! \remarks Destructor. Any currently defined variables are deleted. */
	~Expr()	{ deleteAllVars(); }

	/*! \remarks This method is used to load an expression for parsing. An
	error code is returned indicating if the expression was loaded. A
	successfully loaded expression is then ready for evaluation with the
	<b>eval()</b> method.
	\par Parameters:
	<b>char *s</b>\n\n
	The expression to load.
	\return  See \ref expressionReturnCodes. */
	DllExport int		load(MCHAR *s);
	/*! \remarks This method is used to evaluate the expression loaded using
	<b>load()</b>. It returns either a scalar or vector result.
	\par Parameters:
	<b>float *ans</b>\n\n
	The numeric result of the expression is returned here, i.e. the answer .
	For scalar values this is a pointer to a single float. For vector values,
	<b>ans[0] is x, ans[1] = y, ans[2] = z</b>. You can determine which type of
	result is returned using the method <b>getExprType()</b>.\n\n
	<b>int sRegCt</b>\n\n
	The number of items in the <b>sRegs</b> array of scalar variables.\n\n
	<b>float *sRegs</b>\n\n
	Array of scalar variables.\n\n
	<b>int vRegCt=0</b>\n\n
	The number of items in the <b>vRegs</b> array of vector variables.\n\n
	<b>Point3 *vRegs=NULL</b>\n\n
	Array of vector variables.
	\return  See \ref expressionReturnCodes. */
	DllExport int		eval(float *ans, int sRegCt, float *sRegs, int vRegCt=0, Point3 *vRegs=NULL);
	/*! \remarks Returns the type of expression. See \ref expressionTypes. */
	int					getExprType(void)	{ return exprType; }
	/*! \remarks Returns a pointer to the currently loaded expression string.
	*/
	MCHAR *				getExprStr(void)	{ return origStr; }
	/*! \remarks If there was an error parsing the expression, this method
	returns a string showing what portion of the expression was parsed before
	the error occurred. */
	MCHAR *				getProgressStr(void){ return progressStr; }
	/*! \remarks Defines a named variable that may be used in an expression.
	\par Parameters:
	<b>int type</b>\n\n
	The type of variable. See \ref expressionVariableTypes.\n\n
	<b>MCHAR *name</b>\n\n
	The name of the variable. This name must begin with a letter, may include
	numbers and may be any length.
	\return  The register number (into the <b>sRegs</b> or <b>vRegs</b> array
	passed to <b>eval()</b>) of the variable. */
	DllExport int		defVar(int type, MCHAR *name);
	/*! \remarks This method returns the number of variables defined of the
	specified type. When you call <b>eval()</b> on an expression, you must make
	sure that the variable arrays (<b>sRegs</b> and <b>vRegs</b>) are at least
	the size returned from this method.
	\par Parameters:
	<b>int type</b>\n\n
	See \ref expressionVariableTypes. */
	DllExport int		getVarCount(int type);
	/*! \remarks Returns the name of the variable whose index is passed, or
	NULL if the variable could not be found.
	\par Parameters:
	<b>int type</b>\n\n
	The type the variable. See \ref expressionVariableTypes.\n\n
	<b>int i</b>\n\n
	The register number of the variable. */
	DllExport MCHAR *	getVarName(int type, int i);
	/*! \remarks When you define a variable with <b>defVar()</b>, you get a
	back a register number. If your code is set up in such a way that saving
	that register number is not convenient in the block of code that defines
	it, you can use this method later on to find out what that return value had
	been. For example, one piece of code might have:\n\n
	<b>expr-\>defVar(SCALAR_VAR, "a"); // not saving return value...</b>\n\n
	<b>expr-\>defVar(SCALAR_VAR, "b");</b>\n\n
	and then right before evaluating the expression, you might have some code
	such as:\n\n
	<b>for(i = 0; i \< expr-\>getVarCount(SCALAR_VAR); i++)</b>\n\n
	<b>if(_tcscmp("a", expr-\>getVarName(SCALAR_VAR, i) == 0)</b>\n\n
	<b>aRegNum = expr-\>getVarRegNum(SCALAR_VAR, i);</b>\n\n
	Of course, this is a bit contrived -- most real examples would probably
	have tables to store the variable names, register numbers, etc. and thus
	would not need to call this method. It is available however, and this makes
	the expression object self-contained in that everything you need to
	evaluate an expression with variables (other than the variable values
	themselves) is stored by the expression object.
	\par Parameters:
	<b>int type</b>\n\n
	See \ref expressionVariableTypes.\n\n
	<b>int i</b>\n\n
	The variable index returned from the method <b>defVar()</b>.
	\return  The register index for the variable whose type and index are
	passed. */
	DllExport int		getVarRegNum(int type, int i);
	/*! \remarks Deletes all the variables from the list maintained by the
	expression.
	\return  TRUE if the variables were deleted; otherwise FALSE. */
	DllExport BOOL		deleteAllVars();
	/*! \remarks Deletes the variable whose name is passed from the list
	maintained by the expression. Register numbers never get reassigned, even
	if a variable gets deleted. For example, if you delete variables 0-9, and
	keep variable 10, you're going to need to pass in an array of size at least
	11 to the <b>eval()</b> method, even though the first 10 slots are unused.
	\par Parameters:
	<b>MCHAR *name</b>\n\n
	The name of the variable to delete.
	\return  TRUE if the variable was deleted; otherwise FALSE (the name was
	not found). */
	DllExport BOOL		deleteVar(MCHAR *name);

// pseudo-private: (only to be used by the "instruction" functions
	void		setExprType(int type)	{ exprType = type; }
	void		pushInst(ExprFunc fn, float f) 
					{ if(instStk >= inst.Count()) inst.SetCount(instStk+30); 
					inst[instStk].func = fn; inst[instStk++].sVal = f; }
	void		pushSVal(float f)	{ if(sValStk>=sVal.Count())sVal.SetCount(sValStk+10);sVal[sValStk++]=f; }
	float		popSVal()			{ return sVal[--sValStk]; }
	void		pushVVal(Point3 &v)	{ if(vValStk>=vVal.Count())vVal.SetCount(vValStk+10);vVal[vValStk++]=v; }
	Point3 &	popVVal()			{ return vVal[--vValStk]; }
	int			getSRegCt(void)		{ return sRegCt; }
	float		getSReg(int index)	{ return sRegPtr[index]; }
	int			getVRegCt(void)		{ return vRegCt; }
	Point3 &	getVReg(int index)	{ return vRegPtr[index]; }

	ExprVarTab	vars;			// named variables
private:
	MCHAR *		exprPtr;		// pointer to current str pos during parsing
	MCHAR *		exprStr;		// ptr to original expression string to parse
	MSTR		origStr;		// original expression string that was loaded
	MSTR		progressStr;	// string to hold part of expr successfully parsed
	int			sRegCt;			// actual number of scalar registers passed to "eval"
	float		*sRegPtr;		// pointer to the scalar register array
	int			vRegCt;			// actual number of vector registers passed to "eval"
	Point3		*vRegPtr;		// pointer to the vector register array
	int			exprType;		// expression type: SCALAR_EXPR or VECTOR_EXPR (set by load)

	int			sValStk;		// scalar value stack
	floatTab	sVal;
	int			vValStk;		// vector value stack
	Point3Tab	vVal;
	int			instStk;		// instruction stack
	InstTab		inst;

	int			nextScalar;		// next scalar slot
	int			nextVector;		// next vector slot

	friend		int yylex();
	friend		int yyerror(char *);
};

/*! \defgroup expressionReturnCodes Expression Return Codes */
//@{
#define EXPR_NORMAL			 0	//!< No problems, expression evaluated successfully.
#define EXPR_INST_OVERFLOW	-1	//!< Instruction stack overflow during parsing.
#define EXPR_UNKNOWN_TOKEN	-2  //!< Unknown function, const, or reg during parsing.
#define EXPR_TOO_MANY_VARS	-3	//!< Value stack overflow.
#define EXPR_TOO_MANY_REGS	-4	//!< Register array overflow, or reg number too big.
#define EXPR_CANT_EVAL		-5	//!< Function can't be evaluated with given arguments.
#define EXPR_CANT_PARSE		-6	//!< Expression can't be parsed syntactically.
//@}

