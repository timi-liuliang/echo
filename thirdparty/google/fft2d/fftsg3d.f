! Fast Fourier/Cosine/Sine Transform
!     dimension   :three
!     data length :power of 2
!     decimation  :frequency
!     radix       :split-radix, row-column
!     data        :inplace
!     table       :use
! subroutines
!     cdft3d: Complex Discrete Fourier Transform
!     rdft3d: Real Discrete Fourier Transform
!     ddct3d: Discrete Cosine Transform
!     ddst3d: Discrete Sine Transform
! necessary package
!     fftsg.f  : 1D-FFT package
!
!
! -------- Complex DFT (Discrete Fourier Transform) --------
!     [definition]
!         <case1>
!             X(k1,k2,k3) = sum_j1=0^n1-1 sum_j2=0^n2-1 sum_j3=0^n3-1
!                               x(j1,j2,j3) * 
!                               exp(2*pi*i*j1*k1/n1) * 
!                               exp(2*pi*i*j2*k2/n2) * 
!                               exp(2*pi*i*j3*k3/n3), 
!                               0<=k1<n1, 0<=k2<n2, 0<=k3<n3
!         <case2>
!             X(k1,k2,k3) = sum_j1=0^n1-1 sum_j2=0^n2-1 sum_j3=0^n3-1
!                               x(j1,j2,j3) * 
!                               exp(-2*pi*i*j1*k1/n1) * 
!                               exp(-2*pi*i*j2*k2/n2) * 
!                               exp(-2*pi*i*j3*k3/n3), 
!                               0<=k1<n1, 0<=k2<n2, 0<=k3<n3
!         (notes: sum_j=0^n-1 is a summation from j=0 to n-1)
!     [usage]
!         <case1>
!             ip(0) = 0  ! first time only
!             call cdft3d(n1max, n2max, 2*n1, n2, n3, 1, a, t, ip, w)
!         <case2>
!             ip(0) = 0  ! first time only
!             call cdft3d(n1max, n2max, 2*n1, n2, n3, -1, a, t, ip, w)
!     [parameters]
!         n1max  :row1 size of the 3D array (integer)
!         n2max  :row2 size of the 3D array (integer)
!         2*n1   :data length (integer)
!                 n1 >= 1, n1 = power of 2
!         n2     :data length (integer)
!                 n2 >= 1, n2 = power of 2
!         n3     :data length (integer)
!                 n3 >= 1, n3 = power of 2
!         a(0:2*n1-1,0:n2-1,0:n3-1)
!                :input/output data (real*8)
!                 input data
!                     a(2*j1,j2,j3) = Re(x(j1,j2,j3)), 
!                     a(2*j1+1,j2,j3) = Im(x(j1,j2,j3)), 
!                     0<=j1<n1, 0<=j2<n2, 0<=j3<n3
!                 output data
!                     a(2*k1,k2,k3) = Re(X(k1,k2,k3)), 
!                     a(2*k1+1,k2,k3) = Im(X(k1,k2,k3)), 
!                     0<=k1<n1, 0<=k2<n2, 0<=k3<n3
!         t(0:*) :work area (real*8)
!                 length of t >= max(8*n2, 8*n3)
!         ip(0:*):work area for bit reversal (integer)
!                 length of ip >= 2+sqrt(n)
!                 (n = max(n1, n2, n3))
!                 ip(0),ip(1) are pointers of the cos/sin table.
!         w(0:*) :cos/sin table (real*8)
!                 length of w >= max(n1/2, n2/2, n3/2)
!                 w(),ip() are initialized if ip(0) = 0.
!     [remark]
!         Inverse of 
!             call cdft3d(n1max, n2max, 2*n1, n2, n3, -1, a, t, ip, w)
!         is 
!             call cdft3d(n1max, n2max, 2*n1, n2, n3, 1, a, t, ip, w)
!             do j3 = 0, n3 - 1
!                 do j2 = 0, n2 - 1
!                     do j1 = 0, 2 * n1 - 1
!                         a(j1,j2,j3) = a(j1,j2,j3) * (1.0d0/n1/n2/n3)
!                     end do
!                 end do
!             end do
!         .
!
!
! -------- Real DFT / Inverse of Real DFT --------
!     [definition]
!         <case1> RDFT
!             R(k1,k2,k3) = sum_j1=0^n1-1 sum_j2=0^n2-1 sum_j3=0^n3-1
!                               a(j1,j2,j3) * 
!                               cos(2*pi*j1*k1/n1 + 2*pi*j2*k2/n2 + 
!                                   2*pi*j3*k3/n3), 
!                               0<=k1<n1, 0<=k2<n2, 0<=k3<n3
!             I(k1,k2,k3) = sum_j1=0^n1-1 sum_j2=0^n2-1 sum_j3=0^n3-1
!                               a(j1,j2,j3) * 
!                               sin(2*pi*j1*k1/n1 + 2*pi*j2*k2/n2 + 
!                                   2*pi*j3*k3/n3), 
!                               0<=k1<n1, 0<=k2<n2, 0<=k3<n3
!         <case2> IRDFT (excluding scale)
!             a(k1,k2,k3) = (1/2) * sum_j1=0^n1-1 sum_j2=0^n2-1 sum_j3=0^n3-1
!                               (R(j1,j2,j3) * 
!                               cos(2*pi*j1*k1/n1 + 2*pi*j2*k2/n2 + 
!                                   2*pi*j3*k3/n3) + 
!                               I(j1,j2,j3) * 
!                               sin(2*pi*j1*k1/n1 + 2*pi*j2*k2/n2 + 
!                                   2*pi*j3*k3/n3)), 
!                               0<=k1<n1, 0<=k2<n2, 0<=k3<n3
!         (notes: R(mod(n1-k1,n1),mod(n2-k2,n2),mod(n3-k3,n3)) = R(k1,k2,k3), 
!                 I(mod(n1-k1,n1),mod(n2-k2,n2),mod(n3-k3,n3)) = -I(k1,k2,k3), 
!                 0<=k1<n1, 0<=k2<n2, 0<=k3<n3)
!     [usage]
!         <case1>
!             ip(0) = 0  ! first time only
!             call rdft3d(n1max, n2max, n1, n2, n3, 1, a, t, ip, w)
!         <case2>
!             ip(0) = 0  ! first time only
!             call rdft3d(n1max, n2max, n1, n2, n3, -1, a, t, ip, w)
!     [parameters]
!         n1max  :row1 size of the 3D array (integer)
!         n2max  :row2 size of the 3D array (integer)
!         n1     :data length (integer)
!                 n1 >= 2, n1 = power of 2
!         n2     :data length (integer)
!                 n2 >= 2, n2 = power of 2
!         n3     :data length (integer)
!                 n3 >= 2, n3 = power of 2
!         a(0:n1-1,0:n2-1,0:n3-1)
!                :input/output data (real*8)
!                 <case1>
!                     output data
!                         a(2*k1,k2,k3) = R(k1,k2,k3)
!                                 = R(n1-k1,mod(n2-k2,n2),mod(n3-k3,n3)), 
!                         a(2*k1+1,k2,k3) = I(k1,k2,k3)
!                                 = -I(n1-k1,mod(n2-k2,n2),mod(n3-k3,n3)), 
!                             0<k1<n1/2, 0<=k2<n2, 0<=k3<n3, 
!                         a(0,k2,k3) = R(0,k2,k3)
!                                    = R(0,n2-k2,mod(n3-k3,n3)), 
!                         a(1,k2,k3) = I(0,k2,k3)
!                                    = -I(0,n2-k2,mod(n3-k3,n3)), 
!                         a(1,n2-k2,k3) = R(n1/2,k2,mod(n3-k3,n3))
!                                       = R(n1/2,n2-k2,k3), 
!                         a(0,n2-k2,k3) = -I(n1/2,k2,mod(n3-k3,n3))
!                                       = I(n1/2,n2-k2,k3), 
!                             0<k2<n2/2, 0<=k3<n3, 
!                         a(0,0,k3) = R(0,0,k3)
!                                   = R(0,0,n3-k3), 
!                         a(1,0,k3) = I(0,0,k3)
!                                   = -I(0,0,n3-k3), 
!                         a(0,n2/2,k3) = R(0,n2/2,k3)
!                                      = R(0,n2/2,n3-k3), 
!                         a(1,n2/2,k3) = I(0,n2/2,k3)
!                                      = -I(0,n2/2,n3-k3), 
!                         a(1,0,n3-k3) = R(n1/2,0,k3)
!                                      = R(n1/2,0,n3-k3), 
!                         a(0,0,n3-k3) = -I(n1/2,0,k3)
!                                      = I(n1/2,0,n3-k3), 
!                         a(1,n2/2,n3-k3) = R(n1/2,n2/2,k3)
!                                         = R(n1/2,n2/2,n3-k3), 
!                         a(0,n2/2,n3-k3) = -I(n1/2,n2/2,k3)
!                                         = I(n1/2,n2/2,n3-k3), 
!                             0<k3<n3/2, 
!                         a(0,0,0) = R(0,0,0), 
!                         a(1,0,0) = R(n1/2,0,0), 
!                         a(0,0,n3/2) = R(0,0,n3/2), 
!                         a(1,0,n3/2) = R(n1/2,0,n3/2), 
!                         a(0,n2/2,0) = R(0,n2/2,0), 
!                         a(1,n2/2,0) = R(n1/2,n2/2,0), 
!                         a(0,n2/2,n3/2) = R(0,n2/2,n3/2), 
!                         a(1,n2/2,n3/2) = R(n1/2,n2/2,n3/2)
!                 <case2>
!                     input data
!                         a(2*j1,j2,j3) = R(j1,j2,j3)
!                                 = R(n1-j1,mod(n2-j2,n2),mod(n3-j3,n3)), 
!                         a(2*j1+1,j2,j3) = I(j1,j2,j3)
!                                 = -I(n1-j1,mod(n2-j2,n2),mod(n3-j3,n3)), 
!                             0<j1<n1/2, 0<=j2<n2, 0<=j3<n3, 
!                         a(0,j2,j3) = R(0,j2,j3)
!                                    = R(0,n2-j2,mod(n3-j3,n3)), 
!                         a(1,j2,j3) = I(0,j2,j3)
!                                    = -I(0,n2-j2,mod(n3-j3,n3)), 
!                         a(1,n2-j2,j3) = R(n1/2,j2,mod(n3-j3,n3))
!                                       = R(n1/2,n2-j2,j3), 
!                         a(0,n2-j2,j3) = -I(n1/2,j2,mod(n3-j3,n3))
!                                       = I(n1/2,n2-j2,j3), 
!                             0<j2<n2/2, 0<=j3<n3, 
!                         a(0,0,j3) = R(0,0,j3)
!                                   = R(0,0,n3-j3), 
!                         a(1,0,j3) = I(0,0,j3)
!                                   = -I(0,0,n3-j3), 
!                         a(0,n2/2,j3) = R(0,n2/2,j3)
!                                      = R(0,n2/2,n3-j3), 
!                         a(1,n2/2,j3) = I(0,n2/2,j3)
!                                      = -I(0,n2/2,n3-j3), 
!                         a(1,0,n3-j3) = R(n1/2,0,j3)
!                                      = R(n1/2,0,n3-j3), 
!                         a(0,0,n3-j3) = -I(n1/2,0,j3)
!                                      = I(n1/2,0,n3-j3), 
!                         a(1,n2/2,n3-j3) = R(n1/2,n2/2,j3)
!                                         = R(n1/2,n2/2,n3-j3), 
!                         a(0,n2/2,n3-j3) = -I(n1/2,n2/2,j3)
!                                         = I(n1/2,n2/2,n3-j3), 
!                             0<j3<n3/2, 
!                         a(0,0,0) = R(0,0,0), 
!                         a(1,0,0) = R(n1/2,0,0), 
!                         a(0,0,n3/2) = R(0,0,n3/2), 
!                         a(1,0,n3/2) = R(n1/2,0,n3/2), 
!                         a(0,n2/2,0) = R(0,n2/2,0), 
!                         a(1,n2/2,0) = R(n1/2,n2/2,0), 
!                         a(0,n2/2,n3/2) = R(0,n2/2,n3/2), 
!                         a(1,n2/2,n3/2) = R(n1/2,n2/2,n3/2)
!                 ---- output ordering ----
!                     call rdft3d(n1max, n2max, n1, n2, n3, 1, a, t, ip, w)
!                     call rdft3dsort(n1max, n2max, n1, n2, n3, 1, a)
!                     ! stored data is a(0:n1-1,0:n2-1,0:n3+1):
!                     ! a(2*k1,k2,k3) = R(k1,k2,k3), 
!                     ! a(2*k1+1,k2,k3) = I(k1,k2,k3), 
!                     ! 0<=k1<=n1/2, 0<=k2<n2, 0<=k3<n3.
!                     ! the stored data is larger than the input data!
!                 ---- input ordering ----
!                     call rdft3dsort(n1max, n2max, n1, n2, n3, -1, a)
!                     call rdft3d(n1max, n2max, n1, n2, n3, -1, a, t, ip, w)
!         t(0:*) :work area (real*8)
!                 length of t >= max(8*n2, 8*n3)
!         ip(0:*):work area for bit reversal (integer)
!                 length of ip >= 2+sqrt(n)
!                 (n = max(n1/2, n2, n3))
!                 ip(0),ip(1) are pointers of the cos/sin table.
!         w(0:*) :cos/sin table (real*8)
!                 length of w >= max(n1/4, n2/2, n3/2) + n1/4
!                 w(),ip() are initialized if ip(0) = 0.
!     [remark]
!         Inverse of 
!             call rdft3d(n1max, n2max, n1, n2, n3, 1, a, t, ip, w)
!         is 
!             call rdft3d(n1max, n2max, n1, n2, n3, -1, a, t, ip, w)
!             do j3 = 0, n3 - 1
!                 do j2 = 0, n2 - 1
!                     do j1 = 0, n1 - 1
!                         a(j1,j2,j3) = a(j1,j2,j3) * (2.0d0/n1/n2/n3)
!                     end do
!                 end do
!             end do
!         .
!
!
! -------- DCT (Discrete Cosine Transform) / Inverse of DCT --------
!     [definition]
!         <case1> IDCT (excluding scale)
!             C(k1,k2,k3) = sum_j1=0^n1-1 sum_j2=0^n2-1 sum_j3=0^n3-1
!                               a(j1,j2,j3) * 
!                               cos(pi*j1*(k1+1/2)/n1) * 
!                               cos(pi*j2*(k2+1/2)/n2) * 
!                               cos(pi*j3*(k3+1/2)/n3), 
!                               0<=k1<n1, 0<=k2<n2, 0<=k3<n3
!         <case2> DCT
!             C(k1,k2,k3) = sum_j1=0^n1-1 sum_j2=0^n2-1 sum_j3=0^n3-1
!                               a(j1,j2,j3) * 
!                               cos(pi*(j1+1/2)*k1/n1) * 
!                               cos(pi*(j2+1/2)*k2/n2) * 
!                               cos(pi*(j3+1/2)*k3/n3), 
!                               0<=k1<n1, 0<=k2<n2, 0<=k3<n3
!     [usage]
!         <case1>
!             ip(0) = 0  ! first time only
!             call ddct3d(n1max, n2max, n1, n2, n3, 1, a, t, ip, w)
!         <case2>
!             ip(0) = 0  ! first time only
!             call ddct3d(n1max, n2max, n1, n2, n3, -1, a, t, ip, w)
!     [parameters]
!         n1max  :row1 size of the 3D array (integer)
!         n2max  :row2 size of the 3D array (integer)
!         n1     :data length (integer)
!                 n1 >= 2, n1 = power of 2
!         n2     :data length (integer)
!                 n2 >= 2, n2 = power of 2
!         n3     :data length (integer)
!                 n3 >= 2, n3 = power of 2
!         a(0:n1-1,0:n2-1,0:n3-1)
!                :input/output data (real*8)
!                 output data
!                     a(k1,k2,k3) = C(k1,k2,k3), 
!                         0<=k1<n1, 0<=k2<n2, 0<=k3<n3
!         t(0:*) :work area (real*8)
!                 length of t >= max(4*n2, 4*n3)
!         ip(0:*):work area for bit reversal (integer)
!                 length of ip >= 2+sqrt(n)
!                 (n = max(n1/2, n2/2, n3/2))
!                 ip(0),ip(1) are pointers of the cos/sin table.
!         w(0:*) :cos/sin table (real*8)
!                 length of w >= max(n1*3/2, n2*3/2, n3*3/2)
!                 w(),ip() are initialized if ip(0) = 0.
!     [remark]
!         Inverse of 
!             call ddct3d(n1max, n2max, n1, n2, n3, -1, a, t, ip, w)
!         is 
!             do j3 = 0, n3 - 1
!                 do j2 = 0, n2 - 1
!                     a(0, j2, j3) = a(0, j2, j3) * 0.5d0
!                 end do
!                 do j1 = 0, n1 - 1
!                     a(j1, 0, j3) = a(j1, 0, j3) * 0.5d0
!                 end do
!             end do
!             do j2 = 0, n2 - 1
!                 do j1 = 0, n1 - 1
!                     a(j1, j2, 0) = a(j1, j2, 0) * 0.5d0
!                 end do
!             end do
!             call ddct3d(n1max, n2max, n1, n2, n3, 1, a, t, ip, w)
!             do j3 = 0, n3 - 1
!                 do j2 = 0, n2 - 1
!                     do j1 = 0, n1 - 1
!                         a(j1,j2,j3) = a(j1,j2,j3) * (8.0d0/n1/n2/n3)
!                     end do
!                 end do
!             end do
!         .
!
!
! -------- DST (Discrete Sine Transform) / Inverse of DST --------
!     [definition]
!         <case1> IDST (excluding scale)
!             S(k1,k2,k3) = sum_j1=1^n1 sum_j2=1^n2 sum_j3=1^n3
!                               A(j1,j2,j3) * 
!                               sin(pi*j1*(k1+1/2)/n1) * 
!                               sin(pi*j2*(k2+1/2)/n2) * 
!                               sin(pi*j3*(k3+1/2)/n3), 
!                               0<=k1<n1, 0<=k2<n2, 0<=k3<n3
!         <case2> DST
!             S(k1,k2,k3) = sum_j1=0^n1-1 sum_j2=0^n2-1 sum_j3=0^n3-1
!                               a(j1,j2,j3) * 
!                               sin(pi*(j1+1/2)*k1/n1) * 
!                               sin(pi*(j2+1/2)*k2/n2) * 
!                               sin(pi*(j3+1/2)*k3/n3), 
!                               0<k1<=n1, 0<k2<=n2, 0<k3<=n3
!     [usage]
!         <case1>
!             ip(0) = 0  ! first time only
!             call ddst3d(n1max, n2max, n1, n2, n3, 1, a, t, ip, w)
!         <case2>
!             ip(0) = 0  ! first time only
!             call ddst3d(n1max, n2max, n1, n2, n3, -1, a, t, ip, w)
!     [parameters]
!         n1max  :row1 size of the 3D array (integer)
!         n2max  :row2 size of the 3D array (integer)
!         n1     :data length (integer)
!                 n1 >= 2, n1 = power of 2
!         n2     :data length (integer)
!                 n2 >= 2, n2 = power of 2
!         n3     :data length (integer)
!                 n3 >= 2, n3 = power of 2
!         a(0:n1-1,0:n2-1,0:n3-1)
!                :input/output data (real*8)
!                 <case1>
!                     input data
!                         a(mod(j1,n1),mod(j2,n2),mod(j3,n3)) = A(j1,j2,j3), 
!                             0<j1<=n1, 0<j2<=n2, 0<j3<=n3
!                     output data
!                         a(k1,k2,k3) = S(k1,k2,k3), 
!                             0<=k1<n1, 0<=k2<n2, 0<=k3<n3
!                 <case2>
!                     output data
!                         a(mod(k1,n1),mod(k2,n2),mod(k3,n3)) = S(k1,k2,k3), 
!                             0<k1<=n1, 0<k2<=n2, 0<k3<=n3
!         t(0:*) :work area (real*8)
!                 length of t >= max(4*n2, 4*n3)
!         ip(0:*):work area for bit reversal (integer)
!                 length of ip >= 2+sqrt(n)
!                 (n = max(n1/2, n2/2, n3/2))
!                 ip(0),ip(1) are pointers of the cos/sin table.
!         w(0:*) :cos/sin table (real*8)
!                 length of w >= max(n1*3/2, n2*3/2, n3*3/2)
!                 w(),ip() are initialized if ip(0) = 0.
!     [remark]
!         Inverse of 
!             call ddst3d(n1max, n2max, n1, n2, n3, -1, a, t, ip, w)
!         is 
!             do j3 = 0, n3 - 1
!                 do j2 = 0, n2 - 1
!                     a(0, j2, j3) = a(0, j2, j3) * 0.5d0
!                 end do
!                 do j1 = 0, n1 - 1
!                     a(j1, 0, j3) = a(j1, 0, j3) * 0.5d0
!                 end do
!             end do
!             do j2 = 0, n2 - 1
!                 do j1 = 0, n1 - 1
!                     a(j1, j2, 0) = a(j1, j2, 0) * 0.5d0
!                 end do
!             end do
!             call ddst3d(n1max, n2max, n1, n2, n3, 1, a, t, ip, w)
!             do j3 = 0, n3 - 1
!                 do j2 = 0, n2 - 1
!                     do j1 = 0, n1 - 1
!                         a(j1,j2,j3) = a(j1,j2,j3) * (8.0d0/n1/n2/n3)
!                     end do
!                 end do
!             end do
!         .
!
!
      subroutine cdft3d(n1max, n2max, n1, n2, n3, isgn, a, 
     &    t, ip, w)
      integer n1max, n2max, n1, n2, n3, isgn, ip(0 : *), n
      real*8 a(0 : n1max - 1, 0 : n2max - 1, 0 : n3 - 1), 
     &    t(0 : *), w(0 : *)
      n = 2 * max(n2, n3)
      n = max(n, n1)
      if (n .gt. 4 * ip(0)) then
          call makewt(n / 4, ip, w)
      end if
      call xdft3da_sub(n1max, n2max, n1, n2, n3, 0, 
     &    isgn, a, t, ip, w)
      call cdft3db_sub(n1max, n2max, n1, n2, n3, 
     &    isgn, a, t, ip, w)
      end
