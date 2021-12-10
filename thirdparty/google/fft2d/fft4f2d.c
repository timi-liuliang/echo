/*
Fast Fourier/Cosine/Sine Transform
    dimension   :two
    data length :power of 2
    decimation  :frequency
    radix       :4, 2, row-column
    data        :inplace
    table       :use
functions
    cdft2d: Complex Discrete Fourier Transform
    rdft2d: Real Discrete Fourier Transform
    ddct2d: Discrete Cosine Transform
    ddst2d: Discrete Sine Transform
function prototypes
    void cdft2d(int, int, int, double **, int *, double *);
    void rdft2d(int, int, int, double **, int *, double *);
    void ddct2d(int, int, int, double **, double **, int *, double *);
    void ddst2d(int, int, int, double **, double **, int *, double *);


-------- Complex DFT (Discrete Fourier Transform) --------
    [definition]
        <case1>
            X[k1][k2] = sum_j1=0^n1-1 sum_j2=0^n2-1 x[j1][j2] * 
                            exp(2*pi*i*j1*k1/n1) * 
                            exp(2*pi*i*j2*k2/n2), 0<=k1<n1, 0<=k2<n2
        <case2>
            X[k1][k2] = sum_j1=0^n1-1 sum_j2=0^n2-1 x[j1][j2] * 
                            exp(-2*pi*i*j1*k1/n1) * 
                            exp(-2*pi*i*j2*k2/n2), 0<=k1<n1, 0<=k2<n2
        (notes: sum_j=0^n-1 is a summation from j=0 to n-1)
    [usage]
        <case1>
            ip[0] = 0; // first time only
            cdft2d(n1, 2*n2, 1, a, ip, w);
        <case2>
            ip[0] = 0; // first time only
            cdft2d(n1, 2*n2, -1, a, ip, w);
    [parameters]
        n1     :data length (int)
                n1 >= 1, n1 = power of 2
        2*n2   :data length (int)
                n2 >= 1, n2 = power of 2
        a[0...n1-1][0...2*n2-1]
               :input/output data (double **)
                input data
                    a[j1][2*j2] = Re(x[j1][j2]), 
                    a[j1][2*j2+1] = Im(x[j1][j2]), 
                    0<=j1<n1, 0<=j2<n2
                output data
                    a[k1][2*k2] = Re(X[k1][k2]), 
                    a[k1][2*k2+1] = Im(X[k1][k2]), 
                    0<=k1<n1, 0<=k2<n2
        ip[0...*]
               :work area for bit reversal (int *)
                length of ip >= 2+sqrt(n)
                (n = max(n1, n2))
                ip[0],ip[1] are pointers of the cos/sin table.
        w[0...*]
               :cos/sin table (double *)
                length of w >= max(n1/2, n2/2)
                w[],ip[] are initialized if ip[0] == 0.
    [remark]
        Inverse of 
            cdft2d(n1, 2*n2, -1, a, ip, w);
        is 
            cdft2d(n1, 2*n2, 1, a, ip, w);
            for (j1 = 0; j1 <= n1 - 1; j1++) {
                for (j2 = 0; j2 <= 2 * n2 - 1; j2++) {
                    a[j1][j2] *= 1.0 / (n1 * n2);
                }
            }
        .


-------- Real DFT / Inverse of Real DFT --------
    [definition]
        <case1> RDFT
            R[k1][k2] = sum_j1=0^n1-1 sum_j2=0^n2-1 a[j1][j2] * 
                            cos(2*pi*j1*k1/n1 + 2*pi*j2*k2/n2), 
                            0<=k1<n1, 0<=k2<n2
            I[k1][k2] = sum_j1=0^n1-1 sum_j2=0^n2-1 a[j1][j2] * 
                            sin(2*pi*j1*k1/n1 + 2*pi*j2*k2/n2), 
                            0<=k1<n1, 0<=k2<n2
        <case2> IRDFT (excluding scale)
            a[k1][k2] = (1/2) * sum_j1=0^n1-1 sum_j2=0^n2-1
                            (R[j1][j2] * 
                            cos(2*pi*j1*k1/n1 + 2*pi*j2*k2/n2) + 
                            I[j1][j2] * 
                            sin(2*pi*j1*k1/n1 + 2*pi*j2*k2/n2)), 
                            0<=k1<n1, 0<=k2<n2
        (notes: R[n1-k1][n2-k2] = R[k1][k2], 
                I[n1-k1][n2-k2] = -I[k1][k2], 
                R[n1-k1][0] = R[k1][0], 
                I[n1-k1][0] = -I[k1][0], 
                R[0][n2-k2] = R[0][k2], 
                I[0][n2-k2] = -I[0][k2], 
                0<k1<n1, 0<k2<n2)
    [usage]
        <case1>
            ip[0] = 0; // first time only
            rdft2d(n1, n2, 1, a, ip, w);
        <case2>
            ip[0] = 0; // first time only
            rdft2d(n1, n2, -1, a, ip, w);
    [parameters]
        n1     :data length (int)
                n1 >= 2, n1 = power of 2
        n2     :data length (int)
                n2 >= 2, n2 = power of 2
        a[0...n1-1][0...n2-1]
               :input/output data (double **)
                <case1>
                    output data
                        a[k1][2*k2] = R[k1][k2] = R[n1-k1][n2-k2], 
                        a[k1][2*k2+1] = I[k1][k2] = -I[n1-k1][n2-k2], 
                            0<k1<n1, 0<k2<n2/2, 
                        a[0][2*k2] = R[0][k2] = R[0][n2-k2], 
                        a[0][2*k2+1] = I[0][k2] = -I[0][n2-k2], 
                            0<k2<n2/2, 
                        a[k1][0] = R[k1][0] = R[n1-k1][0], 
                        a[k1][1] = I[k1][0] = -I[n1-k1][0], 
                        a[n1-k1][1] = R[k1][n2/2] = R[n1-k1][n2/2], 
                        a[n1-k1][0] = -I[k1][n2/2] = I[n1-k1][n2/2], 
                            0<k1<n1/2, 
                        a[0][0] = R[0][0], 
                        a[0][1] = R[0][n2/2], 
                        a[n1/2][0] = R[n1/2][0], 
                        a[n1/2][1] = R[n1/2][n2/2]
                <case2>
                    input data
                        a[j1][2*j2] = R[j1][j2] = R[n1-j1][n2-j2], 
                        a[j1][2*j2+1] = I[j1][j2] = -I[n1-j1][n2-j2], 
                            0<j1<n1, 0<j2<n2/2, 
                        a[0][2*j2] = R[0][j2] = R[0][n2-j2], 
                        a[0][2*j2+1] = I[0][j2] = -I[0][n2-j2], 
                            0<j2<n2/2, 
                        a[j1][0] = R[j1][0] = R[n1-j1][0], 
                        a[j1][1] = I[j1][0] = -I[n1-j1][0], 
                        a[n1-j1][1] = R[j1][n2/2] = R[n1-j1][n2/2], 
                        a[n1-j1][0] = -I[j1][n2/2] = I[n1-j1][n2/2], 
                            0<j1<n1/2, 
                        a[0][0] = R[0][0], 
                        a[0][1] = R[0][n2/2], 
                        a[n1/2][0] = R[n1/2][0], 
                        a[n1/2][1] = R[n1/2][n2/2]
        ip[0...*]
               :work area for bit reversal (int *)
                length of ip >= 2+sqrt(n)
                (n = max(n1, n2/2))
                ip[0],ip[1] are pointers of the cos/sin table.
        w[0...*]
               :cos/sin table (double *)
                length of w >= max(n1/2, n2/4) + n2/4
                w[],ip[] are initialized if ip[0] == 0.
    [remark]
        Inverse of 
            rdft2d(n1, n2, 1, a, ip, w);
        is 
            rdft2d(n1, n2, -1, a, ip, w);
            for (j1 = 0; j1 <= n1 - 1; j1++) {
                for (j2 = 0; j2 <= n2 - 1; j2++) {
                    a[j1][j2] *= 2.0 / (n1 * n2);
                }
            }
        .


-------- DCT (Discrete Cosine Transform) / Inverse of DCT --------
    [definition]
        <case1> IDCT (excluding scale)
            C[k1][k2] = sum_j1=0^n1-1 sum_j2=0^n2-1 a[j1][j2] * 
                            cos(pi*j1*(k1+1/2)/n1) * 
                            cos(pi*j2*(k2+1/2)/n2), 
                            0<=k1<n1, 0<=k2<n2
        <case2> DCT
            C[k1][k2] = sum_j1=0^n1-1 sum_j2=0^n2-1 a[j1][j2] * 
                            cos(pi*(j1+1/2)*k1/n1) * 
                            cos(pi*(j2+1/2)*k2/n2), 
                            0<=k1<n1, 0<=k2<n2
    [usage]
        <case1>
            ip[0] = 0; // first time only
            ddct2d(n1, n2, 1, a, t, ip, w);
        <case2>
            ip[0] = 0; // first time only
            ddct2d(n1, n2, -1, a, t, ip, w);
    [parameters]
        n1     :data length (int)
                n1 >= 2, n1 = power of 2
        n2     :data length (int)
                n2 >= 2, n2 = power of 2
        a[0...n1-1][0...n2-1]
               :input/output data (double **)
                output data
                    a[k1][k2] = C[k1][k2], 0<=k1<n1, 0<=k2<n2
        t[0...n1-1][0...n2-1]
               :work area (double **)
        ip[0...*]
               :work area for bit reversal (int *)
                length of ip >= 2+sqrt(n)
                (n = max(n1, n2/2))
                ip[0],ip[1] are pointers of the cos/sin table.
        w[0...*]
               :cos/sin table (double *)
                length of w >= max(n1/2, n2/4) + max(n1, n2)
                w[],ip[] are initialized if ip[0] == 0.
    [remark]
        Inverse of 
            ddct2d(n1, n2, -1, a, t, ip, w);
        is 
            for (j1 = 0; j1 <= n1 - 1; j1++) {
                a[j1][0] *= 0.5;
            }
            for (j2 = 0; j2 <= n2 - 1; j2++) {
                a[0][j2] *= 0.5;
            }
            ddct2d(n1, n2, 1, a, t, ip, w);
            for (j1 = 0; j1 <= n1 - 1; j1++) {
                for (j2 = 0; j2 <= n2 - 1; j2++) {
                    a[j1][j2] *= 4.0 / (n1 * n2);
                }
            }
        .


-------- DST (Discrete Sine Transform) / Inverse of DST --------
    [definition]
        <case1> IDST (excluding scale)
            S[k1][k2] = sum_j1=1^n1 sum_j2=1^n2 A[j1][j2] * 
                            sin(pi*j1*(k1+1/2)/n1) * 
                            sin(pi*j2*(k2+1/2)/n2), 
                            0<=k1<n1, 0<=k2<n2
        <case2> DST
            S[k1][k2] = sum_j1=0^n1-1 sum_j2=0^n2-1 a[j1][j2] * 
                            sin(pi*(j1+1/2)*k1/n1) * 
                            sin(pi*(j2+1/2)*k2/n2), 
                            0<k1<=n1, 0<k2<=n2
    [usage]
        <case1>
            ip[0] = 0; // first time only
            ddst2d(n1, n2, 1, a, t, ip, w);
        <case2>
            ip[0] = 0; // first time only
            ddst2d(n1, n2, -1, a, t, ip, w);
    [parameters]
        n1     :data length (int)
                n1 >= 2, n1 = power of 2
        n2     :data length (int)
                n2 >= 2, n2 = power of 2
        a[0...n1-1][0...n2-1]
               :input/output data (double **)
                <case1>
                    input data
                        a[j1][j2] = A[j1][j2], 0<j1<n1, 0<j2<n2, 
                        a[j1][0] = A[j1][n2], 0<j1<n1, 
                        a[0][j2] = A[n1][j2], 0<j2<n2, 
                        a[0][0] = A[n1][n2]
                        (i.e. A[j1][j2] = a[j1 % n1][j2 % n2])
                    output data
                        a[k1][k2] = S[k1][k2], 0<=k1<n1, 0<=k2<n2
                <case2>
                    output data
                        a[k1][k2] = S[k1][k2], 0<k1<n1, 0<k2<n2, 
                        a[k1][0] = S[k1][n2], 0<k1<n1, 
                        a[0][k2] = S[n1][k2], 0<k2<n2, 
                        a[0][0] = S[n1][n2]
                        (i.e. S[k1][k2] = a[k1 % n1][k2 % n2])
        t[0...n1-1][0...n2-1]
               :work area (double **)
        ip[0...*]
               :work area for bit reversal (int *)
                length of ip >= 2+sqrt(n)
                (n = max(n1, n2/2))
                ip[0],ip[1] are pointers of the cos/sin table.
        w[0...*]
               :cos/sin table (double *)
                length of w >= max(n1/2, n2/4) + max(n1, n2)
                w[],ip[] are initialized if ip[0] == 0.
    [remark]
        Inverse of 
            ddst2d(n1, n2, -1, a, t, ip, w);
        is 
            for (j1 = 0; j1 <= n1 - 1; j1++) {
                a[j1][0] *= 0.5;
            }
            for (j2 = 0; j2 <= n2 - 1; j2++) {
                a[0][j2] *= 0.5;
            }
            ddst2d(n1, n2, 1, a, t, ip, w);
            for (j1 = 0; j1 <= n1 - 1; j1++) {
                for (j2 = 0; j2 <= n2 - 1; j2++) {
                    a[j1][j2] *= 4.0 / (n1 * n2);
                }
            }
        .
*/


