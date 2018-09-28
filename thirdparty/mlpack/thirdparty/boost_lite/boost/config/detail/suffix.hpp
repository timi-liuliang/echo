//  Boost config.hpp configuration header file  ------------------------------//
//  boostinspect:ndprecated_macros -- tell the inspect tool to ignore this file

//  Copyright (c) 2001-2003 John Maddock
//  Copyright (c) 2001 Darin Adler
//  Copyright (c) 2001 Peter Dimov
//  Copyright (c) 2002 Bill Kempf
//  Copyright (c) 2002 Jens Maurer
//  Copyright (c) 2002-2003 David Abrahams
//  Copyright (c) 2003 Gennaro Prota
//  Copyright (c) 2003 Eric Friedman
//  Copyright (c) 2010 Eric Jourdanneau, Joel Falcou
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/ for most recent version.

//  Boost config.hpp policy and rationale documentation has been moved to
//  http://www.boost.org/libs/config/
//
//  This file is intended to be stable, and relatively unchanging.
//  It should contain boilerplate code only - no compiler specific
//  code unless it is unavoidable - no changes unless unavoidable.

#ifndef BOOST_CONFIG_SUFFIX_HPP
#define BOOST_CONFIG_SUFFIX_HPP

//
// Helper macros BOOST_NOEXCEPT, BOOST_NOEXCEPT_IF, BOOST_NOEXCEPT_EXPR
// These aid the transition to C++11 while still supporting C++03 compilers
//
#ifdef BOOST_NO_CXX11_NOEXCEPT
#  define BOOST_NOEXCEPT
#  define BOOST_NOEXCEPT_OR_NOTHROW throw()
#  define BOOST_NOEXCEPT_IF(Predicate)
#  define BOOST_NOEXCEPT_EXPR(Expression) false
#else
#  define BOOST_NOEXCEPT noexcept
#  define BOOST_NOEXCEPT_OR_NOTHROW noexcept
#  define BOOST_NOEXCEPT_IF(Predicate) noexcept((Predicate))
#  define BOOST_NOEXCEPT_EXPR(Expression) noexcept((Expression))
#endif
//
// Helper macro BOOST_FALLTHROUGH
// Fallback definition of BOOST_FALLTHROUGH macro used to mark intended
// fall-through between case labels in a switch statement. We use a definition
// that requires a semicolon after it to avoid at least one type of misuse even
// on unsupported compilers.
//
#ifndef BOOST_FALLTHROUGH
#  define BOOST_FALLTHROUGH ((void)0)
#endif

//
// constexpr workarounds
//
#if defined(BOOST_NO_CXX11_CONSTEXPR)
#define BOOST_CONSTEXPR
#define BOOST_CONSTEXPR_OR_CONST const
#else
#define BOOST_CONSTEXPR constexpr
#define BOOST_CONSTEXPR_OR_CONST constexpr
#endif
#if defined(BOOST_NO_CXX14_CONSTEXPR)
#define BOOST_CXX14_CONSTEXPR
#else
#define BOOST_CXX14_CONSTEXPR constexpr
#endif

#endif