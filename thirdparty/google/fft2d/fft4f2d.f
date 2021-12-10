! Fast Fourier/Cosine/Sine Transform
!     dimension   :two
!     data length :power of 2
!     decimation  :frequency
!     radix       :4, 2, row-column
!     data        :inplace
!     table       :use
! subroutines
!     cdft2d: Complex Discrete Fourier Transform
!     rdft2d: Real Discrete Fourier Transform
!     ddct2d: Discrete Cosine Transform
!     ddst2d: Discrete Sine Transform
!
!
! -------- Complex DFT (Discrete Fourier Transform) --------
!     [definition]
!         <case1>
!             X(k1,k2) = sum_j1=0^n1-1 sum_j2=0^n2-1 x(j1,j2) * 
!                            exp(2*pi*i*j1*k1/n1) * 
!                            exp(2*pi*i*j2*k2/n2), 
!                            0<=k1<n1, 0<=k2<n2
!         <case2>
!             X(k1,k2) = sum_j1=0^n1-1 sum_j2=0^n2-1 x(j1,j2) * 
!                            exp(-2*pi*i*j1*k1/n1) * 
!                            exp(-2*pi*i*j2*k2/n2), 
!                            0<=k1<n1, 0<=k2<n2
!         (notes: sum_j=0^n-1 is a summation from j=0 to n-1)
!     [usage]
!         <case1>
!             ip(0) = 0  ! first time only
!             call cdft2d(n1max, 2*n1, n2, 1, a, ip, w)
!         <case2>
!             ip(0) = 0  ! first time only
!             call cdft2d(n1max, 2*n1, n2, -1, a, ip, w)
!     [parameters]
!         n1max  :row size of the 2D array (integer)
!         2*n1   :data length (integer)
!                 n1 >= 1, n1 = power of 2
!         n2     :data length (integer)
!                 n2 >= 1, n2 = power of 2
!         a(0:2*n1-1,0:n2-1)
!                :input/output data (real*8)
!                 input data
!                     a(2*j1,j2) = Re(x(j1,j2)), 
!                     a(2*j1+1,j2) = Im(x(j1,j2)), 
!                     0<=j1<n1, 0<=j2<n2
!                 output data
!                     a(2*k1,k2) = Re(X(k1,k2)), 
!                     a(2*k1+1,k2) = Im(X(k1,k2)), 
!                     0<=k1<n1, 0<=k2<n2
!         ip(0:*):work area for bit reversal (integer)
!                 length of ip >= 2+sqrt(n)
!                 (n = max(n1, n2))
!                 ip(0),ip(1) are pointers of the cos/sin table.
!         w(0:*) :cos/sin table (real*8)
!                 length of w >= max(n1/2, n2/2)
!                 w(),ip() are initialized if ip(0) = 0.
!     [remark]
!         Inverse of 
!             call cdft2d(n1max, 2*n1, n2, -1, a, ip, w)
!         is 
!             call cdft2d(n1max, 2*n1, n2, 1, a, ip, w)
!             do j2 = 0, n2 - 1
!                 do j1 = 0, 2 * n1 - 1
!                     a(j1, j2) = a(j1, j2) * (1.0d0 / (n1 * n2))
!                 end do
!             end do
!         .
!
!
! -------- Real DFT / Inverse of Real DFT --------
!     [definition]
!         <case1> RDFT
!             R(k1,k2) = sum_j1=0^n1-1 sum_j2=0^n2-1 a(j1,j2) * 
!                            cos(2*pi*j1*k1/n1 + 2*pi*j2*k2/n2), 
!                            0<=k1<n1, 0<=k2<n2
!             I(k1,k2) = sum_j1=0^n1-1 sum_j2=0^n2-1 a(j1,j2) * 
!                            sin(2*pi*j1*k1/n1 + 2*pi*j2*k2/n2), 
!                            0<=k1<n1, 0<=k2<n2
!         <case2> IRDFT (excluding scale)
!             a(k1,k2) = (1/2) * sum_j1=0^n1-1 sum_j2=0^n2-1
!                            (R(j1,j2) * 
!                            cos(2*pi*j1*k1/n1 + 2*pi*j2*k2/n2) + 
!                            I(j1,j2) * 
!                            sin(2*pi*j1*k1/n1 + 2*pi*j2*k2/n2)), 
!                            0<=k1<n1, 0<=k2<n2
!         (notes: R(n1-k1,n2-k2) = R(k1,k2), 
!                 I(n1-k1,n2-k2) = -I(k1,k2), 
!                 R(n1-k1,0) = R(k1,0), 
!                 I(n1-k1,0) = -I(k1,0), 
!                 R(0,n2-k2) = R(0,k2), 
!                 I(0,n2-k2) = -I(0,k2), 
!                 0<k1<n1, 0<k2<n2)
!     [usage]
!         <case1>
!             ip(0) = 0  ! first time only
!             call rdft2d(n1max, n1, n2, 1, a, ip, w)
!         <case2>
!             ip(0) = 0  ! first time only
!             call rdft2d(n1max, n1, n2, -1, a, ip, w)
!     [parameters]
!         n1max  :row size of the 2D array (integer)
!         n1     :data length (integer)
!                 n1 >= 2, n1 = power of 2
!         n2     :data length (integer)
!                 n2 >= 2, n2 = power of 2
!         a(0:n1-1,0:n2-1)
!                :input/output data (real*8)
!                 <case1>
!                     output data
!                         a(2*k1,k2) = R(k1,k2) = R(n1-k1,n2-k2), 
!                         a(2*k1+1,k2) = I(k1,k2) = -I(n1-k1,n2-k2), 
!                             0<k1<n1/2, 0<k2<n2, 
!                         a(2*k1,0) = R(k1,0) = R(n1-k1,0), 
!                         a(2*k1+1,0) = I(k1,0) = -I(n1-k1,0), 
!                             0<k1<n1/2, 
!                         a(0,k2) = R(0,k2) = R(0,n2-k2), 
!                         a(1,k2) = I(0,k2) = -I(0,n2-k2), 
!                         a(1,n2-k2) = R(n1/2,k2) = R(n1/2,n2-k2), 
!                         a(0,n2-k2) = -I(n1/2,k2) = I(n1/2,n2-k2), 
!                             0<k2<n2/2, 
!                         a(0,0) = R(0,0), 
!                         a(1,0) = R(n1/2,0), 
!                         a(0,n2/2) = R(0,n2/2), 
!                         a(1,n2/2) = R(n1/2,n2/2)
!                 <case2>
!                     input data
!                         a(2*j1,j2) = R(j1,j2) = R(n1-j1,n2-j2), 
!                         a(2*j1+1,j2) = I(j1,j2) = -I(n1-j1,n2-j2), 
!                             0<j1<n1/2, 0<j2<n2, 
!                         a(2*j1,0) = R(j1,0) = R(n1-j1,0), 
!                         a(2*j1+1,0) = I(j1,0) = -I(n1-j1,0), 
!                             0<j1<n1/2, 
!                         a(0,j2) = R(0,j2) = R(0,n2-j2), 
!                         a(1,j2) = I(0,j2) = -I(0,n2-j2), 
!                         a(1,n2-j2) = R(n1/2,j2) = R(n1/2,n2-j2), 
!                         a(0,n2-j2) = -I(n1/2,j2) = I(n1/2,n2-j2), 
!                             0<j2<n2/2, 
!                         a(0,0) = R(0,0), 
!                         a(1,0) = R(n1/2,0), 
!                         a(0,n2/2) = R(0,n2/2), 
!                         a(1,n2/2) = R(n1/2,n2/2)
!         ip(0:*):work area for bit reversal (integer)
!                 length of ip >= 2+sqrt(n)
!                 (n = max(n1/2, n2))
!                 ip(0),ip(1) are pointers of the cos/sin table.
!         w(0:*) :cos/sin table (real*8)
!                 length of w >= max(n1/4, n2/2) + n1/4
!                 w(),ip() are initialized if ip(0) = 0.
!     [remark]
!         Inverse of 
!             call rdft2d(n1max, n1, n2, 1, a, ip, w)
!         is 
!             call rdft2d(n1max, n1, n2, -1, a, ip, w)
!             do j2 = 0, n2 - 1
!                 do j1 = 0, n1 - 1
!                     a(j1, j2) = a(j1, j2) * (2.0d0 / (n1 * n2))
!                 end do
!             end do
!         .
!
!
! -------- DCT (Discrete Cosine Transform) / Inverse of DCT --------
!     [definition]
!         <case1> IDCT (excluding scale)
!             C(k1,k2) = sum_j1=0^n1-1 sum_j2=0^n2-1 a(j1,j2) * 
!                            cos(pi*j1*(k1+1/2)/n1) * 
!                            cos(pi*j2*(k2+1/2)/n2), 
!                            0<=k1<n1, 0<=k2<n2
!         <case2> DCT
!             C(k1,k2) = sum_j1=0^n1-1 sum_j2=0^n2-1 a(j1,j2) * 
!                            cos(pi*(j1+1/2)*k1/n1) * 
!                            cos(pi*(j2+1/2)*k2/n2), 
!                            0<=k1<n1, 0<=k2<n2
!     [usage]
!         <case1>
!             ip(0) = 0  ! first time only
!             call ddct2d(n1max, n1, n2, 1, a, t, ip, w)
!         <case2>
!             ip(0) = 0  ! first time only
!             call ddct2d(n1max, n1, n2, -1, a, t, ip, w)
!     [parameters]
!         n1max  :row size of the 2D array (integer)
!         n1     :data length (integer)
!                 n1 >= 2, n1 = power of 2
!         n2     :data length (integer)
!                 n2 >= 2, n2 = power of 2
!         a(0:n1-1,0:n2-1)
!                :input/output data (real*8)
!                 output data
!                     a(k1,k2) = C(k1,k2), 0<=k1<n1, 0<=k2<n2
!         t(0:n1-1,0:n2-1)
!                :work area (real*8)
!         ip(0:*):work area for bit reversal (integer)
!                 length of ip >= 2+sqrt(n)
!                 (n = max(n1/2, n2))
!                 ip(0),ip(1) are pointers of the cos/sin table.
!         w(0:*) :cos/sin table (real*8)
!                 length of w >= max(n1/4, n2/2) + max(n1, n2)
!                 w(),ip() are initialized if ip(0) = 0.
!     [remark]
!         Inverse of 
!             call ddct2d(n1max, n1, n2, -1, a, t, ip, w)
!         is 
!             do j1 = 0, n1 - 1
!                 a(j1, 0) = a(j1, 0) * 0.5d0
!             end do
!             do j2 = 0, n2 - 1
!                 a(0, j2) = a(0, j2) * 0.5d0
!             end do
!             call ddct2d(n1max, n1, n2, 1, a, t, ip, w)
!             do j2 = 0, n2 - 1
!                 do j1 = 0, n1 - 1
!                     a(j1, j2) = a(j1, j2) * (4.0d0 / (n1 * n2))
!                 end do
!             end do
!         .
!
!
! -------- DST (Discrete Sine Transform) / Inverse of DST --------
!     [definition]
!         <case1> IDST (excluding scale)
!             S(k1,k2) = sum_j1=1^n1 sum_j2=1^n2 A(j1,j2) * 
!                            sin(pi*j1*(k1+1/2)/n1) * 
!                            sin(pi*j2*(k2+1/2)/n2), 
!                            0<=k1<n1, 0<=k2<n2
!         <case2> DST
!             S(k1,k2) = sum_j1=0^n1-1 sum_j2=0^n2-1 a(j1,j2) * 
!                            sin(pi*(j1+1/2)*k1/n1) * 
!                            sin(pi*(j2+1/2)*k2/n2), 
!                            0<k1<=n1, 0<k2<=n2
!     [usage]
!         <case1>
!             ip(0) = 0  ! first time only
!             call ddst2d(n1max, n1, n2, 1, a, t, ip, w)
!         <case2>
!             ip(0) = 0  ! first time only
!             call ddst2d(n1max, n1, n2, -1, a, t, ip, w)
!     [parameters]
!         n1max  :row size of the 2D array (integer)
!         n1     :data length (integer)
!                 n1 >= 2, n1 = power of 2
!         n2     :data length (integer)
!                 n2 >= 2, n2 = power of 2
!         a(0:n1-1,0:n2-1)
!                :input/output data (real*8)
!                 <case1>
!                     input data
!                         a(j1,j2) = A(j1,j2), 0<j1<n1, 0<j2<n2, 
!                         a(j1,0) = A(j1,n2), 0<j1<n1, 
!                         a(0,j2) = A(n1,j2), 0<j2<n2, 
!                         a(0,0) = A(n1,n2)
!                         (i.e. A(j1,j2) = a(mod(j1,n1),mod(j2,n2)))
!                     output data
!                         a(k1,k2) = S(k1,k2), 0<=k1<n1, 0<=k2<n2
!                 <case2>
!                     output data
!                         a(k1,k2) = S(k1,k2), 0<k1<n1, 0<k2<n2, 
!                         a(k1,0) = S(k1,n2), 0<k1<n1, 
!                         a(0,k2) = S(n1,k2), 0<k2<n2, 
!                         a(0,0) = S(n1,n2)
!                         (i.e. S(k1,k2) = a(mod(k1,n1),mod(k2,n2)))
!         t(0:n1-1,0:n2-1)
!                :work area (real*8)
!         ip(0:*):work area for bit reversal (integer)
!                 length of ip >= 2+sqrt(n)
!                 (n = max(n1/2, n2))
!                 ip(0),ip(1) are pointers of the cos/sin table.
!         w(0:*) :cos/sin table (real*8)
!                 length of w >= max(n1/4, n2/2) + max(n1, n2)
!                 w(),ip() are initialized if ip(0) = 0.
!     [remark]
!         Inverse of 
!             call ddst2d(n1max, n1, n2, -1, a, t, ip, w)
!         is 
!             do j1 = 0, n1 - 1
!                 a(j1, 0) = a(j1, 0) * 0.5d0
!             end do
!             do j2 = 0, n2 - 1
!                 a(0, j2) = a(0, j2) * 0.5d0
!             end do
!             call ddst2d(n1max, n1, n2, 1, a, t, ip, w)
!             do j2 = 0, n2 - 1
!                 do j1 = 0, n1 - 1
!                     a(j1, j2) = a(j1, j2) * (4.0d0 / (n1 * n2))
!                 end do
!             end do
!         .
!
!
      subroutine cdft2d(n1max, n1, n2, isgn, a, ip, w)
      integer n1max, n1, n2, isgn, ip(0 : *), n
      real*8 a(0 : n1max - 1, 0 : n2 - 1), w(0 : *)
      n = max(n1, 2 * n2)
      if (n .gt. 4 * ip(0)) then
          call makewt(n / 4, ip, w)
      end if
      if (n1 .gt. 4) then
          call bitrv2row(n1max, n1, n2, ip(2), a)
      end if
      if (n2 .gt. 2) then
          call bitrv2col(n1max, n1, n2, ip(2), a)
      end if
      if (isgn .lt. 0) then
          call cftfrow(n1max, n1, n2, a, w)
          call cftfcol(n1max, n1, n2, a, w)
      else
          call cftbrow(n1max, n1, n2, a, w)
          call cftbcol(n1max, n1, n2, a, w)
      end if
      end
