// ======================================================================== //
// Copyright 2009-2012 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#ifndef __EMBREE_AVXB_H__
#define __EMBREE_AVXB_H__

namespace embree
{
  /*! 8-wide AVX bool type. */
  struct avxb
  {
    typedef avxb Mask;         // mask type for us
    enum   { size = 8 };       // number of SIMD elements
    union  {                   // data
      __m256 m256; 
      struct { __m128 l,h; }; 
      int32 v[8]; 
    };  

    ////////////////////////////////////////////////////////////////////////////////
    /// Constructors, Assignment & Cast Operators
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline avxb           () {}
    __forceinline avxb           ( const avxb& a ) { m256 = a.m256; }
    __forceinline avxb& operator=( const avxb& a ) { m256 = a.m256; return *this; }

    __forceinline avxb( const __m256 a ) : m256(a) {}
    __forceinline operator const __m256&( void ) const { return m256; }
    __forceinline operator const __m256i( void ) const { return _mm256_castps_si256(m256); }
    __forceinline operator const __m256d( void ) const { return _mm256_castps_pd(m256); }
    
    __forceinline avxb ( const  sseb& a                ) : m256(_mm256_insertf128_ps(_mm256_castps128_ps256(a),a,1)) {}
    __forceinline avxb ( const  sseb& a, const  sseb& b) : m256(_mm256_insertf128_ps(_mm256_castps128_ps256(a),b,1)) {}
    __forceinline avxb ( const __m128 a, const __m128 b) : l(a), h(b) {}

    __forceinline avxb ( bool a ) : m256(avxb(sseb(a), sseb(a))) {}
    __forceinline avxb ( bool a, bool b) : m256(avxb(sseb(a), sseb(b))) {}
    __forceinline avxb ( bool a, bool b, bool c, bool d) : m256(avxb(sseb(a,b), sseb(c,d))) {}
    __forceinline avxb ( bool a, bool b, bool c, bool d, bool e, bool f, bool g, bool h ) : m256(avxb(sseb(a,b,c,d), sseb(e,f,g,h))) {}

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline avxb( FalseTy ) : m256(_mm256_setzero_ps()) {}
    __forceinline avxb( TrueTy  ) : m256(_mm256_cmp_ps(_mm256_setzero_ps(), _mm256_setzero_ps(), _CMP_EQ_OQ)) {}

    ////////////////////////////////////////////////////////////////////////////////
    /// Array Access
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline bool   operator []( const size_t i ) const { assert(i < 8); return (_mm256_movemask_ps(m256) >> i) & 1; }
    __forceinline int32& operator []( const size_t i )       { assert(i < 8); return v[i]; }
  };

  ////////////////////////////////////////////////////////////////////////////////
  /// Unary Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const avxb operator !( const avxb& a ) { return _mm256_xor_ps(a, avxb(True)); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Binary Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const avxb operator &( const avxb& a, const avxb& b ) { return _mm256_and_ps(a, b); }
  __forceinline const avxb operator |( const avxb& a, const avxb& b ) { return _mm256_or_ps (a, b); }
  __forceinline const avxb operator ^( const avxb& a, const avxb& b ) { return _mm256_xor_ps(a, b); }

  __forceinline avxb operator &=( avxb& a, const avxb& b ) { return a = a & b; }
  __forceinline avxb operator |=( avxb& a, const avxb& b ) { return a = a | b; }
  __forceinline avxb operator ^=( avxb& a, const avxb& b ) { return a = a ^ b; }

  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators + Select
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const avxb operator !=( const avxb& a, const avxb& b ) { return _mm256_xor_ps(a, b); }
  __forceinline const avxb operator ==( const avxb& a, const avxb& b ) { return _mm256_xor_ps(_mm256_xor_ps(a,b),avxb(True)); }

  __forceinline const avxb select( const avxb& mask, const avxb& t, const avxb& f ) { 
    return _mm256_blendv_ps(f, t, mask); 
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// Movement/Shifting/Shuffling Functions
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline avxb unpacklo( const avxb& a, const avxb& b ) { return _mm256_unpacklo_ps(a.m256, b.m256); }
  __forceinline avxb unpackhi( const avxb& a, const avxb& b ) { return _mm256_unpackhi_ps(a.m256, b.m256); }

  template<size_t i> __forceinline const avxb shuffle( const avxb& a ) {
    return _mm256_permute_ps(a, _MM_SHUFFLE(i, i, i, i));
  }

  template<size_t i0, size_t i1> __forceinline const avxb shuffle( const avxb& a ) {
    return _mm256_permute2f128_ps(a, a, (i1 << 4) | (i0 << 0));
  }

  template<size_t i0, size_t i1> __forceinline const avxb shuffle( const avxb& a,  const avxb& b) {
    return _mm256_permute2f128_ps(a, b, (i1 << 4) | (i0 << 0));
  }

  template<size_t i0, size_t i1, size_t i2, size_t i3> __forceinline const avxb shuffle( const avxb& a ) {
    return _mm256_permute_ps(a, _MM_SHUFFLE(i3, i2, i1, i0));
  }

  template<size_t i0, size_t i1, size_t i2, size_t i3> __forceinline const avxb shuffle( const avxb& a, const avxb& b ) {
    return _mm256_shuffle_ps(a, b, _MM_SHUFFLE(i3, i2, i1, i0));
  }

  template<> __forceinline const avxb shuffle<0, 0, 2, 2>( const avxb& b ) { return _mm256_moveldup_ps(b); }
  template<> __forceinline const avxb shuffle<1, 1, 3, 3>( const avxb& b ) { return _mm256_movehdup_ps(b); }
  template<> __forceinline const avxb shuffle<0, 1, 0, 1>( const avxb& b ) { return _mm256_castpd_ps(_mm256_movedup_pd(_mm256_castps_pd(b))); }

  template<size_t i> __forceinline const avxb insert (const avxb& a, const sseb& b) { return _mm256_insertf128_ps (a,b,i); }
  template<size_t i> __forceinline const sseb extract(const avxb& a               ) { return _mm256_extractf128_ps(a  ,i); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Reduction Operations
  ////////////////////////////////////////////////////////////////////////////////

#if defined(__USE_POP_COUNT__)
  __forceinline size_t popcnt( const avxb& a ) { return __popcnt(_mm256_movemask_ps(a)); }
#else
  __forceinline size_t popcnt( const avxb& a ) { return bool(a[0])+bool(a[1])+bool(a[2])+bool(a[3])+bool(a[4])+bool(a[5])+bool(a[6])+bool(a[7]); }
#endif

  __forceinline bool reduce_and( const avxb& a ) { return _mm256_movemask_ps(a) == 0xff; }
  __forceinline bool reduce_or ( const avxb& a ) { return !_mm256_testz_ps(a,a); }
  __forceinline bool all       ( const avxb& a ) { return _mm256_movemask_ps(a) == 0xff; }
  __forceinline bool none      ( const avxb& a ) { return _mm256_testz_ps(a,a) != 0; }
  __forceinline bool any       ( const avxb& a ) { return !_mm256_testz_ps(a,a); }

  __forceinline size_t movemask( const avxb& a ) { return _mm256_movemask_ps(a); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Output Operators
  ////////////////////////////////////////////////////////////////////////////////

  inline std::ostream& operator<<(std::ostream& cout, const avxb& a) {
    return cout << "<" << a[0] << ", " << a[1] << ", " << a[2] << ", " << a[3] << ", "
                       << a[4] << ", " << a[5] << ", " << a[6] << ", " << a[7] << ">";
  }
}

#endif
