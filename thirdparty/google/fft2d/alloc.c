/* ---- memory allocation ---- */
#include "alloc.h"


#define alloc_error_check(p) { \
    if ((p) == NULL) { \
        fprintf(stderr, "Allocation Failure!\n"); \
        exit(1); \
    } \
}


int *alloc_1d_int(int n1)
{
    int *i;
    
    i = (int *) malloc(sizeof(int) * n1);
    alloc_error_check(i);
    return i;
}


void free_1d_int(int *i)
{
    free(i);
}


double *alloc_1d_double(int n1)
{
    double *d;
    
    d = (double *) malloc(sizeof(double) * n1);
    alloc_error_check(d);
    return d;
}


void free_1d_double(double *d)
{
    free(d);
}


int **alloc_2d_int(int n1, int n2)
{
    int **ii, *i;
    int j;
    
    ii = (int **) malloc(sizeof(int *) * n1);
    alloc_error_check(ii);
    i = (int *) malloc(sizeof(int) * n1 * n2);
    alloc_error_check(i);
    ii[0] = i;
    for (j = 1; j < n1; j++) {
        ii[j] = ii[j - 1] + n2;
    }
    return ii;
}


void free_2d_int(int **ii)
{
    free(ii[0]);
    free(ii);
}


double **alloc_2d_double(int n1, int n2)
{
    double **dd, *d;
    int j;
    
    dd = (double **) malloc(sizeof(double *) * n1);
    alloc_error_check(dd);
    d = (double *) malloc(sizeof(double) * n1 * n2);
    alloc_error_check(d);
    dd[0] = d;
    for (j = 1; j < n1; j++) {
        dd[j] = dd[j - 1] + n2;
    }
    return dd;
}


void free_2d_double(double **dd)
{
    free(dd[0]);
    free(dd);
}


int ***alloc_3d_int(int n1, int n2, int n3)
{
    int ***iii, **ii, *i;
    int j;
    
    iii = (int ***) malloc(sizeof(int **) * n1);
    alloc_error_check(iii);
    ii = (int **) malloc(sizeof(int *) * n1 * n2);
    alloc_error_check(ii);
    iii[0] = ii;
    for (j = 1; j < n1; j++) {
        iii[j] = iii[j - 1] + n2;
    }
    i = (int *) malloc(sizeof(int) * n1 * n2 * n3);
    alloc_error_check(i);
    ii[0] = i;
    for (j = 1; j < n1 * n2; j++) {
        ii[j] = ii[j - 1] + n3;
    }
    return iii;
}


void free_3d_int(int ***iii)
{
    free(iii[0][0]);
    free(iii[0]);
    free(iii);
}


double ***alloc_3d_double(int n1, int n2, int n3)
{
    double ***ddd, **dd, *d;
    int j;
    
    ddd = (double ***) malloc(sizeof(double **) * n1);
    alloc_error_check(ddd);
    dd = (double **) malloc(sizeof(double *) * n1 * n2);
    alloc_error_check(dd);
    ddd[0] = dd;
    for (j = 1; j < n1; j++) {
        ddd[j] = ddd[j - 1] + n2;
    }
    d = (double *) malloc(sizeof(double) * n1 * n2 * n3);
    alloc_error_check(d);
    dd[0] = d;
    for (j = 1; j < n1 * n2; j++) {
        dd[j] = dd[j - 1] + n3;
    }
    return ddd;
}


void free_3d_double(double ***ddd)
{
    free(ddd[0][0]);
    free(ddd[0]);
    free(ddd);
}