!
      subroutine rdft2d(n1max, n1, n2, isgn, a, ip, w)
      integer n1max, n1, n2, isgn, ip(0 : *), n, nw, nc, n2h, i, j
      real*8 a(0 : n1max - 1, 0 : n2 - 1), w(0 : *), xi
      n = max(n1, 2 * n2)
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
      n2h = n2 / 2
      if (isgn .lt. 0) then
          do i = 1, n2h - 1
              j = n2 - i
              xi = a(0, i) - a(0, j)
              a(0, i) = a(0, i) + a(0, j)
              a(0, j) = xi
              xi = a(1, j) - a(1, i)
              a(1, i) = a(1, i) + a(1, j)
              a(1, j) = xi
          end do
          if (n2 .gt. 2) then
              call bitrv2col(n1max, n1, n2, ip(2), a)
          end if
          call cftfcol(n1max, n1, n2, a, w)
          do i = 0, n2 - 1
              a(1, i) = 0.5d0 * (a(0, i) - a(1, i))
              a(0, i) = a(0, i) - a(1, i)
          end do
          if (n1 .gt. 4) then
              call rftfrow(n1max, n1, n2, a, nc, w(nw))
              call bitrv2row(n1max, n1, n2, ip(2), a)
          end if
          call cftfrow(n1max, n1, n2, a, w)
      else
          if (n1 .gt. 4) then
              call bitrv2row(n1max, n1, n2, ip(2), a)
          end if
          call cftbrow(n1max, n1, n2, a, w)
          if (n1 .gt. 4) then
              call rftbrow(n1max, n1, n2, a, nc, w(nw))
          end if
          do i = 0, n2 - 1
              xi = a(0, i) - a(1, i)
              a(0, i) = a(0, i) + a(1, i)
              a(1, i) = xi
          end do
          if (n2 .gt. 2) then
              call bitrv2col(n1max, n1, n2, ip(2), a)
          end if
          call cftbcol(n1max, n1, n2, a, w)
          do i = 1, n2h - 1
              j = n2 - i
              a(0, j) = 0.5d0 * (a(0, i) - a(0, j))
              a(0, i) = a(0, i) - a(0, j)
              a(1, j) = 0.5d0 * (a(1, i) + a(1, j))
              a(1, i) = a(1, i) - a(1, j)
          end do
      end if
      end