!
      subroutine rdft3d(n1max, n2max, n1, n2, n3, isgn, a, 
     &    t, ip, w)
      integer n1max, n2max, n1, n2, n3, isgn, ip(0 : *), 
     &    n, nw, nc
      real*8 a(0 : n1max - 1, 0 : n2max - 1, 0 : n3 - 1), 
     &    t(0 : *), w(0 : *)
      n = 2 * max(n2, n3)
      n = max(n, n1)
      nw = ip(0)
      if (n .gt. 4 * nw) then
          nw = n / 4
          call makewt(nw, ip, w)
      end if
      nc = ip(1)
      if (n1 .gt. 4 * nc) then
          nc = n1 / 4
          call makect(nc, ip, w(nw))
      end if
      if (isgn .lt. 0) then
          call rdft3d_sub(n1max, n2max, n1, n2, n3, isgn, a)
          call cdft3db_sub(n1max, n2max, n1, n2, n3, 
     &        isgn, a, t, ip, w)
          call xdft3da_sub(n1max, n2max, n1, n2, n3, 1, 
     &        isgn, a, t, ip, w)
      else
          call xdft3da_sub(n1max, n2max, n1, n2, n3, 1, 
     &        isgn, a, t, ip, w)
          call cdft3db_sub(n1max, n2max, n1, n2, n3, 
     &        isgn, a, t, ip, w)
          call rdft3d_sub(n1max, n2max, n1, n2, n3, isgn, a)
      end if
      end
