/**************************************************************************************************
*
* \file ExpressionTemplates.cpp
* \brief C++ Training - Expression Template Programming Task
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
* Task: Use Expression Templates to implement the addition operation. Benchmark the addition
*       of two small (in-cache) and two large (out-of-cache) vectors.
*
**************************************************************************************************/

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <stdexcept>

using std::size_t;


//=================================================================================================
// Inlining
//=================================================================================================

#if USE_STRONG_INLINE && ( defined(_MSC_VER) || defined(__INTEL_COMPILER) )
#  define STRONG_INLINE __forceinline
#else
#  define STRONG_INLINE inline
#endif

#if defined(__GNUC__)
#  define ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#  define ALWAYS_INLINE STRONG_INLINE
#endif


//=================================================================================================
// allocate() / deallocate()
//=================================================================================================

template< typename Type >
Type* allocate( size_t size )
{
   static_assert( std::is_fundamental<Type>::value, "Invalid data type detected" );

   const size_t alignment( 32U );  // Proper alignment for AVX

   void* raw( nullptr );

#if defined(_MSC_VER)
   raw = _aligned_malloc( size*sizeof(Type), alignment );
   if( raw == nullptr ) {
#else
   if( posix_memalign( &raw, alignment, size*sizeof(Type) ) ) {
#endif
      throw std::bad_alloc();
   }

   return reinterpret_cast<Type*>( raw );
}


template< typename Type >
void deallocate( Type* address )
{
   static_assert( std::is_fundamental<Type>::value, "Invalid data type detected" );

   if( address == nullptr )
      return;

#if defined(_MSC_VER)
   _aligned_free( address );
#else
   free( address );
#endif
}


//=================================================================================================
// class DenseVector
//=================================================================================================

template< typename VT >
class DenseVector
{
 public:
   VT&       operator~()       { return static_cast<VT&>( *this ); }
   const VT& operator~() const { return static_cast<const VT&>( *this ); }
};


//=================================================================================================
// class DynamicVector
//=================================================================================================

template< typename Type >
class DynamicVector
   : public DenseVector< DynamicVector<Type> >
{
 public:
   using value_type     = Type;
   using iterator       = Type*;
   using const_iterator = const Type*;

   explicit DynamicVector() = default;

   explicit DynamicVector( size_t n, Type value = Type{} )
      : size_    ( n )
      , capacity_( n )
      , v_       ( allocate<Type>( n ) )
   {
      std::fill( begin(), end(), value );
   }

   DynamicVector( const DynamicVector& rhs )
      : size_    ( rhs.size_ )
      , capacity_( rhs.size_ )
      , v_       ( allocate<Type>( rhs.size_ ) )
   {
      assign( ~rhs );
   }

   template< typename Other >
   DynamicVector( const DynamicVector<Other>& rhs )
      : size_    ( rhs.size() )
      , capacity_( rhs.capacity() )
      , v_       ( allocate<Type>( capacity_ ) )
   {
      assign( ~rhs );
   }

   DynamicVector( DynamicVector&& rhs )
      : size_    ( rhs.size_ )
      , capacity_( rhs.capacity_ )
      , v_       ( rhs.v_ )
   {
      rhs.v_ = nullptr;
   }

   template< typename VT >
   DynamicVector( const DenseVector<VT>& rhs )
      : size_    ( (~rhs).size() )
      , capacity_( (~rhs).capacity() )
      , v_       ( allocate<Type>( capacity_ ) )
   {
      assign( ~rhs );
   }

   ~DynamicVector()
   {
      deallocate( v_ );
   }

   size_t size() const
   {
      return size_;
   }

   size_t capacity() const
   {
      return capacity_;
   }

   Type* data()
   {
      return v_;
   }

   const Type* data() const
   {
      return v_;
   }

   Type& operator[]( size_t index )
   {
      assert( index < size_ );
      return v_[index];
   }

   const Type& operator[]( size_t index ) const
   {
      assert( index < size_ );
      return v_[index];
   }

   Type& at( size_t index )
   {
      if( index >= size_ ) {
         throw std::invalid_argument( "Out-of-bounds access detected" );
      }
      return (*this)[index];
   }

   const Type& at( size_t index ) const
   {
      if( index >= size_ ) {
         throw std::invalid_argument( "Out-of-bounds access detected" );
      }
      return (*this)[index];
   }

   iterator       begin()        { return v_; }
   const_iterator begin()  const { return v_; }
   const_iterator cbegin() const { return v_; }
   iterator       end()          { return v_ + size_; }
   const_iterator end()    const { return v_ + size_; }
   const_iterator cend()   const { return v_ + size_; }

   DynamicVector& operator=( const DynamicVector& rhs )
   {
      resize( rhs.size() );
      assign( ~rhs );
      return *this;
   }

   template< typename Other >
   DynamicVector& operator=( const DynamicVector<Other>& rhs )
   {
      resize( rhs.size() );
      assign( ~rhs );
      return *this;
   }

   DynamicVector& operator=( DynamicVector&& rhs )
   {
      deallocate( v_ );

      size_     = rhs.size_;
      capacity_ = rhs.capacity_;
      v_        = rhs.v_;

      rhs.v_ = nullptr;

      return *this;
   }

   template< typename VT >
   DynamicVector& operator=( const DenseVector<VT>& v )
   {
      if( (~v).size() != size_ )
         throw std::invalid_argument( "Vector size does not match" );

      assign( ~v );

      return *this;
   }

   void resize( size_t n )
   {
      using std::swap;

      if( n > capacity_ )
      {
         // Allocating a new array
         Type* tmp = allocate<Type>( n );

         // Initializing the new array
         std::copy( begin(), end(), tmp );
         std::fill( tmp+size_, tmp+n, Type{} );

         // Replacing the old array
         swap( v_, tmp );
         deallocate( tmp );
         capacity_ = n;
      }

      size_ = n;
   }

 private:
   template< typename VT >
   void assign( const VT& v )
   {
      for( size_t i=0U; i<size_; ++i ) {
         v_[i] = v[i];
      }
   }

   size_t size_    { 0UL };
   size_t capacity_{ 0UL };
   Type* v_        { nullptr };

   static_assert( std::is_fundamental<Type>::value, "Invalid data type detected" );
};


//=================================================================================================
// l2Norm()
//=================================================================================================

template< typename VT >
auto l2Norm( const DenseVector<VT>& vec ) -> decltype( std::sqrt( std::declval< typename VT::value_type >() ) )
{
   using value_type = typename VT::value_type;

   return std::sqrt( std::inner_product( (~vec).begin(), (~vec).end(), (~vec).begin(), value_type{} ) );
}


//=================================================================================================
// struct VecVecAddExpr
//=================================================================================================

template< typename VT1, typename VT2 >
struct VecVecAddExpr : public DenseVector< VecVecAddExpr<VT1,VT2> >
{
 public:
   using ElementType = decltype( std::declval<VT1>()[0U] + std::declval<VT2>()[0U] );

   explicit VecVecAddExpr( const VT1& lhs, const VT2& rhs )
      : lhs_( lhs )
      , rhs_( rhs )
   {
      assert( lhs_.size() == rhs_.size() );
   }

   size_t size() const noexcept { return lhs_.size(); }

   ElementType operator[]( size_t index ) const
   {
      assert( index < size() );
      return lhs_[index] + rhs_[index];
   }

 private:
   const VT1& lhs_;
   const VT2& rhs_;
};


//=================================================================================================
// operator+()
//=================================================================================================

template< typename T1, typename T2 >
VecVecAddExpr<T1,T2> operator+( const DenseVector<T1>& lhs, const DenseVector<T2>& rhs )
{
   if( (~lhs).size() != (~rhs).size() )
      throw std::invalid_argument( "Vector size does not match" );

   return VecVecAddExpr<T1,T2>( ~lhs, ~rhs );
}


//=================================================================================================
// add()
//=================================================================================================

template< typename T >
void add( DynamicVector<T>& dst, const DynamicVector<T>& lhs, const DynamicVector<T>& rhs )
{
   if( dst.size() != lhs.size() || lhs.size() != rhs.size() )
      throw std::invalid_argument( "Vector sizes don't match" );

   for( size_t i=0U; i<dst.size(); ++i ) {
      dst[i] = lhs[i] + rhs[i];
   }
}


//=================================================================================================
// operator<<()
//=================================================================================================

template< typename T >
std::ostream& operator<<( std::ostream& os, const DynamicVector<T>& v )
{
   os << "(";
   for( const T& e : v ) {
      os << " " << e;
   }
   return os << " )";
}


//=================================================================================================
// main()
//=================================================================================================

int main()
{
   const size_t N( 1000U );
   const size_t repetitions( 3U );
   const size_t steps( 1000000U );

   DynamicVector<double> a( N, 2.0 );
   DynamicVector<double> b( N, 3.0 );
   DynamicVector<double> c( N, 0.0 );

   c = a + b;

   for( size_t rep=0U; rep<repetitions; ++rep )
   {
      std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
      start = std::chrono::high_resolution_clock::now();

      for( size_t step=0U; step<steps; ++step )
      {
         c = a + b;
      }

      end = std::chrono::high_resolution_clock::now();
      const std::chrono::duration<double> elapsedTime = end - start;

      if( c[0U] != 5.0 ) { std::cerr << "\n ERROR DETECTED!\n\n"; }

      const double seconds( elapsedTime.count() );
      const double mflops ( ( N * steps ) / ( 1E6 * seconds ) );

      std::cerr << " Run " << (rep+1U) << ": " << seconds << "s (" << mflops << " MFlops)\n";
   }
}

