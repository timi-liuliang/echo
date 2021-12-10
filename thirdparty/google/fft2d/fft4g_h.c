/*
Fast Fourier/Cosine/Sine Transform
    dimension   :one
    data length :power of 2
    decimation  :frequency
    radix       :4, 2
    data        :inplace
    table       :not use
functions
    cdft: Complex Discrete Fourier Transform
    rdft: Real Discrete Fourier Transform
    ddct: Discrete Cosine Transform
    ddst: Discrete Sine Transform
    dfct: Cosine Transform of RDFT (Real Symmetric DFT)
    dfst: Sine Transform of RDFT (Real Anti-symmetric DFT)
function prototypes
    void cdft(int, int, double *);
    void rdft(int, int, double *);
    void ddct(int, int, double *);
    void ddst(int, int, double *);
    void dfct(int, double *);
    void dfst(int, double *);


-------- Complex DFT (Discrete Fourier Transform) --------
    [definition]
        <case1>
            X[k] = sum_j=0^n-1 x[j]*exp(2*pi*i*j*k/n), 0<=k<n
        <case2>
            X[k] = sum_j=0^n-1 x[j]*exp(-2*pi*i*j*k/n), 0<=k<n
        (notes: sum_j=0^n-1 is a summation from j=0 to n-1)
    [usage]
        <case1>
            cdft(2*n, 1, a);
        <case2>
            cdft(2*n, -1, a);
    [parameters]
        2*n            :data length (int)
                        n >= 1, n = power of 2
        a[0...2*n-1]   :input/output data (double *)
                        input data
                            a[2*j] = Re(x[j]), 
                            a[2*j+1] = Im(x[j]), 0<=j<n
                        output data
                            a[2*k] = Re(X[k]), 
                            a[2*k+1] = Im(X[k]), 0<=k<n
    [remark]
        Inverse of 
            cdft(2*n, -1, a);
        is 
            cdft(2*n, 1, a);
            for (j = 0; j <= 2 * n - 1; j++) {
                a[j] *= 1.0 / n;
            }
        .


-------- Real DFT / Inverse of Real DFT --------
    [definition]
        <case1> RDFT
            R[k] = sum_j=0^n-1 a[j]*cos(2*pi*j*k/n), 0<=k<=n/2
            I[k] = sum_j=0^n-1 a[j]*sin(2*pi*j*k/n), 0<k<n/2
        <case2> IRDFT (excluding scale)
            a[k] = (R[0] + R[n/2]*cos(pi*k))/2 + 
                   sum_j=1^n/2-1 R[j]*cos(2*pi*j*k/n) + 
                   sum_j=1^n/2-1 I[j]*sin(2*pi*j*k/n), 0<=k<n
    [usage]
        <case1>
            rdft(n, 1, a);
        <case2>
            rdft(n, -1, a);
    [parameters]
        n              :data length (int)
                        n >= 2, n = power of 2
        a[0...n-1]     :input/output data (double *)
                        <case1>
                            output data
                                a[2*k] = R[k], 0<=k<n/2
                                a[2*k+1] = I[k], 0<k<n/2
                                a[1] = R[n/2]
                        <case2>
                            input data
                                a[2*j] = R[j], 0<=j<n/2
                                a[2*j+1] = I[j], 0<j<n/2
                                a[1] = R[n/2]
    [remark]
        Inverse of 
            rdft(n, 1, a);
        is 
            rdft(n, -1, a);
            for (j = 0; j <= n - 1; j++) {
                a[j] *= 2.0 / n;
            }
        .


-------- DCT (Discrete Cosine Transform) / Inverse of DCT --------
    [definition]
        <case1> IDCT (excluding scale)
            C[k] = sum_j=0^n-1 a[j]*cos(pi*j*(k+1/2)/n), 0<=k<n
        <case2> DCT
            C[k] = sum_j=0^n-1 a[j]*cos(pi*(j+1/2)*k/n), 0<=k<n
    [usage]
        <case1>
            ddct(n, 1, a);
        <case2>
            ddct(n, -1, a);
    [parameters]
        n              :data length (int)
                        n >= 2, n = power of 2
        a[0...n-1]     :input/output data (double *)
                        output data
                            a[k] = C[k], 0<=k<n
    [remark]
        Inverse of 
            ddct(n, -1, a);
        is 
            a[0] *= 0.5;
            ddct(n, 1, a);
            for (j = 0; j <= n - 1; j++) {
                a[j] *= 2.0 / n;
            }
        .


-------- DST (Discrete Sine Transform) / Inverse of DST --------
    [definition]
        <case1> IDST (excluding scale)
            S[k] = sum_j=1^n A[j]*sin(pi*j*(k+1/2)/n), 0<=k<n
        <case2> DST
            S[k] = sum_j=0^n-1 a[j]*sin(pi*(j+1/2)*k/n), 0<k<=n
    [usage]
        <case1>
            ddst(n, 1, a);
        <case2>
            ddst(n, -1, a);
    [parameters]
        n              :data length (int)
                        n >= 2, n = power of 2
        a[0...n-1]     :input/output data (double *)
                        <case1>
                            input data
                                a[j] = A[j], 0<j<n
                                a[0] = A[n]
                            output data
                                a[k] = S[k], 0<=k<n
                        <case2>
                            output data
                                a[k] = S[k], 0<k<n
                                a[0] = S[n]
    [remark]
        Inverse of 
            ddst(n, -1, a);
        is 
            a[0] *= 0.5;
            ddst(n, 1, a);
            for (j = 0; j <= n - 1; j++) {
                a[j] *= 2.0 / n;
            }
        .


-------- Cosine Transform of RDFT (Real Symmetric DFT) --------
    [definition]
        C[k] = sum_j=0^n a[j]*cos(pi*j*k/n), 0<=k<=n
    [usage]
        dfct(n, a);
    [parameters]
        n              :data length - 1 (int)
                        n >= 2, n = power of 2
        a[0...n]       :input/output data (double *)
                        output data
                            a[k] = C[k], 0<=k<=n
    [remark]
        Inverse of 
            a[0] *= 0.5;
            a[n] *= 0.5;
            dfct(n, a);
        is 
            a[0] *= 0.5;
            a[n] *= 0.5;
            dfct(n, a);
            for (j = 0; j <= n; j++) {
                a[j] *= 2.0 / n;
            }
        .


-------- Sine Transform of RDFT (Real Anti-symmetric DFT) --------
    [definition]
        S[k] = sum_j=1^n-1 a[j]*sin(pi*j*k/n), 0<k<n
    [usage]
        dfst(n, a);
    [parameters]
        n              :data length + 1 (int)
                        n >= 2, n = power of 2
        a[0...n-1]     :input/output data (double *)
                        output data
                            a[k] = S[k], 0<k<n
                        (a[0] is used for work area)
    [remark]
        Inverse of 
            dfst(n, a);
        is 
            dfst(n, a);
            for (j = 1; j <= n - 1; j++) {
                a[j] *= 2.0 / n;
            }
        .
*/