void cdft2d(int n1, int n2, int isgn, double **a, int *ip, double *w)
{
    void makewt(int nw, int *ip, double *w);
    void bitrv2col(int n1, int n, int *ip, double **a);
    void bitrv2row(int n, int n2, int *ip, double **a);
    void cftbcol(int n1, int n, double **a, double *w);
    void cftbrow(int n, int n2, double **a, double *w);
    void cftfcol(int n1, int n, double **a, double *w);
    void cftfrow(int n, int n2, double **a, double *w);
    int n;
    
    n = n1 << 1;
    if (n < n2) {
        n = n2;
    }
    if (n > (ip[0] << 2)) {
        makewt(n >> 2, ip, w);
    }
    if (n2 > 4) {
        bitrv2col(n1, n2, ip + 2, a);
    }
    if (n1 > 2) {
        bitrv2row(n1, n2, ip + 2, a);
    }
    if (isgn < 0) {
        cftfcol(n1, n2, a, w);
        cftfrow(n1, n2, a, w);
    } else {
        cftbcol(n1, n2, a, w);
        cftbrow(n1, n2, a, w);
    }
}


void rdft2d(int n1, int n2, int isgn, double **a, int *ip, double *w)
{
    void makewt(int nw, int *ip, double *w);
    void makect(int nc, int *ip, double *c);
    void bitrv2col(int n1, int n, int *ip, double **a);
    void bitrv2row(int n, int n2, int *ip, double **a);
    void cftbcol(int n1, int n, double **a, double *w);
    void cftbrow(int n, int n2, double **a, double *w);
    void cftfcol(int n1, int n, double **a, double *w);
    void cftfrow(int n, int n2, double **a, double *w);
    void rftbcol(int n1, int n, double **a, int nc, double *c);
    void rftfcol(int n1, int n, double **a, int nc, double *c);
    int n, nw, nc, n1h, i, j;
    double xi;
    
    n = n1 << 1;
    if (n < n2) {
        n = n2;
    }
    nw = ip[0];
    if (n > (nw << 2)) {
        nw = n >> 2;
        makewt(nw, ip, w);
    }
    nc = ip[1];
    if (n2 > (nc << 2)) {
        nc = n2 >> 2;
        makect(nc, ip, w + nw);
    }
    n1h = n1 >> 1;
    if (isgn < 0) {
        for (i = 1; i <= n1h - 1; i++) {
            j = n1 - i;
            xi = a[i][0] - a[j][0];
            a[i][0] += a[j][0];
            a[j][0] = xi;
            xi = a[j][1] - a[i][1];
            a[i][1] += a[j][1];
            a[j][1] = xi;
        }
        if (n1 > 2) {
            bitrv2row(n1, n2, ip + 2, a);
        }
        cftfrow(n1, n2, a, w);
        for (i = 0; i <= n1 - 1; i++) {
            a[i][1] = 0.5 * (a[i][0] - a[i][1]);
            a[i][0] -= a[i][1];
        }
        if (n2 > 4) {
            rftfcol(n1, n2, a, nc, w + nw);
            bitrv2col(n1, n2, ip + 2, a);
        }
        cftfcol(n1, n2, a, w);
    } else {
        if (n2 > 4) {
            bitrv2col(n1, n2, ip + 2, a);
        }
        cftbcol(n1, n2, a, w);
        if (n2 > 4) {
            rftbcol(n1, n2, a, nc, w + nw);
        }
        for (i = 0; i <= n1 - 1; i++) {
            xi = a[i][0] - a[i][1];
            a[i][0] += a[i][1];
            a[i][1] = xi;
        }
        if (n1 > 2) {
            bitrv2row(n1, n2, ip + 2, a);
        }
        cftbrow(n1, n2, a, w);
        for (i = 1; i <= n1h - 1; i++) {
            j = n1 - i;
            a[j][0] = 0.5 * (a[i][0] - a[j][0]);
            a[i][0] -= a[j][0];
            a[j][1] = 0.5 * (a[i][1] + a[j][1]);
            a[i][1] -= a[j][1];
        }
    }
}


