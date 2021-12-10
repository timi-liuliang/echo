/*
Short Discrete Cosine Transform
    data length :8x8, 16x16
    method      :row-column, radix 4 FFT
functions
    ddct8x8s  : 8x8 DCT
    ddct16x16s: 16x16 DCT
function prototypes
    void ddct8x8s(int isgn, double **a);
    void ddct16x16s(int isgn, double **a);
*/


/*
-------- 8x8 DCT (Discrete Cosine Transform) / Inverse of DCT --------
    [definition]
        <case1> Normalized 8x8 IDCT
            C[k1][k2] = (1/4) * sum_j1=0^7 sum_j2=0^7 
                            a[j1][j2] * s[j1] * s[j2] * 
                            cos(pi*j1*(k1+1/2)/8) * 
                            cos(pi*j2*(k2+1/2)/8), 0<=k1<8, 0<=k2<8
                            (s[0] = 1/sqrt(2), s[j] = 1, j > 0)
        <case2> Normalized 8x8 DCT
            C[k1][k2] = (1/4) * s[k1] * s[k2] * sum_j1=0^7 sum_j2=0^7 
                            a[j1][j2] * 
                            cos(pi*(j1+1/2)*k1/8) * 
                            cos(pi*(j2+1/2)*k2/8), 0<=k1<8, 0<=k2<8
                            (s[0] = 1/sqrt(2), s[j] = 1, j > 0)
    [usage]
        <case1>
            ddct8x8s(1, a);
        <case2>
            ddct8x8s(-1, a);
    [parameters]
        a[0...7][0...7] :input/output data (double **)
                         output data
                             a[k1][k2] = C[k1][k2], 0<=k1<8, 0<=k2<8
*/


/* Cn_kR = sqrt(2.0/n) * cos(pi/2*k/n) */
/* Cn_kI = sqrt(2.0/n) * sin(pi/2*k/n) */
/* Wn_kR = cos(pi/2*k/n) */
/* Wn_kI = sin(pi/2*k/n) */
#define C8_1R   0.49039264020161522456
#define C8_1I   0.09754516100806413392
#define C8_2R   0.46193976625564337806
#define C8_2I   0.19134171618254488586
#define C8_3R   0.41573480615127261854
#define C8_3I   0.27778511650980111237
#define C8_4R   0.35355339059327376220
#define W8_4R   0.70710678118654752440