!
      subroutine rdft3dsort(n1max, n2max, n1, n2, n3, isgn, a)
      integer n1max, n2max, n1, n2, n3, isgn, n2h, n3h, j, k
      real*8 a(0 : n1max - 1, 0 : n2max - 1, 0 : n3 - 1), x, y
      n2h = n2 / 2
      n3h = n3 / 2
      if (isgn .lt. 0) then
          do k = 0, n3 - 1
              do j = n2h + 1, n2 - 1
                  a(0, j, k) = a(n1 + 1, j, k)
                  a(1, j, k) = a(n1, j, k)
              end do
          end do
          do k = n3h + 1, n3 - 1
              a(0, 0, k) = a(n1 + 1, 0, k)
              a(1, 0, k) = a(n1, 0, k)
              a(0, n2h, k) = a(n1 + 1, n2h, k)
              a(1, n2h, k) = a(n1, n2h, k)
          end do
          a(1, 0, 0) = a(n1, 0, 0)
          a(1, n2h, 0) = a(n1, n2h, 0)
          a(1, 0, n3h) = a(n1, 0, n3h)
          a(1, n2h, n3h) = a(n1, n2h, n3h)
      else
          do j = n2h + 1, n2 - 1
              y = a(0, j, 0)
              x = a(1, j, 0)
              a(n1, j, 0) = x
              a(n1 + 1, j, 0) = y
              a(n1, n2 - j, 0) = x
              a(n1 + 1, n2 - j, 0) = -y
              a(0, j, 0) = a(0, n2 - j, 0)
              a(1, j, 0) = -a(1, n2 - j, 0)
          end do
          do k = 1, n3 - 1
              do j = n2h + 1, n2 - 1
                  y = a(0, j, k)
                  x = a(1, j, k)
                  a(n1, j, k) = x
                  a(n1 + 1, j, k) = y
                  a(n1, n2 - j, n3 - k) = x
                  a(n1 + 1, n2 - j, n3 - k) = -y
                  a(0, j, k) = a(0, n2 - j, n3 - k)
                  a(1, j, k) = -a(1, n2 - j, n3 - k)
              end do
          end do
          do k = n3h + 1, n3 - 1
              y = a(0, 0, k)
              x = a(1, 0, k)
              a(n1, 0, k) = x
              a(n1 + 1, 0, k) = y
              a(n1, 0, n3 - k) = x
              a(n1 + 1, 0, n3 - k) = -y
              a(0, 0, k) = a(0, 0, n3 - k)
              a(1, 0, k) = -a(1, 0, n3 - k)
              y = a(0, n2h, k)
              x = a(1, n2h, k)
              a(n1, n2h, k) = x
              a(n1 + 1, n2h, k) = y
              a(n1, n2h, n3 - k) = x
              a(n1 + 1, n2h, n3 - k) = -y
              a(0, n2h, k) = a(0, n2h, n3 - k)
              a(1, n2h, k) = -a(1, n2h, n3 - k)
          end do
          a(n1, 0, 0) = a(1, 0, 0)
          a(n1 + 1, 0, 0) = 0
          a(1, 0, 0) = 0
          a(n1, n2h, 0) = a(1, n2h, 0)
          a(n1 + 1, n2h, 0) = 0
          a(1, n2h, 0) = 0
          a(n1, 0, n3h) = a(1, 0, n3h)
          a(n1 + 1, 0, n3h) = 0
          a(1, 0, n3h) = 0
          a(n1, n2h, n3h) = a(1, n2h, n3h)
          a(n1 + 1, n2h, n3h) = 0
          a(1, n2h, n3h) = 0
      end if
      end