void ddct2d(int n1, int n2, int isgn, double **a, double **t, 
    int *ip, double *w)
{
    void makewt(int nw, int *ip, double *w);
    void makect(int nc, int *ip, double *c);
    void bitrv2col(int n1, int n, int *ip, double **a);
    void bitrv2row(int n, int n2, int *ip, double **a);
    void cftbcol(int n1, int n, double **a, double *w);
    void cftbrow(int n, int n2, double **a, double *w);
    void cftfcol(int n1, int n, double **a, double *w);
    void cftfrow(int n, int n2, double **a, double *w);
    void rftbcol(int n1, int n, double **a, int nc, double *c);
    void rftfcol(int n1, int n, double **a, int nc, double *c);
    void dctbsub(int n1, int n2, double **a, int nc, double *c);
    void dctfsub(int n1, int n2, double **a, int nc, double *c);
    int n, nw, nc, n1h, n2h, i, ix, ic, j, jx, jc;
    double xi;
    
    n = n1 << 1;
    if (n < n2) {
        n = n2;
    }
    nw = ip[0];
    if (n > (nw << 2)) {
        nw = n >> 2;
        makewt(nw, ip, w);
    }
    nc = ip[1];
    if (n1 > nc || n2 > nc) {
        if (n1 > n2) {
            nc = n1;
        } else {
            nc = n2;
        }
        makect(nc, ip, w + nw);
    }
    n1h = n1 >> 1;
    n2h = n2 >> 1;
    if (isgn >= 0) {
        for (i = 0; i <= n1 - 1; i++) {
            for (j = 1; j <= n2h - 1; j++) {
                jx = j << 1;
                t[i][jx] = a[i][j];
                t[i][jx + 1] = a[i][n2 - j];
            }
        }
        t[0][0] = a[0][0];
        t[0][1] = a[0][n2h];
        t[n1h][0] = a[n1h][0];
        t[n1h][1] = a[n1h][n2h];
        for (i = 1; i <= n1h - 1; i++) {
            ic = n1 - i;
            t[i][0] = a[i][0];
            t[ic][1] = a[i][n2h];
            t[i][1] = a[ic][0];
            t[ic][0] = a[ic][n2h];
        }
        dctfsub(n1, n2, t, nc, w + nw);
        if (n1 > 2) {
            bitrv2row(n1, n2, ip + 2, t);
        }
        cftfrow(n1, n2, t, w);
        for (i = 0; i <= n1 - 1; i++) {
            t[i][1] = 0.5 * (t[i][0] - t[i][1]);
            t[i][0] -= t[i][1];
        }
        if (n2 > 4) {
            rftfcol(n1, n2, t, nc, w + nw);
            bitrv2col(n1, n2, ip + 2, t);
        }
        cftfcol(n1, n2, t, w);
        for (i = 0; i <= n1h - 1; i++) {
            ix = i << 1;
            ic = n1 - 1 - i;
            for (j = 0; j <= n2h - 1; j++) {
                jx = j << 1;
                jc = n2 - 1 - j;
                a[ix][jx] = t[i][j];
                a[ix][jx + 1] = t[i][jc];
                a[ix + 1][jx] = t[ic][j];
                a[ix + 1][jx + 1] = t[ic][jc];
            }
        }
    } else {
        for (i = 0; i <= n1h - 1; i++) {
            ix = i << 1;
            ic = n1 - 1 - i;
            for (j = 0; j <= n2h - 1; j++) {
                jx = j << 1;
                jc = n2 - 1 - j;
                t[i][j] = a[ix][jx];
                t[i][jc] = a[ix][jx + 1];
                t[ic][j] = a[ix + 1][jx];
                t[ic][jc] = a[ix + 1][jx + 1];
            }
        }
        if (n2 > 4) {
            bitrv2col(n1, n2, ip + 2, t);
        }
        cftbcol(n1, n2, t, w);
        if (n2 > 4) {
            rftbcol(n1, n2, t, nc, w + nw);
        }
        for (i = 0; i <= n1 - 1; i++) {
            xi = t[i][0] - t[i][1];
            t[i][0] += t[i][1];
            t[i][1] = xi;
        }
        if (n1 > 2) {
            bitrv2row(n1, n2, ip + 2, t);
        }
        cftbrow(n1, n2, t, w);
        dctbsub(n1, n2, t, nc, w + nw);
        for (i = 0; i <= n1 - 1; i++) {
            for (j = 1; j <= n2h - 1; j++) {
                jx = j << 1;
                a[i][j] = t[i][jx];
                a[i][n2 - j] = t[i][jx + 1];
            }
        }
        a[0][0] = t[0][0];
        a[0][n2h] = t[0][1];
        a[n1h][0] = t[n1h][0];
        a[n1h][n2h] = t[n1h][1];
        for (i = 1; i <= n1h - 1; i++) {
            ic = n1 - i;
            a[i][0] = t[i][0];
            a[i][n2h] = t[ic][1];
            a[ic][0] = t[i][1];
            a[ic][n2h] = t[ic][0];
        }
    }
}


void ddst2d(int n1, int n2, int isgn, double **a, double **t, 
    int *ip, double *w)
{
    void makewt(int nw, int *ip, double *w);
    void makect(int nc, int *ip, double *c);
    void bitrv2col(int n1, int n, int *ip, double **a);
    void bitrv2row(int n, int n2, int *ip, double **a);
    void cftbcol(int n1, int n, double **a, double *w);
    void cftbrow(int n, int n2, double **a, double *w);
    void cftfcol(int n1, int n, double **a, double *w);
    void cftfrow(int n, int n2, double **a, double *w);
    void rftbcol(int n1, int n, double **a, int nc, double *c);
    void rftfcol(int n1, int n, double **a, int nc, double *c);
    void dstbsub(int n1, int n2, double **a, int nc, double *c);
    void dstfsub(int n1, int n2, double **a, int nc, double *c);
    int n, nw, nc, n1h, n2h, i, ix, ic, j, jx, jc;
    double xi;
    
    n = n1 << 1;
    if (n < n2) {
        n = n2;
    }
    nw = ip[0];
    if (n > (nw << 2)) {
        nw = n >> 2;
        makewt(nw, ip, w);
    }
    nc = ip[1];
    if (n1 > nc || n2 > nc) {
        if (n1 > n2) {
            nc = n1;
        } else {
            nc = n2;
        }
        makect(nc, ip, w + nw);
    }
    n1h = n1 >> 1;
    n2h = n2 >> 1;
    if (isgn >= 0) {
        for (i = 0; i <= n1 - 1; i++) {
            for (j = 1; j <= n2h - 1; j++) {
                jx = j << 1;
                t[i][jx] = a[i][j];
                t[i][jx + 1] = a[i][n2 - j];
            }
        }
        t[0][0] = a[0][0];
        t[0][1] = a[0][n2h];
        t[n1h][0] = a[n1h][0];
        t[n1h][1] = a[n1h][n2h];
        for (i = 1; i <= n1h - 1; i++) {
            ic = n1 - i;
            t[i][0] = a[i][0];
            t[ic][1] = a[i][n2h];
            t[i][1] = a[ic][0];
            t[ic][0] = a[ic][n2h];
        }
        dstfsub(n1, n2, t, nc, w + nw);
        if (n1 > 2) {
            bitrv2row(n1, n2, ip + 2, t);
        }
        cftfrow(n1, n2, t, w);
        for (i = 0; i <= n1 - 1; i++) {
            t[i][1] = 0.5 * (t[i][0] - t[i][1]);
            t[i][0] -= t[i][1];
        }
        if (n2 > 4) {
            rftfcol(n1, n2, t, nc, w + nw);
            bitrv2col(n1, n2, ip + 2, t);
        }
        cftfcol(n1, n2, t, w);
        for (i = 0; i <= n1h - 1; i++) {
            ix = i << 1;
            ic = n1 - 1 - i;
            for (j = 0; j <= n2h - 1; j++) {
                jx = j << 1;
                jc = n2 - 1 - j;
                a[ix][jx] = t[i][j];
                a[ix][jx + 1] = -t[i][jc];
                a[ix + 1][jx] = -t[ic][j];
                a[ix + 1][jx + 1] = t[ic][jc];
            }
        }
    } else {
        for (i = 0; i <= n1h - 1; i++) {
            ix = i << 1;
            ic = n1 - 1 - i;
            for (j = 0; j <= n2h - 1; j++) {
                jx = j << 1;
                jc = n2 - 1 - j;
                t[i][j] = a[ix][jx];
                t[i][jc] = -a[ix][jx + 1];
                t[ic][j] = -a[ix + 1][jx];
                t[ic][jc] = a[ix + 1][jx + 1];
            }
        }
        if (n2 > 4) {
            bitrv2col(n1, n2, ip + 2, t);
        }
        cftbcol(n1, n2, t, w);
        if (n2 > 4) {
            rftbcol(n1, n2, t, nc, w + nw);
        }
        for (i = 0; i <= n1 - 1; i++) {
            xi = t[i][0] - t[i][1];
            t[i][0] += t[i][1];
            t[i][1] = xi;
        }
        if (n1 > 2) {
            bitrv2row(n1, n2, ip + 2, t);
        }
        cftbrow(n1, n2, t, w);
        dstbsub(n1, n2, t, nc, w + nw);
        for (i = 0; i <= n1 - 1; i++) {
            for (j = 1; j <= n2h - 1; j++) {
                jx = j << 1;
                a[i][j] = t[i][jx];
                a[i][n2 - j] = t[i][jx + 1];
            }
        }
        a[0][0] = t[0][0];
        a[0][n2h] = t[0][1];
        a[n1h][0] = t[n1h][0];
        a[n1h][n2h] = t[n1h][1];
        for (i = 1; i <= n1h - 1; i++) {
            ic = n1 - i;
            a[i][0] = t[i][0];
            a[i][n2h] = t[ic][1];
            a[ic][0] = t[i][1];
            a[ic][n2h] = t[ic][0];
        }
    }
}


