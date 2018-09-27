// Copyright 2008-2016 Conrad Sanderson (http://conradsanderson.id.au)
// Copyright 2008-2016 National ICT Australia (NICTA)
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ------------------------------------------------------------------------


#include <climits>
#include <limits>
#include <complex>

#if (__cplusplus >= 201103L)
  #undef  ARMA_USE_CXX11
  #define ARMA_USE_CXX11
#endif

#include "armadillo_bits/config.hpp"
#undef ARMA_USE_WRAPPER

#include "armadillo_bits/compiler_setup.hpp"
#include "armadillo_bits/typedef_elem.hpp"
#include "armadillo_bits/include_atlas.hpp"
#include "armadillo_bits/include_superlu.hpp"


#if defined(ARMA_USE_EXTERN_CXX11_RNG)
  #include <random>
  #include <ctime>
  
  #if defined(ARMA_HAVE_GETTIMEOFDAY)
    #include <sys/time.h>
  #endif
  
  namespace arma
    {
    #include "armadillo_bits/arma_rng_cxx11.hpp"
    thread_local arma_rng_cxx11 arma_rng_cxx11_instance;
    }
#endif

#if defined(ARMA_USE_HDF5_ALT)
  #include <hdf5.h>
  
  #if defined(H5_USE_16_API_DEFAULT) || defined(H5_USE_16_API)
    // #pragma message ("disabling use of HDF5 due to its incompatible configuration")
    #undef ARMA_USE_HDF5_ALT
  #endif

#endif

namespace arma
{

#include "armadillo_bits/def_blas.hpp"
#include "armadillo_bits/def_lapack.hpp"
#include "armadillo_bits/def_arpack.hpp"
#include "armadillo_bits/def_superlu.hpp"
// no need to include def_hdf5.hpp -- it only contains #defines for when ARMA_USE_HDF5_ALT is not defined.


#if defined(ARMA_USE_HDF5_ALT)
  // Wrapper functions: arma::H5open() and arma::H5check_version() to hijack calls to H5open() and H5check_version()
  herr_t H5open()
    {
    return ::H5open();
    }
  
  herr_t H5check_version(unsigned majnum, unsigned minnum, unsigned relnum)
    {
    return ::H5check_version(majnum, minnum, relnum);
    }
#endif



// at this stage we have prototypes for the real blas, lapack and atlas functions

// now we make the wrapper functions


extern "C"
  {
  #if defined(ARMA_USE_BLAS)
    
    float arma_fortran_prefix(arma_sasum)(blas_int* n, const float* x, blas_int* incx)
      {
      return arma_fortran_noprefix(arma_sasum)(n, x, incx);
      }
    
    double arma_fortran_prefix(arma_dasum)(blas_int* n, const double* x, blas_int* incx)
      {
      return arma_fortran_noprefix(arma_dasum)(n, x, incx);
      }
    
    
    
    float arma_fortran_prefix(arma_snrm2)(blas_int* n, const float* x, blas_int* incx)
      {
      return arma_fortran_noprefix(arma_snrm2)(n, x, incx);
      }
    
    double arma_fortran_prefix(arma_dnrm2)(blas_int* n, const double* x, blas_int* incx)
      {
      return arma_fortran_noprefix(arma_dnrm2)(n, x, incx);
      }
    
    
    
    float arma_fortran_prefix(arma_sdot)(blas_int* n, const float*  x, blas_int* incx, const float*  y, blas_int* incy)
      {
      return arma_fortran_noprefix(arma_sdot)(n, x, incx, y, incy);
      }
    
    double arma_fortran_prefix(arma_ddot)(blas_int* n, const double* x, blas_int* incx, const double* y, blas_int* incy)
      {
      return arma_fortran_noprefix(arma_ddot)(n, x, incx, y, incy);
      }
    
    
    
    void arma_fortran_prefix(arma_sgemv)(const char* transA, const blas_int* m, const blas_int* n, const float*  alpha, const float*  A, const blas_int* ldA, const float*  x, const blas_int* incx, const float*  beta, float*  y, const blas_int* incy)
      {
      arma_fortran_noprefix(arma_sgemv)(transA, m, n, alpha, A, ldA, x, incx, beta, y, incy);
      }
    
    void arma_fortran_prefix(arma_dgemv)(const char* transA, const blas_int* m, const blas_int* n, const double* alpha, const double* A, const blas_int* ldA, const double* x, const blas_int* incx, const double* beta, double* y, const blas_int* incy)
      {
      arma_fortran_noprefix(arma_dgemv)(transA, m, n, alpha, A, ldA, x, incx, beta, y, incy);
      }
    
    void arma_fortran_prefix(arma_cgemv)(const char* transA, const blas_int* m, const blas_int* n, const void*   alpha, const void*   A, const blas_int* ldA, const void*   x, const blas_int* incx, const void*   beta, void*   y, const blas_int* incy)
      {
      arma_fortran_noprefix(arma_cgemv)(transA, m, n, alpha, A, ldA, x, incx, beta, y, incy);
      }
    
    void arma_fortran_prefix(arma_zgemv)(const char* transA, const blas_int* m, const blas_int* n, const void*   alpha, const void*   A, const blas_int* ldA, const void*   x, const blas_int* incx, const void*   beta, void*   y, const blas_int* incy)
      {
      arma_fortran_noprefix(arma_zgemv)(transA, m, n, alpha, A, ldA, x, incx, beta, y, incy);
      }
    
    
    
    void arma_fortran_prefix(arma_sgemm)(const char* transA, const char* transB, const blas_int* m, const blas_int* n, const blas_int* k, const float*  alpha, const float*  A, const blas_int* ldA, const float*  B, const blas_int* ldB, const float*  beta, float*  C, const blas_int* ldC)
      {
      arma_fortran_noprefix(arma_sgemm)(transA, transB, m, n, k, alpha, A, ldA, B, ldB, beta, C, ldC);
      }
    
    void arma_fortran_prefix(arma_dgemm)(const char* transA, const char* transB, const blas_int* m, const blas_int* n, const blas_int* k, const double* alpha, const double* A, const blas_int* ldA, const double* B, const blas_int* ldB, const double* beta, double* C, const blas_int* ldC)
      {
      arma_fortran_noprefix(arma_dgemm)(transA, transB, m, n, k, alpha, A, ldA, B, ldB, beta, C, ldC);
      }
    
    void arma_fortran_prefix(arma_cgemm)(const char* transA, const char* transB, const blas_int* m, const blas_int* n, const blas_int* k, const void*   alpha, const void*   A, const blas_int* ldA, const void*   B, const blas_int* ldB, const void*   beta, void*   C, const blas_int* ldC)
      {
      arma_fortran_noprefix(arma_cgemm)(transA, transB, m, n, k, alpha, A, ldA, B, ldB, beta, C, ldC);
      }
    
    void arma_fortran_prefix(arma_zgemm)(const char* transA, const char* transB, const blas_int* m, const blas_int* n, const blas_int* k, const void*   alpha, const void*   A, const blas_int* ldA, const void*   B, const blas_int* ldB, const void*   beta, void*   C, const blas_int* ldC)
      {
      arma_fortran_noprefix(arma_zgemm)(transA, transB, m, n, k, alpha, A, ldA, B, ldB, beta, C, ldC);
      }
    
    
    
    void arma_fortran_prefix(arma_ssyrk)(const char* uplo, const char* transA, const blas_int* n, const blas_int* k, const  float* alpha, const  float* A, const blas_int* ldA, const  float* beta,  float* C, const blas_int* ldC)
      {
      arma_fortran_noprefix(arma_ssyrk)(uplo, transA, n, k, alpha, A, ldA, beta, C, ldC);
      }
    
    void arma_fortran_prefix(arma_dsyrk)(const char* uplo, const char* transA, const blas_int* n, const blas_int* k, const double* alpha, const double* A, const blas_int* ldA, const double* beta, double* C, const blas_int* ldC)
      {
      arma_fortran_noprefix(arma_dsyrk)(uplo, transA, n, k, alpha, A, ldA, beta, C, ldC);
      }
    
    
    
    void arma_fortran_prefix(arma_cherk)(const char* uplo, const char* transA, const blas_int* n, const blas_int* k, const  float* alpha, const void* A, const blas_int* ldA, const  float* beta, void* C, const blas_int* ldC)
      {
      arma_fortran_noprefix(arma_cherk)(uplo, transA, n, k, alpha, A, ldA, beta, C, ldC);
      }
    
    void arma_fortran_prefix(arma_zherk)(const char* uplo, const char* transA, const blas_int* n, const blas_int* k, const double* alpha, const void* A, const blas_int* ldA, const double* beta, void* C, const blas_int* ldC)
      {
      arma_fortran_noprefix(arma_zherk)(uplo, transA, n, k, alpha, A, ldA, beta, C, ldC);
      }
    
  #endif
  
  
  
  #if defined(ARMA_USE_LAPACK)
    
    void arma_fortran_prefix(arma_sgetrf)(blas_int* m, blas_int* n,  float* a, blas_int* lda, blas_int* ipiv, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgetrf)(m, n, a, lda, ipiv, info);
      }
    