!
      subroutine ddct3d(n1max, n2max, n1, n2, n3, isgn, a, 
     &    t, ip, w)
      integer n1max, n2max, n1, n2, n3, isgn, ip(0 : *), 
     &    n, nw, nc
      real*8 a(0 : n1max - 1, 0 : n2max - 1, 0 : n3 - 1), 
     &    t(0 : *), w(0 : *)
      n = max(n2, n3)
      n = max(n, n1)
      nw = ip(0)
      if (n .gt. 4 * nw) then
          nw = n / 4
          call makewt(nw, ip, w)
      end if
      nc = ip(1)
      if (n .gt. nc) then
          nc = n
          call makect(nc, ip, w(nw))
      end if
      call ddxt3da_sub(n1max, n2max, n1, n2, n3, 0, 
     &    isgn, a, t, ip, w)
      call ddxt3db_sub(n1max, n2max, n1, n2, n3, 0, 
     &    isgn, a, t, ip, w)
      end
!
      subroutine ddst3d(n1max, n2max, n1, n2, n3, isgn, a, 
     &    t, ip, w)
      integer n1max, n2max, n1, n2, n3, isgn, ip(0 : *), 
     &    n, nw, nc
      real*8 a(0 : n1max - 1, 0 : n2max - 1, 0 : n3 - 1), 
     &    t(0 : *), w(0 : *)
      n = max(n2, n3)
      n = max(n, n1)
      nw = ip(0)
      if (n .gt. 4 * nw) then
          nw = n / 4
          call makewt(nw, ip, w)
      end if
      nc = ip(1)
      if (n .gt. nc) then
          nc = n
          call makect(nc, ip, w(nw))
      end if
      call ddxt3da_sub(n1max, n2max, n1, n2, n3, 1, 
     &    isgn, a, t, ip, w)
      call ddxt3db_sub(n1max, n2max, n1, n2, n3, 1, 
     &    isgn, a, t, ip, w)
      end