void cdft(int n, int isgn, double *a)
{
    void bitrv2(int n, double *a);
    void bitrv2conj(int n, double *a);
    void cftfsub(int n, double *a);
    void cftbsub(int n, double *a);
    
    if (n > 4) {
        if (isgn >= 0) {
            bitrv2(n, a);
            cftfsub(n, a);
        } else {
            bitrv2conj(n, a);
            cftbsub(n, a);
        }
    } else if (n == 4) {
        cftfsub(n, a);
    }
}


void rdft(int n, int isgn, double *a)
{
    void bitrv2(int n, double *a);
    void cftfsub(int n, double *a);
    void cftbsub(int n, double *a);
    void rftfsub(int n, double *a);
    void rftbsub(int n, double *a);
    double xi;
    
    if (isgn >= 0) {
        if (n > 4) {
            bitrv2(n, a);
            cftfsub(n, a);
            rftfsub(n, a);
        } else if (n == 4) {
            cftfsub(n, a);
        }
        xi = a[0] - a[1];
        a[0] += a[1];
        a[1] = xi;
    } else {
        a[1] = 0.5 * (a[0] - a[1]);
        a[0] -= a[1];
        if (n > 4) {
            rftbsub(n, a);
            bitrv2(n, a);
            cftbsub(n, a);
        } else if (n == 4) {
            cftfsub(n, a);
        }
    }
}


void ddct(int n, int isgn, double *a)
{
    void bitrv2(int n, double *a);
    void cftfsub(int n, double *a);
    void cftbsub(int n, double *a);
    void rftfsub(int n, double *a);
    void rftbsub(int n, double *a);
    void dctsub(int n, double *a);
    void dctsub4(int n, double *a);
    int j;
    double xr;
    
    if (isgn < 0) {
        xr = a[n - 1];
        for (j = n - 2; j >= 2; j -= 2) {
            a[j + 1] = a[j] - a[j - 1];
            a[j] += a[j - 1];
        }
        a[1] = a[0] - xr;
        a[0] += xr;
        if (n > 4) {
            rftbsub(n, a);
            bitrv2(n, a);
            cftbsub(n, a);
        } else if (n == 4) {
            cftfsub(n, a);
        }
    }
    if (n > 4) {
        dctsub(n, a);
    } else {
        dctsub4(n, a);
    }
    if (isgn >= 0) {
        if (n > 4) {
            bitrv2(n, a);
            cftfsub(n, a);
            rftfsub(n, a);
        } else if (n == 4) {
            cftfsub(n, a);
        }
        xr = a[0] - a[1];
        a[0] += a[1];
        for (j = 2; j < n; j += 2) {
            a[j - 1] = a[j] - a[j + 1];
            a[j] += a[j + 1];
        }
        a[n - 1] = xr;
    }
}


void ddst(int n, int isgn, double *a)
{
    void bitrv2(int n, double *a);
    void cftfsub(int n, double *a);
    void cftbsub(int n, double *a);
    void rftfsub(int n, double *a);
    void rftbsub(int n, double *a);
    void dstsub(int n, double *a);
    void dstsub4(int n, double *a);
    int j;
    double xr;
    
    if (isgn < 0) {
        xr = a[n - 1];
        for (j = n - 2; j >= 2; j -= 2) {
            a[j + 1] = -a[j] - a[j - 1];
            a[j] -= a[j - 1];
        }
        a[1] = a[0] + xr;
        a[0] -= xr;
        if (n > 4) {
            rftbsub(n, a);
            bitrv2(n, a);
            cftbsub(n, a);
        } else if (n == 4) {
            cftfsub(n, a);
        }
    }
    if (n > 4) {
        dstsub(n, a);
    } else {
        dstsub4(n, a);
    }
    if (isgn >= 0) {
        if (n > 4) {
            bitrv2(n, a);
            cftfsub(n, a);
            rftfsub(n, a);
        } else if (n == 4) {
            cftfsub(n, a);
        }
        xr = a[0] - a[1];
        a[0] += a[1];
        for (j = 2; j < n; j += 2) {
            a[j - 1] = -a[j] - a[j + 1];
            a[j] -= a[j + 1];
        }
        a[n - 1] = -xr;
    }
}