/* -------- initializing routines -------- */


#include <math.h>

void makewt(int nw, int *ip, double *w)
{
    void bitrv2(int n, int *ip, double *a);
    int nwh, j;
    double delta, x, y;
    
    ip[0] = nw;
    ip[1] = 1;
    if (nw > 2) {
        nwh = nw >> 1;
        delta = atan(1.0) / nwh;
        w[0] = 1;
        w[1] = 0;
        w[nwh] = cos(delta * nwh);
        w[nwh + 1] = w[nwh];
        for (j = 2; j <= nwh - 2; j += 2) {
            x = cos(delta * j);
            y = sin(delta * j);
            w[j] = x;
            w[j + 1] = y;
            w[nw - j] = y;
            w[nw - j + 1] = x;
        }
        bitrv2(nw, ip + 2, w);
    }
}


void makect(int nc, int *ip, double *c)
{
    int nch, j;
    double delta;
    
    ip[1] = nc;
    if (nc > 1) {
        nch = nc >> 1;
        delta = atan(1.0) / nch;
        c[0] = 0.5;
        c[nch] = 0.5 * cos(delta * nch);
        for (j = 1; j <= nch - 1; j++) {
            c[j] = 0.5 * cos(delta * j);
            c[nc - j] = 0.5 * sin(delta * j);
        }
    }
}


/* -------- child routines -------- */


void bitrv2(int n, int *ip, double *a)
{
    int j, j1, k, k1, l, m, m2;
    double xr, xi;
    
    ip[0] = 0;
    l = n;
    m = 1;
    while ((m << 2) < l) {
        l >>= 1;
        for (j = 0; j <= m - 1; j++) {
            ip[m + j] = ip[j] + l;
        }
        m <<= 1;
    }
    if ((m << 2) > l) {
        for (k = 1; k <= m - 1; k++) {
            for (j = 0; j <= k - 1; j++) {
                j1 = (j << 1) + ip[k];
                k1 = (k << 1) + ip[j];
                xr = a[j1];
                xi = a[j1 + 1];
                a[j1] = a[k1];
                a[j1 + 1] = a[k1 + 1];
                a[k1] = xr;
                a[k1 + 1] = xi;
            }
        }
    } else {
        m2 = m << 1;
        for (k = 1; k <= m - 1; k++) {
            for (j = 0; j <= k - 1; j++) {
                j1 = (j << 1) + ip[k];
                k1 = (k << 1) + ip[j];
                xr = a[j1];
                xi = a[j1 + 1];
                a[j1] = a[k1];
                a[j1 + 1] = a[k1 + 1];
                a[k1] = xr;
                a[k1 + 1] = xi;
                j1 += m2;
                k1 += m2;
                xr = a[j1];
                xi = a[j1 + 1];
                a[j1] = a[k1];
                a[j1 + 1] = a[k1 + 1];
                a[k1] = xr;
                a[k1 + 1] = xi;
            }
        }
    }
}


void bitrv2col(int n1, int n, int *ip, double **a)
{
    int i, j, j1, k, k1, l, m, m2;
    double xr, xi;
    
    ip[0] = 0;
    l = n;
    m = 1;
    while ((m << 2) < l) {
        l >>= 1;
        for (j = 0; j <= m - 1; j++) {
            ip[m + j] = ip[j] + l;
        }
        m <<= 1;
    }
    if ((m << 2) > l) {
        for (i = 0; i <= n1 - 1; i++) {
            for (k = 1; k <= m - 1; k++) {
                for (j = 0; j <= k - 1; j++) {
                    j1 = (j << 1) + ip[k];
                    k1 = (k << 1) + ip[j];
                    xr = a[i][j1];
                    xi = a[i][j1 + 1];
                    a[i][j1] = a[i][k1];
                    a[i][j1 + 1] = a[i][k1 + 1];
                    a[i][k1] = xr;
                    a[i][k1 + 1] = xi;
                }
            }
        }
    } else {
        m2 = m << 1;
        for (i = 0; i <= n1 - 1; i++) {
            for (k = 1; k <= m - 1; k++) {
                for (j = 0; j <= k - 1; j++) {
                    j1 = (j << 1) + ip[k];
                    k1 = (k << 1) + ip[j];
                    xr = a[i][j1];
                    xi = a[i][j1 + 1];
                    a[i][j1] = a[i][k1];
                    a[i][j1 + 1] = a[i][k1 + 1];
                    a[i][k1] = xr;
                    a[i][k1 + 1] = xi;
                    j1 += m2;
                    k1 += m2;
                    xr = a[i][j1];
                    xi = a[i][j1 + 1];
                    a[i][j1] = a[i][k1];
                    a[i][j1 + 1] = a[i][k1 + 1];
                    a[i][k1] = xr;
                    a[i][k1 + 1] = xi;
                }
            }
        }
    }
}


void bitrv2row(int n, int n2, int *ip, double **a)
{
    int i, j, j1, k, k1, l, m;
    double xr, xi;
    
    ip[0] = 0;
    l = n;
    m = 1;
    while ((m << 1) < l) {
        l >>= 1;
        for (j = 0; j <= m - 1; j++) {
            ip[m + j] = ip[j] + l;
        }
        m <<= 1;
    }
    if ((m << 1) > l) {
        for (k = 1; k <= m - 1; k++) {
            for (j = 0; j <= k - 1; j++) {
                j1 = j + ip[k];
                k1 = k + ip[j];
                for (i = 0; i <= n2 - 2; i += 2) {
                    xr = a[j1][i];
                    xi = a[j1][i + 1];
                    a[j1][i] = a[k1][i];
                    a[j1][i + 1] = a[k1][i + 1];
                    a[k1][i] = xr;
                    a[k1][i + 1] = xi;
                }
            }
        }
    } else {
        for (k = 1; k <= m - 1; k++) {
            for (j = 0; j <= k - 1; j++) {
                j1 = j + ip[k];
                k1 = k + ip[j];
                for (i = 0; i <= n2 - 2; i += 2) {
                    xr = a[j1][i];
                    xi = a[j1][i + 1];
                    a[j1][i] = a[k1][i];
                    a[j1][i + 1] = a[k1][i + 1];
                    a[k1][i] = xr;
                    a[k1][i + 1] = xi;
                }
                j1 += m;
                k1 += m;
                for (i = 0; i <= n2 - 2; i += 2) {
                    xr = a[j1][i];
                    xi = a[j1][i + 1];
                    a[j1][i] = a[k1][i];
                    a[j1][i + 1] = a[k1][i + 1];
                    a[k1][i] = xr;
                    a[k1][i + 1] = xi;
                }
            }
        }
    }
}