!
! -------- child routines --------
!
      subroutine xdft3da_sub(n1max, n2max, n1, n2, n3, icr, 
     &    isgn, a, t, ip, w)
      integer n1max, n2max, n1, n2, n3, icr, isgn, 
     &    ip(0 : *), i, j, k
      real*8 a(0 : n1max - 1, 0 : n2max - 1, 0 : n3 - 1), 
     &    t(0 : *), w(0 : *)
      do k = 0, n3 - 1
          if (icr .eq. 0) then
              do j = 0, n2 - 1
                  call cdft(n1, isgn, a(0, j, k), ip, w)
              end do
          else if (isgn .ge. 0) then
              do j = 0, n2 - 1
                  call rdft(n1, isgn, a(0, j, k), ip, w)
              end do
          end if
          if (n1 .gt. 4) then
              do i = 0, n1 - 8, 8
                  do j = 0, n2 - 1
                      t(2 * j) = a(i, j, k)
                      t(2 * j + 1) = a(i + 1, j, k)
                      t(2 * n2 + 2 * j) = a(i + 2, j, k)
                      t(2 * n2 + 2 * j + 1) = a(i + 3, j, k)
                      t(4 * n2 + 2 * j) = a(i + 4, j, k)
                      t(4 * n2 + 2 * j + 1) = a(i + 5, j, k)
                      t(6 * n2 + 2 * j) = a(i + 6, j, k)
                      t(6 * n2 + 2 * j + 1) = a(i + 7, j, k)
                  end do
                  call cdft(2 * n2, isgn, t, ip, w)
                  call cdft(2 * n2, isgn, t(2 * n2), ip, w)
                  call cdft(2 * n2, isgn, t(4 * n2), ip, w)
                  call cdft(2 * n2, isgn, t(6 * n2), ip, w)
                  do j = 0, n2 - 1
                      a(i, j, k) = t(2 * j)
                      a(i + 1, j, k) = t(2 * j + 1)
                      a(i + 2, j, k) = t(2 * n2 + 2 * j)
                      a(i + 3, j, k) = t(2 * n2 + 2 * j + 1)
                      a(i + 4, j, k) = t(4 * n2 + 2 * j)
                      a(i + 5, j, k) = t(4 * n2 + 2 * j + 1)
                      a(i + 6, j, k) = t(6 * n2 + 2 * j)
                      a(i + 7, j, k) = t(6 * n2 + 2 * j + 1)
                  end do
              end do
          else if (n1 .eq. 4) then
              do j = 0, n2 - 1
                  t(2 * j) = a(0, j, k)
                  t(2 * j + 1) = a(1, j, k)
                  t(2 * n2 + 2 * j) = a(2, j, k)
                  t(2 * n2 + 2 * j + 1) = a(3, j, k)
              end do
              call cdft(2 * n2, isgn, t, ip, w)
              call cdft(2 * n2, isgn, t(2 * n2), ip, w)
              do j = 0, n2 - 1
                  a(0, j, k) = t(2 * j)
                  a(1, j, k) = t(2 * j + 1)
                  a(2, j, k) = t(2 * n2 + 2 * j)
                  a(3, j, k) = t(2 * n2 + 2 * j + 1)
              end do
          else if (n1 .eq. 2) then
              do j = 0, n2 - 1
                  t(2 * j) = a(0, j, k)
                  t(2 * j + 1) = a(1, j, k)
              end do
              call cdft(2 * n2, isgn, t, ip, w)
              do j = 0, n2 - 1
                  a(0, j, k) = t(2 * j)
                  a(1, j, k) = t(2 * j + 1)
              end do
          end if
          if (icr .ne. 0 .and. isgn .lt. 0) then
              do j = 0, n2 - 1
                  call rdft(n1, isgn, a(0, j, k), ip, w)
              end do
          end if
      end do
      end