void dfct(int n, double *a)
{
    void ddct(int n, int isgn, double *a);
    void bitrv1(int n, double *a);
    int j, k, m, mh;
    double xr, xi, yr, yi, an;
    
    m = n >> 1;
    for (j = 0; j < m; j++) {
        k = n - j;
        xr = a[j] + a[k];
        a[j] -= a[k];
        a[k] = xr;
    }
    an = a[n];
    while (m >= 2) {
        ddct(m, 1, a);
        bitrv1(m, a);
        mh = m >> 1;
        xi = a[m];
        a[m] = a[0];
        a[0] = an - xi;
        an += xi;
        for (j = 1; j < mh; j++) {
            k = m - j;
            xr = a[m + k];
            xi = a[m + j];
            yr = a[j];
            yi = a[k];
            a[m + j] = yr;
            a[m + k] = yi;
            a[j] = xr - xi;
            a[k] = xr + xi;
        }
        xr = a[mh];
        a[mh] = a[m + mh];
        a[m + mh] = xr;
        m = mh;
    }
    xi = a[1];
    a[1] = a[0];
    a[0] = an + xi;
    a[n] = an - xi;
    bitrv1(n, a);
}


void dfst(int n, double *a)
{
    void ddst(int n, int isgn, double *a);
    void bitrv1(int n, double *a);
    int j, k, m, mh;
    double xr, xi, yr, yi;
    
    m = n >> 1;
    for (j = 1; j < m; j++) {
        k = n - j;
        xr = a[j] - a[k];
        a[j] += a[k];
        a[k] = xr;
    }
    a[0] = a[m];
    while (m >= 2) {
        ddst(m, 1, a);
        bitrv1(m, a);
        mh = m >> 1;
        for (j = 1; j < mh; j++) {
            k = m - j;
            xr = a[m + k];
            xi = a[m + j];
            yr = a[j];
            yi = a[k];
            a[m + j] = yr;
            a[m + k] = yi;
            a[j] = xr + xi;
            a[k] = xr - xi;
        }
        a[m] = a[0];
        a[0] = a[m + mh];
        a[m + mh] = a[mh];
        m = mh;
    }
    a[1] = a[0];
    a[0] = 0;
    bitrv1(n, a);
}


/* -------- child routines -------- */


#include <math.h>
#ifndef M_PI_2
#define M_PI_2      1.570796326794896619231321691639751442098584699687
#endif
#ifndef WR5000  /* cos(M_PI_2*0.5000) */
#define WR5000      0.707106781186547524400844362104849039284835937688
#endif
#ifndef WR2500  /* cos(M_PI_2*0.2500) */
#define WR2500      0.923879532511286756128183189396788286822416625863
#endif
#ifndef WI2500  /* sin(M_PI_2*0.2500) */
#define WI2500      0.382683432365089771728459984030398866761344562485
#endif


#ifndef RDFT_LOOP_DIV  /* control of the RDFT's speed & tolerance */
#define RDFT_LOOP_DIV 64
#endif

#ifndef DCST_LOOP_DIV  /* control of the DCT,DST's speed & tolerance */
#define DCST_LOOP_DIV 64
#endif


void bitrv2(int n, double *a)
{
    int j0, k0, j1, k1, l, m, i, j, k;
    double xr, xi, yr, yi;
    
    l = n >> 2;
    m = 2;
    while (m < l) {
        l >>= 1;
        m <<= 1;
    }
    if (m == l) {
        j0 = 0;
        for (k0 = 0; k0 < m; k0 += 2) {
            k = k0;
            for (j = j0; j < j0 + k0; j += 2) {
                xr = a[j];
                xi = a[j + 1];
                yr = a[k];
                yi = a[k + 1];
                a[j] = yr;
                a[j + 1] = yi;
                a[k] = xr;
                a[k + 1] = xi;
                j1 = j + m;
                k1 = k + 2 * m;
                xr = a[j1];
                xi = a[j1 + 1];
                yr = a[k1];
                yi = a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                j1 += m;
                k1 -= m;
                xr = a[j1];
                xi = a[j1 + 1];
                yr = a[k1];
                yi = a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                j1 += m;
                k1 += 2 * m;
                xr = a[j1];
                xi = a[j1 + 1];
                yr = a[k1];
                yi = a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                for (i = n >> 1; i > (k ^= i); i >>= 1);
            }
            j1 = j0 + k0 + m;
            k1 = j1 + m;
            xr = a[j1];
            xi = a[j1 + 1];
            yr = a[k1];
            yi = a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
            for (i = n >> 1; i > (j0 ^= i); i >>= 1);
        }
    } else {
        j0 = 0;
        for (k0 = 2; k0 < m; k0 += 2) {
            for (i = n >> 1; i > (j0 ^= i); i >>= 1);
            k = k0;
            for (j = j0; j < j0 + k0; j += 2) {
                xr = a[j];
                xi = a[j + 1];
                yr = a[k];
                yi = a[k + 1];
                a[j] = yr;
                a[j + 1] = yi;
                a[k] = xr;
                a[k + 1] = xi;
                j1 = j + m;
                k1 = k + m;
                xr = a[j1];
                xi = a[j1 + 1];
                yr = a[k1];
                yi = a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                for (i = n >> 1; i > (k ^= i); i >>= 1);
            }
        }
    }
}