!
      subroutine ddct2d(n1max, n1, n2, isgn, a, t, ip, w)
      integer n1max, n1, n2, isgn, ip(0 : *), n, nw, nc, n1h, n2h, 
     &    i, ix, ic, j, jx, jc
      real*8 a(0 : n1max - 1, 0 : n2 - 1), 
     &    t(0 : n1max - 1, 0 : n2 - 1), w(0 : *), xi
      n = max(n1, 2 * n2)
      nw = ip(0)
      if (n .gt. 4 * nw) then
          nw = n / 4
          call makewt(nw, ip, w)
      end if
      nc = ip(1)
      if (n1 .gt. nc .or. n2 .gt. nc) then
          nc = max(n1, n2)
          call makect(nc, ip, w(nw))
      end if
      n1h = n1 / 2
      n2h = n2 / 2
      if (isgn .ge. 0) then
          do i = 0, n2 - 1
              do j = 1, n1h - 1
                  jx = 2 * j
                  t(jx, i) = a(j, i)
                  t(jx + 1, i) = a(n1 - j, i)
              end do
          end do
          t(0, 0) = a(0, 0)
          t(1, 0) = a(n1h, 0)
          t(0, n2h) = a(0, n2h)
          t(1, n2h) = a(n1h, n2h)
          do i = 1, n2h - 1
              ic = n2 - i
              t(0, i) = a(0, i)
              t(1, ic) = a(n1h, i)
              t(1, i) = a(0, ic)
              t(0, ic) = a(n1h, ic)
          end do
          call dctfsub(n1max, n1, n2, t, nc, w(nw))
          if (n2 .gt. 2) then
              call bitrv2col(n1max, n1, n2, ip(2), t)
          end if
          call cftfcol(n1max, n1, n2, t, w)
          do i = 0, n2 - 1
              t(1, i) = 0.5d0 * (t(0, i) - t(1, i))
              t(0, i) = t(0, i) - t(1, i)
          end do
          if (n1 .gt. 4) then
              call rftfrow(n1max, n1, n2, t, nc, w(nw))
              call bitrv2row(n1max, n1, n2, ip(2), t)
          end if
          call cftfrow(n1max, n1, n2, t, w)
          do i = 0, n2h - 1
              ix = 2 * i
              ic = n2 - 1 - i
              do j = 0, n1h - 1
                  jx = 2 * j
                  jc = n1 - 1 - j
                  a(jx, ix) = t(j, i)
                  a(jx + 1, ix) = t(jc, i)
                  a(jx, ix + 1) = t(j, ic)
                  a(jx + 1, ix + 1) = t(jc, ic)
              end do
          end do
      else
          do i = 0, n2h - 1
              ix = 2 * i
              ic = n2 - 1 - i
              do j = 0, n1h - 1
                  jx = 2 * j
                  jc = n1 - 1 - j
                  t(j, i) = a(jx, ix)
                  t(jc, i) = a(jx + 1, ix)
                  t(j, ic) = a(jx, ix + 1)
                  t(jc, ic) = a(jx + 1, ix + 1)
              end do
          end do
          if (n1 .gt. 4) then
              call bitrv2row(n1max, n1, n2, ip(2), t)
          end if
          call cftbrow(n1max, n1, n2, t, w)
          if (n1 .gt. 4) then
              call rftbrow(n1max, n1, n2, t, nc, w(nw))
          end if
          do i = 0, n2 - 1
              xi = t(0, i) - t(1, i)
              t(0, i) = t(0, i) + t(1, i)
              t(1, i) = xi
          end do
          if (n2 .gt. 2) then
              call bitrv2col(n1max, n1, n2, ip(2), t)
          end if
          call cftbcol(n1max, n1, n2, t, w)
          call dctbsub(n1max, n1, n2, t, nc, w(nw))
          do i = 0, n2 - 1
              do j = 1, n1h - 1
                  jx = 2 * j
                  a(j, i) = t(jx, i)
                  a(n1 - j, i) = t(jx + 1, i)
              end do
          end do
          a(0, 0) = t(0, 0)
          a(n1h, 0) = t(1, 0)
          a(0, n2h) = t(0, n2h)
          a(n1h, n2h) = t(1, n2h)
          do i = 1, n2h - 1
              ic = n2 - i
              a(0, i) = t(0, i)
              a(n1h, i) = t(1, ic)
              a(0, ic) = t(1, i)
              a(n1h, ic) = t(0, ic)
          end do
      end if
      end
!
      subroutine ddst2d(n1max, n1, n2, isgn, a, t, ip, w)
      integer n1max, n1, n2, isgn, ip(0 : *), n, nw, nc, n1h, n2h, 
     &    i, ix, ic, j, jx, jc
      real*8 a(0 : n1max - 1, 0 : n2 - 1), 
     &    t(0 : n1max - 1, 0 : n2 - 1), w(0 : *), xi
      n = max(n1, 2 * n2)
      nw = ip(0)
      if (n .gt. 4 * nw) then
          nw = n / 4
          call makewt(nw, ip, w)
      end if
      nc = ip(1)
      if (n1 .gt. nc .or. n2 .gt. nc) then
          nc = max(n1, n2)
          call makect(nc, ip, w(nw))
      end if
      n1h = n1 / 2
      n2h = n2 / 2
      if (isgn .ge. 0) then
          do i = 0, n2 - 1
              do j = 1, n1h - 1
                  jx = 2 * j
                  t(jx, i) = a(j, i)
                  t(jx + 1, i) = a(n1 - j, i)
              end do
          end do
          t(0, 0) = a(0, 0)
          t(1, 0) = a(n1h, 0)
          t(0, n2h) = a(0, n2h)
          t(1, n2h) = a(n1h, n2h)
          do i = 1, n2h - 1
              ic = n2 - i
              t(0, i) = a(0, i)
              t(1, ic) = a(n1h, i)
              t(1, i) = a(0, ic)
              t(0, ic) = a(n1h, ic)
          end do
          call dstfsub(n1max, n1, n2, t, nc, w(nw))
          if (n2 .gt. 2) then
              call bitrv2col(n1max, n1, n2, ip(2), t)
          end if
          call cftfcol(n1max, n1, n2, t, w)
          do i = 0, n2 - 1
              t(1, i) = 0.5d0 * (t(0, i) - t(1, i))
              t(0, i) = t(0, i) - t(1, i)
          end do
          if (n1 .gt. 4) then
              call rftfrow(n1max, n1, n2, t, nc, w(nw))
              call bitrv2row(n1max, n1, n2, ip(2), t)
          end if
          call cftfrow(n1max, n1, n2, t, w)
          do i = 0, n2h - 1
              ix = 2 * i
              ic = n2 - 1 - i
              do j = 0, n1h - 1
                  jx = 2 * j
                  jc = n1 - 1 - j
                  a(jx, ix) = t(j, i)
                  a(jx + 1, ix) = -t(jc, i)
                  a(jx, ix + 1) = -t(j, ic)
                  a(jx + 1, ix + 1) = t(jc, ic)
              end do
          end do
      else
          do i = 0, n2h - 1
              ix = 2 * i
              ic = n2 - 1 - i
              do j = 0, n1h - 1
                  jx = 2 * j
                  jc = n1 - 1 - j
                  t(j, i) = a(jx, ix)
                  t(jc, i) = -a(jx + 1, ix)
                  t(j, ic) = -a(jx, ix + 1)
                  t(jc, ic) = a(jx + 1, ix + 1)
              end do
          end do
          if (n1 .gt. 4) then
              call bitrv2row(n1max, n1, n2, ip(2), t)
          end if
          call cftbrow(n1max, n1, n2, t, w)
          if (n1 .gt. 4) then
              call rftbrow(n1max, n1, n2, t, nc, w(nw))
          end if
          do i = 0, n2 - 1
              xi = t(0, i) - t(1, i)
              t(0, i) = t(0, i) + t(1, i)
              t(1, i) = xi
          end do
          if (n2 .gt. 2) then
              call bitrv2col(n1max, n1, n2, ip(2), t)
          end if
          call cftbcol(n1max, n1, n2, t, w)
          call dstbsub(n1max, n1, n2, t, nc, w(nw))
          do i = 0, n2 - 1
              do j = 1, n1h - 1
                  jx = 2 * j
                  a(j, i) = t(jx, i)
                  a(n1 - j, i) = t(jx + 1, i)
              end do
          end do
          a(0, 0) = t(0, 0)
          a(n1h, 0) = t(1, 0)
          a(0, n2h) = t(0, n2h)
          a(n1h, n2h) = t(1, n2h)
          do i = 1, n2h - 1
              ic = n2 - i
              a(0, i) = t(0, i)
              a(n1h, i) = t(1, ic)
              a(0, ic) = t(1, i)
              a(n1h, ic) = t(0, ic)
          end do
      end if
      end