!
      subroutine cdft3db_sub(n1max, n2max, n1, n2, n3, 
     &    isgn, a, t, ip, w)
      integer n1max, n2max, n1, n2, n3, isgn, ip(0 : *), 
     &    i, j, k
      real*8 a(0 : n1max - 1, 0 : n2max - 1, 0 : n3 - 1), 
     &    t(0 : *), w(0 : *)
      if (n1 .gt. 4) then
          do j = 0, n2 - 1
              do i = 0, n1 - 8, 8
                  do k = 0, n3 - 1
                      t(2 * k) = a(i, j, k)
                      t(2 * k + 1) = a(i + 1, j, k)
                      t(2 * n3 + 2 * k) = a(i + 2, j, k)
                      t(2 * n3 + 2 * k + 1) = a(i + 3, j, k)
                      t(4 * n3 + 2 * k) = a(i + 4, j, k)
                      t(4 * n3 + 2 * k + 1) = a(i + 5, j, k)
                      t(6 * n3 + 2 * k) = a(i + 6, j, k)
                      t(6 * n3 + 2 * k + 1) = a(i + 7, j, k)
                  end do
                  call cdft(2 * n3, isgn, t, ip, w)
                  call cdft(2 * n3, isgn, t(2 * n3), ip, w)
                  call cdft(2 * n3, isgn, t(4 * n3), ip, w)
                  call cdft(2 * n3, isgn, t(6 * n3), ip, w)
                  do k = 0, n3 - 1
                      a(i, j, k) = t(2 * k)
                      a(i + 1, j, k) = t(2 * k + 1)
                      a(i + 2, j, k) = t(2 * n3 + 2 * k)
                      a(i + 3, j, k) = t(2 * n3 + 2 * k + 1)
                      a(i + 4, j, k) = t(4 * n3 + 2 * k)
                      a(i + 5, j, k) = t(4 * n3 + 2 * k + 1)
                      a(i + 6, j, k) = t(6 * n3 + 2 * k)
                      a(i + 7, j, k) = t(6 * n3 + 2 * k + 1)
                  end do
              end do
          end do
      else if (n1 .eq. 4) then
          do j = 0, n2 - 1
              do k = 0, n3 - 1
                  t(2 * k) = a(0, j, k)
                  t(2 * k + 1) = a(1, j, k)
                  t(2 * n3 + 2 * k) = a(2, j, k)
                  t(2 * n3 + 2 * k + 1) = a(3, j, k)
              end do
              call cdft(2 * n3, isgn, t, ip, w)
              call cdft(2 * n3, isgn, t(2 * n3), ip, w)
              do k = 0, n3 - 1
                  a(0, j, k) = t(2 * k)
                  a(1, j, k) = t(2 * k + 1)
                  a(2, j, k) = t(2 * n3 + 2 * k)
                  a(3, j, k) = t(2 * n3 + 2 * k + 1)
              end do
          end do
      else if (n1 .eq. 2) then
          do j = 0, n2 - 1
              do k = 0, n3 - 1
                  t(2 * k) = a(0, j, k)
                  t(2 * k + 1) = a(1, j, k)
              end do
              call cdft(2 * n3, isgn, t, ip, w)
              do k = 0, n3 - 1
                  a(0, j, k) = t(2 * k)
                  a(1, j, k) = t(2 * k + 1)
              end do
          end do
      end if
      end