void cftbcol(int n1, int n, double **a, double *w)
{
    int i, j, j1, j2, j3, k, k1, ks, l, m;
    double wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
    
    for (i = 0; i <= n1 - 1; i++) {
        l = 2;
        while ((l << 1) < n) {
            m = l << 2;
            for (j = 0; j <= l - 2; j += 2) {
                j1 = j + l;
                j2 = j1 + l;
                j3 = j2 + l;
                x0r = a[i][j] + a[i][j1];
                x0i = a[i][j + 1] + a[i][j1 + 1];
                x1r = a[i][j] - a[i][j1];
                x1i = a[i][j + 1] - a[i][j1 + 1];
                x2r = a[i][j2] + a[i][j3];
                x2i = a[i][j2 + 1] + a[i][j3 + 1];
                x3r = a[i][j2] - a[i][j3];
                x3i = a[i][j2 + 1] - a[i][j3 + 1];
                a[i][j] = x0r + x2r;
                a[i][j + 1] = x0i + x2i;
                a[i][j2] = x0r - x2r;
                a[i][j2 + 1] = x0i - x2i;
                a[i][j1] = x1r - x3i;
                a[i][j1 + 1] = x1i + x3r;
                a[i][j3] = x1r + x3i;
                a[i][j3 + 1] = x1i - x3r;
            }
            if (m < n) {
                wk1r = w[2];
                for (j = m; j <= l + m - 2; j += 2) {
                    j1 = j + l;
                    j2 = j1 + l;
                    j3 = j2 + l;
                    x0r = a[i][j] + a[i][j1];
                    x0i = a[i][j + 1] + a[i][j1 + 1];
                    x1r = a[i][j] - a[i][j1];
                    x1i = a[i][j + 1] - a[i][j1 + 1];
                    x2r = a[i][j2] + a[i][j3];
                    x2i = a[i][j2 + 1] + a[i][j3 + 1];
                    x3r = a[i][j2] - a[i][j3];
                    x3i = a[i][j2 + 1] - a[i][j3 + 1];
                    a[i][j] = x0r + x2r;
                    a[i][j + 1] = x0i + x2i;
                    a[i][j2] = x2i - x0i;
                    a[i][j2 + 1] = x0r - x2r;
                    x0r = x1r - x3i;
                    x0i = x1i + x3r;
                    a[i][j1] = wk1r * (x0r - x0i);
                    a[i][j1 + 1] = wk1r * (x0r + x0i);
                    x0r = x3i + x1r;
                    x0i = x3r - x1i;
                    a[i][j3] = wk1r * (x0i - x0r);
                    a[i][j3 + 1] = wk1r * (x0i + x0r);
                }
                k1 = 1;
                ks = -1;
                for (k = (m << 1); k <= n - m; k += m) {
                    k1++;
                    ks = -ks;
                    wk1r = w[k1 << 1];
                    wk1i = w[(k1 << 1) + 1];
                    wk2r = ks * w[k1];
                    wk2i = w[k1 + ks];
                    wk3r = wk1r - 2 * wk2i * wk1i;
                    wk3i = 2 * wk2i * wk1r - wk1i;
                    for (j = k; j <= l + k - 2; j += 2) {
                        j1 = j + l;
                        j2 = j1 + l;
                        j3 = j2 + l;
                        x0r = a[i][j] + a[i][j1];
                        x0i = a[i][j + 1] + a[i][j1 + 1];
                        x1r = a[i][j] - a[i][j1];
                        x1i = a[i][j + 1] - a[i][j1 + 1];
                        x2r = a[i][j2] + a[i][j3];
                        x2i = a[i][j2 + 1] + a[i][j3 + 1];
                        x3r = a[i][j2] - a[i][j3];
                        x3i = a[i][j2 + 1] - a[i][j3 + 1];
                        a[i][j] = x0r + x2r;
                        a[i][j + 1] = x0i + x2i;
                        x0r -= x2r;
                        x0i -= x2i;
                        a[i][j2] = wk2r * x0r - wk2i * x0i;
                        a[i][j2 + 1] = wk2r * x0i + wk2i * x0r;
                        x0r = x1r - x3i;
                        x0i = x1i + x3r;
                        a[i][j1] = wk1r * x0r - wk1i * x0i;
                        a[i][j1 + 1] = wk1r * x0i + wk1i * x0r;
                        x0r = x1r + x3i;
                        x0i = x1i - x3r;
                        a[i][j3] = wk3r * x0r - wk3i * x0i;
                        a[i][j3 + 1] = wk3r * x0i + wk3i * x0r;
                    }
                }
            }
            l = m;
        }
        if (l < n) {
            for (j = 0; j <= l - 2; j += 2) {
                j1 = j + l;
                x0r = a[i][j] - a[i][j1];
                x0i = a[i][j + 1] - a[i][j1 + 1];
                a[i][j] += a[i][j1];
                a[i][j + 1] += a[i][j1 + 1];
                a[i][j1] = x0r;
                a[i][j1 + 1] = x0i;
            }
        }
    }
}


void cftbrow(int n, int n2, double **a, double *w)
{
    int i, j, j1, j2, j3, k, k1, ks, l, m;
    double wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
    
    l = 1;
    while ((l << 1) < n) {
        m = l << 2;
        for (j = 0; j <= l - 1; j++) {
            j1 = j + l;
            j2 = j1 + l;
            j3 = j2 + l;
            for (i = 0; i <= n2 - 2; i += 2) {
                x0r = a[j][i] + a[j1][i];
                x0i = a[j][i + 1] + a[j1][i + 1];
                x1r = a[j][i] - a[j1][i];
                x1i = a[j][i + 1] - a[j1][i + 1];
                x2r = a[j2][i] + a[j3][i];
                x2i = a[j2][i + 1] + a[j3][i + 1];
                x3r = a[j2][i] - a[j3][i];
                x3i = a[j2][i + 1] - a[j3][i + 1];
                a[j][i] = x0r + x2r;
                a[j][i + 1] = x0i + x2i;
                a[j2][i] = x0r - x2r;
                a[j2][i + 1] = x0i - x2i;
                a[j1][i] = x1r - x3i;
                a[j1][i + 1] = x1i + x3r;
                a[j3][i] = x1r + x3i;
                a[j3][i + 1] = x1i - x3r;
            }
        }
        if (m < n) {
            wk1r = w[2];
            for (j = m; j <= l + m - 1; j++) {
                j1 = j + l;
                j2 = j1 + l;
                j3 = j2 + l;
                for (i = 0; i <= n2 - 2; i += 2) {
                    x0r = a[j][i] + a[j1][i];
                    x0i = a[j][i + 1] + a[j1][i + 1];
                    x1r = a[j][i] - a[j1][i];
                    x1i = a[j][i + 1] - a[j1][i + 1];
                    x2r = a[j2][i] + a[j3][i];
                    x2i = a[j2][i + 1] + a[j3][i + 1];
                    x3r = a[j2][i] - a[j3][i];
                    x3i = a[j2][i + 1] - a[j3][i + 1];
                    a[j][i] = x0r + x2r;
                    a[j][i + 1] = x0i + x2i;
                    a[j2][i] = x2i - x0i;
                    a[j2][i + 1] = x0r - x2r;
                    x0r = x1r - x3i;
                    x0i = x1i + x3r;
                    a[j1][i] = wk1r * (x0r - x0i);
                    a[j1][i + 1] = wk1r * (x0r + x0i);
                    x0r = x3i + x1r;
                    x0i = x3r - x1i;
                    a[j3][i] = wk1r * (x0i - x0r);
                    a[j3][i + 1] = wk1r * (x0i + x0r);
                }
            }
            k1 = 1;
            ks = -1;
            for (k = (m << 1); k <= n - m; k += m) {
                k1++;
                ks = -ks;
                wk1r = w[k1 << 1];
                wk1i = w[(k1 << 1) + 1];
                wk2r = ks * w[k1];
                wk2i = w[k1 + ks];
                wk3r = wk1r - 2 * wk2i * wk1i;
                wk3i = 2 * wk2i * wk1r - wk1i;
                for (j = k; j <= l + k - 1; j++) {
                    j1 = j + l;
                    j2 = j1 + l;
                    j3 = j2 + l;
                    for (i = 0; i <= n2 - 2; i += 2) {
                        x0r = a[j][i] + a[j1][i];
                        x0i = a[j][i + 1] + a[j1][i + 1];
                        x1r = a[j][i] - a[j1][i];
                        x1i = a[j][i + 1] - a[j1][i + 1];
                        x2r = a[j2][i] + a[j3][i];
                        x2i = a[j2][i + 1] + a[j3][i + 1];
                        x3r = a[j2][i] - a[j3][i];
                        x3i = a[j2][i + 1] - a[j3][i + 1];
                        a[j][i] = x0r + x2r;
                        a[j][i + 1] = x0i + x2i;
                        x0r -= x2r;
                        x0i -= x2i;
                        a[j2][i] = wk2r * x0r - wk2i * x0i;
                        a[j2][i + 1] = wk2r * x0i + wk2i * x0r;
                        x0r = x1r - x3i;
                        x0i = x1i + x3r;
                        a[j1][i] = wk1r * x0r - wk1i * x0i;
                        a[j1][i + 1] = wk1r * x0i + wk1i * x0r;
                        x0r = x1r + x3i;
                        x0i = x1i - x3r;
                        a[j3][i] = wk3r * x0r - wk3i * x0i;
                        a[j3][i + 1] = wk3r * x0i + wk3i * x0r;
                    }
                }
            }
        }
        l = m;
    }
    if (l < n) {
        for (j = 0; j <= l - 1; j++) {
            j1 = j + l;
            for (i = 0; i <= n2 - 2; i += 2) {
                x0r = a[j][i] - a[j1][i];
                x0i = a[j][i + 1] - a[j1][i + 1];
                a[j][i] += a[j1][i];
                a[j][i + 1] += a[j1][i + 1];
                a[j1][i] = x0r;
                a[j1][i + 1] = x0i;
            }
        }
    }
}