!
! -------- initializing routines --------
!
      subroutine makewt(nw, ip, w)
      integer nw, ip(0 : *), nwh, j
      real*8 w(0 : nw - 1), delta, x, y
      ip(0) = nw
      ip(1) = 1
      if (nw .gt. 2) then
          nwh = nw / 2
          delta = atan(1.0d0) / nwh
          w(0) = 1
          w(1) = 0
          w(nwh) = cos(delta * nwh)
          w(nwh + 1) = w(nwh)
          do j = 2, nwh - 2, 2
              x = cos(delta * j)
              y = sin(delta * j)
              w(j) = x
              w(j + 1) = y
              w(nw - j) = y
              w(nw - j + 1) = x
          end do
          call bitrv2(nw, ip(2), w)
      end if
      end
!
      subroutine makect(nc, ip, c)
      integer nc, ip(0 : *), nch, j
      real*8 c(0 : nc - 1), delta
      ip(1) = nc
      if (nc .gt. 1) then
          nch = nc / 2
          delta = atan(1.0d0) / nch
          c(0) = 0.5d0
          c(nch) = 0.5d0 * cos(delta * nch)
          do j = 1, nch - 1
              c(j) = 0.5d0 * cos(delta * j)
              c(nc - j) = 0.5d0 * sin(delta * j)
          end do
      end if
      end
!
! -------- child routines --------
!
      subroutine bitrv2(n, ip, a)
      integer n, ip(0 : *), j, j1, k, k1, l, m, m2
      real*8 a(0 : n - 1), xr, xi
      ip(0) = 0
      l = n
      m = 1
      do while (4 * m .lt. l)
          l = l / 2
          do j = 0, m - 1
              ip(m + j) = ip(j) + l
          end do
          m = m * 2
      end do
      if (4 * m .gt. l) then
          do k = 1, m - 1
              do j = 0, k - 1
                  j1 = 2 * j + ip(k)
                  k1 = 2 * k + ip(j)
                  xr = a(j1)
                  xi = a(j1 + 1)
                  a(j1) = a(k1)
                  a(j1 + 1) = a(k1 + 1)
                  a(k1) = xr
                  a(k1 + 1) = xi
              end do
          end do
      else
          m2 = 2 * m
          do k = 1, m - 1
              do j = 0, k - 1
                  j1 = 2 * j + ip(k)
                  k1 = 2 * k + ip(j)
                  xr = a(j1)
                  xi = a(j1 + 1)
                  a(j1) = a(k1)
                  a(j1 + 1) = a(k1 + 1)
                  a(k1) = xr
                  a(k1 + 1) = xi
                  j1 = j1 + m2
                  k1 = k1 + m2
                  xr = a(j1)
                  xi = a(j1 + 1)
                  a(j1) = a(k1)
                  a(j1 + 1) = a(k1 + 1)
                  a(k1) = xr
                  a(k1 + 1) = xi
              end do
          end do
      end if
      end
!
      subroutine bitrv2row(n1max, n, n2, ip, a)
      integer n1max, n, n2, ip(0 : *), i, j, j1, k, k1, l, m, m2
      real*8 a(0 : n1max - 1, 0 : n2 - 1), xr, xi
      ip(0) = 0
      l = n
      m = 1
      do while (4 * m .lt. l)
          l = l / 2
          do j = 0, m - 1
              ip(m + j) = ip(j) + l
          end do
          m = m * 2
      end do
      if (4 * m .gt. l) then
          do i = 0, n2 - 1
              do k = 1, m - 1
                  do j = 0, k - 1
                      j1 = 2 * j + ip(k)
                      k1 = 2 * k + ip(j)
                      xr = a(j1, i)
                      xi = a(j1 + 1, i)
                      a(j1, i) = a(k1, i)
                      a(j1 + 1, i) = a(k1 + 1, i)
                      a(k1, i) = xr
                      a(k1 + 1, i) = xi
                  end do
              end do
          end do
      else
          m2 = 2 * m
          do i = 0, n2 - 1
              do k = 1, m - 1
                  do j = 0, k - 1
                      j1 = 2 * j + ip(k)
                      k1 = 2 * k + ip(j)
                      xr = a(j1, i)
                      xi = a(j1 + 1, i)
                      a(j1, i) = a(k1, i)
                      a(j1 + 1, i) = a(k1 + 1, i)
                      a(k1, i) = xr
                      a(k1 + 1, i) = xi
                      j1 = j1 + m2
                      k1 = k1 + m2
                      xr = a(j1, i)
                      xi = a(j1 + 1, i)
                      a(j1, i) = a(k1, i)
                      a(j1 + 1, i) = a(k1 + 1, i)
                      a(k1, i) = xr
                      a(k1 + 1, i) = xi
                  end do
              end do
          end do
      end if
      end
!
      subroutine bitrv2col(n1max, n1, n, ip, a)
      integer n1max, n1, n, ip(0 : *), i, j, j1, k, k1, l, m
      real*8 a(0 : n1max - 1, 0 : n - 1), xr, xi
      ip(0) = 0
      l = n
      m = 1
      do while (2 * m .lt. l)
          l = l / 2
          do j = 0, m - 1
              ip(m + j) = ip(j) + l
          end do
          m = m * 2
      end do
      if (2 * m .gt. l) then
          do k = 1, m - 1
              do j = 0, k - 1
                  j1 = j + ip(k)
                  k1 = k + ip(j)
                  do i = 0, n1 - 2, 2
                      xr = a(i, j1)
                      xi = a(i + 1, j1)
                      a(i, j1) = a(i, k1)
                      a(i + 1, j1) = a(i + 1, k1)
                      a(i, k1) = xr
                      a(i + 1, k1) = xi
                  end do
              end do
          end do
      else
          do k = 1, m - 1
              do j = 0, k - 1
                  j1 = j + ip(k)
                  k1 = k + ip(j)
                  do i = 0, n1 - 2, 2
                      xr = a(i, j1)
                      xi = a(i + 1, j1)
                      a(i, j1) = a(i, k1)
                      a(i + 1, j1) = a(i + 1, k1)
                      a(i, k1) = xr
                      a(i + 1, k1) = xi
                  end do
                  j1 = j1 + m
                  k1 = k1 + m
                  do i = 0, n1 - 2, 2
                      xr = a(i, j1)
                      xi = a(i + 1, j1)
                      a(i, j1) = a(i, k1)
                      a(i + 1, j1) = a(i + 1, k1)
                      a(i, k1) = xr
                      a(i + 1, k1) = xi
                  end do
              end do
          end do
      end if
      end