void bitrv2conj(int n, double *a)
{
    int j0, k0, j1, k1, l, m, i, j, k;
    double xr, xi, yr, yi;
    
    l = n >> 2;
    m = 2;
    while (m < l) {
        l >>= 1;
        m <<= 1;
    }
    if (m == l) {
        j0 = 0;
        for (k0 = 0; k0 < m; k0 += 2) {
            k = k0;
            for (j = j0; j < j0 + k0; j += 2) {
                xr = a[j];
                xi = -a[j + 1];
                yr = a[k];
                yi = -a[k + 1];
                a[j] = yr;
                a[j + 1] = yi;
                a[k] = xr;
                a[k + 1] = xi;
                j1 = j + m;
                k1 = k + 2 * m;
                xr = a[j1];
                xi = -a[j1 + 1];
                yr = a[k1];
                yi = -a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                j1 += m;
                k1 -= m;
                xr = a[j1];
                xi = -a[j1 + 1];
                yr = a[k1];
                yi = -a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                j1 += m;
                k1 += 2 * m;
                xr = a[j1];
                xi = -a[j1 + 1];
                yr = a[k1];
                yi = -a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                for (i = n >> 1; i > (k ^= i); i >>= 1);
            }
            k1 = j0 + k0;
            a[k1 + 1] = -a[k1 + 1];
            j1 = k1 + m;
            k1 = j1 + m;
            xr = a[j1];
            xi = -a[j1 + 1];
            yr = a[k1];
            yi = -a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
            k1 += m;
            a[k1 + 1] = -a[k1 + 1];
            for (i = n >> 1; i > (j0 ^= i); i >>= 1);
        }
    } else {
        a[1] = -a[1];
        a[m + 1] = -a[m + 1];
        j0 = 0;
        for (k0 = 2; k0 < m; k0 += 2) {
            for (i = n >> 1; i > (j0 ^= i); i >>= 1);
            k = k0;
            for (j = j0; j < j0 + k0; j += 2) {
                xr = a[j];
                xi = -a[j + 1];
                yr = a[k];
                yi = -a[k + 1];
                a[j] = yr;
                a[j + 1] = yi;
                a[k] = xr;
                a[k + 1] = xi;
                j1 = j + m;
                k1 = k + m;
                xr = a[j1];
                xi = -a[j1 + 1];
                yr = a[k1];
                yi = -a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                for (i = n >> 1; i > (k ^= i); i >>= 1);
            }
            k1 = j0 + k0;
            a[k1 + 1] = -a[k1 + 1];
            a[k1 + m + 1] = -a[k1 + m + 1];
        }
    }
}


void bitrv1(int n, double *a)
{
    int j0, k0, j1, k1, l, m, i, j, k;
    double x;
    
    l = n >> 2;
    m = 1;
    while (m < l) {
        l >>= 1;
        m <<= 1;
    }
    if (m == l) {
        j0 = 0;
        for (k0 = 0; k0 < m; k0++) {
            k = k0;
            for (j = j0; j < j0 + k0; j++) {
                x = a[j];
                a[j] = a[k];
                a[k] = x;
                j1 = j + m;
                k1 = k + 2 * m;
                x = a[j1];
                a[j1] = a[k1];
                a[k1] = x;
                j1 += m;
                k1 -= m;
                x = a[j1];
                a[j1] = a[k1];
                a[k1] = x;
                j1 += m;
                k1 += 2 * m;
                x = a[j1];
                a[j1] = a[k1];
                a[k1] = x;
                for (i = n >> 1; i > (k ^= i); i >>= 1);
            }
            j1 = j0 + k0 + m;
            k1 = j1 + m;
            x = a[j1];
            a[j1] = a[k1];
            a[k1] = x;
            for (i = n >> 1; i > (j0 ^= i); i >>= 1);
        }
    } else {
        j0 = 0;
        for (k0 = 1; k0 < m; k0++) {
            for (i = n >> 1; i > (j0 ^= i); i >>= 1);
            k = k0;
            for (j = j0; j < j0 + k0; j++) {
                x = a[j];
                a[j] = a[k];
                a[k] = x;
                j1 = j + m;
                k1 = k + m;
                x = a[j1];
                a[j1] = a[k1];
                a[k1] = x;
                for (i = n >> 1; i > (k ^= i); i >>= 1);
            }
        }
    }
}


void cftfsub(int n, double *a)
{
    void cft1st(int n, double *a);
    void cftmdl(int n, int l, double *a);
    int j, j1, j2, j3, l;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
    
    l = 2;
    if (n > 8) {
        cft1st(n, a);
        l = 8;
        while ((l << 2) < n) {
            cftmdl(n, l, a);
            l <<= 2;
        }
    }
    if ((l << 2) == n) {
        for (j = 0; j < l; j += 2) {
            j1 = j + l;
            j2 = j1 + l;
            j3 = j2 + l;
            x0r = a[j] + a[j1];
            x0i = a[j + 1] + a[j1 + 1];
            x1r = a[j] - a[j1];
            x1i = a[j + 1] - a[j1 + 1];
            x2r = a[j2] + a[j3];
            x2i = a[j2 + 1] + a[j3 + 1];
            x3r = a[j2] - a[j3];
            x3i = a[j2 + 1] - a[j3 + 1];
            a[j] = x0r + x2r;
            a[j + 1] = x0i + x2i;
            a[j2] = x0r - x2r;
            a[j2 + 1] = x0i - x2i;
            a[j1] = x1r - x3i;
            a[j1 + 1] = x1i + x3r;
            a[j3] = x1r + x3i;
            a[j3 + 1] = x1i - x3r;
        }
    } else {
        for (j = 0; j < l; j += 2) {
            j1 = j + l;
            x0r = a[j] - a[j1];
            x0i = a[j + 1] - a[j1 + 1];
            a[j] += a[j1];
            a[j + 1] += a[j1 + 1];
            a[j1] = x0r;
            a[j1 + 1] = x0i;
        }
    }
}