void ddct8x8s(int isgn, double **a)
{
    int j;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
    double xr, xi;
    
    if (isgn < 0) {
        for (j = 0; j <= 7; j++) {
            x0r = a[0][j] + a[7][j];
            x1r = a[0][j] - a[7][j];
            x0i = a[2][j] + a[5][j];
            x1i = a[2][j] - a[5][j];
            x2r = a[4][j] + a[3][j];
            x3r = a[4][j] - a[3][j];
            x2i = a[6][j] + a[1][j];
            x3i = a[6][j] - a[1][j];
            xr = x0r + x2r;
            xi = x0i + x2i;
            a[0][j] = C8_4R * (xr + xi);
            a[4][j] = C8_4R * (xr - xi);
            xr = x0r - x2r;
            xi = x0i - x2i;
            a[2][j] = C8_2R * xr - C8_2I * xi;
            a[6][j] = C8_2R * xi + C8_2I * xr;
            xr = W8_4R * (x1i - x3i);
            x1i = W8_4R * (x1i + x3i);
            x3i = x1i - x3r;
            x1i += x3r;
            x3r = x1r - xr;
            x1r += xr;
            a[1][j] = C8_1R * x1r - C8_1I * x1i;
            a[7][j] = C8_1R * x1i + C8_1I * x1r;
            a[3][j] = C8_3R * x3r - C8_3I * x3i;
            a[5][j] = C8_3R * x3i + C8_3I * x3r;
        }
        for (j = 0; j <= 7; j++) {
            x0r = a[j][0] + a[j][7];
            x1r = a[j][0] - a[j][7];
            x0i = a[j][2] + a[j][5];
            x1i = a[j][2] - a[j][5];
            x2r = a[j][4] + a[j][3];
            x3r = a[j][4] - a[j][3];
            x2i = a[j][6] + a[j][1];
            x3i = a[j][6] - a[j][1];
            xr = x0r + x2r;
            xi = x0i + x2i;
            a[j][0] = C8_4R * (xr + xi);
            a[j][4] = C8_4R * (xr - xi);
            xr = x0r - x2r;
            xi = x0i - x2i;
            a[j][2] = C8_2R * xr - C8_2I * xi;
            a[j][6] = C8_2R * xi + C8_2I * xr;
            xr = W8_4R * (x1i - x3i);
            x1i = W8_4R * (x1i + x3i);
            x3i = x1i - x3r;
            x1i += x3r;
            x3r = x1r - xr;
            x1r += xr;
            a[j][1] = C8_1R * x1r - C8_1I * x1i;
            a[j][7] = C8_1R * x1i + C8_1I * x1r;
            a[j][3] = C8_3R * x3r - C8_3I * x3i;
            a[j][5] = C8_3R * x3i + C8_3I * x3r;
        }
    } else {
        for (j = 0; j <= 7; j++) {
            x1r = C8_1R * a[1][j] + C8_1I * a[7][j];
            x1i = C8_1R * a[7][j] - C8_1I * a[1][j];
            x3r = C8_3R * a[3][j] + C8_3I * a[5][j];
            x3i = C8_3R * a[5][j] - C8_3I * a[3][j];
            xr = x1r - x3r;
            xi = x1i + x3i;
            x1r += x3r;
            x3i -= x1i;
            x1i = W8_4R * (xr + xi);
            x3r = W8_4R * (xr - xi);
            xr = C8_2R * a[2][j] + C8_2I * a[6][j];
            xi = C8_2R * a[6][j] - C8_2I * a[2][j];
            x0r = C8_4R * (a[0][j] + a[4][j]);
            x0i = C8_4R * (a[0][j] - a[4][j]);
            x2r = x0r - xr;
            x2i = x0i - xi;
            x0r += xr;
            x0i += xi;
            a[0][j] = x0r + x1r;
            a[7][j] = x0r - x1r;
            a[2][j] = x0i + x1i;
            a[5][j] = x0i - x1i;
            a[4][j] = x2r - x3i;
            a[3][j] = x2r + x3i;
            a[6][j] = x2i - x3r;
            a[1][j] = x2i + x3r;
        }
        for (j = 0; j <= 7; j++) {
            x1r = C8_1R * a[j][1] + C8_1I * a[j][7];
            x1i = C8_1R * a[j][7] - C8_1I * a[j][1];
            x3r = C8_3R * a[j][3] + C8_3I * a[j][5];
            x3i = C8_3R * a[j][5] - C8_3I * a[j][3];
            xr = x1r - x3r;
            xi = x1i + x3i;
            x1r += x3r;
            x3i -= x1i;
            x1i = W8_4R * (xr + xi);
            x3r = W8_4R * (xr - xi);
            xr = C8_2R * a[j][2] + C8_2I * a[j][6];
            xi = C8_2R * a[j][6] - C8_2I * a[j][2];
            x0r = C8_4R * (a[j][0] + a[j][4]);
            x0i = C8_4R * (a[j][0] - a[j][4]);
            x2r = x0r - xr;
            x2i = x0i - xi;
            x0r += xr;
            x0i += xi;
            a[j][0] = x0r + x1r;
            a[j][7] = x0r - x1r;
            a[j][2] = x0i + x1i;
            a[j][5] = x0i - x1i;
            a[j][4] = x2r - x3i;
            a[j][3] = x2r + x3i;
            a[j][6] = x2i - x3r;
            a[j][1] = x2i + x3r;
        }
    }
}