!
      subroutine cftbrow(n1max, n, n2, a, w)
      integer n1max, n, n2, i, j, j1, j2, j3, k, k1, ks, l, m
      real*8 a(0 : n1max - 1, 0 : n2 - 1), w(0 : *)
      real*8 wk1r, wk1i, wk2r, wk2i, wk3r, wk3i
      real*8 x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i
      do i = 0, n2 - 1
          l = 2
          do while (2 * l .lt. n)
              m = 4 * l
              do j = 0, l - 2, 2
                  j1 = j + l
                  j2 = j1 + l
                  j3 = j2 + l
                  x0r = a(j, i) + a(j1, i)
                  x0i = a(j + 1, i) + a(j1 + 1, i)
                  x1r = a(j, i) - a(j1, i)
                  x1i = a(j + 1, i) - a(j1 + 1, i)
                  x2r = a(j2, i) + a(j3, i)
                  x2i = a(j2 + 1, i) + a(j3 + 1, i)
                  x3r = a(j2, i) - a(j3, i)
                  x3i = a(j2 + 1, i) - a(j3 + 1, i)
                  a(j, i) = x0r + x2r
                  a(j + 1, i) = x0i + x2i
                  a(j2, i) = x0r - x2r
                  a(j2 + 1, i) = x0i - x2i
                  a(j1, i) = x1r - x3i
                  a(j1 + 1, i) = x1i + x3r
                  a(j3, i) = x1r + x3i
                  a(j3 + 1, i) = x1i - x3r
              end do
              if (m .lt. n) then
                  wk1r = w(2)
                  do j = m, l + m - 2, 2
                      j1 = j + l
                      j2 = j1 + l
                      j3 = j2 + l
                      x0r = a(j, i) + a(j1, i)
                      x0i = a(j + 1, i) + a(j1 + 1, i)
                      x1r = a(j, i) - a(j1, i)
                      x1i = a(j + 1, i) - a(j1 + 1, i)
                      x2r = a(j2, i) + a(j3, i)
                      x2i = a(j2 + 1, i) + a(j3 + 1, i)
                      x3r = a(j2, i) - a(j3, i)
                      x3i = a(j2 + 1, i) - a(j3 + 1, i)
                      a(j, i) = x0r + x2r
                      a(j + 1, i) = x0i + x2i
                      a(j2, i) = x2i - x0i
                      a(j2 + 1, i) = x0r - x2r
                      x0r = x1r - x3i
                      x0i = x1i + x3r
                      a(j1, i) = wk1r * (x0r - x0i)
                      a(j1 + 1, i) = wk1r * (x0r + x0i)
                      x0r = x3i + x1r
                      x0i = x3r - x1i
                      a(j3, i) = wk1r * (x0i - x0r)
                      a(j3 + 1, i) = wk1r * (x0i + x0r)
                  end do
                  k1 = 1
                  ks = -1
                  do k = 2 * m, n - m, m
                      k1 = k1 + 1
                      ks = -ks
                      wk1r = w(2 * k1)
                      wk1i = w(2 * k1 + 1)
                      wk2r = ks * w(k1)
                      wk2i = w(k1 + ks)
                      wk3r = wk1r - 2 * wk2i * wk1i
                      wk3i = 2 * wk2i * wk1r - wk1i
                      do j = k, l + k - 2, 2
                          j1 = j + l
                          j2 = j1 + l
                          j3 = j2 + l
                          x0r = a(j, i) + a(j1, i)
                          x0i = a(j + 1, i) + a(j1 + 1, i)
                          x1r = a(j, i) - a(j1, i)
                          x1i = a(j + 1, i) - a(j1 + 1, i)
                          x2r = a(j2, i) + a(j3, i)
                          x2i = a(j2 + 1, i) + a(j3 + 1, i)
                          x3r = a(j2, i) - a(j3, i)
                          x3i = a(j2 + 1, i) - a(j3 + 1, i)
                          a(j, i) = x0r + x2r
                          a(j + 1, i) = x0i + x2i
                          x0r = x0r - x2r
                          x0i = x0i - x2i
                          a(j2, i) = wk2r * x0r - wk2i * x0i
                          a(j2 + 1, i) = wk2r * x0i + wk2i * x0r
                          x0r = x1r - x3i
                          x0i = x1i + x3r
                          a(j1, i) = wk1r * x0r - wk1i * x0i
                          a(j1 + 1, i) = wk1r * x0i + wk1i * x0r
                          x0r = x1r + x3i
                          x0i = x1i - x3r
                          a(j3, i) = wk3r * x0r - wk3i * x0i
                          a(j3 + 1, i) = wk3r * x0i + wk3i * x0r
                      end do
                  end do
              end if
              l = m
          end do
          if (l .lt. n) then
              do j = 0, l - 2, 2
                  j1 = j + l
                  x0r = a(j, i) - a(j1, i)
                  x0i = a(j + 1, i) - a(j1 + 1, i)
                  a(j, i) = a(j, i) + a(j1, i)
                  a(j + 1, i) = a(j + 1, i) + a(j1 + 1, i)
                  a(j1, i) = x0r
                  a(j1 + 1, i) = x0i
              end do
          end if
      end do
      end
!
      subroutine cftbcol(n1max, n1, n, a, w)
      integer n1max, n1, n, i, j, j1, j2, j3, k, k1, ks, l, m
      real*8 a(0 : n1max - 1, 0 : n - 1), w(0 : *)
      real*8 wk1r, wk1i, wk2r, wk2i, wk3r, wk3i
      real*8 x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i
      l = 1
      do while (2 * l .lt. n)
          m = 4 * l
          do j = 0, l - 1
              j1 = j + l
              j2 = j1 + l
              j3 = j2 + l
              do i = 0, n1 - 2, 2
                  x0r = a(i, j) + a(i, j1)
                  x0i = a(i + 1, j) + a(i + 1, j1)
                  x1r = a(i, j) - a(i, j1)
                  x1i = a(i + 1, j) - a(i + 1, j1)
                  x2r = a(i, j2) + a(i, j3)
                  x2i = a(i + 1, j2) + a(i + 1, j3)
                  x3r = a(i, j2) - a(i, j3)
                  x3i = a(i + 1, j2) - a(i + 1, j3)
                  a(i, j) = x0r + x2r
                  a(i + 1, j) = x0i + x2i
                  a(i, j2) = x0r - x2r
                  a(i + 1, j2) = x0i - x2i
                  a(i, j1) = x1r - x3i
                  a(i + 1, j1) = x1i + x3r
                  a(i, j3) = x1r + x3i
                  a(i + 1, j3) = x1i - x3r
              end do
          end do
          if (m .lt. n) then
              wk1r = w(2)
              do j = m, l + m - 1
                  j1 = j + l
                  j2 = j1 + l
                  j3 = j2 + l
                  do i = 0, n1 - 2, 2
                      x0r = a(i, j) + a(i, j1)
                      x0i = a(i + 1, j) + a(i + 1, j1)
                      x1r = a(i, j) - a(i, j1)
                      x1i = a(i + 1, j) - a(i + 1, j1)
                      x2r = a(i, j2) + a(i, j3)
                      x2i = a(i + 1, j2) + a(i + 1, j3)
                      x3r = a(i, j2) - a(i, j3)
                      x3i = a(i + 1, j2) - a(i + 1, j3)
                      a(i, j) = x0r + x2r
                      a(i + 1, j) = x0i + x2i
                      a(i, j2) = x2i - x0i
                      a(i + 1, j2) = x0r - x2r
                      x0r = x1r - x3i
                      x0i = x1i + x3r
                      a(i, j1) = wk1r * (x0r - x0i)
                      a(i + 1, j1) = wk1r * (x0r + x0i)
                      x0r = x3i + x1r
                      x0i = x3r - x1i
                      a(i, j3) = wk1r * (x0i - x0r)
                      a(i + 1, j3) = wk1r * (x0i + x0r)
                  end do
              end do
              k1 = 1
              ks = -1
              do k = 2 * m, n - m, m
                  k1 = k1 + 1
                  ks = -ks
                  wk1r = w(2 * k1)
                  wk1i = w(2 * k1 + 1)
                  wk2r = ks * w(k1)
                  wk2i = w(k1 + ks)
                  wk3r = wk1r - 2 * wk2i * wk1i
                  wk3i = 2 * wk2i * wk1r - wk1i
                  do j = k, l + k - 1
                      j1 = j + l
                      j2 = j1 + l
                      j3 = j2 + l
                      do i = 0, n1 - 2, 2
                          x0r = a(i, j) + a(i, j1)
                          x0i = a(i + 1, j) + a(i + 1, j1)
                          x1r = a(i, j) - a(i, j1)
                          x1i = a(i + 1, j) - a(i + 1, j1)
                          x2r = a(i, j2) + a(i, j3)
                          x2i = a(i + 1, j2) + a(i + 1, j3)
                          x3r = a(i, j2) - a(i, j3)
                          x3i = a(i + 1, j2) - a(i + 1, j3)
                          a(i, j) = x0r + x2r
                          a(i + 1, j) = x0i + x2i
                          x0r = x0r - x2r
                          x0i = x0i - x2i
                          a(i, j2) = wk2r * x0r - wk2i * x0i
                          a(i + 1, j2) = wk2r * x0i + wk2i * x0r
                          x0r = x1r - x3i
                          x0i = x1i + x3r
                          a(i, j1) = wk1r * x0r - wk1i * x0i
                          a(i + 1, j1) = wk1r * x0i + wk1i * x0r
                          x0r = x1r + x3i
                          x0i = x1i - x3r
                          a(i, j3) = wk3r * x0r - wk3i * x0i
                          a(i + 1, j3) = wk3r * x0i + wk3i * x0r
                      end do
                  end do
              end do
          end if
          l = m
      end do
      if (l .lt. n) then
          do j = 0, l - 1
              j1 = j + l
              do i = 0, n1 - 2, 2
                  x0r = a(i, j) - a(i, j1)
                  x0i = a(i + 1, j) - a(i + 1, j1)
                  a(i, j) = a(i, j) + a(i, j1)
                  a(i + 1, j) = a(i + 1, j) + a(i + 1, j1)
                  a(i, j1) = x0r
                  a(i + 1, j1) = x0i
              end do
          end do
      end if
      end