void cftbsub(int n, double *a)
{
    void cft1st(int n, double *a);
    void cftmdl(int n, int l, double *a);
    int j, j1, j2, j3, l;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
    
    l = 2;
    if (n > 8) {
        cft1st(n, a);
        l = 8;
        while ((l << 2) < n) {
            cftmdl(n, l, a);
            l <<= 2;
        }
    }
    if ((l << 2) == n) {
        for (j = 0; j < l; j += 2) {
            j1 = j + l;
            j2 = j1 + l;
            j3 = j2 + l;
            x0r = a[j] + a[j1];
            x0i = -a[j + 1] - a[j1 + 1];
            x1r = a[j] - a[j1];
            x1i = -a[j + 1] + a[j1 + 1];
            x2r = a[j2] + a[j3];
            x2i = a[j2 + 1] + a[j3 + 1];
            x3r = a[j2] - a[j3];
            x3i = a[j2 + 1] - a[j3 + 1];
            a[j] = x0r + x2r;
            a[j + 1] = x0i - x2i;
            a[j2] = x0r - x2r;
            a[j2 + 1] = x0i + x2i;
            a[j1] = x1r - x3i;
            a[j1 + 1] = x1i - x3r;
            a[j3] = x1r + x3i;
            a[j3 + 1] = x1i + x3r;
        }
    } else {
        for (j = 0; j < l; j += 2) {
            j1 = j + l;
            x0r = a[j] - a[j1];
            x0i = -a[j + 1] + a[j1 + 1];
            a[j] += a[j1];
            a[j + 1] = -a[j + 1] - a[j1 + 1];
            a[j1] = x0r;
            a[j1 + 1] = x0i;
        }
    }
}


void cft1st(int n, double *a)
{
    int j, kj, kr;
    double ew, wn4r, wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
    
    x0r = a[0] + a[2];
    x0i = a[1] + a[3];
    x1r = a[0] - a[2];
    x1i = a[1] - a[3];
    x2r = a[4] + a[6];
    x2i = a[5] + a[7];
    x3r = a[4] - a[6];
    x3i = a[5] - a[7];
    a[0] = x0r + x2r;
    a[1] = x0i + x2i;
    a[4] = x0r - x2r;
    a[5] = x0i - x2i;
    a[2] = x1r - x3i;
    a[3] = x1i + x3r;
    a[6] = x1r + x3i;
    a[7] = x1i - x3r;
    wn4r = WR5000;
    x0r = a[8] + a[10];
    x0i = a[9] + a[11];
    x1r = a[8] - a[10];
    x1i = a[9] - a[11];
    x2r = a[12] + a[14];
    x2i = a[13] + a[15];
    x3r = a[12] - a[14];
    x3i = a[13] - a[15];
    a[8] = x0r + x2r;
    a[9] = x0i + x2i;
    a[12] = x2i - x0i;
    a[13] = x0r - x2r;
    x0r = x1r - x3i;
    x0i = x1i + x3r;
    a[10] = wn4r * (x0r - x0i);
    a[11] = wn4r * (x0r + x0i);
    x0r = x3i + x1r;
    x0i = x3r - x1i;
    a[14] = wn4r * (x0i - x0r);
    a[15] = wn4r * (x0i + x0r);
    ew = M_PI_2 / n;
    kr = 0;
    for (j = 16; j < n; j += 16) {
        for (kj = n >> 2; kj > (kr ^= kj); kj >>= 1);
        wk1r = cos(ew * kr);
        wk1i = sin(ew * kr);
        wk2r = 1 - 2 * wk1i * wk1i;
        wk2i = 2 * wk1i * wk1r;
        wk3r = wk1r - 2 * wk2i * wk1i;
        wk3i = 2 * wk2i * wk1r - wk1i;
        x0r = a[j] + a[j + 2];
        x0i = a[j + 1] + a[j + 3];
        x1r = a[j] - a[j + 2];
        x1i = a[j + 1] - a[j + 3];
        x2r = a[j + 4] + a[j + 6];
        x2i = a[j + 5] + a[j + 7];
        x3r = a[j + 4] - a[j + 6];
        x3i = a[j + 5] - a[j + 7];
        a[j] = x0r + x2r;
        a[j + 1] = x0i + x2i;
        x0r -= x2r;
        x0i -= x2i;
        a[j + 4] = wk2r * x0r - wk2i * x0i;
        a[j + 5] = wk2r * x0i + wk2i * x0r;
        x0r = x1r - x3i;
        x0i = x1i + x3r;
        a[j + 2] = wk1r * x0r - wk1i * x0i;
        a[j + 3] = wk1r * x0i + wk1i * x0r;
        x0r = x1r + x3i;
        x0i = x1i - x3r;
        a[j + 6] = wk3r * x0r - wk3i * x0i;
        a[j + 7] = wk3r * x0i + wk3i * x0r;
        x0r = wn4r * (wk1r - wk1i);
        wk1i = wn4r * (wk1r + wk1i);
        wk1r = x0r;
        wk3r = wk1r - 2 * wk2r * wk1i;
        wk3i = 2 * wk2r * wk1r - wk1i;
        x0r = a[j + 8] + a[j + 10];
        x0i = a[j + 9] + a[j + 11];
        x1r = a[j + 8] - a[j + 10];
        x1i = a[j + 9] - a[j + 11];
        x2r = a[j + 12] + a[j + 14];
        x2i = a[j + 13] + a[j + 15];
        x3r = a[j + 12] - a[j + 14];
        x3i = a[j + 13] - a[j + 15];
        a[j + 8] = x0r + x2r;
        a[j + 9] = x0i + x2i;
        x0r -= x2r;
        x0i -= x2i;
        a[j + 12] = -wk2i * x0r - wk2r * x0i;
        a[j + 13] = -wk2i * x0i + wk2r * x0r;
        x0r = x1r - x3i;
        x0i = x1i + x3r;
        a[j + 10] = wk1r * x0r - wk1i * x0i;
        a[j + 11] = wk1r * x0i + wk1i * x0r;
        x0r = x1r + x3i;
        x0i = x1i - x3r;
        a[j + 14] = wk3r * x0r - wk3i * x0i;
        a[j + 15] = wk3r * x0i + wk3i * x0r;
    }
}