!
      subroutine rdft3d_sub(n1max, n2max, n1, n2, n3, isgn, a)
      integer n1max, n2max, n1, n2, n3, isgn, 
     &    n2h, n3h, i, j, k, l
      real*8 a(0 : n1max - 1, 0 : n2max - 1, 0 : n3 - 1), xi
      n2h = n2 / 2
      n3h = n3 / 2
      if (isgn .lt. 0) then
          do k = 1, n3h - 1
              l = n3 - k
              xi = a(0, 0, k) - a(0, 0, l)
              a(0, 0, k) = a(0, 0, k) + a(0, 0, l)
              a(0, 0, l) = xi
              xi = a(1, 0, l) - a(1, 0, k)
              a(1, 0, k) = a(1, 0, k) + a(1, 0, l)
              a(1, 0, l) = xi
              xi = a(0, n2h, k) - a(0, n2h, l)
              a(0, n2h, k) = a(0, n2h, k) + a(0, n2h, l)
              a(0, n2h, l) = xi
              xi = a(1, n2h, l) - a(1, n2h, k)
              a(1, n2h, k) = a(1, n2h, k) + a(1, n2h, l)
              a(1, n2h, l) = xi
              do i = 1, n2h - 1
                  j = n2 - i
                  xi = a(0, i, k) - a(0, j, l)
                  a(0, i, k) = a(0, i, k) + a(0, j, l)
                  a(0, j, l) = xi
                  xi = a(1, j, l) - a(1, i, k)
                  a(1, i, k) = a(1, i, k) + a(1, j, l)
                  a(1, j, l) = xi
                  xi = a(0, i, l) - a(0, j, k)
                  a(0, i, l) = a(0, i, l) + a(0, j, k)
                  a(0, j, k) = xi
                  xi = a(1, j, k) - a(1, i, l)
                  a(1, i, l) = a(1, i, l) + a(1, j, k)
                  a(1, j, k) = xi
              end do
          end do
          do i = 1, n2h - 1
              j = n2 - i
              xi = a(0, i, 0) - a(0, j, 0)
              a(0, i, 0) = a(0, i, 0) + a(0, j, 0)
              a(0, j, 0) = xi
              xi = a(1, j, 0) - a(1, i, 0)
              a(1, i, 0) = a(1, i, 0) + a(1, j, 0)
              a(1, j, 0) = xi
              xi = a(0, i, n3h) - a(0, j, n3h)
              a(0, i, n3h) = a(0, i, n3h) + a(0, j, n3h)
              a(0, j, n3h) = xi
              xi = a(1, j, n3h) - a(1, i, n3h)
              a(1, i, n3h) = a(1, i, n3h) + a(1, j, n3h)
              a(1, j, n3h) = xi
          end do
      else
          do k = 1, n3h - 1
              l = n3 - k
              a(0, 0, l) = 0.5d0 * (a(0, 0, k) - a(0, 0, l))
              a(0, 0, k) = a(0, 0, k) - a(0, 0, l)
              a(1, 0, l) = 0.5d0 * (a(1, 0, k) + a(1, 0, l))
              a(1, 0, k) = a(1, 0, k) - a(1, 0, l)
              a(0, n2h, l) = 0.5d0 * (a(0, n2h, k) - a(0, n2h, l))
              a(0, n2h, k) = a(0, n2h, k) - a(0, n2h, l)
              a(1, n2h, l) = 0.5d0 * (a(1, n2h, k) + a(1, n2h, l))
              a(1, n2h, k) = a(1, n2h, k) - a(1, n2h, l)
              do i = 1, n2h - 1
                  j = n2 - i
                  a(0, j, l) = 0.5d0 * (a(0, i, k) - a(0, j, l))
                  a(0, i, k) = a(0, i, k) - a(0, j, l)
                  a(1, j, l) = 0.5d0 * (a(1, i, k) + a(1, j, l))
                  a(1, i, k) = a(1, i, k) - a(1, j, l)
                  a(0, j, k) = 0.5d0 * (a(0, i, l) - a(0, j, k))
                  a(0, i, l) = a(0, i, l) - a(0, j, k)
                  a(1, j, k) = 0.5d0 * (a(1, i, l) + a(1, j, k))
                  a(1, i, l) = a(1, i, l) - a(1, j, k)
              end do
          end do
          do i = 1, n2h - 1
              j = n2 - i
              a(0, j, 0) = 0.5d0 * (a(0, i, 0) - a(0, j, 0))
              a(0, i, 0) = a(0, i, 0) - a(0, j, 0)
              a(1, j, 0) = 0.5d0 * (a(1, i, 0) + a(1, j, 0))
              a(1, i, 0) = a(1, i, 0) - a(1, j, 0)
              a(0, j, n3h) = 0.5d0 * (a(0, i, n3h) - a(0, j, n3h))
              a(0, i, n3h) = a(0, i, n3h) - a(0, j, n3h)
              a(1, j, n3h) = 0.5d0 * (a(1, i, n3h) + a(1, j, n3h))
              a(1, i, n3h) = a(1, i, n3h) - a(1, j, n3h)
          end do
      end if
      end