/*
-------- 16x16 DCT (Discrete Cosine Transform) / Inverse of DCT --------
    [definition]
        <case1> Normalized 16x16 IDCT
            C[k1][k2] = (1/8) * sum_j1=0^15 sum_j2=0^15 
                            a[j1][j2] * s[j1] * s[j2] * 
                            cos(pi*j1*(k1+1/2)/16) * 
                            cos(pi*j2*(k2+1/2)/16), 0<=k1<16, 0<=k2<16
                            (s[0] = 1/sqrt(2), s[j] = 1, j > 0)
        <case2> Normalized 16x16 DCT
            C[k1][k2] = (1/8) * s[k1] * s[k2] * sum_j1=0^15 sum_j2=0^15 
                            a[j1][j2] * 
                            cos(pi*(j1+1/2)*k1/16) * 
                            cos(pi*(j2+1/2)*k2/16), 0<=k1<16, 0<=k2<16
                            (s[0] = 1/sqrt(2), s[j] = 1, j > 0)
    [usage]
        <case1>
            ddct16x16s(1, a);
        <case2>
            ddct16x16s(-1, a);
    [parameters]
        a[0...15][0...15] :input/output data (double **)
                           output data
                               a[k1][k2] = C[k1][k2], 0<=k1<16, 0<=k2<16
*/


/* Cn_kR = sqrt(2.0/n) * cos(pi/2*k/n) */
/* Cn_kI = sqrt(2.0/n) * sin(pi/2*k/n) */
/* Wn_kR = cos(pi/2*k/n) */
/* Wn_kI = sin(pi/2*k/n) */
#define C16_1R   0.35185093438159561476
#define C16_1I   0.03465429229977286565
#define C16_2R   0.34675996133053686546
#define C16_2I   0.06897484482073575308
#define C16_3R   0.33832950029358816957
#define C16_3I   0.10263113188058934529
#define C16_4R   0.32664074121909413196
#define C16_4I   0.13529902503654924610
#define C16_5R   0.31180625324666780814
#define C16_5I   0.16666391461943662432
#define C16_6R   0.29396890060483967924
#define C16_6I   0.19642373959677554532
#define C16_7R   0.27330046675043937206
#define C16_7I   0.22429189658565907106
#define C16_8R   0.25
#define W16_4R   0.92387953251128675613
#define W16_4I   0.38268343236508977173
#define W16_8R   0.70710678118654752440