void cftmdl(int n, int l, double *a)
{
    int j, j1, j2, j3, k, kj, kr, m, m2;
    double ew, wn4r, wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
    
    m = l << 2;
    for (j = 0; j < l; j += 2) {
        j1 = j + l;
        j2 = j1 + l;
        j3 = j2 + l;
        x0r = a[j] + a[j1];
        x0i = a[j + 1] + a[j1 + 1];
        x1r = a[j] - a[j1];
        x1i = a[j + 1] - a[j1 + 1];
        x2r = a[j2] + a[j3];
        x2i = a[j2 + 1] + a[j3 + 1];
        x3r = a[j2] - a[j3];
        x3i = a[j2 + 1] - a[j3 + 1];
        a[j] = x0r + x2r;
        a[j + 1] = x0i + x2i;
        a[j2] = x0r - x2r;
        a[j2 + 1] = x0i - x2i;
        a[j1] = x1r - x3i;
        a[j1 + 1] = x1i + x3r;
        a[j3] = x1r + x3i;
        a[j3 + 1] = x1i - x3r;
    }
    wn4r = WR5000;
    for (j = m; j < l + m; j += 2) {
        j1 = j + l;
        j2 = j1 + l;
        j3 = j2 + l;
        x0r = a[j] + a[j1];
        x0i = a[j + 1] + a[j1 + 1];
        x1r = a[j] - a[j1];
        x1i = a[j + 1] - a[j1 + 1];
        x2r = a[j2] + a[j3];
        x2i = a[j2 + 1] + a[j3 + 1];
        x3r = a[j2] - a[j3];
        x3i = a[j2 + 1] - a[j3 + 1];
        a[j] = x0r + x2r;
        a[j + 1] = x0i + x2i;
        a[j2] = x2i - x0i;
        a[j2 + 1] = x0r - x2r;
        x0r = x1r - x3i;
        x0i = x1i + x3r;
        a[j1] = wn4r * (x0r - x0i);
        a[j1 + 1] = wn4r * (x0r + x0i);
        x0r = x3i + x1r;
        x0i = x3r - x1i;
        a[j3] = wn4r * (x0i - x0r);
        a[j3 + 1] = wn4r * (x0i + x0r);
    }
    ew = M_PI_2 / n;
    kr = 0;
    m2 = 2 * m;
    for (k = m2; k < n; k += m2) {
        for (kj = n >> 2; kj > (kr ^= kj); kj >>= 1);
        wk1r = cos(ew * kr);
        wk1i = sin(ew * kr);
        wk2r = 1 - 2 * wk1i * wk1i;
        wk2i = 2 * wk1i * wk1r;
        wk3r = wk1r - 2 * wk2i * wk1i;
        wk3i = 2 * wk2i * wk1r - wk1i;
        for (j = k; j < l + k; j += 2) {
            j1 = j + l;
            j2 = j1 + l;
            j3 = j2 + l;
            x0r = a[j] + a[j1];
            x0i = a[j + 1] + a[j1 + 1];
            x1r = a[j] - a[j1];
            x1i = a[j + 1] - a[j1 + 1];
            x2r = a[j2] + a[j3];
            x2i = a[j2 + 1] + a[j3 + 1];
            x3r = a[j2] - a[j3];
            x3i = a[j2 + 1] - a[j3 + 1];
            a[j] = x0r + x2r;
            a[j + 1] = x0i + x2i;
            x0r -= x2r;
            x0i -= x2i;
            a[j2] = wk2r * x0r - wk2i * x0i;
            a[j2 + 1] = wk2r * x0i + wk2i * x0r;
            x0r = x1r - x3i;
            x0i = x1i + x3r;
            a[j1] = wk1r * x0r - wk1i * x0i;
            a[j1 + 1] = wk1r * x0i + wk1i * x0r;
            x0r = x1r + x3i;
            x0i = x1i - x3r;
            a[j3] = wk3r * x0r - wk3i * x0i;
            a[j3 + 1] = wk3r * x0i + wk3i * x0r;
        }
        x0r = wn4r * (wk1r - wk1i);
        wk1i = wn4r * (wk1r + wk1i);
        wk1r = x0r;
        wk3r = wk1r - 2 * wk2r * wk1i;
        wk3i = 2 * wk2r * wk1r - wk1i;
        for (j = k + m; j < l + (k + m); j += 2) {
            j1 = j + l;
            j2 = j1 + l;
            j3 = j2 + l;
            x0r = a[j] + a[j1];
            x0i = a[j + 1] + a[j1 + 1];
            x1r = a[j] - a[j1];
            x1i = a[j + 1] - a[j1 + 1];
            x2r = a[j2] + a[j3];
            x2i = a[j2 + 1] + a[j3 + 1];
            x3r = a[j2] - a[j3];
            x3i = a[j2 + 1] - a[j3 + 1];
            a[j] = x0r + x2r;
            a[j + 1] = x0i + x2i;
            x0r -= x2r;
            x0i -= x2i;
            a[j2] = -wk2i * x0r - wk2r * x0i;
            a[j2 + 1] = -wk2i * x0i + wk2r * x0r;
            x0r = x1r - x3i;
            x0i = x1i + x3r;
            a[j1] = wk1r * x0r - wk1i * x0i;
            a[j1 + 1] = wk1r * x0i + wk1i * x0r;
            x0r = x1r + x3i;
            x0i = x1i - x3r;
            a[j3] = wk3r * x0r - wk3i * x0i;
            a[j3 + 1] = wk3r * x0i + wk3i * x0r;
        }
    }
}