    void arma_fortran_prefix(arma_dgetrf)(blas_int* m, blas_int* n, double* a, blas_int* lda, blas_int* ipiv, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgetrf)(m, n, a, lda, ipiv, info);
      }

    void arma_fortran_prefix(arma_cgetrf)(blas_int* m, blas_int* n,   void* a, blas_int* lda, blas_int* ipiv, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgetrf)(m, n, a, lda, ipiv, info);
      }
    
    void arma_fortran_prefix(arma_zgetrf)(blas_int* m, blas_int* n,   void* a, blas_int* lda, blas_int* ipiv, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgetrf)(m, n, a, lda, ipiv, info);
      }
    
    
    
    void arma_fortran_prefix(arma_sgetri)(blas_int* n,  float* a, blas_int* lda, blas_int* ipiv,  float* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgetri)(n, a, lda, ipiv, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_dgetri)(blas_int* n, double* a, blas_int* lda, blas_int* ipiv, double* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgetri)(n, a, lda, ipiv, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_cgetri)(blas_int* n,  void*  a, blas_int* lda, blas_int* ipiv,   void* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgetri)(n, a, lda, ipiv, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_zgetri)(blas_int* n,  void*  a, blas_int* lda, blas_int* ipiv,   void* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgetri)(n, a, lda, ipiv, work, lwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_strtri)(char* uplo, char* diag, blas_int* n,  float* a, blas_int* lda, blas_int* info)
      {
      arma_fortran_noprefix(arma_strtri)(uplo, diag, n, a, lda, info);
      }
    
    void arma_fortran_prefix(arma_dtrtri)(char* uplo, char* diag, blas_int* n, double* a, blas_int* lda, blas_int* info)
      {
      arma_fortran_noprefix(arma_dtrtri)(uplo, diag, n, a, lda, info);
      }
    
    void arma_fortran_prefix(arma_ctrtri)(char* uplo, char* diag, blas_int* n,   void* a, blas_int* lda, blas_int* info)
      {
      arma_fortran_noprefix(arma_ctrtri)(uplo, diag, n, a, lda, info);
      }
    
    void arma_fortran_prefix(arma_ztrtri)(char* uplo, char* diag, blas_int* n,   void* a, blas_int* lda, blas_int* info)
      {
      arma_fortran_noprefix(arma_ztrtri)(uplo, diag, n, a, lda, info);
      }
    
    
    
    void arma_fortran_prefix(arma_ssyev)(char* jobz, char* uplo, blas_int* n,  float* a, blas_int* lda,  float* w,  float* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_ssyev)(jobz, uplo, n, a, lda, w, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_dsyev)(char* jobz, char* uplo, blas_int* n, double* a, blas_int* lda, double* w, double* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dsyev)(jobz, uplo, n, a, lda, w, work, lwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_cheev)(char* jobz, char* uplo, blas_int* n,   void* a, blas_int* lda,  float* w,   void* work, blas_int* lwork,  float* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cheev)(jobz, uplo, n, a, lda, w, work, lwork, rwork, info);
      }
    
    void arma_fortran_prefix(arma_zheev)(char* jobz, char* uplo, blas_int* n,   void* a, blas_int* lda, double* w,   void* work, blas_int* lwork, double* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zheev)(jobz, uplo, n, a, lda, w, work, lwork, rwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_ssyevd)(char* jobz, char* uplo, blas_int* n,  float* a, blas_int* lda,  float* w,  float* work, blas_int* lwork, blas_int* iwork, blas_int* liwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_ssyevd)(jobz, uplo, n, a, lda, w, work, lwork, iwork, liwork, info);
      }
    
    void arma_fortran_prefix(arma_dsyevd)(char* jobz, char* uplo, blas_int* n, double* a, blas_int* lda, double* w, double* work, blas_int* lwork, blas_int* iwork, blas_int* liwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dsyevd)(jobz, uplo, n, a, lda, w, work, lwork, iwork, liwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_cheevd)(char* jobz, char* uplo, blas_int* n,   void* a, blas_int* lda,  float* w,   void* work, blas_int* lwork,  float* rwork, blas_int* lrwork, blas_int* iwork, blas_int* liwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cheevd)(jobz, uplo, n, a, lda, w, work, lwork, rwork, lrwork, iwork, liwork, info);
      }
    
    void arma_fortran_prefix(arma_zheevd)(char* jobz, char* uplo, blas_int* n,   void* a, blas_int* lda, double* w,   void* work, blas_int* lwork, double* rwork, blas_int* lrwork, blas_int* iwork, blas_int* liwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zheevd)(jobz, uplo, n, a, lda, w, work, lwork, rwork, lrwork, iwork, liwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_sgeev)(char* jobvl, char* jobvr, blas_int* n,  float* a, blas_int* lda,  float* wr,  float* wi,  float* vl, blas_int* ldvl,  float* vr, blas_int* ldvr,  float* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgeev)(jobvl, jobvr, n, a, lda, wr, wi, vl, ldvl, vr, ldvr, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_dgeev)(char* jobvl, char* jobvr, blas_int* n, double* a, blas_int* lda, double* wr, double* wi, double* vl, blas_int* ldvl, double* vr, blas_int* ldvr, double* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgeev)(jobvl, jobvr, n, a, lda, wr, wi, vl, ldvl, vr, ldvr, work, lwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_cgeev)(char* jobvl, char* jobvr, blas_int* n, void* a, blas_int* lda, void* w, void* vl, blas_int* ldvl, void* vr, blas_int* ldvr, void* work, blas_int* lwork,  float* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgeev)(jobvl, jobvr, n, a, lda, w, vl, ldvl, vr, ldvr, work, lwork, rwork, info);
      }
    
    void arma_fortran_prefix(arma_zgeev)(char* jobvl, char* jobvr, blas_int* n, void* a, blas_int* lda, void* w, void* vl, blas_int* ldvl, void* vr, blas_int* ldvr, void* work, blas_int* lwork, double* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgeev)(jobvl, jobvr, n, a, lda, w, vl, ldvl, vr, ldvr, work, lwork, rwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_sggev)(char* jobvl, char* jobvr, blas_int* n,  float* a, blas_int* lda,  float* b, blas_int* ldb,  float* alphar,  float* alphai,  float* beta,  float* vl, blas_int* ldvl,  float* vr, blas_int* ldvr,  float* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sggev)(jobvl, jobvr, n, a, lda, b, ldb, alphar, alphai, beta, vl, ldvl, vr, ldvr, work, lwork, info);
      }
      
    void arma_fortran_prefix(arma_dggev)(char* jobvl, char* jobvr, blas_int* n, double* a, blas_int* lda, double* b, blas_int* ldb, double* alphar, double* alphai, double* beta, double* vl, blas_int* ldvl, double* vr, blas_int* ldvr, double* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dggev)(jobvl, jobvr, n, a, lda, b, ldb, alphar, alphai, beta, vl, ldvl, vr, ldvr, work, lwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_cggev)(char* jobvl, char* jobvr, blas_int* n, void* a, blas_int* lda, void* b, blas_int* ldb, void* alpha, void* beta, void* vl, blas_int* ldvl, void* vr, blas_int* ldvr, void* work, blas_int* lwork,  float* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cggev)(jobvl, jobvr, n, a, lda, b, ldb, alpha, beta, vl, ldvl, vr, ldvr, work, lwork, rwork, info);
      }
    
    void arma_fortran_prefix(arma_zggev)(char* jobvl, char* jobvr, blas_int* n, void* a, blas_int* lda, void* b, blas_int* ldb, void* alpha, void* beta, void* vl, blas_int* ldvl, void* vr, blas_int* ldvr, void* work, blas_int* lwork, double* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zggev)(jobvl, jobvr, n, a, lda, b, ldb, alpha, beta, vl, ldvl, vr, ldvr, work, lwork, rwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_spotrf)(char* uplo, blas_int* n,  float* a, blas_int* lda, blas_int* info)
      {
      arma_fortran_noprefix(arma_spotrf)(uplo, n, a, lda, info);
      }
    
    void arma_fortran_prefix(arma_dpotrf)(char* uplo, blas_int* n, double* a, blas_int* lda, blas_int* info)
      {
      arma_fortran_noprefix(arma_dpotrf)(uplo, n, a, lda, info);
      }
    
    void arma_fortran_prefix(arma_cpotrf)(char* uplo, blas_int* n,   void* a, blas_int* lda, blas_int* info)
      {
      arma_fortran_noprefix(arma_cpotrf)(uplo, n, a, lda, info);
      }
    
    void arma_fortran_prefix(arma_zpotrf)(char* uplo, blas_int* n,   void* a, blas_int* lda, blas_int* info)
      {
      arma_fortran_noprefix(arma_zpotrf)(uplo, n, a, lda, info);
      }
    
    
    
    void arma_fortran_prefix(arma_spbtrf)(char* uplo, blas_int* n, blas_int* kd,  float* ab, blas_int* ldab, blas_int* info)
      {
      arma_fortran_noprefix(arma_spbtrf)(uplo, n, kd, ab, ldab, info);
      }
    
    void arma_fortran_prefix(arma_dpbtrf)(char* uplo, blas_int* n, blas_int* kd, double* ab, blas_int* ldab, blas_int* info)
      {
      arma_fortran_noprefix(arma_dpbtrf)(uplo, n, kd, ab, ldab, info);
      }
    
    void arma_fortran_prefix(arma_cpbtrf)(char* uplo, blas_int* n, blas_int* kd,   void* ab, blas_int* ldab, blas_int* info)
      {
      arma_fortran_noprefix(arma_cpbtrf)(uplo, n, kd, ab, ldab, info);
      }
    
    void arma_fortran_prefix(arma_zpbtrf)(char* uplo, blas_int* n, blas_int* kd,   void* ab, blas_int* ldab, blas_int* info)
      {
      arma_fortran_noprefix(arma_zpbtrf)(uplo, n, kd, ab, ldab, info);
      }
    
    
    
    void arma_fortran_prefix(arma_spotri)(char* uplo, blas_int* n,  float* a, blas_int* lda, blas_int* info)
      {
      arma_fortran_noprefix(arma_spotri)(uplo, n, a, lda, info);
      }
    
    void arma_fortran_prefix(arma_dpotri)(char* uplo, blas_int* n, double* a, blas_int* lda, blas_int* info)
      {
      arma_fortran_noprefix(arma_dpotri)(uplo, n, a, lda, info);
      }
    
    void arma_fortran_prefix(arma_cpotri)(char* uplo, blas_int* n,   void* a, blas_int* lda, blas_int* info)
      {
      arma_fortran_noprefix(arma_cpotri)(uplo, n, a, lda, info);
      }
    
    void arma_fortran_prefix(arma_zpotri)(char* uplo, blas_int* n,   void* a, blas_int* lda, blas_int* info)
      {
      arma_fortran_noprefix(arma_zpotri)(uplo, n, a, lda, info);
      }
    
    
    
    void arma_fortran_prefix(arma_sgeqrf)(blas_int* m, blas_int* n,  float* a, blas_int* lda,  float* tau,  float* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgeqrf)(m, n, a, lda, tau, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_dgeqrf)(blas_int* m, blas_int* n, double* a, blas_int* lda, double* tau, double* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgeqrf)(m, n, a, lda, tau, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_cgeqrf)(blas_int* m, blas_int* n,   void* a, blas_int* lda,   void* tau,   void* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgeqrf)(m, n, a, lda, tau, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_zgeqrf)(blas_int* m, blas_int* n,   void* a, blas_int* lda,   void* tau,   void* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgeqrf)(m, n, a, lda, tau, work, lwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_sorgqr)(blas_int* m, blas_int* n, blas_int* k,  float* a, blas_int* lda,  float* tau,  float* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sorgqr)(m, n, k, a, lda, tau, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_dorgqr)(blas_int* m, blas_int* n, blas_int* k, double* a, blas_int* lda, double* tau, double* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dorgqr)(m, n, k, a, lda, tau, work, lwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_cungqr)(blas_int* m, blas_int* n, blas_int* k,   void* a, blas_int* lda,   void* tau,   void* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cungqr)(m, n, k, a, lda, tau, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_zungqr)(blas_int* m, blas_int* n, blas_int* k,   void* a, blas_int* lda,   void* tau,   void* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zungqr)(m, n, k, a, lda, tau, work, lwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_sgesvd)(char* jobu, char* jobvt, blas_int* m, blas_int* n, float*  a, blas_int* lda, float*  s, float*  u, blas_int* ldu, float*  vt, blas_int* ldvt, float*  work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgesvd)(jobu, jobvt, m, n, a, lda, s, u, ldu, vt, ldvt, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_dgesvd)(char* jobu, char* jobvt, blas_int* m, blas_int* n, double* a, blas_int* lda, double* s, double* u, blas_int* ldu, double* vt, blas_int* ldvt, double* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgesvd)(jobu, jobvt, m, n, a, lda, s, u, ldu, vt, ldvt, work, lwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_cgesvd)(char* jobu, char* jobvt, blas_int* m, blas_int* n, void*   a, blas_int* lda, float*  s, void*   u, blas_int* ldu, void*   vt, blas_int* ldvt, void*   work, blas_int* lwork, float*  rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgesvd)(jobu, jobvt, m, n, a, lda, s, u, ldu, vt, ldvt, work, lwork, rwork, info);
      }
    
    void arma_fortran_prefix(arma_zgesvd)(char* jobu, char* jobvt, blas_int* m, blas_int* n, void*   a, blas_int* lda, double* s, void*   u, blas_int* ldu, void*   vt, blas_int* ldvt, void*   work, blas_int* lwork, double* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgesvd)(jobu, jobvt, m, n, a, lda, s, u, ldu, vt, ldvt, work, lwork, rwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_sgesdd)(char* jobz, blas_int* m, blas_int* n, float*  a, blas_int* lda, float*  s, float*  u, blas_int* ldu, float*  vt, blas_int* ldvt, float*  work, blas_int* lwork, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgesdd)(jobz, m, n, a, lda, s, u, ldu, vt, ldvt, work, lwork, iwork, info);
      }
    
    void arma_fortran_prefix(arma_dgesdd)(char* jobz, blas_int* m, blas_int* n, double* a, blas_int* lda, double* s, double* u, blas_int* ldu, double* vt, blas_int* ldvt, double* work, blas_int* lwork, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgesdd)(jobz, m, n, a, lda, s, u, ldu, vt, ldvt, work, lwork, iwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_cgesdd)(char* jobz, blas_int* m, blas_int* n, void* a, blas_int* lda, float*  s, void* u, blas_int* ldu, void* vt, blas_int* ldvt, void* work, blas_int* lwork, float*  rwork, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgesdd)(jobz, m, n, a, lda, s, u, ldu, vt, ldvt, work, lwork, rwork, iwork, info);
      }
    
    void arma_fortran_prefix(arma_zgesdd)(char* jobz, blas_int* m, blas_int* n, void* a, blas_int* lda, double* s, void* u, blas_int* ldu, void* vt, blas_int* ldvt, void* work, blas_int* lwork, double* rwork, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgesdd)(jobz, m, n, a, lda, s, u, ldu, vt, ldvt, work, lwork, rwork, iwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_sgesv)(blas_int* n, blas_int* nrhs, float*  a, blas_int* lda, blas_int* ipiv, float*  b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgesv)(n, nrhs, a, lda, ipiv, b, ldb, info);
      }
    
    void arma_fortran_prefix(arma_dgesv)(blas_int* n, blas_int* nrhs, double* a, blas_int* lda, blas_int* ipiv, double* b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgesv)(n, nrhs, a, lda, ipiv, b, ldb, info);
      }
    
    void arma_fortran_prefix(arma_cgesv)(blas_int* n, blas_int* nrhs, void*   a, blas_int* lda, blas_int* ipiv, void*   b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgesv)(n, nrhs, a, lda, ipiv, b, ldb, info);
      }
    
    void arma_fortran_prefix(arma_zgesv)(blas_int* n, blas_int* nrhs, void*   a, blas_int* lda, blas_int* ipiv, void*   b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgesv)(n, nrhs, a, lda, ipiv, b, ldb, info);
      }
    
    
    
    void arma_fortran_prefix(arma_sgesvx)(char* fact, char* trans, blas_int* n, blas_int* nrhs,  float* a, blas_int* lda,  float* af, blas_int* ldaf, blas_int* ipiv, char* equed,  float* r,  float* c,  float* b, blas_int* ldb,  float* x, blas_int* ldx,  float* rcond,  float* ferr,  float* berr,  float* work, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgesvx)(fact, trans, n, nrhs, a, lda, af, ldaf, ipiv, equed, r, c, b, ldb, x, ldx, rcond, ferr, berr, work, iwork, info);
      }
    
    void arma_fortran_prefix(arma_dgesvx)(char* fact, char* trans, blas_int* n, blas_int* nrhs, double* a, blas_int* lda, double* af, blas_int* ldaf, blas_int* ipiv, char* equed, double* r, double* c, double* b, blas_int* ldb, double* x, blas_int* ldx, double* rcond, double* ferr, double* berr, double* work, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgesvx)(fact, trans, n, nrhs, a, lda, af, ldaf, ipiv, equed, r, c, b, ldb, x, ldx, rcond, ferr, berr, work, iwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_cgesvx)(char* fact, char* trans, blas_int* n, blas_int* nrhs, void* a, blas_int* lda, void* af, blas_int* ldaf, blas_int* ipiv, char* equed,  float* r,  float* c, void* b, blas_int* ldb, void* x, blas_int* ldx,  float* rcond,  float* ferr,  float* berr, void* work,  float* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgesvx)(fact, trans, n, nrhs, a, lda, af, ldaf, ipiv, equed, r, c, b, ldb, x, ldx, rcond, ferr, berr, work, rwork, info);
      }
    
    void arma_fortran_prefix(arma_zgesvx)(char* fact, char* trans, blas_int* n, blas_int* nrhs, void* a, blas_int* lda, void* af, blas_int* ldaf, blas_int* ipiv, char* equed, double* r, double* c, void* b, blas_int* ldb, void* x, blas_int* ldx, double* rcond, double* ferr, double* berr, void* work, double* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgesvx)(fact, trans, n, nrhs, a, lda, af, ldaf, ipiv, equed, r, c, b, ldb, x, ldx, rcond, ferr, berr, work, rwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_sposv)(char* uplo, blas_int* n, blas_int* nrhs,  float* a, blas_int* lda,  float* b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_sposv)(uplo, n, nrhs, a, lda, b, ldb, info);
      }
    
    void arma_fortran_prefix(arma_dposv)(char* uplo, blas_int* n, blas_int* nrhs, double* a, blas_int* lda, double* b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_dposv)(uplo, n, nrhs, a, lda, b, ldb, info);
      }
    
    void arma_fortran_prefix(arma_cposv)(char* uplo, blas_int* n, blas_int* nrhs,   void* a, blas_int* lda,   void* b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_cposv)(uplo, n, nrhs, a, lda, b, ldb, info);
      }
    
    void arma_fortran_prefix(arma_zposv)(char* uplo, blas_int* n, blas_int* nrhs,   void* a, blas_int* lda,   void* b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_zposv)(uplo, n, nrhs, a, lda, b, ldb, info);
      }
    
    
    
    void arma_fortran_prefix(arma_sposvx)(char* fact, char* uplo, blas_int* n, blas_int* nrhs,  float* a, blas_int* lda,  float* af, blas_int* ldaf, char* equed,  float* s,  float* b, blas_int* ldb,  float* x, blas_int* ldx,  float* rcond,  float* ferr,  float* berr,  float* work, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sposvx)(fact, uplo, n, nrhs, a, lda, af, ldaf, equed, s, b, ldb, x, ldx, rcond, ferr, berr, work, iwork, info);
      }
    
    void arma_fortran_prefix(arma_dposvx)(char* fact, char* uplo, blas_int* n, blas_int* nrhs, double* a, blas_int* lda, double* af, blas_int* ldaf, char* equed, double* s, double* b, blas_int* ldb, double* x, blas_int* ldx, double* rcond, double* ferr, double* berr, double* work, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dposvx)(fact, uplo, n, nrhs, a, lda, af, ldaf, equed, s, b, ldb, x, ldx, rcond, ferr, berr, work, iwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_cposvx)(char* fact, char* uplo, blas_int* n, blas_int* nrhs, void* a, blas_int* lda, void* af, blas_int* ldaf, char* equed,  float* s, void* b, blas_int* ldb, void* x, blas_int* ldx,  float* rcond,  float* ferr,  float* berr, void* work,  float* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cposvx)(fact, uplo, n, nrhs, a, lda, af, ldaf, equed, s, b, ldb, x, ldx, rcond, ferr, berr, work, rwork, info);
      }
    
    void arma_fortran_prefix(arma_zposvx)(char* fact, char* uplo, blas_int* n, blas_int* nrhs, void* a, blas_int* lda, void* af, blas_int* ldaf, char* equed, double* s, void* b, blas_int* ldb, void* x, blas_int* ldx, double* rcond, double* ferr, double* berr, void* work, double* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zposvx)(fact, uplo, n, nrhs, a, lda, af, ldaf, equed, s, b, ldb, x, ldx, rcond, ferr, berr, work, rwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_sgels)(char* trans, blas_int* m, blas_int* n, blas_int* nrhs, float*  a, blas_int* lda, float*  b, blas_int* ldb, float*  work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgels)(trans, m, n, nrhs, a, lda, b, ldb, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_dgels)(char* trans, blas_int* m, blas_int* n, blas_int* nrhs, double* a, blas_int* lda, double* b, blas_int* ldb, double* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgels)(trans, m, n, nrhs, a, lda, b, ldb, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_cgels)(char* trans, blas_int* m, blas_int* n, blas_int* nrhs, void*   a, blas_int* lda, void*   b, blas_int* ldb, void*   work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgels)(trans, m, n, nrhs, a, lda, b, ldb, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_zgels)(char* trans, blas_int* m, blas_int* n, blas_int* nrhs, void*   a, blas_int* lda, void*   b, blas_int* ldb, void*   work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgels)(trans, m, n, nrhs, a, lda, b, ldb, work, lwork, info);
      }
    
    
    
    
    void arma_fortran_prefix(arma_sgelsd)(blas_int* m, blas_int* n, blas_int* nrhs,  float* a, blas_int* lda,  float* b, blas_int* ldb,  float* S,  float* rcond, blas_int* rank,  float* work, blas_int* lwork, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgelsd)(m, n, nrhs, a, lda, b, ldb, S, rcond, rank, work, lwork, iwork, info);
      }
    
    void arma_fortran_prefix(arma_dgelsd)(blas_int* m, blas_int* n, blas_int* nrhs, double* a, blas_int* lda, double* b, blas_int* ldb, double* S, double* rcond, blas_int* rank, double* work, blas_int* lwork, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgelsd)(m, n, nrhs, a, lda, b, ldb, S, rcond, rank, work, lwork, iwork, info);
      }
    
    void arma_fortran_prefix(arma_cgelsd)(blas_int* m, blas_int* n, blas_int* nrhs, void* a, blas_int* lda, void* b, blas_int* ldb,  float* S,  float* rcond, blas_int* rank, void* work, blas_int* lwork,  float* rwork, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgelsd)(m, n, nrhs, a, lda, b, ldb, S, rcond, rank, work, lwork, rwork, iwork, info);
      }
    
    void arma_fortran_prefix(arma_zgelsd)(blas_int* m, blas_int* n, blas_int* nrhs, void* a, blas_int* lda, void* b, blas_int* ldb, double* S, double* rcond, blas_int* rank, void* work, blas_int* lwork, double* rwork, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgelsd)(m, n, nrhs, a, lda, b, ldb, S, rcond, rank, work, lwork, rwork, iwork, info);
      }
    
    
    
    
    void arma_fortran_prefix(arma_strtrs)(char* uplo, char* trans, char* diag, blas_int* n, blas_int* nrhs, const float*  a, blas_int* lda, float*  b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_strtrs)(uplo, trans, diag, n, nrhs, a, lda, b, ldb, info);
      }
    
    void arma_fortran_prefix(arma_dtrtrs)(char* uplo, char* trans, char* diag, blas_int* n, blas_int* nrhs, const double* a, blas_int* lda, double* b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_dtrtrs)(uplo, trans, diag, n, nrhs, a, lda, b, ldb, info);
      }
    
    void arma_fortran_prefix(arma_ctrtrs)(char* uplo, char* trans, char* diag, blas_int* n, blas_int* nrhs, const void*   a, blas_int* lda, void*   b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_ctrtrs)(uplo, trans, diag, n, nrhs, a, lda, b, ldb, info);
      }
    
    void arma_fortran_prefix(arma_ztrtrs)(char* uplo, char* trans, char* diag, blas_int* n, blas_int* nrhs, const void*   a, blas_int* lda, void*   b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_ztrtrs)(uplo, trans, diag, n, nrhs, a, lda, b, ldb, info);
      }
    
    
    
    
    void arma_fortran_prefix(arma_sgbsv)(blas_int* n, blas_int* kl, blas_int* ku, blas_int* nrhs,  float* ab, blas_int* ldab, blas_int* ipiv,  float* b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgbsv)(n, kl, ku, nrhs, ab, ldab, ipiv, b, ldb, info);
      }
    
    void arma_fortran_prefix(arma_dgbsv)(blas_int* n, blas_int* kl, blas_int* ku, blas_int* nrhs, double* ab, blas_int* ldab, blas_int* ipiv, double* b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgbsv)(n, kl, ku, nrhs, ab, ldab, ipiv, b, ldb, info);
      }
    
    void arma_fortran_prefix(arma_cgbsv)(blas_int* n, blas_int* kl, blas_int* ku, blas_int* nrhs,   void* ab, blas_int* ldab, blas_int* ipiv,   void* b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgbsv)(n, kl, ku, nrhs, ab, ldab, ipiv, b, ldb, info);
      }
    
    void arma_fortran_prefix(arma_zgbsv)(blas_int* n, blas_int* kl, blas_int* ku, blas_int* nrhs,   void* ab, blas_int* ldab, blas_int* ipiv,   void* b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgbsv)(n, kl, ku, nrhs, ab, ldab, ipiv, b, ldb, info);
      }
    
    
    
    
    void arma_fortran_prefix(arma_sgbsvx)(char* fact, char* trans, blas_int* n, blas_int* kl, blas_int* ku, blas_int* nrhs,  float* ab, blas_int* ldab,  float* afb, blas_int* ldafb, blas_int* ipiv, char* equed,  float* r,  float* c,  float* b, blas_int* ldb,  float* x, blas_int* ldx,  float* rcond,  float* ferr,  float* berr,  float* work, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgbsvx)(fact, trans, n, kl, ku, nrhs, ab, ldab, afb, ldafb, ipiv, equed, r, c, b, ldb, x, ldx, rcond, ferr, berr, work, iwork, info);
      }
    
    void arma_fortran_prefix(arma_dgbsvx)(char* fact, char* trans, blas_int* n, blas_int* kl, blas_int* ku, blas_int* nrhs, double* ab, blas_int* ldab, double* afb, blas_int* ldafb, blas_int* ipiv, char* equed, double* r, double* c, double* b, blas_int* ldb, double* x, blas_int* ldx, double* rcond, double* ferr, double* berr, double* work, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgbsvx)(fact, trans, n, kl, ku, nrhs, ab, ldab, afb, ldafb, ipiv, equed, r, c, b, ldb, x, ldx, rcond, ferr, berr, work, iwork, info);
      }
    
    
    void arma_fortran_prefix(arma_cgbsvx)(char* fact, char* trans, blas_int* n, blas_int* kl, blas_int* ku, blas_int* nrhs, void* ab, blas_int* ldab, void* afb, blas_int* ldafb, blas_int* ipiv, char* equed,  float* r,  float* c, void* b, blas_int* ldb, void* x, blas_int* ldx,  float* rcond,  float* ferr,  float* berr, void* work,  float* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgbsvx)(fact, trans, n, kl, ku, nrhs, ab, ldab, afb, ldafb, ipiv, equed, r, c, b, ldb, x, ldx, rcond, ferr, berr, work, rwork, info);
      }
    
    void arma_fortran_prefix(arma_zgbsvx)(char* fact, char* trans, blas_int* n, blas_int* kl, blas_int* ku, blas_int* nrhs, void* ab, blas_int* ldab, void* afb, blas_int* ldafb, blas_int* ipiv, char* equed, double* r, double* c, void* b, blas_int* ldb, void* x, blas_int* ldx, double* rcond, double* ferr, double* berr, void* work, double* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgbsvx)(fact, trans, n, kl, ku, nrhs, ab, ldab, afb, ldafb, ipiv, equed, r, c, b, ldb, x, ldx, rcond, ferr, berr, work, rwork, info);
      }
    
    
    
    
    void arma_fortran_prefix(arma_sgtsv)(blas_int* n, blas_int* nrhs,  float* dl,  float* d,  float* du,  float* b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgtsv)(n, nrhs, dl, d, du, b, ldb, info);
      }
    
    void arma_fortran_prefix(arma_dgtsv)(blas_int* n, blas_int* nrhs, double* dl, double* d, double* du, double* b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgtsv)(n, nrhs, dl, d, du, b, ldb, info);
      }
    
    void arma_fortran_prefix(arma_cgtsv)(blas_int* n, blas_int* nrhs,   void* dl,   void* d,   void* du,   void* b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgtsv)(n, nrhs, dl, d, du, b, ldb, info);
      }
    
    void arma_fortran_prefix(arma_zgtsv)(blas_int* n, blas_int* nrhs,   void* dl,   void* d,   void* du,   void* b, blas_int* ldb, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgtsv)(n, nrhs, dl, d, du, b, ldb, info);
      }
    
    
    
    
    void arma_fortran_prefix(arma_sgtsvx)(char* fact, char* trans, blas_int* n, blas_int* nrhs,  float* dl,  float* d,  float* du,  float* dlf,  float* df,  float* duf,  float* du2, blas_int* ipiv,  float* b, blas_int* ldb,  float* x, blas_int* ldx,  float* rcond,  float* ferr,  float* berr,  float* work, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgtsvx)(fact, trans, n, nrhs, dl, d, du, dlf, df, duf, du2, ipiv, b, ldb, x, ldx, rcond, ferr, berr, work, iwork, info);
      }
    
    void arma_fortran_prefix(arma_dgtsvx)(char* fact, char* trans, blas_int* n, blas_int* nrhs, double* dl, double* d, double* du, double* dlf, double* df, double* duf, double* du2, blas_int* ipiv, double* b, blas_int* ldb, double* x, blas_int* ldx, double* rcond, double* ferr, double* berr, double* work, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgtsvx)(fact, trans, n, nrhs, dl, d, du, dlf, df, duf, du2, ipiv, b, ldb, x, ldx, rcond, ferr, berr, work, iwork, info);
      }
    
    
    void arma_fortran_prefix(arma_cgtsvx)(char* fact, char* trans, blas_int* n, blas_int* nrhs, void* dl, void* d, void* du, void* dlf, void* df, void* duf, void* du2, blas_int* ipiv, void* b, blas_int* ldb, void* x, blas_int* ldx,  float* rcond,  float* ferr,  float* berr, void* work,  float* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgtsvx)(fact, trans, n, nrhs, dl, d, du, dlf, df, duf, du2, ipiv, b, ldb, x, ldx, rcond, ferr, berr, work, rwork, info);
      }
    
    void arma_fortran_prefix(arma_zgtsvx)(char* fact, char* trans, blas_int* n, blas_int* nrhs, void* dl, void* d, void* du, void* dlf, void* df, void* duf, void* du2, blas_int* ipiv, void* b, blas_int* ldb, void* x, blas_int* ldx, double* rcond, double* ferr, double* berr, void* work, double* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgtsvx)(fact, trans, n, nrhs, dl, d, du, dlf, df, duf, du2, ipiv, b, ldb, x, ldx, rcond, ferr, berr, work, rwork, info);
      }
    
    
    
    
    void arma_fortran_prefix(arma_sgees)(char* jobvs, char* sort, void* select, blas_int* n, float*  a, blas_int* lda, blas_int* sdim, float*  wr, float*  wi, float*  vs, blas_int* ldvs, float*  work, blas_int* lwork, blas_int* bwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgees)(jobvs, sort, select, n, a, lda, sdim, wr, wi, vs, ldvs, work, lwork, bwork, info);
      }
      
    void arma_fortran_prefix(arma_dgees)(char* jobvs, char* sort, void* select, blas_int* n, double* a, blas_int* lda, blas_int* sdim, double* wr, double* wi, double* vs, blas_int* ldvs, double* work, blas_int* lwork, blas_int* bwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgees)(jobvs, sort, select, n, a, lda, sdim, wr, wi, vs, ldvs, work, lwork, bwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_cgees)(char* jobvs, char* sort, void* select, blas_int* n, void* a, blas_int* lda, blas_int* sdim, void* w, void* vs, blas_int* ldvs, void* work, blas_int* lwork, float*  rwork, blas_int* bwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgees)(jobvs, sort, select, n, a, lda, sdim, w, vs, ldvs, work, lwork, rwork, bwork, info);
      }
    
    void arma_fortran_prefix(arma_zgees)(char* jobvs, char* sort, void* select, blas_int* n, void* a, blas_int* lda, blas_int* sdim, void* w, void* vs, blas_int* ldvs, void* work, blas_int* lwork, double* rwork, blas_int* bwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgees)(jobvs, sort, select, n, a, lda, sdim, w, vs, ldvs, work, lwork, rwork, bwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_strsyl)(char* transa, char* transb, blas_int* isgn, blas_int* m, blas_int* n, const float*  a, blas_int* lda, const float*  b, blas_int* ldb, float*  c, blas_int* ldc, float*  scale, blas_int* info)
      {
      arma_fortran_noprefix(arma_strsyl)(transa, transb, isgn, m, n, a, lda, b, ldb, c, ldc, scale, info);
      }
    
    void arma_fortran_prefix(arma_dtrsyl)(char* transa, char* transb, blas_int* isgn, blas_int* m, blas_int* n, const double* a, blas_int* lda, const double* b, blas_int* ldb, double* c, blas_int* ldc, double* scale, blas_int* info)
      {
      arma_fortran_noprefix(arma_dtrsyl)(transa, transb, isgn, m, n, a, lda, b, ldb, c, ldc, scale, info);
      }
    
    void arma_fortran_prefix(arma_ctrsyl)(char* transa, char* transb, blas_int* isgn, blas_int* m, blas_int* n, const void*   a, blas_int* lda, const void*   b, blas_int* ldb, void*   c, blas_int* ldc, float*  scale, blas_int* info)
      {
      arma_fortran_noprefix(arma_ctrsyl)(transa, transb, isgn, m, n, a, lda, b, ldb, c, ldc, scale, info);
      }
    
    void arma_fortran_prefix(arma_ztrsyl)(char* transa, char* transb, blas_int* isgn, blas_int* m, blas_int* n, const void*   a, blas_int* lda, const void*   b, blas_int* ldb, void*   c, blas_int* ldc, double* scale, blas_int* info)
      {
      arma_fortran_noprefix(arma_ztrsyl)(transa, transb, isgn, m, n, a, lda, b, ldb, c, ldc, scale, info);
      }
    
    
    
    
    void arma_fortran_prefix(arma_sgges)(char* jobvsl, char* jobvsr, char* sort, void* selctg, blas_int* n,  float* a, blas_int* lda,  float* b, blas_int* ldb, blas_int* sdim,  float* alphar,  float* alphai,  float* beta,  float* vsl, blas_int* ldvsl,  float* vsr, blas_int* ldvsr,  float* work, blas_int* lwork,  float* bwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgges)(jobvsl, jobvsr, sort, selctg, n, a, lda, b, ldb, sdim, alphar, alphai, beta, vsl, ldvsl, vsr, ldvsr, work, lwork, bwork, info);
      }
    
    void arma_fortran_prefix(arma_dgges)(char* jobvsl, char* jobvsr, char* sort, void* selctg, blas_int* n, double* a, blas_int* lda, double* b, blas_int* ldb, blas_int* sdim, double* alphar, double* alphai, double* beta, double* vsl, blas_int* ldvsl, double* vsr, blas_int* ldvsr, double* work, blas_int* lwork, double* bwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgges)(jobvsl, jobvsr, sort, selctg, n, a, lda, b, ldb, sdim, alphar, alphai, beta, vsl, ldvsl, vsr, ldvsr, work, lwork, bwork, info);
      }
    
    void arma_fortran_prefix(arma_cgges)(char* jobvsl, char* jobvsr, char* sort, void* selctg, blas_int* n, void* a, blas_int* lda, void* b, blas_int* ldb, blas_int* sdim, void* alpha, void* beta, void* vsl, blas_int* ldvsl, void* vsr, blas_int* ldvsr, void* work, blas_int* lwork,  float* rwork,  float* bwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgges)(jobvsl, jobvsr, sort, selctg, n, a, lda, b, ldb, sdim, alpha, beta, vsl, ldvsl, vsr, ldvsr, work, lwork, rwork, bwork, info);
      }
    
    void arma_fortran_prefix(arma_zgges)(char* jobvsl, char* jobvsr, char* sort, void* selctg, blas_int* n, void* a, blas_int* lda, void* b, blas_int* ldb, blas_int* sdim, void* alpha, void* beta, void* vsl, blas_int* ldvsl, void* vsr, blas_int* ldvsr, void* work, blas_int* lwork, double* rwork, double* bwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgges)(jobvsl, jobvsr, sort, selctg, n, a, lda, b, ldb, sdim, alpha, beta, vsl, ldvsl, vsr, ldvsr, work, lwork, rwork, bwork, info);
      }
    
    
    
    
    float  arma_fortran_prefix(arma_slange)(char* norm, blas_int* m, blas_int* n,  float* a, blas_int* lda,  float* work)
      {
      return arma_fortran_noprefix(arma_slange)(norm, m, n, a, lda, work);
      }
    
    double arma_fortran_prefix(arma_dlange)(char* norm, blas_int* m, blas_int* n, double* a, blas_int* lda, double* work)
      {
      return arma_fortran_noprefix(arma_dlange)(norm, m, n, a, lda, work);
      }
    
    float  arma_fortran_prefix(arma_clange)(char* norm, blas_int* m, blas_int* n,   void* a, blas_int* lda,  float* work)
      {
      return arma_fortran_noprefix(arma_clange)(norm, m, n, a, lda, work);
      }
    
    double arma_fortran_prefix(arma_zlange)(char* norm, blas_int* m, blas_int* n,   void* a, blas_int* lda, double* work)
      {
      return arma_fortran_noprefix(arma_zlange)(norm, m, n, a, lda, work);
      }
    
    
    
    
    void arma_fortran_prefix(arma_sgecon)(char* norm, blas_int* n,  float* a, blas_int* lda,  float* anorm,  float* rcond,  float* work, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgecon)(norm, n, a, lda, anorm, rcond, work, iwork, info);
      }
    
    void arma_fortran_prefix(arma_dgecon)(char* norm, blas_int* n, double* a, blas_int* lda, double* anorm, double* rcond, double* work, blas_int* iwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgecon)(norm, n, a, lda, anorm, rcond, work, iwork, info);
      }
    
    void arma_fortran_prefix(arma_cgecon)(char* norm, blas_int* n, void* a, blas_int* lda,  float* anorm,  float* rcond, void* work,  float* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgecon)(norm, n, a, lda, anorm, rcond, work, rwork, info);
      }
    
    void arma_fortran_prefix(arma_zgecon)(char* norm, blas_int* n, void* a, blas_int* lda, double* anorm, double* rcond, void* work, double* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgecon)(norm, n, a, lda, anorm, rcond, work, rwork, info);
      }
    
    
    
    blas_int arma_fortran_prefix(arma_ilaenv)(blas_int* ispec, char* name, char* opts, blas_int* n1, blas_int* n2, blas_int* n3, blas_int* n4)
      {
      return arma_fortran_noprefix(arma_ilaenv)(ispec, name, opts, n1, n2, n3, n4);
      }
    
    
    
    void arma_fortran_prefix(arma_slahqr)(blas_int* wantt, blas_int* wantz, blas_int* n, blas_int* ilo, blas_int* ihi, float*  h, blas_int* ldh, float*  wr, float*  wi, blas_int* iloz, blas_int* ihiz, float*  z, blas_int* ldz, blas_int* info)
      {
      arma_fortran_noprefix(arma_slahqr)(wantt, wantz, n, ilo, ihi, h, ldh, wr, wi, iloz, ihiz, z, ldz, info);
      }
    
    void arma_fortran_prefix(arma_dlahqr)(blas_int* wantt, blas_int* wantz, blas_int* n, blas_int* ilo, blas_int* ihi, double* h, blas_int* ldh, double* wr, double* wi, blas_int* iloz, blas_int* ihiz, double* z, blas_int* ldz, blas_int* info)
      {
      arma_fortran_noprefix(arma_dlahqr)(wantt, wantz, n, ilo, ihi, h, ldh, wr, wi, iloz, ihiz, z, ldz, info);
      }
    
    
    
    void arma_fortran_prefix(arma_sstedc)(char* compz, blas_int* n, float*  d, float*  e, float*  z, blas_int* ldz, float*  work, blas_int* lwork, blas_int* iwork, blas_int* liwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sstedc)(compz, n, d, e, z, ldz, work, lwork, iwork, liwork, info);
      }
    
    void arma_fortran_prefix(arma_dstedc)(char* compz, blas_int* n, double* d, double* e, double* z, blas_int* ldz, double* work, blas_int* lwork, blas_int* iwork, blas_int* liwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dstedc)(compz, n, d, e, z, ldz, work, lwork, iwork, liwork, info);
      }
    
    
    
    void arma_fortran_prefix(arma_strevc)(char* side, char* howmny, blas_int* select, blas_int* n, float*  t, blas_int* ldt, float*  vl, blas_int* ldvl, float*  vr, blas_int* ldvr, blas_int* mm, blas_int* m, float*  work, blas_int* info)
      {
      arma_fortran_noprefix(arma_strevc)(side, howmny, select, n, t, ldt, vl, ldvl, vr, ldvr, mm, m, work, info);
      }
    
    void arma_fortran_prefix(arma_dtrevc)(char* side, char* howmny, blas_int* select, blas_int* n, double* t, blas_int* ldt, double* vl, blas_int* ldvl, double* vr, blas_int* ldvr, blas_int* mm, blas_int* m, double* work, blas_int* info)
      {
      arma_fortran_noprefix(arma_dtrevc)(side, howmny, select, n, t, ldt, vl, ldvl, vr, ldvr, mm, m, work, info);
      }
    
    
    
    void arma_fortran_prefix(arma_slarnv)(blas_int* idist, blas_int* iseed, blas_int* n, float*  x)
      {
      arma_fortran_noprefix(arma_slarnv)(idist, iseed, n, x);
      }
    
    void arma_fortran_prefix(arma_dlarnv)(blas_int* idist, blas_int* iseed, blas_int* n, double* x)
      {
      arma_fortran_noprefix(arma_dlarnv)(idist, iseed, n, x);
      }
    
    
    
    void arma_fortran_prefix(arma_sgehrd)(blas_int* n, blas_int* ilo, blas_int* ihi, float*  a, blas_int* lda, float*  tao, float*  work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_sgehrd)(n, ilo, ihi, a, lda, tao, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_dgehrd)(blas_int* n, blas_int* ilo, blas_int* ihi, double* a, blas_int* lda, double* tao, double* work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_dgehrd)(n, ilo, ihi, a, lda, tao, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_cgehrd)(blas_int* n, blas_int* ilo, blas_int* ihi, void*   a, blas_int* lda, void*   tao, void*   work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cgehrd)(n, ilo, ihi, a, lda, tao, work, lwork, info);
      }
    
    void arma_fortran_prefix(arma_zgehrd)(blas_int* n, blas_int* ilo, blas_int* ihi, void*   a, blas_int* lda, void*   tao, void*   work, blas_int* lwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zgehrd)(n, ilo, ihi, a, lda, tao, work, lwork, info);
      }
    
  #endif
  
  
  
  #if defined(ARMA_USE_ATLAS)
    
    float wrapper_cblas_sasum(const int N, const float  *X, const int incX)
      {
      return      cblas_sasum(N, X, incX);
      }
    
    double wrapper_cblas_dasum(const int N, const double *X, const int incX)
      {
      return       cblas_dasum(N, X, incX);
      }
    
    
    
    float wrapper_cblas_snrm2(const int N, const float  *X, const int incX)
      {
      return      cblas_snrm2(N, X, incX);
      }
    
    double wrapper_cblas_dnrm2(const int N, const double *X, const int incX)
      {
      return       cblas_dnrm2(N, X, incX);
      }
    
    
    
    float wrapper_cblas_sdot(const int N, const float  *X, const int incX, const float  *Y, const int incY)
      {
      return      cblas_sdot(N, X, incX, Y, incY);
      }
    
    double wrapper_cblas_ddot(const int N, const double *X, const int incX, const double *Y, const int incY)
      {
      return       cblas_ddot(N, X, incX, Y, incY);
      }
    
    void wrapper_cblas_cdotu_sub(const int N, const void *X, const int incX, const void *Y, const int incY, void *dotu)
      {
                 cblas_cdotu_sub(N, X, incX, Y, incY, dotu);
      }
    
    void wrapper_cblas_zdotu_sub(const int N, const void *X, const int incX, const void *Y, const int incY, void *dotu)
      {
                 cblas_zdotu_sub(N, X, incX, Y, incY, dotu);
      }
    
    
    
    void wrapper_cblas_sgemv(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA, const int M, const int N, const float alpha,
                             const float *A, const int lda, const float *X, const int incX, const float beta, float *Y, const int incY)
      {
                 cblas_sgemv(Order, TransA, M, N, alpha, A, lda, X, incX, beta, Y, incY);
      }
    
    void wrapper_cblas_dgemv(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA, const int M, const int N, const double alpha,
                             const double *A, const int lda, const double *X, const int incX, const double beta, double *Y, const int incY)
      {
                 cblas_dgemv(Order, TransA, M, N, alpha, A, lda, X, incX, beta, Y, incY);
      }
    
    void wrapper_cblas_cgemv(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA, const int M, const int N, const void *alpha,
                             const void *A, const int lda, const void *X, const int incX, const void *beta, void *Y, const int incY)
      {
                 cblas_cgemv(Order, TransA, M, N, alpha, A, lda, X, incX, beta, Y, incY);
      }
    
    void wrapper_cblas_zgemv(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA, const int M, const int N, const void *alpha,
                             const void *A, const int lda, const void *X, const int incX, const void *beta, void *Y, const int incY)
      {
                 cblas_zgemv(Order, TransA, M, N, alpha, A, lda, X, incX, beta, Y, incY);
      }
    
    
    
    void wrapper_cblas_sgemm(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_TRANSPOSE TransB,
                             const int M, const int N, const int K, const float alpha,
                             const float *A, const int lda, const float *B, const int ldb, const float beta, float *C, const int ldc)
      {
                 cblas_sgemm(Order, TransA, TransB, M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
      }
    
    void wrapper_cblas_dgemm(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_TRANSPOSE TransB,
                             const int M, const int N, const int K, const double alpha,
                             const double *A, const int lda, const double *B, const int ldb, const double beta, double *C, const int ldc)
      {
                 cblas_dgemm(Order, TransA, TransB, M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
      }
    
    void wrapper_cblas_cgemm(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_TRANSPOSE TransB,
                             const int M, const int N, const int K, const void *alpha,
                             const void *A, const int lda, const void *B, const int ldb, const void *beta, void *C, const int ldc)
      {
                 cblas_cgemm(Order, TransA, TransB, M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
      }
    
    void wrapper_cblas_zgemm(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA, const enum CBLAS_TRANSPOSE TransB,
                             const int M, const int N, const int K, const void *alpha,
                             const void *A, const int lda, const void *B, const int ldb, const void *beta, void *C, const int ldc)
      {
                 cblas_zgemm(Order, TransA, TransB, M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
      }
    
    
    
    void wrapper_cblas_ssyrk(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE Trans,
                             const int N, const int K, const float alpha,
                             const float *A, const int lda, const float beta, float *C, const int ldc)
      {
                 cblas_ssyrk(Order, Uplo, Trans, N, K, alpha, A, lda, beta, C, ldc);
      }
    
    void wrapper_cblas_dsyrk(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE Trans,
                             const int N, const int K, const double alpha,
                             const double *A, const int lda, const double beta, double *C, const int ldc)
      {
                 cblas_dsyrk(Order, Uplo, Trans, N, K, alpha, A, lda, beta, C, ldc);
      }
    
    
    
    void wrapper_cblas_cherk(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE Trans,
                             const int N, const int K, const float alpha,
                             const void *A, const int lda, const float beta, void *C, const int ldc)
      {
                 cblas_cherk(Order, Uplo, Trans, N, K, alpha, A, lda, beta, C, ldc);
      }
    
    void wrapper_cblas_zherk(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE Trans,
                             const int N, const int K, const double alpha,
                             const void *A, const int lda, const double beta, void *C, const int ldc)
      {
                 cblas_zherk(Order, Uplo, Trans, N, K, alpha, A, lda, beta, C, ldc);
      }
    
    
    
    int wrapper_clapack_sgetrf(const enum CBLAS_ORDER Order, const int M, const int N, float  *A, const int lda, int *ipiv)
      {
      return    clapack_sgetrf(Order, M, N, A, lda, ipiv);
      }
    
    int wrapper_clapack_dgetrf(const enum CBLAS_ORDER Order, const int M, const int N, double *A, const int lda, int *ipiv)
      {
      return    clapack_dgetrf(Order, M, N, A, lda, ipiv);
      }
    
    int wrapper_clapack_cgetrf(const enum CBLAS_ORDER Order, const int M, const int N, void   *A, const int lda, int *ipiv)
      {
      return    clapack_cgetrf(Order, M, N, A, lda, ipiv);
      }
    
    int wrapper_clapack_zgetrf(const enum CBLAS_ORDER Order, const int M, const int N, void   *A, const int lda, int *ipiv)
      {
      return    clapack_zgetrf(Order, M, N, A, lda, ipiv);
      }
    
    
    
    int wrapper_clapack_sgetri(const enum CBLAS_ORDER Order, const int N, float  *A, const int lda, const int *ipiv)
      {
      return    clapack_sgetri(Order, N, A, lda, ipiv);
      }
    
    int wrapper_clapack_dgetri(const enum CBLAS_ORDER Order, const int N, double *A, const int lda, const int *ipiv)
      {
      return    clapack_dgetri(Order, N, A, lda, ipiv);
      }
    
    int wrapper_clapack_cgetri(const enum CBLAS_ORDER Order, const int N, void   *A, const int lda, const int *ipiv)
      {
      return    clapack_cgetri(Order, N, A, lda, ipiv);
      }
    
    int wrapper_clapack_zgetri(const enum CBLAS_ORDER Order, const int N, void   *A, const int lda, const int *ipiv)
      {
      return    clapack_zgetri(Order, N, A, lda, ipiv);
      }
    
    
    
    int wrapper_clapack_sgesv(const enum CBLAS_ORDER Order, const int N, const int NRHS, float  *A, const int lda, int *ipiv, float  *B, const int ldb)
      {
      return    clapack_sgesv(Order, N, NRHS, A, lda, ipiv, B, ldb);
      }
    
    int wrapper_clapack_dgesv(const enum CBLAS_ORDER Order, const int N, const int NRHS, double *A, const int lda, int *ipiv, double *B, const int ldb)
      {
      return    clapack_dgesv(Order, N, NRHS, A, lda, ipiv, B, ldb);
      }
    
    int wrapper_clapack_cgesv(const enum CBLAS_ORDER Order, const int N, const int NRHS, void   *A, const int lda, int *ipiv, void   *B, const int ldb)
      {
      return    clapack_cgesv(Order, N, NRHS, A, lda, ipiv, B, ldb);
      }
    
    int wrapper_clapack_zgesv(const enum CBLAS_ORDER Order, const int N, const int NRHS, void   *A, const int lda, int *ipiv, void   *B, const int ldb)
      {
      return    clapack_zgesv(Order, N, NRHS, A, lda, ipiv, B, ldb);
      }
    
    
    
    int wrapper_clapack_spotrf(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const int N,  float *A, const int lda)
      {
      return    clapack_spotrf(Order, Uplo, N, A, lda);
      }
    
    int wrapper_clapack_dpotrf(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const int N, double *A, const int lda)
      {
      return    clapack_dpotrf(Order, Uplo, N, A, lda);
      }
    
    int wrapper_clapack_cpotrf(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const int N,   void *A, const int lda)
      {
      return    clapack_cpotrf(Order, Uplo, N, A, lda);
      }
    
    int wrapper_clapack_zpotrf(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const int N,   void *A, const int lda)
      {
      return    clapack_zpotrf(Order, Uplo, N, A, lda);
      }
    
    
    
    int wrapper_clapack_spotri(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const int N,  float *A, const int lda)
      {
      return    clapack_spotri(Order, Uplo, N, A, lda);
      }
    
    int wrapper_clapack_dpotri(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const int N, double *A, const int lda)
      {
      return    clapack_dpotri(Order, Uplo, N, A, lda);
      }
    
    int wrapper_clapack_cpotri(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const int N,   void *A, const int lda)
      {
      return    clapack_cpotri(Order, Uplo, N, A, lda);
      }
    
    int wrapper_clapack_zpotri(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const int N,   void *A, const int lda)
      {
      return    clapack_zpotri(Order, Uplo, N, A, lda);
      }
    
    
    
    int wrapper_clapack_sposv(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const int N, const int NRHS,  float *A, const int lda,  float *B, const int ldb)
      {
      return    clapack_sposv(Order, Uplo, N, NRHS, A, lda, B, ldb);
      }
    
    int wrapper_clapack_dposv(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const int N, const int NRHS, double *A, const int lda, double *B, const int ldb)
      {
      return    clapack_dposv(Order, Uplo, N, NRHS, A, lda, B, ldb);
      }
    
    int wrapper_clapack_cposv(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const int N, const int NRHS,   void *A, const int lda,   void *B, const int ldb)
      {
      return    clapack_cposv(Order, Uplo, N, NRHS, A, lda, B, ldb);
      }
    
    int wrapper_clapack_zposv(const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo, const int N, const int NRHS,   void *A, const int lda,   void *B, const int ldb)
      {
      return    clapack_zposv(Order, Uplo, N, NRHS, A, lda, B, ldb);
      }
    
    
  #endif



  #if defined(ARMA_USE_ARPACK)

    void arma_fortran_prefix(arma_snaupd)(blas_int* ido, char* bmat, blas_int* n, char* which, blas_int* nev, float* tol, float* resid, blas_int* ncv, float* v, blas_int* ldv, blas_int* iparam, blas_int* ipntr, float* workd, float* workl, blas_int* lworkl, blas_int* info)
      {
      arma_fortran_noprefix(arma_snaupd)(ido, bmat, n, which, nev, tol, resid, ncv, v, ldv, iparam, ipntr, workd, workl, lworkl, info);
      }

    void arma_fortran_prefix(arma_dnaupd)(blas_int* ido, char* bmat, blas_int* n, char* which, blas_int* nev, double* tol, double* resid, blas_int* ncv, double* v, blas_int* ldv, blas_int* iparam, blas_int* ipntr, double* workd, double* workl, blas_int* lworkl, blas_int* info)
      {
      arma_fortran_noprefix(arma_dnaupd)(ido, bmat, n, which, nev, tol, resid, ncv, v, ldv, iparam, ipntr, workd, workl, lworkl, info);
      }

    void arma_fortran_prefix(arma_cnaupd)(blas_int* ido, char* bmat, blas_int* n, char* which, blas_int* nev, float* tol, void* resid, blas_int* ncv, void* v, blas_int* ldv, blas_int* iparam, blas_int* ipntr, void* workd, void* workl, blas_int* lworkl, float* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cnaupd)(ido, bmat, n, which, nev, tol, resid, ncv, v, ldv, iparam, ipntr, workd, workl, lworkl, rwork, info);
      }

    void arma_fortran_prefix(arma_znaupd)(blas_int* ido, char* bmat, blas_int* n, char* which, blas_int* nev, double* tol, void* resid, blas_int* ncv, void* v, blas_int* ldv, blas_int* iparam, blas_int* ipntr, void* workd, void* workl, blas_int* lworkl, double* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_znaupd)(ido, bmat, n, which, nev, tol, resid, ncv, v, ldv, iparam, ipntr, workd, workl, lworkl, rwork, info);
      }


    void arma_fortran_prefix(arma_sneupd)(blas_int* rvec, char* howmny, blas_int* select, float* dr, float* di, float* z, blas_int* ldz, float* sigmar, float* sigmai, float* workev, char* bmat, blas_int* n, char* which, blas_int* nev, float* tol, float* resid, blas_int* ncv, float* v, blas_int* ldv, blas_int* iparam, blas_int* ipntr, float* workd, float* workl, blas_int* lworkl, blas_int* info)
      {
      arma_fortran_noprefix(arma_sneupd)(rvec, howmny, select, dr, di, z, ldz, sigmar, sigmai, workev, bmat, n, which, nev, tol, resid, ncv, v, ldv, iparam, ipntr, workd, workl, lworkl, info);
      }

    void arma_fortran_prefix(arma_dneupd)(blas_int* rvec, char* howmny, blas_int* select, double* dr, double* di, double* z, blas_int* ldz, double* sigmar, double* sigmai, double* workev, char* bmat, blas_int* n, char* which, blas_int* nev, double* tol, double* resid, blas_int* ncv, double* v, blas_int* ldv, blas_int* iparam, blas_int* ipntr, double* workd, double* workl, blas_int* lworkl, blas_int* info)
      {
      arma_fortran_noprefix(arma_dneupd)(rvec, howmny, select, dr, di, z, ldz, sigmar, sigmai, workev, bmat, n, which, nev, tol, resid, ncv, v, ldv, iparam, ipntr, workd, workl, lworkl, info);
      }

    void arma_fortran_prefix(arma_cneupd)(blas_int* rvec, char* howmny, blas_int* select, void* d, void* z, blas_int* ldz, void* sigma, void* workev, char* bmat, blas_int* n, char* which, blas_int* nev, float* tol, void* resid, blas_int* ncv, void* v, blas_int* ldv, blas_int* iparam, blas_int* ipntr, void* workd, void* workl, blas_int* lworkl, float* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_cneupd)(rvec, howmny, select, d, z, ldz, sigma, workev, bmat, n, which, nev, tol, resid, ncv, v, ldv, iparam, ipntr, workd, workl, lworkl, rwork, info);
      }

    void arma_fortran_prefix(arma_zneupd)(blas_int* rvec, char* howmny, blas_int* select, void* d, void* z, blas_int* ldz, void* sigma, void* workev, char* bmat, blas_int* n, char* which, blas_int* nev, double* tol, void* resid, blas_int* ncv, void* v, blas_int* ldv, blas_int* iparam, blas_int* ipntr, void* workd, void* workl, blas_int* lworkl, double* rwork, blas_int* info)
      {
      arma_fortran_noprefix(arma_zneupd)(rvec, howmny, select, d, z, ldz, sigma, workev, bmat, n, which, nev, tol, resid, ncv, v, ldv, iparam, ipntr, workd, workl, lworkl, rwork, info);
      }


    void arma_fortran_prefix(arma_ssaupd)(blas_int* ido, char* bmat, blas_int* n, char* which, blas_int* nev, float* tol, float* resid, blas_int* ncv, float* v, blas_int* ldv, blas_int* iparam, blas_int* ipntr, float* workd, float* workl, blas_int* lworkl, blas_int* info)
      {
      arma_fortran_noprefix(arma_ssaupd)(ido, bmat, n, which, nev, tol, resid, ncv, v, ldv, iparam, ipntr, workd, workl, lworkl, info);
      }

    void arma_fortran_prefix(arma_dsaupd)(blas_int* ido, char* bmat, blas_int* n, char* which, blas_int* nev, double* tol, double* resid, blas_int* ncv, double* v, blas_int* ldv, blas_int* iparam, blas_int* ipntr, double* workd, double* workl, blas_int* lworkl, blas_int* info)
      {
      arma_fortran_noprefix(arma_dsaupd)(ido, bmat, n, which, nev, tol, resid, ncv, v, ldv, iparam, ipntr, workd, workl, lworkl, info);
      }


    void arma_fortran_prefix(arma_sseupd)(blas_int* rvec, char* howmny, blas_int* select, float* d, float* z, blas_int* ldz, float* sigma, char* bmat, blas_int* n, char* which, blas_int* nev, float* tol, float* resid, blas_int* ncv, float* v, blas_int* ldv, blas_int* iparam, blas_int* ipntr, float* workd, float* workl, blas_int* lworkl, blas_int* info)
      {
      arma_fortran_noprefix(arma_sseupd)(rvec, howmny, select, d, z, ldz, sigma, bmat, n, which, nev, tol, resid, ncv, v, ldv, iparam, ipntr, workd, workl, lworkl, info);
      }

    void arma_fortran_prefix(arma_dseupd)(blas_int* rvec, char* howmny, blas_int* select, double* d, double* z, blas_int* ldz, double* sigma, char* bmat, blas_int* n, char* which, blas_int* nev, double* tol, double* resid, blas_int* ncv, double* v, blas_int* ldv, blas_int* iparam, blas_int* ipntr, double* workd, double* workl, blas_int* lworkl, blas_int* info)
      {
      arma_fortran_noprefix(arma_dseupd)(rvec, howmny, select, d, z, ldz, sigma, bmat, n, which, nev, tol, resid, ncv, v, ldv, iparam, ipntr, workd, workl, lworkl, info);
      }

  #endif
  
  
  
  #if defined(ARMA_USE_SUPERLU)
    
    void wrapper_sgssv(superlu::superlu_options_t* a, superlu::SuperMatrix* b, int* c, int* d, superlu::SuperMatrix* e, superlu::SuperMatrix* f, superlu::SuperMatrix* g, superlu::SuperLUStat_t* h, int* i)
      {
      sgssv(a,b,c,d,e,f,g,h,i);
      }
    
    void wrapper_dgssv(superlu::superlu_options_t* a, superlu::SuperMatrix* b, int* c, int* d, superlu::SuperMatrix* e, superlu::SuperMatrix* f, superlu::SuperMatrix* g, superlu::SuperLUStat_t* h, int* i)
      {
      dgssv(a,b,c,d,e,f,g,h,i);
      }
    
    void wrapper_cgssv(superlu::superlu_options_t* a, superlu::SuperMatrix* b, int* c, int* d, superlu::SuperMatrix* e, superlu::SuperMatrix* f, superlu::SuperMatrix* g, superlu::SuperLUStat_t* h, int* i)
      {
      cgssv(a,b,c,d,e,f,g,h,i);
      }
    
    void wrapper_zgssv(superlu::superlu_options_t* a, superlu::SuperMatrix* b, int* c, int* d, superlu::SuperMatrix* e, superlu::SuperMatrix* f, superlu::SuperMatrix* g, superlu::SuperLUStat_t* h, int* i)
      {
      zgssv(a,b,c,d,e,f,g,h,i);
      }
    
    
    
    
    void wrapper_sgssvx(superlu::superlu_options_t* a, superlu::SuperMatrix* b, int* c, int* d, int* e, char* f,  float* g,  float* h, superlu::SuperMatrix* i, superlu::SuperMatrix* j, void* k, int l, superlu::SuperMatrix* m, superlu::SuperMatrix* n,  float* o,  float* p,  float* q,  float* r, superlu::GlobalLU_t* s, superlu::mem_usage_t* t, superlu::SuperLUStat_t* u, int* v)
      {
      sgssvx(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v);
      }
    
    void wrapper_dgssvx(superlu::superlu_options_t* a, superlu::SuperMatrix* b, int* c, int* d, int* e, char* f, double* g, double* h, superlu::SuperMatrix* i, superlu::SuperMatrix* j, void* k, int l, superlu::SuperMatrix* m, superlu::SuperMatrix* n, double* o, double* p, double* q, double* r, superlu::GlobalLU_t* s, superlu::mem_usage_t* t, superlu::SuperLUStat_t* u, int* v)
      {
      dgssvx(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v);
      }
    
    void wrapper_cgssvx(superlu::superlu_options_t* a, superlu::SuperMatrix* b, int* c, int* d, int* e, char* f,  float* g,  float* h, superlu::SuperMatrix* i, superlu::SuperMatrix* j, void* k, int l, superlu::SuperMatrix* m, superlu::SuperMatrix* n,  float* o,  float* p,  float* q,  float* r, superlu::GlobalLU_t* s, superlu::mem_usage_t* t, superlu::SuperLUStat_t* u, int* v)
      {
      cgssvx(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v);
      }
    
    void wrapper_zgssvx(superlu::superlu_options_t* a, superlu::SuperMatrix* b, int* c, int* d, int* e, char* f, double* g, double* h, superlu::SuperMatrix* i, superlu::SuperMatrix* j, void* k, int l, superlu::SuperMatrix* m, superlu::SuperMatrix* n, double* o, double* p, double* q, double* r, superlu::GlobalLU_t* s, superlu::mem_usage_t* t, superlu::SuperLUStat_t* u, int* v)
      {
      zgssvx(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v);
      }
    
    
    
    
    void wrapper_StatInit(superlu::SuperLUStat_t* a)
      {
      StatInit(a);
      }

    void wrapper_StatFree(superlu::SuperLUStat_t* a)
      {
      StatFree(a);
      }
      
    void wrapper_set_default_options(superlu::superlu_options_t* a)
      {
      set_default_options(a);
      }
    
    void wrapper_Destroy_SuperNode_Matrix(superlu::SuperMatrix* a)
      {
      Destroy_SuperNode_Matrix(a);
      }

    void wrapper_Destroy_CompCol_Matrix(superlu::SuperMatrix* a)
      {
      Destroy_CompCol_Matrix(a);
      }

    void wrapper_Destroy_SuperMatrix_Store(superlu::SuperMatrix* a)
      {
      Destroy_SuperMatrix_Store(a);
      }
    
    void* wrapper_superlu_malloc(size_t a)
      {
      return superlu_malloc(a);
      }
    
    void wrapper_superlu_free(void* a)
      {
      superlu_free(a);
      }
    
  #endif
  
  
  
  #if defined(ARMA_USE_HDF5_ALT)
  
    hid_t arma_H5Tcopy(hid_t dtype_id)
      {
      return H5Tcopy(dtype_id);
      }
    
    hid_t arma_H5Tcreate(H5T_class_t cl, size_t size)
      {
      return H5Tcreate(cl, size);
      }
    
    herr_t arma_H5Tinsert(hid_t dtype_id, const char* name, size_t offset, hid_t field_id)
      {
      return H5Tinsert(dtype_id, name, offset, field_id);
      }
    
    htri_t arma_H5Tequal(hid_t dtype_id1, hid_t dtype_id2)
      {
      return H5Tequal(dtype_id1, dtype_id2);
      }
    
    herr_t arma_H5Tclose(hid_t dtype_id)
      {
      return H5Tclose(dtype_id);
      }
    
    hid_t arma_H5Dopen(hid_t loc_id, const char* name, hid_t dapl_id)
      {
      return H5Dopen(loc_id, name, dapl_id);
      }
    
    hid_t arma_H5Dget_type(hid_t dataset_id)
      {
      return H5Dget_type(dataset_id);
      }
    
    hid_t arma_H5Dcreate(hid_t loc_id, const char* name, hid_t dtype_id, hid_t space_id, hid_t lcpl_id, hid_t dcpl_id, hid_t dapl_id)
      {
      return H5Dcreate(loc_id, name, dtype_id, space_id, lcpl_id, dcpl_id, dapl_id);
      }
    
    herr_t arma_H5Dwrite(hid_t dataset_id, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, const void* buf)
      {
      return H5Dwrite(dataset_id, mem_type_id, mem_space_id, file_space_id, xfer_plist_id, buf);
      }
    
    herr_t arma_H5Dclose(hid_t dataset_id)
      {
      return H5Dclose(dataset_id);
      }
    
    hid_t arma_H5Dget_space(hid_t dataset_id)
      {
      return H5Dget_space(dataset_id);
      }
    
    herr_t arma_H5Dread(hid_t dataset_id, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id, void* buf)
      {
      return H5Dread(dataset_id, mem_type_id, mem_space_id, file_space_id, xfer_plist_id, buf);
      }
    
    int arma_H5Sget_simple_extent_ndims(hid_t space_id)
      {
      return H5Sget_simple_extent_ndims(space_id);
      }
    
    int arma_H5Sget_simple_extent_dims(hid_t space_id, hsize_t* dims, hsize_t* maxdims)
      {
      return H5Sget_simple_extent_dims(space_id, dims, maxdims);
      }
    
    herr_t arma_H5Sclose(hid_t space_id)
      {
      return H5Sclose(space_id);
      }
    
    hid_t arma_H5Screate_simple(int rank, const hsize_t* current_dims, const hsize_t* maximum_dims)
      {
      return H5Screate_simple(rank, current_dims, maximum_dims);
      }
    
    herr_t arma_H5Ovisit(hid_t object_id, H5_index_t index_type, H5_iter_order_t order, H5O_iterate_t op, void* op_data)
      {
      return H5Ovisit(object_id, index_type, order, op, op_data);
      }
    
    herr_t arma_H5Eset_auto(hid_t estack_id, H5E_auto_t func, void* client_data)
      {
      return H5Eset_auto(estack_id, func, client_data);
      }
    
    herr_t arma_H5Eget_auto(hid_t estack_id, H5E_auto_t* func, void** client_data)
      {
      return H5Eget_auto(estack_id, func, client_data);
      }
    
    hid_t arma_H5Fopen(const char* name, unsigned flags, hid_t fapl_id)
      {
      return H5Fopen(name, flags, fapl_id);
      }
    
    hid_t arma_H5Fcreate(const char* name, unsigned flags, hid_t fcpl_id, hid_t fapl_id)
      {
      return H5Fcreate(name, flags, fcpl_id, fapl_id);
      }
    
    herr_t arma_H5Fclose(hid_t file_id)
      {
      return H5Fclose(file_id);
      }
    
    htri_t arma_H5Fis_hdf5(const char* name)
      {
      return H5Fis_hdf5(name);
      }
    
    hid_t arma_H5Gcreate(hid_t loc_id, const char* name, hid_t lcpl_id, hid_t gcpl_id, hid_t gapl_id)
      {
      return H5Gcreate(loc_id, name, lcpl_id, gcpl_id, gapl_id);
      }
    
    hid_t arma_H5Gopen(hid_t loc_id, const char* name, hid_t gapl_id)
      {
      return H5Gopen(loc_id, name, gapl_id);
      }
    
    herr_t arma_H5Gclose(hid_t group_id)
      {
      return H5Gclose(group_id);
      }
    
    htri_t arma_H5Lexists(hid_t loc_id, const char* name, hid_t lapl_id)
      {
      return H5Lexists(loc_id, name, lapl_id);
      }
    
    herr_t arma_H5Ldelete(hid_t loc_id, const char* name, hid_t lapl_id)
      {
      return H5Ldelete(loc_id, name, lapl_id);
      }
    
    
    // H5T_NATIVE_* types.  The rhs here expands to some macros.
    hid_t arma_H5T_NATIVE_UCHAR  = H5T_NATIVE_UCHAR;
    hid_t arma_H5T_NATIVE_CHAR   = H5T_NATIVE_CHAR;
    hid_t arma_H5T_NATIVE_SHORT  = H5T_NATIVE_SHORT;
    hid_t arma_H5T_NATIVE_USHORT = H5T_NATIVE_USHORT;
    hid_t arma_H5T_NATIVE_INT    = H5T_NATIVE_INT;
    hid_t arma_H5T_NATIVE_UINT   = H5T_NATIVE_UINT;
    hid_t arma_H5T_NATIVE_LONG   = H5T_NATIVE_LONG;
    hid_t arma_H5T_NATIVE_ULONG  = H5T_NATIVE_ULONG;
    hid_t arma_H5T_NATIVE_LLONG  = H5T_NATIVE_LLONG;
    hid_t arma_H5T_NATIVE_ULLONG = H5T_NATIVE_ULLONG;
    hid_t arma_H5T_NATIVE_FLOAT  = H5T_NATIVE_FLOAT;
    hid_t arma_H5T_NATIVE_DOUBLE = H5T_NATIVE_DOUBLE;

  #endif
  
  
  }  // end of extern "C"


}  // end of namespace arma