void cftfcol(int n1, int n, double **a, double *w)
{
    int i, j, j1, j2, j3, k, k1, ks, l, m;
    double wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
    
    for (i = 0; i <= n1 - 1; i++) {
        l = 2;
        while ((l << 1) < n) {
            m = l << 2;
            for (j = 0; j <= l - 2; j += 2) {
                j1 = j + l;
                j2 = j1 + l;
                j3 = j2 + l;
                x0r = a[i][j] + a[i][j1];
                x0i = a[i][j + 1] + a[i][j1 + 1];
                x1r = a[i][j] - a[i][j1];
                x1i = a[i][j + 1] - a[i][j1 + 1];
                x2r = a[i][j2] + a[i][j3];
                x2i = a[i][j2 + 1] + a[i][j3 + 1];
                x3r = a[i][j2] - a[i][j3];
                x3i = a[i][j2 + 1] - a[i][j3 + 1];
                a[i][j] = x0r + x2r;
                a[i][j + 1] = x0i + x2i;
                a[i][j2] = x0r - x2r;
                a[i][j2 + 1] = x0i - x2i;
                a[i][j1] = x1r + x3i;
                a[i][j1 + 1] = x1i - x3r;
                a[i][j3] = x1r - x3i;
                a[i][j3 + 1] = x1i + x3r;
            }
            if (m < n) {
                wk1r = w[2];
                for (j = m; j <= l + m - 2; j += 2) {
                    j1 = j + l;
                    j2 = j1 + l;
                    j3 = j2 + l;
                    x0r = a[i][j] + a[i][j1];
                    x0i = a[i][j + 1] + a[i][j1 + 1];
                    x1r = a[i][j] - a[i][j1];
                    x1i = a[i][j + 1] - a[i][j1 + 1];
                    x2r = a[i][j2] + a[i][j3];
                    x2i = a[i][j2 + 1] + a[i][j3 + 1];
                    x3r = a[i][j2] - a[i][j3];
                    x3i = a[i][j2 + 1] - a[i][j3 + 1];
                    a[i][j] = x0r + x2r;
                    a[i][j + 1] = x0i + x2i;
                    a[i][j2] = x0i - x2i;
                    a[i][j2 + 1] = x2r - x0r;
                    x0r = x1r + x3i;
                    x0i = x1i - x3r;
                    a[i][j1] = wk1r * (x0i + x0r);
                    a[i][j1 + 1] = wk1r * (x0i - x0r);
                    x0r = x3i - x1r;
                    x0i = x3r + x1i;
                    a[i][j3] = wk1r * (x0r + x0i);
                    a[i][j3 + 1] = wk1r * (x0r - x0i);
                }
                k1 = 1;
                ks = -1;
                for (k = (m << 1); k <= n - m; k += m) {
                    k1++;
                    ks = -ks;
                    wk1r = w[k1 << 1];
                    wk1i = w[(k1 << 1) + 1];
                    wk2r = ks * w[k1];
                    wk2i = w[k1 + ks];
                    wk3r = wk1r - 2 * wk2i * wk1i;
                    wk3i = 2 * wk2i * wk1r - wk1i;
                    for (j = k; j <= l + k - 2; j += 2) {
                        j1 = j + l;
                        j2 = j1 + l;
                        j3 = j2 + l;
                        x0r = a[i][j] + a[i][j1];
                        x0i = a[i][j + 1] + a[i][j1 + 1];
                        x1r = a[i][j] - a[i][j1];
                        x1i = a[i][j + 1] - a[i][j1 + 1];
                        x2r = a[i][j2] + a[i][j3];
                        x2i = a[i][j2 + 1] + a[i][j3 + 1];
                        x3r = a[i][j2] - a[i][j3];
                        x3i = a[i][j2 + 1] - a[i][j3 + 1];
                        a[i][j] = x0r + x2r;
                        a[i][j + 1] = x0i + x2i;
                        x0r -= x2r;
                        x0i -= x2i;
                        a[i][j2] = wk2r * x0r + wk2i * x0i;
                        a[i][j2 + 1] = wk2r * x0i - wk2i * x0r;
                        x0r = x1r + x3i;
                        x0i = x1i - x3r;
                        a[i][j1] = wk1r * x0r + wk1i * x0i;
                        a[i][j1 + 1] = wk1r * x0i - wk1i * x0r;
                        x0r = x1r - x3i;
                        x0i = x1i + x3r;
                        a[i][j3] = wk3r * x0r + wk3i * x0i;
                        a[i][j3 + 1] = wk3r * x0i - wk3i * x0r;
                    }
                }
            }
            l = m;
        }
        if (l < n) {
            for (j = 0; j <= l - 2; j += 2) {
                j1 = j + l;
                x0r = a[i][j] - a[i][j1];
                x0i = a[i][j + 1] - a[i][j1 + 1];
                a[i][j] += a[i][j1];
                a[i][j + 1] += a[i][j1 + 1];
                a[i][j1] = x0r;
                a[i][j1 + 1] = x0i;
            }
        }
    }
}


