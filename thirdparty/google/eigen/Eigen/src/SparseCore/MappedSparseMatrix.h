// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2014 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_MAPPED_SPARSEMATRIX_H
#define EIGEN_MAPPED_SPARSEMATRIX_H

namespace Eigen {

/** \deprecated Use Map<SparseMatrix<> >
  * \class MappedSparseMatrix
  *
  * \brief Sparse matrix
  *
  * \param Scalar_ the scalar type, i.e. the type of the coefficients
  *
  * See http://www.netlib.org/linalg/html_templates/node91.html for details on the storage scheme.
  *
  */
namespace internal {
template<typename Scalar_, int _Flags, typename StorageIndex_>
struct traits<MappedSparseMatrix<Scalar_, _Flags, StorageIndex_> > : traits<SparseMatrix<Scalar_, _Flags, StorageIndex_> >
{};
} // end namespace internal

template<typename Scalar_, int _Flags, typename StorageIndex_>
class MappedSparseMatrix
  : public Map<SparseMatrix<Scalar_, _Flags, StorageIndex_> >
{
    typedef Map<SparseMatrix<Scalar_, _Flags, StorageIndex_> > Base;

  public:
    
    typedef typename Base::StorageIndex StorageIndex;
    typedef typename Base::Scalar Scalar;

    inline MappedSparseMatrix(Index rows, Index cols, Index nnz, StorageIndex* outerIndexPtr, StorageIndex* innerIndexPtr, Scalar* valuePtr, StorageIndex* innerNonZeroPtr = 0)
      : Base(rows, cols, nnz, outerIndexPtr, innerIndexPtr, valuePtr, innerNonZeroPtr)
    {}

    /** Empty destructor */
    inline ~MappedSparseMatrix() {}
};

namespace internal {

template<typename Scalar_, int Options_, typename StorageIndex_>
struct evaluator<MappedSparseMatrix<Scalar_,Options_,StorageIndex_> >
  : evaluator<SparseCompressedBase<MappedSparseMatrix<Scalar_,Options_,StorageIndex_> > >
{
  typedef MappedSparseMatrix<Scalar_,Options_,StorageIndex_> XprType;
  typedef evaluator<SparseCompressedBase<XprType> > Base;
  
  evaluator() : Base() {}
  explicit evaluator(const XprType &mat) : Base(mat) {}
};

}

} // end namespace Eigen

#endif // EIGEN_MAPPED_SPARSEMATRIX_H