void ddct16x16s(int isgn, double **a)
{
    int j;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
    double x4r, x4i, x5r, x5i, x6r, x6i, x7r, x7i;
    double xr, xi;
    
    if (isgn < 0) {
        for (j = 0; j <= 15; j++) {
            x4r = a[0][j] - a[15][j];
            xr = a[0][j] + a[15][j];
            x4i = a[8][j] - a[7][j];
            xi = a[8][j] + a[7][j];
            x0r = xr + xi;
            x0i = xr - xi;
            x5r = a[2][j] - a[13][j];
            xr = a[2][j] + a[13][j];
            x5i = a[10][j] - a[5][j];
            xi = a[10][j] + a[5][j];
            x1r = xr + xi;
            x1i = xr - xi;
            x6r = a[4][j] - a[11][j];
            xr = a[4][j] + a[11][j];
            x6i = a[12][j] - a[3][j];
            xi = a[12][j] + a[3][j];
            x2r = xr + xi;
            x2i = xr - xi;
            x7r = a[6][j] - a[9][j];
            xr = a[6][j] + a[9][j];
            x7i = a[14][j] - a[1][j];
            xi = a[14][j] + a[1][j];
            x3r = xr + xi;
            x3i = xr - xi;
            xr = x0r + x2r;
            xi = x1r + x3r;
            a[0][j] = C16_8R * (xr + xi);
            a[8][j] = C16_8R * (xr - xi);
            xr = x0r - x2r;
            xi = x1r - x3r;
            a[4][j] = C16_4R * xr - C16_4I * xi;
            a[12][j] = C16_4R * xi + C16_4I * xr;
            x0r = W16_8R * (x1i - x3i);
            x2r = W16_8R * (x1i + x3i);
            xr = x0i + x0r;
            xi = x2r + x2i;
            a[2][j] = C16_2R * xr - C16_2I * xi;
            a[14][j] = C16_2R * xi + C16_2I * xr;
            xr = x0i - x0r;
            xi = x2r - x2i;
            a[6][j] = C16_6R * xr - C16_6I * xi;
            a[10][j] = C16_6R * xi + C16_6I * xr;
            xr = W16_8R * (x6r - x6i);
            xi = W16_8R * (x6i + x6r);
            x6r = x4r - xr;
            x6i = x4i - xi;
            x4r += xr;
            x4i += xi;
            xr = W16_4I * x7r - W16_4R * x7i;
            xi = W16_4I * x7i + W16_4R * x7r;
            x7r = W16_4R * x5r - W16_4I * x5i;
            x7i = W16_4R * x5i + W16_4I * x5r;
            x5r = x7r + xr;
            x5i = x7i + xi;
            x7r -= xr;
            x7i -= xi;
            xr = x4r + x5r;
            xi = x5i + x4i;
            a[1][j] = C16_1R * xr - C16_1I * xi;
            a[15][j] = C16_1R * xi + C16_1I * xr;
            xr = x4r - x5r;
            xi = x5i - x4i;
            a[7][j] = C16_7R * xr - C16_7I * xi;
            a[9][j] = C16_7R * xi + C16_7I * xr;
            xr = x6r - x7i;
            xi = x7r + x6i;
            a[5][j] = C16_5R * xr - C16_5I * xi;
            a[11][j] = C16_5R * xi + C16_5I * xr;
            xr = x6r + x7i;
            xi = x7r - x6i;
            a[3][j] = C16_3R * xr - C16_3I * xi;
            a[13][j] = C16_3R * xi + C16_3I * xr;
        }
        for (j = 0; j <= 15; j++) {
            x4r = a[j][0] - a[j][15];
            xr = a[j][0] + a[j][15];
            x4i = a[j][8] - a[j][7];
            xi = a[j][8] + a[j][7];
            x0r = xr + xi;
            x0i = xr - xi;
            x5r = a[j][2] - a[j][13];
            xr = a[j][2] + a[j][13];
            x5i = a[j][10] - a[j][5];
            xi = a[j][10] + a[j][5];
            x1r = xr + xi;
            x1i = xr - xi;
            x6r = a[j][4] - a[j][11];
            xr = a[j][4] + a[j][11];
            x6i = a[j][12] - a[j][3];
            xi = a[j][12] + a[j][3];
            x2r = xr + xi;
            x2i = xr - xi;
            x7r = a[j][6] - a[j][9];
            xr = a[j][6] + a[j][9];
            x7i = a[j][14] - a[j][1];
            xi = a[j][14] + a[j][1];
            x3r = xr + xi;
            x3i = xr - xi;
            xr = x0r + x2r;
            xi = x1r + x3r;
            a[j][0] = C16_8R * (xr + xi);
            a[j][8] = C16_8R * (xr - xi);
            xr = x0r - x2r;
            xi = x1r - x3r;
            a[j][4] = C16_4R * xr - C16_4I * xi;
            a[j][12] = C16_4R * xi + C16_4I * xr;
            x0r = W16_8R * (x1i - x3i);
            x2r = W16_8R * (x1i + x3i);
            xr = x0i + x0r;
            xi = x2r + x2i;
            a[j][2] = C16_2R * xr - C16_2I * xi;
            a[j][14] = C16_2R * xi + C16_2I * xr;
            xr = x0i - x0r;
            xi = x2r - x2i;
            a[j][6] = C16_6R * xr - C16_6I * xi;
            a[j][10] = C16_6R * xi + C16_6I * xr;
            xr = W16_8R * (x6r - x6i);
            xi = W16_8R * (x6i + x6r);
            x6r = x4r - xr;
            x6i = x4i - xi;
            x4r += xr;
            x4i += xi;
            xr = W16_4I * x7r - W16_4R * x7i;
            xi = W16_4I * x7i + W16_4R * x7r;
            x7r = W16_4R * x5r - W16_4I * x5i;
            x7i = W16_4R * x5i + W16_4I * x5r;
            x5r = x7r + xr;
            x5i = x7i + xi;
            x7r -= xr;
            x7i -= xi;
            xr = x4r + x5r;
            xi = x5i + x4i;
            a[j][1] = C16_1R * xr - C16_1I * xi;
            a[j][15] = C16_1R * xi + C16_1I * xr;
            xr = x4r - x5r;
            xi = x5i - x4i;
            a[j][7] = C16_7R * xr - C16_7I * xi;
            a[j][9] = C16_7R * xi + C16_7I * xr;
            xr = x6r - x7i;
            xi = x7r + x6i;
            a[j][5] = C16_5R * xr - C16_5I * xi;
            a[j][11] = C16_5R * xi + C16_5I * xr;
            xr = x6r + x7i;
            xi = x7r - x6i;
            a[j][3] = C16_3R * xr - C16_3I * xi;
            a[j][13] = C16_3R * xi + C16_3I * xr;
        }
    } else {
        for (j = 0; j <= 15; j++) {
            x5r = C16_1R * a[1][j] + C16_1I * a[15][j];
            x5i = C16_1R * a[15][j] - C16_1I * a[1][j];
            xr = C16_7R * a[7][j] + C16_7I * a[9][j];
            xi = C16_7R * a[9][j] - C16_7I * a[7][j];
            x4r = x5r + xr;
            x4i = x5i - xi;
            x5r -= xr;
            x5i += xi;
            x7r = C16_5R * a[5][j] + C16_5I * a[11][j];
            x7i = C16_5R * a[11][j] - C16_5I * a[5][j];
            xr = C16_3R * a[3][j] + C16_3I * a[13][j];
            xi = C16_3R * a[13][j] - C16_3I * a[3][j];
            x6r = x7r + xr;
            x6i = x7i - xi;
            x7r -= xr;
            x7i += xi;
            xr = x4r - x6r;
            xi = x4i - x6i;
            x4r += x6r;
            x4i += x6i;
            x6r = W16_8R * (xi + xr);
            x6i = W16_8R * (xi - xr);
            xr = x5r + x7i;
            xi = x5i - x7r;
            x5r -= x7i;
            x5i += x7r;
            x7r = W16_4I * x5r + W16_4R * x5i;
            x7i = W16_4I * x5i - W16_4R * x5r;
            x5r = W16_4R * xr + W16_4I * xi;
            x5i = W16_4R * xi - W16_4I * xr;
            xr = C16_4R * a[4][j] + C16_4I * a[12][j];
            xi = C16_4R * a[12][j] - C16_4I * a[4][j];
            x2r = C16_8R * (a[0][j] + a[8][j]);
            x3r = C16_8R * (a[0][j] - a[8][j]);
            x0r = x2r + xr;
            x1r = x3r + xi;
            x2r -= xr;
            x3r -= xi;
            x0i = C16_2R * a[2][j] + C16_2I * a[14][j];
            x2i = C16_2R * a[14][j] - C16_2I * a[2][j];
            x1i = C16_6R * a[6][j] + C16_6I * a[10][j];
            x3i = C16_6R * a[10][j] - C16_6I * a[6][j];
            xr = x0i - x1i;
            xi = x2i + x3i;
            x0i += x1i;
            x2i -= x3i;
            x1i = W16_8R * (xi + xr);
            x3i = W16_8R * (xi - xr);
            xr = x0r + x0i;
            xi = x0r - x0i;
            a[0][j] = xr + x4r;
            a[15][j] = xr - x4r;
            a[8][j] = xi + x4i;
            a[7][j] = xi - x4i;
            xr = x1r + x1i;
            xi = x1r - x1i;
            a[2][j] = xr + x5r;
            a[13][j] = xr - x5r;
            a[10][j] = xi + x5i;
            a[5][j] = xi - x5i;
            xr = x2r + x2i;
            xi = x2r - x2i;
            a[4][j] = xr + x6r;
            a[11][j] = xr - x6r;
            a[12][j] = xi + x6i;
            a[3][j] = xi - x6i;
            xr = x3r + x3i;
            xi = x3r - x3i;
            a[6][j] = xr + x7r;
            a[9][j] = xr - x7r;
            a[14][j] = xi + x7i;
            a[1][j] = xi - x7i;
        }
        for (j = 0; j <= 15; j++) {
            x5r = C16_1R * a[j][1] + C16_1I * a[j][15];
            x5i = C16_1R * a[j][15] - C16_1I * a[j][1];
            xr = C16_7R * a[j][7] + C16_7I * a[j][9];
            xi = C16_7R * a[j][9] - C16_7I * a[j][7];
            x4r = x5r + xr;
            x4i = x5i - xi;
            x5r -= xr;
            x5i += xi;
            x7r = C16_5R * a[j][5] + C16_5I * a[j][11];
            x7i = C16_5R * a[j][11] - C16_5I * a[j][5];
            xr = C16_3R * a[j][3] + C16_3I * a[j][13];
            xi = C16_3R * a[j][13] - C16_3I * a[j][3];
            x6r = x7r + xr;
            x6i = x7i - xi;
            x7r -= xr;
            x7i += xi;
            xr = x4r - x6r;
            xi = x4i - x6i;
            x4r += x6r;
            x4i += x6i;
            x6r = W16_8R * (xi + xr);
            x6i = W16_8R * (xi - xr);
            xr = x5r + x7i;
            xi = x5i - x7r;
            x5r -= x7i;
            x5i += x7r;
            x7r = W16_4I * x5r + W16_4R * x5i;
            x7i = W16_4I * x5i - W16_4R * x5r;
            x5r = W16_4R * xr + W16_4I * xi;
            x5i = W16_4R * xi - W16_4I * xr;
            xr = C16_4R * a[j][4] + C16_4I * a[j][12];
            xi = C16_4R * a[j][12] - C16_4I * a[j][4];
            x2r = C16_8R * (a[j][0] + a[j][8]);
            x3r = C16_8R * (a[j][0] - a[j][8]);
            x0r = x2r + xr;
            x1r = x3r + xi;
            x2r -= xr;
            x3r -= xi;
            x0i = C16_2R * a[j][2] + C16_2I * a[j][14];
            x2i = C16_2R * a[j][14] - C16_2I * a[j][2];
            x1i = C16_6R * a[j][6] + C16_6I * a[j][10];
            x3i = C16_6R * a[j][10] - C16_6I * a[j][6];
            xr = x0i - x1i;
            xi = x2i + x3i;
            x0i += x1i;
            x2i -= x3i;
            x1i = W16_8R * (xi + xr);
            x3i = W16_8R * (xi - xr);
            xr = x0r + x0i;
            xi = x0r - x0i;
            a[j][0] = xr + x4r;
            a[j][15] = xr - x4r;
            a[j][8] = xi + x4i;
            a[j][7] = xi - x4i;
            xr = x1r + x1i;
            xi = x1r - x1i;
            a[j][2] = xr + x5r;
            a[j][13] = xr - x5r;
            a[j][10] = xi + x5i;
            a[j][5] = xi - x5i;
            xr = x2r + x2i;
            xi = x2r - x2i;
            a[j][4] = xr + x6r;
            a[j][11] = xr - x6r;
            a[j][12] = xi + x6i;
            a[j][3] = xi - x6i;
            xr = x3r + x3i;
            xi = x3r - x3i;
            a[j][6] = xr + x7r;
            a[j][9] = xr - x7r;
            a[j][14] = xi + x7i;
            a[j][1] = xi - x7i;
        }
    }
}