!
      subroutine cftfrow(n1max, n, n2, a, w)
      integer n1max, n, n2, i, j, j1, j2, j3, k, k1, ks, l, m
      real*8 a(0 : n1max - 1, 0 : n2 - 1), w(0 : *)
      real*8 wk1r, wk1i, wk2r, wk2i, wk3r, wk3i
      real*8 x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i
      do i = 0, n2 - 1
          l = 2
          do while (2 * l .lt. n)
              m = 4 * l
              do j = 0, l - 2, 2
                  j1 = j + l
                  j2 = j1 + l
                  j3 = j2 + l
                  x0r = a(j, i) + a(j1, i)
                  x0i = a(j + 1, i) + a(j1 + 1, i)
                  x1r = a(j, i) - a(j1, i)
                  x1i = a(j + 1, i) - a(j1 + 1, i)
                  x2r = a(j2, i) + a(j3, i)
                  x2i = a(j2 + 1, i) + a(j3 + 1, i)
                  x3r = a(j2, i) - a(j3, i)
                  x3i = a(j2 + 1, i) - a(j3 + 1, i)
                  a(j, i) = x0r + x2r
                  a(j + 1, i) = x0i + x2i
                  a(j2, i) = x0r - x2r
                  a(j2 + 1, i) = x0i - x2i
                  a(j1, i) = x1r + x3i
                  a(j1 + 1, i) = x1i - x3r
                  a(j3, i) = x1r - x3i
                  a(j3 + 1, i) = x1i + x3r
              end do
              if (m .lt. n) then
                  wk1r = w(2)
                  do j = m, l + m - 2, 2
                      j1 = j + l
                      j2 = j1 + l
                      j3 = j2 + l
                      x0r = a(j, i) + a(j1, i)
                      x0i = a(j + 1, i) + a(j1 + 1, i)
                      x1r = a(j, i) - a(j1, i)
                      x1i = a(j + 1, i) - a(j1 + 1, i)
                      x2r = a(j2, i) + a(j3, i)
                      x2i = a(j2 + 1, i) + a(j3 + 1, i)
                      x3r = a(j2, i) - a(j3, i)
                      x3i = a(j2 + 1, i) - a(j3 + 1, i)
                      a(j, i) = x0r + x2r
                      a(j + 1, i) = x0i + x2i
                      a(j2, i) = x0i - x2i
                      a(j2 + 1, i) = x2r - x0r
                      x0r = x1r + x3i
                      x0i = x1i - x3r
                      a(j1, i) = wk1r * (x0i + x0r)
                      a(j1 + 1, i) = wk1r * (x0i - x0r)
                      x0r = x3i - x1r
                      x0i = x3r + x1i
                      a(j3, i) = wk1r * (x0r + x0i)
                      a(j3 + 1, i) = wk1r * (x0r - x0i)
                  end do
                  k1 = 1
                  ks = -1
                  do k = 2 * m, n - m, m
                      k1 = k1 + 1
                      ks = -ks
                      wk1r = w(2 * k1)
                      wk1i = w(2 * k1 + 1)
                      wk2r = ks * w(k1)
                      wk2i = w(k1 + ks)
                      wk3r = wk1r - 2 * wk2i * wk1i
                      wk3i = 2 * wk2i * wk1r - wk1i
                      do j = k, l + k - 2, 2
                          j1 = j + l
                          j2 = j1 + l
                          j3 = j2 + l
                          x0r = a(j, i) + a(j1, i)
                          x0i = a(j + 1, i) + a(j1 + 1, i)
                          x1r = a(j, i) - a(j1, i)
                          x1i = a(j + 1, i) - a(j1 + 1, i)
                          x2r = a(j2, i) + a(j3, i)
                          x2i = a(j2 + 1, i) + a(j3 + 1, i)
                          x3r = a(j2, i) - a(j3, i)
                          x3i = a(j2 + 1, i) - a(j3 + 1, i)
                          a(j, i) = x0r + x2r
                          a(j + 1, i) = x0i + x2i
                          x0r = x0r - x2r
                          x0i = x0i - x2i
                          a(j2, i) = wk2r * x0r + wk2i * x0i
                          a(j2 + 1, i) = wk2r * x0i - wk2i * x0r
                          x0r = x1r + x3i
                          x0i = x1i - x3r
                          a(j1, i) = wk1r * x0r + wk1i * x0i
                          a(j1 + 1, i) = wk1r * x0i - wk1i * x0r
                          x0r = x1r - x3i
                          x0i = x1i + x3r
                          a(j3, i) = wk3r * x0r + wk3i * x0i
                          a(j3 + 1, i) = wk3r * x0i - wk3i * x0r
                      end do
                  end do
              end if
              l = m
          end do
          if (l .lt. n) then
              do j = 0, l - 2, 2
                  j1 = j + l
                  x0r = a(j, i) - a(j1, i)
                  x0i = a(j + 1, i) - a(j1 + 1, i)
                  a(j, i) = a(j, i) + a(j1, i)
                  a(j + 1, i) = a(j + 1, i) + a(j1 + 1, i)
                  a(j1, i) = x0r
                  a(j1 + 1, i) = x0i
              end do
          end if
      end do
      end
!
      subroutine cftfcol(n1max, n1, n, a, w)
      integer n1max, n1, n, i, j, j1, j2, j3, k, k1, ks, l, m
      real*8 a(0 : n1max - 1, 0 : n - 1), w(0 : *)
      real*8 wk1r, wk1i, wk2r, wk2i, wk3r, wk3i
      real*8 x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i
      l = 1
      do while (2 * l .lt. n)
          m = 4 * l
          do j = 0, l - 1
              j1 = j + l
              j2 = j1 + l
              j3 = j2 + l
              do i = 0, n1 - 2, 2
                  x0r = a(i, j) + a(i, j1)
                  x0i = a(i + 1, j) + a(i + 1, j1)
                  x1r = a(i, j) - a(i, j1)
                  x1i = a(i + 1, j) - a(i + 1, j1)
                  x2r = a(i, j2) + a(i, j3)
                  x2i = a(i + 1, j2) + a(i + 1, j3)
                  x3r = a(i, j2) - a(i, j3)
                  x3i = a(i + 1, j2) - a(i + 1, j3)
                  a(i, j) = x0r + x2r
                  a(i + 1, j) = x0i + x2i
                  a(i, j2) = x0r - x2r
                  a(i + 1, j2) = x0i - x2i
                  a(i, j1) = x1r + x3i
                  a(i + 1, j1) = x1i - x3r
                  a(i, j3) = x1r - x3i
                  a(i + 1, j3) = x1i + x3r
              end do
          end do
          if (m .lt. n) then
              wk1r = w(2)
              do j = m, l + m - 1
                  j1 = j + l
                  j2 = j1 + l
                  j3 = j2 + l
                  do i = 0, n1 - 2, 2
                      x0r = a(i, j) + a(i, j1)
                      x0i = a(i + 1, j) + a(i + 1, j1)
                      x1r = a(i, j) - a(i, j1)
                      x1i = a(i + 1, j) - a(i + 1, j1)
                      x2r = a(i, j2) + a(i, j3)
                      x2i = a(i + 1, j2) + a(i + 1, j3)
                      x3r = a(i, j2) - a(i, j3)
                      x3i = a(i + 1, j2) - a(i + 1, j3)
                      a(i, j) = x0r + x2r
                      a(i + 1, j) = x0i + x2i
                      a(i, j2) = x0i - x2i
                      a(i + 1, j2) = x2r - x0r
                      x0r = x1r + x3i
                      x0i = x1i - x3r
                      a(i, j1) = wk1r * (x0i + x0r)
                      a(i + 1, j1) = wk1r * (x0i - x0r)
                      x0r = x3i - x1r
                      x0i = x3r + x1i
                      a(i, j3) = wk1r * (x0r + x0i)
                      a(i + 1, j3) = wk1r * (x0r - x0i)
                  end do
              end do
              k1 = 1
              ks = -1
              do k = 2 * m, n - m, m
                  k1 = k1 + 1
                  ks = -ks
                  wk1r = w(2 * k1)
                  wk1i = w(2 * k1 + 1)
                  wk2r = ks * w(k1)
                  wk2i = w(k1 + ks)
                  wk3r = wk1r - 2 * wk2i * wk1i
                  wk3i = 2 * wk2i * wk1r - wk1i
                  do j = k, l + k - 1
                      j1 = j + l
                      j2 = j1 + l
                      j3 = j2 + l
                      do i = 0, n1 - 2, 2
                          x0r = a(i, j) + a(i, j1)
                          x0i = a(i + 1, j) + a(i + 1, j1)
                          x1r = a(i, j) - a(i, j1)
                          x1i = a(i + 1, j) - a(i + 1, j1)
                          x2r = a(i, j2) + a(i, j3)
                          x2i = a(i + 1, j2) + a(i + 1, j3)
                          x3r = a(i, j2) - a(i, j3)
                          x3i = a(i + 1, j2) - a(i + 1, j3)
                          a(i, j) = x0r + x2r
                          a(i + 1, j) = x0i + x2i
                          x0r = x0r - x2r
                          x0i = x0i - x2i
                          a(i, j2) = wk2r * x0r + wk2i * x0i
                          a(i + 1, j2) = wk2r * x0i - wk2i * x0r
                          x0r = x1r + x3i
                          x0i = x1i - x3r
                          a(i, j1) = wk1r * x0r + wk1i * x0i
                          a(i + 1, j1) = wk1r * x0i - wk1i * x0r
                          x0r = x1r - x3i
                          x0i = x1i + x3r
                          a(i, j3) = wk3r * x0r + wk3i * x0i
                          a(i + 1, j3) = wk3r * x0i - wk3i * x0r
                      end do
                  end do
              end do
          end if
          l = m
      end do
      if (l .lt. n) then
          do j = 0, l - 1
              j1 = j + l
              do i = 0, n1 - 2, 2
                  x0r = a(i, j) - a(i, j1)
                  x0i = a(i + 1, j) - a(i + 1, j1)
                  a(i, j) = a(i, j) + a(i, j1)
                  a(i + 1, j) = a(i + 1, j) + a(i + 1, j1)
                  a(i, j1) = x0r
                  a(i + 1, j1) = x0i
              end do
          end do
      end if
      end