void cftfrow(int n, int n2, double **a, double *w)
{
    int i, j, j1, j2, j3, k, k1, ks, l, m;
    double wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
    
    l = 1;
    while ((l << 1) < n) {
        m = l << 2;
        for (j = 0; j <= l - 1; j++) {
            j1 = j + l;
            j2 = j1 + l;
            j3 = j2 + l;
            for (i = 0; i <= n2 - 2; i += 2) {
                x0r = a[j][i] + a[j1][i];
                x0i = a[j][i + 1] + a[j1][i + 1];
                x1r = a[j][i] - a[j1][i];
                x1i = a[j][i + 1] - a[j1][i + 1];
                x2r = a[j2][i] + a[j3][i];
                x2i = a[j2][i + 1] + a[j3][i + 1];
                x3r = a[j2][i] - a[j3][i];
                x3i = a[j2][i + 1] - a[j3][i + 1];
                a[j][i] = x0r + x2r;
                a[j][i + 1] = x0i + x2i;
                a[j2][i] = x0r - x2r;
                a[j2][i + 1] = x0i - x2i;
                a[j1][i] = x1r + x3i;
                a[j1][i + 1] = x1i - x3r;
                a[j3][i] = x1r - x3i;
                a[j3][i + 1] = x1i + x3r;
            }
        }
        if (m < n) {
            wk1r = w[2];
            for (j = m; j <= l + m - 1; j++) {
                j1 = j + l;
                j2 = j1 + l;
                j3 = j2 + l;
                for (i = 0; i <= n2 - 2; i += 2) {
                    x0r = a[j][i] + a[j1][i];
                    x0i = a[j][i + 1] + a[j1][i + 1];
                    x1r = a[j][i] - a[j1][i];
                    x1i = a[j][i + 1] - a[j1][i + 1];
                    x2r = a[j2][i] + a[j3][i];
                    x2i = a[j2][i + 1] + a[j3][i + 1];
                    x3r = a[j2][i] - a[j3][i];
                    x3i = a[j2][i + 1] - a[j3][i + 1];
                    a[j][i] = x0r + x2r;
                    a[j][i + 1] = x0i + x2i;
                    a[j2][i] = x0i - x2i;
                    a[j2][i + 1] = x2r - x0r;
                    x0r = x1r + x3i;
                    x0i = x1i - x3r;
                    a[j1][i] = wk1r * (x0i + x0r);
                    a[j1][i + 1] = wk1r * (x0i - x0r);
                    x0r = x3i - x1r;
                    x0i = x3r + x1i;
                    a[j3][i] = wk1r * (x0r + x0i);
                    a[j3][i + 1] = wk1r * (x0r - x0i);
                }
            }
            k1 = 1;
            ks = -1;
            for (k = (m << 1); k <= n - m; k += m) {
                k1++;
                ks = -ks;
                wk1r = w[k1 << 1];
                wk1i = w[(k1 << 1) + 1];
                wk2r = ks * w[k1];
                wk2i = w[k1 + ks];
                wk3r = wk1r - 2 * wk2i * wk1i;
                wk3i = 2 * wk2i * wk1r - wk1i;
                for (j = k; j <= l + k - 1; j++) {
                    j1 = j + l;
                    j2 = j1 + l;
                    j3 = j2 + l;
                    for (i = 0; i <= n2 - 2; i += 2) {
                        x0r = a[j][i] + a[j1][i];
                        x0i = a[j][i + 1] + a[j1][i + 1];
                        x1r = a[j][i] - a[j1][i];
                        x1i = a[j][i + 1] - a[j1][i + 1];
                        x2r = a[j2][i] + a[j3][i];
                        x2i = a[j2][i + 1] + a[j3][i + 1];
                        x3r = a[j2][i] - a[j3][i];
                        x3i = a[j2][i + 1] - a[j3][i + 1];
                        a[j][i] = x0r + x2r;
                        a[j][i + 1] = x0i + x2i;
                        x0r -= x2r;
                        x0i -= x2i;
                        a[j2][i] = wk2r * x0r + wk2i * x0i;
                        a[j2][i + 1] = wk2r * x0i - wk2i * x0r;
                        x0r = x1r + x3i;
                        x0i = x1i - x3r;
                        a[j1][i] = wk1r * x0r + wk1i * x0i;
                        a[j1][i + 1] = wk1r * x0i - wk1i * x0r;
                        x0r = x1r - x3i;
                        x0i = x1i + x3r;
                        a[j3][i] = wk3r * x0r + wk3i * x0i;
                        a[j3][i + 1] = wk3r * x0i - wk3i * x0r;
                    }
                }
            }
        }
        l = m;
    }
    if (l < n) {
        for (j = 0; j <= l - 1; j++) {
            j1 = j + l;
            for (i = 0; i <= n2 - 2; i += 2) {
                x0r = a[j][i] - a[j1][i];
                x0i = a[j][i + 1] - a[j1][i + 1];
                a[j][i] += a[j1][i];
                a[j][i + 1] += a[j1][i + 1];
                a[j1][i] = x0r;
                a[j1][i + 1] = x0i;
            }
        }
    }
}


void rftbcol(int n1, int n, double **a, int nc, double *c)
{
    int i, j, k, kk, ks;
    double wkr, wki, xr, xi, yr, yi;
    
    ks = (nc << 2) / n;
    for (i = 0; i <= n1 - 1; i++) {
        kk = 0;
        for (k = (n >> 1) - 2; k >= 2; k -= 2) {
            j = n - k;
            kk += ks;
            wkr = 0.5 - c[kk];
            wki = c[nc - kk];
            xr = a[i][k] - a[i][j];
            xi = a[i][k + 1] + a[i][j + 1];
            yr = wkr * xr - wki * xi;
            yi = wkr * xi + wki * xr;
            a[i][k] -= yr;
            a[i][k + 1] -= yi;
            a[i][j] += yr;
            a[i][j + 1] -= yi;
        }
    }
}


void rftfcol(int n1, int n, double **a, int nc, double *c)
{
    int i, j, k, kk, ks;
    double wkr, wki, xr, xi, yr, yi;
    
    ks = (nc << 2) / n;
    for (i = 0; i <= n1 - 1; i++) {
        kk = 0;
        for (k = (n >> 1) - 2; k >= 2; k -= 2) {
            j = n - k;
            kk += ks;
            wkr = 0.5 - c[kk];
            wki = c[nc - kk];
            xr = a[i][k] - a[i][j];
            xi = a[i][k + 1] + a[i][j + 1];
            yr = wkr * xr + wki * xi;
            yi = wkr * xi - wki * xr;
            a[i][k] -= yr;
            a[i][k + 1] -= yi;
            a[i][j] += yr;
            a[i][j + 1] -= yi;
        }
    }
}


void dctbsub(int n1, int n2, double **a, int nc, double *c)
{
    int kk1, kk2, ks1, ks2, n1h, j1, k1, k2;
    double w1r, w1i, wkr, wki, wjr, wji, x0r, x0i, x1r, x1i;
    
    ks1 = nc / n1;
    ks2 = nc / n2;
    n1h = n1 >> 1;
    kk1 = ks1;
    for (k1 = 1; k1 <= n1h - 1; k1++) {
        j1 = n1 - k1;
        w1r = 2 * c[kk1];
        w1i = 2 * c[nc - kk1];
        kk1 += ks1;
        kk2 = ks2;
        for (k2 = 2; k2 <= n2 - 2; k2 += 2) {
            x0r = w1r * c[kk2];
            x0i = w1i * c[kk2];
            x1r = w1r * c[nc - kk2];
            x1i = w1i * c[nc - kk2];
            wkr = x0r - x1i;
            wki = x0i + x1r;
            wji = x0r + x1i;
            wjr = x0i - x1r;
            kk2 += ks2;
            x0r = wkr * a[k1][k2] - wki * a[k1][k2 + 1];
            x0i = wkr * a[k1][k2 + 1] + wki * a[k1][k2];
            x1r = wjr * a[j1][k2] - wji * a[j1][k2 + 1];
            x1i = wjr * a[j1][k2 + 1] + wji * a[j1][k2];
            a[k1][k2] = x0r + x1i;
            a[k1][k2 + 1] = x0i - x1r;
            a[j1][k2] = x1r + x0i;
            a[j1][k2 + 1] = x1i - x0r;
        }
        wkr = w1r * 0.5;
        wki = w1i * 0.5;
        wjr = w1r * c[kk2];
        wji = w1i * c[kk2];
        x0r = a[k1][0] + a[j1][0];
        x0i = a[k1][1] - a[j1][1];
        x1r = a[k1][0] - a[j1][0];
        x1i = a[k1][1] + a[j1][1];
        a[k1][0] = wkr * x0r - wki * x0i;
        a[k1][1] = wkr * x0i + wki * x0r;
        a[j1][0] = -wjr * x1r + wji * x1i;
        a[j1][1] = wjr * x1i + wji * x1r;
    }
    w1r = 2 * c[kk1];
    kk2 = ks2;
    for (k2 = 2; k2 <= n2 - 2; k2 += 2) {
        wkr = 2 * c[kk2];
        wki = 2 * c[nc - kk2];
        wjr = w1r * wkr;
        wji = w1r * wki;
        kk2 += ks2;
        x0i = wkr * a[0][k2 + 1] + wki * a[0][k2];
        a[0][k2] = wkr * a[0][k2] - wki * a[0][k2 + 1];
        a[0][k2 + 1] = x0i;
        x0i = wjr * a[n1h][k2 + 1] + wji * a[n1h][k2];
        a[n1h][k2] = wjr * a[n1h][k2] - wji * a[n1h][k2 + 1];
        a[n1h][k2 + 1] = x0i;
    }
    a[0][1] *= w1r;
    a[n1h][0] *= w1r;
    a[n1h][1] *= 0.5;
}