void rftfsub(int n, double *a)
{
    int i, i0, j, k;
    double ec, w1r, w1i, wkr, wki, wdr, wdi, ss, xr, xi, yr, yi;
    
    ec = 2 * M_PI_2 / n;
    wkr = 0;
    wki = 0;
    wdi = cos(ec);
    wdr = sin(ec);
    wdi *= wdr;
    wdr *= wdr;
    w1r = 1 - 2 * wdr;
    w1i = 2 * wdi;
    ss = 2 * w1i;
    i = n >> 1;
    for (;;) {
        i0 = i - 4 * RDFT_LOOP_DIV;
        if (i0 < 4) {
            i0 = 4;
        }
        for (j = i - 4; j >= i0; j -= 4) {
            k = n - j;
            xr = a[j + 2] - a[k - 2];
            xi = a[j + 3] + a[k - 1];
            yr = wdr * xr - wdi * xi;
            yi = wdr * xi + wdi * xr;
            a[j + 2] -= yr;
            a[j + 3] -= yi;
            a[k - 2] += yr;
            a[k - 1] -= yi;
            wkr += ss * wdi;
            wki += ss * (0.5 - wdr);
            xr = a[j] - a[k];
            xi = a[j + 1] + a[k + 1];
            yr = wkr * xr - wki * xi;
            yi = wkr * xi + wki * xr;
            a[j] -= yr;
            a[j + 1] -= yi;
            a[k] += yr;
            a[k + 1] -= yi;
            wdr += ss * wki;
            wdi += ss * (0.5 - wkr);
        }
        if (i0 == 4) {
            break;
        }
        wkr = 0.5 * sin(ec * i0);
        wki = 0.5 * cos(ec * i0);
        wdr = 0.5 - (wkr * w1r - wki * w1i);
        wdi = wkr * w1i + wki * w1r;
        wkr = 0.5 - wkr;
        i = i0;
    }
    xr = a[2] - a[n - 2];
    xi = a[3] + a[n - 1];
    yr = wdr * xr - wdi * xi;
    yi = wdr * xi + wdi * xr;
    a[2] -= yr;
    a[3] -= yi;
    a[n - 2] += yr;
    a[n - 1] -= yi;
}


void rftbsub(int n, double *a)
{
    int i, i0, j, k;
    double ec, w1r, w1i, wkr, wki, wdr, wdi, ss, xr, xi, yr, yi;
    
    ec = 2 * M_PI_2 / n;
    wkr = 0;
    wki = 0;
    wdi = cos(ec);
    wdr = sin(ec);
    wdi *= wdr;
    wdr *= wdr;
    w1r = 1 - 2 * wdr;
    w1i = 2 * wdi;
    ss = 2 * w1i;
    i = n >> 1;
    a[i + 1] = -a[i + 1];
    for (;;) {
        i0 = i - 4 * RDFT_LOOP_DIV;
        if (i0 < 4) {
            i0 = 4;
        }
        for (j = i - 4; j >= i0; j -= 4) {
            k = n - j;
            xr = a[j + 2] - a[k - 2];
            xi = a[j + 3] + a[k - 1];
            yr = wdr * xr + wdi * xi;
            yi = wdr * xi - wdi * xr;
            a[j + 2] -= yr;
            a[j + 3] = yi - a[j + 3];
            a[k - 2] += yr;
            a[k - 1] = yi - a[k - 1];
            wkr += ss * wdi;
            wki += ss * (0.5 - wdr);
            xr = a[j] - a[k];
            xi = a[j + 1] + a[k + 1];
            yr = wkr * xr + wki * xi;
            yi = wkr * xi - wki * xr;
            a[j] -= yr;
            a[j + 1] = yi - a[j + 1];
            a[k] += yr;
            a[k + 1] = yi - a[k + 1];
            wdr += ss * wki;
            wdi += ss * (0.5 - wkr);
        }
        if (i0 == 4) {
            break;
        }
        wkr = 0.5 * sin(ec * i0);
        wki = 0.5 * cos(ec * i0);
        wdr = 0.5 - (wkr * w1r - wki * w1i);
        wdi = wkr * w1i + wki * w1r;
        wkr = 0.5 - wkr;
        i = i0;
    }
    xr = a[2] - a[n - 2];
    xi = a[3] + a[n - 1];
    yr = wdr * xr + wdi * xi;
    yi = wdr * xi - wdi * xr;
    a[2] -= yr;
    a[3] = yi - a[3];
    a[n - 2] += yr;
    a[n - 1] = yi - a[n - 1];
    a[1] = -a[1];
}