!
      subroutine rftbrow(n1max, n, n2, a, nc, c)
      integer n1max, n, n2, nc, i, j, k, kk, ks
      real*8 a(0 : n1max - 1, 0 : n2 - 1), c(0 : nc - 1), 
     &    wkr, wki, xr, xi, yr, yi
      ks = 4 * nc / n
      do i = 0, n2 - 1
          kk = 0
          do k = n / 2 - 2, 2, -2
              j = n - k
              kk = kk + ks
              wkr = 0.5d0 - c(kk)
              wki = c(nc - kk)
              xr = a(k, i) - a(j, i)
              xi = a(k + 1, i) + a(j + 1, i)
              yr = wkr * xr - wki * xi
              yi = wkr * xi + wki * xr
              a(k, i) = a(k, i) - yr
              a(k + 1, i) = a(k + 1, i) - yi
              a(j, i) = a(j, i) + yr
              a(j + 1, i) = a(j + 1, i) - yi
          end do
      end do
      end
!
      subroutine rftfrow(n1max, n, n2, a, nc, c)
      integer n1max, n, n2, nc, i, j, k, kk, ks
      real*8 a(0 : n1max - 1, 0 : n2 - 1), c(0 : nc - 1), 
     &    wkr, wki, xr, xi, yr, yi
      ks = 4 * nc / n
      do i = 0, n2 - 1
          kk = 0
          do k = n / 2 - 2, 2, -2
              j = n - k
              kk = kk + ks
              wkr = 0.5d0 - c(kk)
              wki = c(nc - kk)
              xr = a(k, i) - a(j, i)
              xi = a(k + 1, i) + a(j + 1, i)
              yr = wkr * xr + wki * xi
              yi = wkr * xi - wki * xr
              a(k, i) = a(k, i) - yr
              a(k + 1, i) = a(k + 1, i) - yi
              a(j, i) = a(j, i) + yr
              a(j + 1, i) = a(j + 1, i) - yi
          end do
      end do
      end
!
      subroutine dctbsub(n1max, n1, n2, a, nc, c)
      integer n1max, n1, n2, nc, kk1, kk2, ks1, ks2, n2h, j2, 
     &    k1, k2
      real*8 a(0 : n1max - 1, 0 : n2 - 1), c(0 : nc - 1), 
     &    w2r, w2i, wkr, wki, wjr, wji, x0r, x0i, x1r, x1i
      ks1 = nc / n1
      ks2 = nc / n2
      n2h = n2 / 2
      kk2 = ks2
      do k2 = 1, n2h - 1
          j2 = n2 - k2
          w2r = 2 * c(kk2)
          w2i = 2 * c(nc - kk2)
          kk2 = kk2 + ks2
          kk1 = ks1
          do k1 = 2, n1 - 2, 2
              x0r = w2r * c(kk1)
              x0i = w2i * c(kk1)
              x1r = w2r * c(nc - kk1)
              x1i = w2i * c(nc - kk1)
              wkr = x0r - x1i
              wki = x0i + x1r
              wji = x0r + x1i
              wjr = x0i - x1r
              kk1 = kk1 + ks1
              x0r = wkr * a(k1, k2) - wki * a(k1 + 1, k2)
              x0i = wkr * a(k1 + 1, k2) + wki * a(k1, k2)
              x1r = wjr * a(k1, j2) - wji * a(k1 + 1, j2)
              x1i = wjr * a(k1 + 1, j2) + wji * a(k1, j2)
              a(k1, k2) = x0r + x1i
              a(k1 + 1, k2) = x0i - x1r
              a(k1, j2) = x1r + x0i
              a(k1 + 1, j2) = x1i - x0r
          end do
          wkr = w2r * 0.5d0
          wki = w2i * 0.5d0
          wjr = w2r * c(kk1)
          wji = w2i * c(kk1)
          x0r = a(0, k2) + a(0, j2)
          x0i = a(1, k2) - a(1, j2)
          x1r = a(0, k2) - a(0, j2)
          x1i = a(1, k2) + a(1, j2)
          a(0, k2) = wkr * x0r - wki * x0i
          a(1, k2) = wkr * x0i + wki * x0r
          a(0, j2) = -wjr * x1r + wji * x1i
          a(1, j2) = wjr * x1i + wji * x1r
      end do
      w2r = 2 * c(kk2)
      kk1 = ks1
      do k1 = 2, n1 - 2, 2
          wkr = 2 * c(kk1)
          wki = 2 * c(nc - kk1)
          wjr = w2r * wkr
          wji = w2r * wki
          kk1 = kk1 + ks1
          x0i = wkr * a(k1 + 1, 0) + wki * a(k1, 0)
          a(k1, 0) = wkr * a(k1, 0) - wki * a(k1 + 1, 0)
          a(k1 + 1, 0) = x0i
          x0i = wjr * a(k1 + 1, n2h) + wji * a(k1, n2h)
          a(k1, n2h) = wjr * a(k1, n2h) - wji * a(k1 + 1, n2h)
          a(k1 + 1, n2h) = x0i
      end do
      a(1, 0) = a(1, 0) * w2r
      a(0, n2h) = a(0, n2h) * w2r
      a(1, n2h) = a(1, n2h) * 0.5d0
      end