void dctfsub(int n1, int n2, double **a, int nc, double *c)
{
    int kk1, kk2, ks1, ks2, n1h, j1, k1, k2;
    double w1r, w1i, wkr, wki, wjr, wji, x0r, x0i, x1r, x1i;
    
    ks1 = nc / n1;
    ks2 = nc / n2;
    n1h = n1 >> 1;
    kk1 = ks1;
    for (k1 = 1; k1 <= n1h - 1; k1++) {
        j1 = n1 - k1;
        w1r = 2 * c[kk1];
        w1i = 2 * c[nc - kk1];
        kk1 += ks1;
        kk2 = ks2;
        for (k2 = 2; k2 <= n2 - 2; k2 += 2) {
            x0r = w1r * c[kk2];
            x0i = w1i * c[kk2];
            x1r = w1r * c[nc - kk2];
            x1i = w1i * c[nc - kk2];
            wkr = x0r - x1i;
            wki = x0i + x1r;
            wji = x0r + x1i;
            wjr = x0i - x1r;
            kk2 += ks2;
            x0r = a[k1][k2] - a[j1][k2 + 1];
            x0i = a[j1][k2] + a[k1][k2 + 1];
            x1r = a[j1][k2] - a[k1][k2 + 1];
            x1i = a[k1][k2] + a[j1][k2 + 1];
            a[k1][k2] = wkr * x0r + wki * x0i;
            a[k1][k2 + 1] = wkr * x0i - wki * x0r;
            a[j1][k2] = wjr * x1r + wji * x1i;
            a[j1][k2 + 1] = wjr * x1i - wji * x1r;
        }
        x0r = 2 * c[kk2];
        wjr = x0r * w1r;
        wji = x0r * w1i;
        x0r = w1r * a[k1][0] + w1i * a[k1][1];
        x0i = w1r * a[k1][1] - w1i * a[k1][0];
        x1r = -wjr * a[j1][0] + wji * a[j1][1];
        x1i = wjr * a[j1][1] + wji * a[j1][0];
        a[k1][0] = x0r + x1r;
        a[k1][1] = x1i + x0i;
        a[j1][0] = x0r - x1r;
        a[j1][1] = x1i - x0i;
    }
    w1r = 2 * c[kk1];
    kk2 = ks2;
    for (k2 = 2; k2 <= n2 - 2; k2 += 2) {
        wkr = 2 * c[kk2];
        wki = 2 * c[nc - kk2];
        wjr = w1r * wkr;
        wji = w1r * wki;
        kk2 += ks2;
        x0i = wkr * a[0][k2 + 1] - wki * a[0][k2];
        a[0][k2] = wkr * a[0][k2] + wki * a[0][k2 + 1];
        a[0][k2 + 1] = x0i;
        x0i = wjr * a[n1h][k2 + 1] - wji * a[n1h][k2];
        a[n1h][k2] = wjr * a[n1h][k2] + wji * a[n1h][k2 + 1];
        a[n1h][k2 + 1] = x0i;
    }
    w1r *= 2;
    a[0][0] *= 2;
    a[0][1] *= w1r;
    a[n1h][0] *= w1r;
}


void dstbsub(int n1, int n2, double **a, int nc, double *c)
{
    int kk1, kk2, ks1, ks2, n1h, j1, k1, k2;
    double w1r, w1i, wkr, wki, wjr, wji, x0r, x0i, x1r, x1i;
    
    ks1 = nc / n1;
    ks2 = nc / n2;
    n1h = n1 >> 1;
    kk1 = ks1;
    for (k1 = 1; k1 <= n1h - 1; k1++) {
        j1 = n1 - k1;
        w1r = 2 * c[kk1];
        w1i = 2 * c[nc - kk1];
        kk1 += ks1;
        kk2 = ks2;
        for (k2 = 2; k2 <= n2 - 2; k2 += 2) {
            x0r = w1r * c[kk2];
            x0i = w1i * c[kk2];
            x1r = w1r * c[nc - kk2];
            x1i = w1i * c[nc - kk2];
            wkr = x0r - x1i;
            wki = x0i + x1r;
            wji = x0r + x1i;
            wjr = x0i - x1r;
            kk2 += ks2;
            x0r = wkr * a[k1][k2] - wki * a[k1][k2 + 1];
            x0i = wkr * a[k1][k2 + 1] + wki * a[k1][k2];
            x1r = wjr * a[j1][k2] - wji * a[j1][k2 + 1];
            x1i = wjr * a[j1][k2 + 1] + wji * a[j1][k2];
            a[k1][k2] = x1i - x0r;
            a[k1][k2 + 1] = x1r + x0i;
            a[j1][k2] = x0i - x1r;
            a[j1][k2 + 1] = x0r + x1i;
        }
        wkr = w1r * 0.5;
        wki = w1i * 0.5;
        wjr = w1r * c[kk2];
        wji = w1i * c[kk2];
        x0r = a[k1][0] + a[j1][0];
        x0i = a[k1][1] - a[j1][1];
        x1r = a[k1][0] - a[j1][0];
        x1i = a[k1][1] + a[j1][1];
        a[k1][1] = wkr * x0r - wki * x0i;
        a[k1][0] = wkr * x0i + wki * x0r;
        a[j1][1] = -wjr * x1r + wji * x1i;
        a[j1][0] = wjr * x1i + wji * x1r;
    }
    w1r = 2 * c[kk1];
    kk2 = ks2;
    for (k2 = 2; k2 <= n2 - 2; k2 += 2) {
        wkr = 2 * c[kk2];
        wki = 2 * c[nc - kk2];
        wjr = w1r * wkr;
        wji = w1r * wki;
        kk2 += ks2;
        x0i = wkr * a[0][k2 + 1] + wki * a[0][k2];
        a[0][k2 + 1] = wkr * a[0][k2] - wki * a[0][k2 + 1];
        a[0][k2] = x0i;
        x0i = wjr * a[n1h][k2 + 1] + wji * a[n1h][k2];
        a[n1h][k2 + 1] = wjr * a[n1h][k2] - wji * a[n1h][k2 + 1];
        a[n1h][k2] = x0i;
    }
    a[0][1] *= w1r;
    a[n1h][0] *= w1r;
    a[n1h][1] *= 0.5;
}


void dstfsub(int n1, int n2, double **a, int nc, double *c)
{
    int kk1, kk2, ks1, ks2, n1h, j1, k1, k2;
    double w1r, w1i, wkr, wki, wjr, wji, x0r, x0i, x1r, x1i;
    
    ks1 = nc / n1;
    ks2 = nc / n2;
    n1h = n1 >> 1;
    kk1 = ks1;
    for (k1 = 1; k1 <= n1h - 1; k1++) {
        j1 = n1 - k1;
        w1r = 2 * c[kk1];
        w1i = 2 * c[nc - kk1];
        kk1 += ks1;
        kk2 = ks2;
        for (k2 = 2; k2 <= n2 - 2; k2 += 2) {
            x0r = w1r * c[kk2];
            x0i = w1i * c[kk2];
            x1r = w1r * c[nc - kk2];
            x1i = w1i * c[nc - kk2];
            wkr = x0r - x1i;
            wki = x0i + x1r;
            wji = x0r + x1i;
            wjr = x0i - x1r;
            kk2 += ks2;
            x0r = a[j1][k2 + 1] - a[k1][k2];
            x0i = a[k1][k2 + 1] + a[j1][k2];
            x1r = a[k1][k2 + 1] - a[j1][k2];
            x1i = a[j1][k2 + 1] + a[k1][k2];
            a[k1][k2] = wkr * x0r + wki * x0i;
            a[k1][k2 + 1] = wkr * x0i - wki * x0r;
            a[j1][k2] = wjr * x1r + wji * x1i;
            a[j1][k2 + 1] = wjr * x1i - wji * x1r;
        }
        x0r = 2 * c[kk2];
        wjr = x0r * w1r;
        wji = x0r * w1i;
        x0r = w1r * a[k1][1] + w1i * a[k1][0];
        x0i = w1r * a[k1][0] - w1i * a[k1][1];
        x1r = -wjr * a[j1][1] + wji * a[j1][0];
        x1i = wjr * a[j1][0] + wji * a[j1][1];
        a[k1][0] = x0r + x1r;
        a[k1][1] = x1i + x0i;
        a[j1][0] = x0r - x1r;
        a[j1][1] = x1i - x0i;
    }
    w1r = 2 * c[kk1];
    kk2 = ks2;
    for (k2 = 2; k2 <= n2 - 2; k2 += 2) {
        wkr = 2 * c[kk2];
        wki = 2 * c[nc - kk2];
        wjr = w1r * wkr;
        wji = w1r * wki;
        kk2 += ks2;
        x0i = wkr * a[0][k2] - wki * a[0][k2 + 1];
        a[0][k2] = wkr * a[0][k2 + 1] + wki * a[0][k2];
        a[0][k2 + 1] = x0i;
        x0i = wjr * a[n1h][k2] - wji * a[n1h][k2 + 1];
        a[n1h][k2] = wjr * a[n1h][k2 + 1] + wji * a[n1h][k2];
        a[n1h][k2 + 1] = x0i;
    }
    w1r *= 2;
    a[0][0] *= 2;
    a[0][1] *= w1r;
    a[n1h][0] *= w1r;
}