void dctsub(int n, double *a)
{
    int i, i0, j, k, m;
    double ec, w1r, w1i, wkr, wki, wdr, wdi, ss, xr, xi, yr, yi;
    
    ec = M_PI_2 / n;
    wkr = 0.5;
    wki = 0.5;
    w1r = cos(ec);
    w1i = sin(ec);
    wdr = 0.5 * (w1r - w1i);
    wdi = 0.5 * (w1r + w1i);
    ss = 2 * w1i;
    m = n >> 1;
    i = 0;
    for (;;) {
        i0 = i + 2 * DCST_LOOP_DIV;
        if (i0 > m - 2) {
            i0 = m - 2;
        }
        for (j = i + 2; j <= i0; j += 2) {
            k = n - j;
            xr = wdi * a[j - 1] - wdr * a[k + 1];
            xi = wdr * a[j - 1] + wdi * a[k + 1];
            wkr -= ss * wdi;
            wki += ss * wdr;
            yr = wki * a[j] - wkr * a[k];
            yi = wkr * a[j] + wki * a[k];
            wdr -= ss * wki;
            wdi += ss * wkr;
            a[k + 1] = xr;
            a[k] = yr;
            a[j - 1] = xi;
            a[j] = yi;
        }
        if (i0 == m - 2) {
            break;
        }
        wdr = cos(ec * i0);
        wdi = sin(ec * i0);
        wkr = 0.5 * (wdr - wdi);
        wki = 0.5 * (wdr + wdi);
        wdr = wkr * w1r - wki * w1i;
        wdi = wkr * w1i + wki * w1r;
        i = i0;
    }
    xr = wdi * a[m - 1] - wdr * a[m + 1];
    a[m - 1] = wdr * a[m - 1] + wdi * a[m + 1];
    a[m + 1] = xr;
    a[m] *= wki + ss * wdr;
}


void dstsub(int n, double *a)
{
    int i, i0, j, k, m;
    double ec, w1r, w1i, wkr, wki, wdr, wdi, ss, xr, xi, yr, yi;
    
    ec = M_PI_2 / n;
    wkr = 0.5;
    wki = 0.5;
    w1r = cos(ec);
    w1i = sin(ec);
    wdr = 0.5 * (w1r - w1i);
    wdi = 0.5 * (w1r + w1i);
    ss = 2 * w1i;
    m = n >> 1;
    i = 0;
    for (;;) {
        i0 = i + 2 * DCST_LOOP_DIV;
        if (i0 > m - 2) {
            i0 = m - 2;
        }
        for (j = i + 2; j <= i0; j += 2) {
            k = n - j;
            xr = wdi * a[k + 1] - wdr * a[j - 1];
            xi = wdr * a[k + 1] + wdi * a[j - 1];
            wkr -= ss * wdi;
            wki += ss * wdr;
            yr = wki * a[k] - wkr * a[j];
            yi = wkr * a[k] + wki * a[j];
            wdr -= ss * wki;
            wdi += ss * wkr;
            a[j - 1] = xr;
            a[j] = yr;
            a[k + 1] = xi;
            a[k] = yi;
        }
        if (i0 == m - 2) {
            break;
        }
        wdr = cos(ec * i0);
        wdi = sin(ec * i0);
        wkr = 0.5 * (wdr - wdi);
        wki = 0.5 * (wdr + wdi);
        wdr = wkr * w1r - wki * w1i;
        wdi = wkr * w1i + wki * w1r;
        i = i0;
    }
    xr = wdi * a[m + 1] - wdr * a[m - 1];
    a[m + 1] = wdr * a[m + 1] + wdi * a[m - 1];
    a[m - 1] = xr;
    a[m] *= wki + ss * wdr;
}


void dctsub4(int n, double *a)
{
    int m;
    double wki, wdr, wdi, xr;
    
    wki = WR5000;
    m = n >> 1;
    if (m == 2) {
        wdr = wki * WI2500;
        wdi = wki * WR2500;
        xr = wdi * a[1] - wdr * a[3];
        a[1] = wdr * a[1] + wdi * a[3];
        a[3] = xr;
    }
    a[m] *= wki;
}


void dstsub4(int n, double *a)
{
    int m;
    double wki, wdr, wdi, xr;
    
    wki = WR5000;
    m = n >> 1;
    if (m == 2) {
        wdr = wki * WI2500;
        wdi = wki * WR2500;
        xr = wdi * a[3] - wdr * a[1];
        a[3] = wdr * a[3] + wdi * a[1];
        a[1] = xr;
    }
    a[m] *= wki;
}

