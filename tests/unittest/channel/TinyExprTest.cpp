#include <gtest/gtest.h>
#include <thirdparty/tinyexpr/tinyexpr.h>

TEST(TinyExpr, te_interp)
{
	int error;

	/* Returns 10. */
	double a = te_interp("(5+5)", 0); 
	EXPECT_EQ(a, 10);

	/* Returns 10, error is set to 0. */
	double b = te_interp("(5+5)", &error); 
	EXPECT_EQ(b, 10);
	EXPECT_EQ(error, 0);

	/* Returns NaN, error is set to 4. */
	double c = te_interp("(5+5", &error); 
	EXPECT_EQ(error, 4);
}

TEST(TinyExpr, te_compile)
{
	double x, y;
	/* Store variable names and pointers. */
	te_variable vars[] = { {"x", &x}, {"y", &y} };

	int err;
	/* Compile the expression with variables. */
	te_expr *expr = te_compile("sqrt(x^2+y^2)", vars, 2, &err);

	if (expr) 
	{
		x = 3; y = 4;
		const double h1 = te_eval(expr); /* Returns 5. */
		EXPECT_EQ(h1, 5);

		x = 5; y = 12;
		const double h2 = te_eval(expr); /* Returns 13. */
		EXPECT_EQ(h2, 13);

		te_free(expr);
	}
}

static double my_sum(double a, double b) 
{
	/* Example C function that adds two numbers together. */
	return a + b;
}

TEST(TinyExpr, BindingtoCustomFunctions)
{
	te_variable vars[] = 
	{
		{ "mysum", my_sum, TE_FUNCTION2} /* TE_FUNCTION2 used because my_sum takes two arguments. */
	};

	te_expr* expr = te_compile("mysum(5, 6)", vars, 1, 0);
	if (expr)
	{
		const double h2 = te_eval(expr); /* Returns 13. */
		EXPECT_EQ(h2, 11);

		te_free(expr);
	}
}