!
      subroutine dctfsub(n1max, n1, n2, a, nc, c)
      integer n1max, n1, n2, nc, kk1, kk2, ks1, ks2, n2h, j2, 
     &    k1, k2
      real*8 a(0 : n1max - 1, 0 : n2 - 1), c(0 : nc - 1), 
     &    w2r, w2i, wkr, wki, wjr, wji, x0r, x0i, x1r, x1i
      ks1 = nc / n1
      ks2 = nc / n2
      n2h = n2 / 2
      kk2 = ks2
      do k2 = 1, n2h - 1
          j2 = n2 - k2
          w2r = 2 * c(kk2)
          w2i = 2 * c(nc - kk2)
          kk2 = kk2 + ks2
          kk1 = ks1
          do k1 = 2, n1 - 2, 2
              x0r = w2r * c(kk1)
              x0i = w2i * c(kk1)
              x1r = w2r * c(nc - kk1)
              x1i = w2i * c(nc - kk1)
              wkr = x0r - x1i
              wki = x0i + x1r
              wji = x0r + x1i
              wjr = x0i - x1r
              kk1 = kk1 + ks1
              x0r = a(k1, k2) - a(k1 + 1, j2)
              x0i = a(k1, j2) + a(k1 + 1, k2)
              x1r = a(k1, j2) - a(k1 + 1, k2)
              x1i = a(k1, k2) + a(k1 + 1, j2)
              a(k1, k2) = wkr * x0r + wki * x0i
              a(k1 + 1, k2) = wkr * x0i - wki * x0r
              a(k1, j2) = wjr * x1r + wji * x1i
              a(k1 + 1, j2) = wjr * x1i - wji * x1r
          end do
          x0r = 2 * c(kk1)
          wjr = x0r * w2r
          wji = x0r * w2i
          x0r = w2r * a(0, k2) + w2i * a(1, k2)
          x0i = w2r * a(1, k2) - w2i * a(0, k2)
          x1r = -wjr * a(0, j2) + wji * a(1, j2)
          x1i = wjr * a(1, j2) + wji * a(0, j2)
          a(0, k2) = x0r + x1r
          a(1, k2) = x1i + x0i
          a(0, j2) = x0r - x1r
          a(1, j2) = x1i - x0i
      end do
      w2r = 2 * c(kk2)
      kk1 = ks1
      do k1 = 2, n1 - 2, 2
          wkr = 2 * c(kk1)
          wki = 2 * c(nc - kk1)
          wjr = w2r * wkr
          wji = w2r * wki
          kk1 = kk1 + ks1
          x0i = wkr * a(k1 + 1, 0) - wki * a(k1, 0)
          a(k1, 0) = wkr * a(k1, 0) + wki * a(k1 + 1, 0)
          a(k1 + 1, 0) = x0i
          x0i = wjr * a(k1 + 1, n2h) - wji * a(k1, n2h)
          a(k1, n2h) = wjr * a(k1, n2h) + wji * a(k1 + 1, n2h)
          a(k1 + 1, n2h) = x0i
      end do
      w2r = w2r * 2
      a(0, 0) = a(0, 0) * 2
      a(1, 0) = a(1, 0) * w2r
      a(0, n2h) = a(0, n2h) * w2r
      end
!
      subroutine dstbsub(n1max, n1, n2, a, nc, c)
      integer n1max, n1, n2, nc, kk1, kk2, ks1, ks2, n2h, j2, 
     &    k1, k2
      real*8 a(0 : n1max - 1, 0 : n2 - 1), c(0 : nc - 1), 
     &    w2r, w2i, wkr, wki, wjr, wji, x0r, x0i, x1r, x1i
      ks1 = nc / n1
      ks2 = nc / n2
      n2h = n2 / 2
      kk2 = ks2
      do k2 = 1, n2h - 1
          j2 = n2 - k2
          w2r = 2 * c(kk2)
          w2i = 2 * c(nc - kk2)
          kk2 = kk2 + ks2
          kk1 = ks1
          do k1 = 2, n1 - 2, 2
              x0r = w2r * c(kk1)
              x0i = w2i * c(kk1)
              x1r = w2r * c(nc - kk1)
              x1i = w2i * c(nc - kk1)
              wkr = x0r - x1i
              wki = x0i + x1r
              wji = x0r + x1i
              wjr = x0i - x1r
              kk1 = kk1 + ks1
              x0r = wkr * a(k1, k2) - wki * a(k1 + 1, k2)
              x0i = wkr * a(k1 + 1, k2) + wki * a(k1, k2)
              x1r = wjr * a(k1, j2) - wji * a(k1 + 1, j2)
              x1i = wjr * a(k1 + 1, j2) + wji * a(k1, j2)
              a(k1, k2) = x1i - x0r
              a(k1 + 1, k2) = x1r + x0i
              a(k1, j2) = x0i - x1r
              a(k1 + 1, j2) = x0r + x1i
          end do
          wkr = w2r * 0.5d0
          wki = w2i * 0.5d0
          wjr = w2r * c(kk1)
          wji = w2i * c(kk1)
          x0r = a(0, k2) + a(0, j2)
          x0i = a(1, k2) - a(1, j2)
          x1r = a(0, k2) - a(0, j2)
          x1i = a(1, k2) + a(1, j2)
          a(1, k2) = wkr * x0r - wki * x0i
          a(0, k2) = wkr * x0i + wki * x0r
          a(1, j2) = -wjr * x1r + wji * x1i
          a(0, j2) = wjr * x1i + wji * x1r
      end do
      w2r = 2 * c(kk2)
      kk1 = ks1
      do k1 = 2, n1 - 2, 2
          wkr = 2 * c(kk1)
          wki = 2 * c(nc - kk1)
          wjr = w2r * wkr
          wji = w2r * wki
          kk1 = kk1 + ks1
          x0i = wkr * a(k1 + 1, 0) + wki * a(k1, 0)
          a(k1 + 1, 0) = wkr * a(k1, 0) - wki * a(k1 + 1, 0)
          a(k1, 0) = x0i
          x0i = wjr * a(k1 + 1, n2h) + wji * a(k1, n2h)
          a(k1 + 1, n2h) = wjr * a(k1, n2h) - wji * a(k1 + 1, n2h)
          a(k1, n2h) = x0i
      end do
      a(1, 0) = a(1, 0) * w2r
      a(0, n2h) = a(0, n2h) * w2r
      a(1, n2h) = a(1, n2h) * 0.5d0
      end
!
      subroutine dstfsub(n1max, n1, n2, a, nc, c)
      integer n1max, n1, n2, nc, kk1, kk2, ks1, ks2, n2h, j2, 
     &    k1, k2
      real*8 a(0 : n1max - 1, 0 : n2 - 1), c(0 : nc - 1), 
     &    w2r, w2i, wkr, wki, wjr, wji, x0r, x0i, x1r, x1i
      ks1 = nc / n1
      ks2 = nc / n2
      n2h = n2 / 2
      kk2 = ks2
      do k2 = 1, n2h - 1
          j2 = n2 - k2
          w2r = 2 * c(kk2)
          w2i = 2 * c(nc - kk2)
          kk2 = kk2 + ks2
          kk1 = ks1
          do k1 = 2, n1 - 2, 2
              x0r = w2r * c(kk1)
              x0i = w2i * c(kk1)
              x1r = w2r * c(nc - kk1)
              x1i = w2i * c(nc - kk1)
              wkr = x0r - x1i
              wki = x0i + x1r
              wji = x0r + x1i
              wjr = x0i - x1r
              kk1 = kk1 + ks1
              x0r = a(k1 + 1, j2) - a(k1, k2)
              x0i = a(k1 + 1, k2) + a(k1, j2)
              x1r = a(k1 + 1, k2) - a(k1, j2)
              x1i = a(k1 + 1, j2) + a(k1, k2)
              a(k1, k2) = wkr * x0r + wki * x0i
              a(k1 + 1, k2) = wkr * x0i - wki * x0r
              a(k1, j2) = wjr * x1r + wji * x1i
              a(k1 + 1, j2) = wjr * x1i - wji * x1r
          end do
          x0r = 2 * c(kk1)
          wjr = x0r * w2r
          wji = x0r * w2i
          x0r = w2r * a(1, k2) + w2i * a(0, k2)
          x0i = w2r * a(0, k2) - w2i * a(1, k2)
          x1r = -wjr * a(1, j2) + wji * a(0, j2)
          x1i = wjr * a(0, j2) + wji * a(1, j2)
          a(0, k2) = x0r + x1r
          a(1, k2) = x1i + x0i
          a(0, j2) = x0r - x1r
          a(1, j2) = x1i - x0i
      end do
      w2r = 2 * c(kk2)
      kk1 = ks1
      do k1 = 2, n1 - 2, 2
          wkr = 2 * c(kk1)
          wki = 2 * c(nc - kk1)
          wjr = w2r * wkr
          wji = w2r * wki
          kk1 = kk1 + ks1
          x0i = wkr * a(k1, 0) - wki * a(k1 + 1, 0)
          a(k1, 0) = wkr * a(k1 + 1, 0) + wki * a(k1, 0)
          a(k1 + 1, 0) = x0i
          x0i = wjr * a(k1, n2h) - wji * a(k1 + 1, n2h)
          a(k1, n2h) = wjr * a(k1 + 1, n2h) + wji * a(k1, n2h)
          a(k1 + 1, n2h) = x0i
      end do
      w2r = w2r * 2
      a(0, 0) = a(0, 0) * 2
      a(1, 0) = a(1, 0) * w2r
      a(0, n2h) = a(0, n2h) * w2r
      end
!