!
      subroutine ddxt3da_sub(n1max, n2max, n1, n2, n3, ics, 
     &    isgn, a, t, ip, w)
      integer n1max, n2max, n1, n2, n3, ics, isgn, 
     &    ip(0 : *), i, j, k
      real*8 a(0 : n1max - 1, 0 : n2max - 1, 0 : n3 - 1), 
     &    t(0 : *), w(0 : *)
      do k = 0, n3 - 1
          if (ics .eq. 0) then
              do j = 0, n2 - 1
                  call ddct(n1, isgn, a(0, j, k), ip, w)
              end do
          else
              do j = 0, n2 - 1
                  call ddst(n1, isgn, a(0, j, k), ip, w)
              end do
          end if
          if (n1 .gt. 2) then
              do i = 0, n1 - 4, 4
                  do j = 0, n2 - 1
                      t(j) = a(i, j, k)
                      t(n2 + j) = a(i + 1, j, k)
                      t(2 * n2 + j) = a(i + 2, j, k)
                      t(3 * n2 + j) = a(i + 3, j, k)
                  end do
                  if (ics .eq. 0) then
                      call ddct(n2, isgn, t, ip, w)
                      call ddct(n2, isgn, t(n2), ip, w)
                      call ddct(n2, isgn, t(2 * n2), ip, w)
                      call ddct(n2, isgn, t(3 * n2), ip, w)
                  else
                      call ddst(n2, isgn, t, ip, w)
                      call ddst(n2, isgn, t(n2), ip, w)
                      call ddst(n2, isgn, t(2 * n2), ip, w)
                      call ddst(n2, isgn, t(3 * n2), ip, w)
                  end if
                  do j = 0, n2 - 1
                      a(i, j, k) = t(j)
                      a(i + 1, j, k) = t(n2 + j)
                      a(i + 2, j, k) = t(2 * n2 + j)
                      a(i + 3, j, k) = t(3 * n2 + j)
                  end do
              end do
          else if (n1 .eq. 2) then
              do j = 0, n2 - 1
                  t(j) = a(0, j, k)
                  t(n2 + j) = a(1, j, k)
              end do
              if (ics .eq. 0) then
                  call ddct(n2, isgn, t, ip, w)
                  call ddct(n2, isgn, t(n2), ip, w)
              else
                  call ddst(n2, isgn, t, ip, w)
                  call ddst(n2, isgn, t(n2), ip, w)
              end if
              do j = 0, n2 - 1
                  a(0, j, k) = t(j)
                  a(1, j, k) = t(n2 + j)
              end do
          end if
      end do
      end
!
      subroutine ddxt3db_sub(n1max, n2max, n1, n2, n3, ics, 
     &    isgn, a, t, ip, w)
      integer n1max, n2max, n1, n2, n3, ics, isgn, 
     &    ip(0 : *), i, j, k
      real*8 a(0 : n1max - 1, 0 : n2max - 1, 0 : n3 - 1), 
     &    t(0 : *), w(0 : *)
      if (n1 .gt. 2) then
          do j = 0, n2 - 1
              do i = 0, n1 - 4, 4
                  do k = 0, n3 - 1
                      t(k) = a(i, j, k)
                      t(n3 + k) = a(i + 1, j, k)
                      t(2 * n3 + k) = a(i + 2, j, k)
                      t(3 * n3 + k) = a(i + 3, j, k)
                  end do
                  if (ics .eq. 0) then
                      call ddct(n3, isgn, t, ip, w)
                      call ddct(n3, isgn, t(n3), ip, w)
                      call ddct(n3, isgn, t(2 * n3), ip, w)
                      call ddct(n3, isgn, t(3 * n3), ip, w)
                  else
                      call ddst(n3, isgn, t, ip, w)
                      call ddst(n3, isgn, t(n3), ip, w)
                      call ddst(n3, isgn, t(2 * n3), ip, w)
                      call ddst(n3, isgn, t(3 * n3), ip, w)
                  end if
                  do k = 0, n3 - 1
                      a(i, j, k) = t(k)
                      a(i + 1, j, k) = t(n3 + k)
                      a(i + 2, j, k) = t(2 * n3 + k)
                      a(i + 3, j, k) = t(3 * n3 + k)
                  end do
              end do
          end do
      else if (n1 .eq. 2) then
          do j = 0, n2 - 1
              do k = 0, n3 - 1
                  t(k) = a(0, j, k)
                  t(n3 + k) = a(1, j, k)
              end do
              if (ics .eq. 0) then
                  call ddct(n3, isgn, t, ip, w)
                  call ddct(n3, isgn, t(n3), ip, w)
              else
                  call ddst(n3, isgn, t, ip, w)
                  call ddst(n3, isgn, t(n3), ip, w)
              end if
              do k = 0, n3 - 1
                  a(0, j, k) = t(k)
                  a(1, j, k) = t(n3 + k)
              end do
          end do
      end if
      end
!
