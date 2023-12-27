/**************************************************************************************************
*
* \file Singleton.cpp
* \brief C++ Training - Example for the Singleton Design Pattern
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
* Task: Evaluate the following Singleton implementation with respect to life-time guarantees.
*
**************************************************************************************************/

#include <cstdlib>
#include <memory>
#include <type_traits>


//--InvalidType.h----------------------------------------------------------------------------------

struct INVALID_TYPE;


//--TypeList.h-------------------------------------------------------------------------------------

template< typename... Ts >
struct TypeList
{};


template< typename TL >
struct Length;

template< typename... Ts >
struct Length< TypeList<Ts...> >
{
   static constexpr size_t value = sizeof...( Ts );
};


template< typename TL, size_t Index >
struct TypeAt;

template< typename T, typename... Ts >
struct TypeAt< TypeList<T,Ts...>, 0UL >
{
   using Type = T;
};

template< size_t Index >  // Type list access index
struct TypeAt< TypeList<>, Index >
{
   using Type = INVALID_TYPE;
};

template< typename T, typename... Ts, size_t Index >
struct TypeAt< TypeList<T,Ts...>, Index >
{
   using Type = typename TypeAt< TypeList<Ts...>, Index-1UL >::Type;
};

template< typename TL, size_t Index >
using TypeAt_t = typename TypeAt<TL,Index>::Type;


template< typename TL
        , typename T >
struct Contains;

template< typename T >
struct Contains< TypeList<>, T >
   : public std::false_type
{};

template< typename T
        , typename... Ts >
struct Contains< TypeList<T,Ts...>, T >
   : public std::true_type
{};

template< typename U, typename... Ts, typename T >
struct Contains< TypeList<U,Ts...>, T >
   : public Contains< TypeList<Ts...>, T >
{};


template< typename TL, typename T >
struct Append;

template< typename... Ts, typename T >
struct Append< TypeList<Ts...>, T >
{
   using Type = TypeList<Ts...,T>;
};

template< typename... Ts1, typename... Ts2 >
struct Append< TypeList<Ts1...>, TypeList<Ts2...> >
{
   using Type = TypeList<Ts1...,Ts2...>;
};

template< typename TL, typename T >
using Append_t = typename Append<TL,T>::Type;


//--NullType.h-------------------------------------------------------------------------------------

using NullType = TypeList<>;


//--Dependency.h-----------------------------------------------------------------------------------

template< typename T >
class Dependency
{
 public:
   inline Dependency()
      : dependency_( T::instance() )
   {
      static_assert( std::is_base_of<typename T::SingletonType,T>::value, "Missing type relationship detected" );
   }

 private:
   std::shared_ptr<T> dependency_;
};


//--Singleton.h------------------------------------------------------------------------------------

template< typename > class Dependency;
template< typename T, typename TL, bool C > struct HasCyclicDependency;


template< typename TL, typename D, size_t N = Length<D>::value >
struct HasCyclicDependencyHelper;

template< typename TL, size_t N >
struct HasCyclicDependencyHelper<TL,NullType,N>
{
   static constexpr bool value = false;
};

template< typename TL, typename D >
struct HasCyclicDependencyHelper<TL,D,1>
{
   using D1 = TypeAt_t<D,0>;

   static constexpr bool value =
      HasCyclicDependency<D1,TL,Contains<TL,D1>::value>::value;
};

template< typename TL, typename D >
struct HasCyclicDependencyHelper<TL,D,2>
{
   using D1 = TypeAt_t<D,0>;
   using D2 = TypeAt_t<D,1>;

   static constexpr bool value =
      HasCyclicDependency<D1,TL,Contains<TL,D1>::value>::value ||
      HasCyclicDependency<D2,TL,Contains<TL,D2>::value>::value;
};

template< typename TL, typename D >
struct HasCyclicDependencyHelper<TL,D,3>
{
   using D1 = TypeAt_t<D,0>;
   using D2 = TypeAt_t<D,1>;
   using D3 = TypeAt_t<D,2>;

   static constexpr bool value =
      HasCyclicDependency<D1,TL,Contains<TL,D1>::value>::value ||
      HasCyclicDependency<D2,TL,Contains<TL,D2>::value>::value ||
      HasCyclicDependency<D3,TL,Contains<TL,D3>::value>::value;
};


template< typename T                        // The type to be checked for cyclic lifetime dependencies
        , typename TL                       // Type list of checked lifetime dependencies
        , bool C = Contains<TL,T>::value >  // Flag to indicate whether T is contained in TL
struct HasCyclicDependency
{
   static constexpr bool value = HasCyclicDependencyHelper<Append_t<TL,T>,typename T::Dependencies>::value;
};

template< typename T, typename TL >
struct HasCyclicDependency<T,TL,true>
{
   static constexpr bool value = true;
};


#define DETECT_CYCLIC_LIFETIME_DEPENDENCY(T) \
   static_assert( ( !HasCyclicDependency<T,NullType>::value ), "Cyclic dependency detected" )

#define CHECK_INHERITANCE_RELATIONSHIP(B,D) \
   static_assert( std::is_base_of<B,D>::value, "Invalid inheritance relationship detected" )


#define BEFRIEND_SINGLETON \
   template< typename, typename, typename, typename > friend class ::Singleton; \
   template< typename, typename, bool > friend struct ::HasCyclicDependency; \
   template< typename > friend class ::Dependency


template< typename T                // Type of the singleton (CRTP pattern)
        , typename D1 = NullType    // Type of the first lifetime dependency
        , typename D2 = NullType    // Type of the second lifetime dependency
        , typename D3 = NullType >  // Type of the third lifetime dependency
class Singleton
{
 public:
   using SingletonType = Singleton<T,D1,D2,D3>;
   using Dependencies = TypeList< D1, D2, D3 >;

 protected:
   explicit Singleton()
      : dependency1_( D1::instance() )
      , dependency2_( D2::instance() )
      , dependency3_( D3::instance() )
   {
      CHECK_INHERITANCE_RELATIONSHIP( SingletonType, T );
      CHECK_INHERITANCE_RELATIONSHIP( typename D1::SingletonType, D1 );
      CHECK_INHERITANCE_RELATIONSHIP( typename D2::SingletonType, D2 );
      CHECK_INHERITANCE_RELATIONSHIP( typename D3::SingletonType, D3 );
      DETECT_CYCLIC_LIFETIME_DEPENDENCY( D1 );
      DETECT_CYCLIC_LIFETIME_DEPENDENCY( D2 );
      DETECT_CYCLIC_LIFETIME_DEPENDENCY( D3 );
   }

   Singleton( const Singleton& ) = default;
   Singleton& operator=( const Singleton& ) = default;
   ~Singleton() = default;

 public:
   static std::shared_ptr<T> instance()
   {
      static std::shared_ptr<T> object( new T() );
      return object;
   }

 private:
   std::shared_ptr<D1> dependency1_;
   std::shared_ptr<D2> dependency2_;
   std::shared_ptr<D3> dependency3_;
};


template< typename T     // Type of the singleton (CRTP pattern)
        , typename D1    // Type of the first lifetime dependency
        , typename D2 >  // Type of the second lifetime dependency
class Singleton<T,D1,D2,NullType>
{
 public:
   using SingletonType = Singleton<T,D1,D2,NullType>;
   using Dependencies = TypeList< D1, D2 >;

 protected:
   explicit Singleton()
      : dependency1_( D1::instance() )
      , dependency2_( D2::instance() )
   {
      CHECK_INHERITANCE_RELATIONSHIP( SingletonType, T );
      CHECK_INHERITANCE_RELATIONSHIP( typename D1::SingletonType, D1 );
      CHECK_INHERITANCE_RELATIONSHIP( typename D2::SingletonType, D2 );
      DETECT_CYCLIC_LIFETIME_DEPENDENCY( D1 );
      DETECT_CYCLIC_LIFETIME_DEPENDENCY( D2 );
   }

   Singleton( const Singleton& ) = default;
   Singleton& operator=( const Singleton& ) = default;
   ~Singleton() = default;

 public:
   static std::shared_ptr<T> instance()
   {
      static std::shared_ptr<T> object( new T() );
      return object;
   }

 private:
   std::shared_ptr<D1> dependency1_;
   std::shared_ptr<D2> dependency2_;
};




template< typename T     // Type of the singleton (CRTP pattern)
        , typename D1 >  // Type of the first lifetime dependency
class Singleton<T,D1,NullType,NullType>
{
 public:
   using SingletonType = Singleton<T,D1,NullType,NullType>;
   using Dependencies = TypeList< D1 >;

 protected:
   explicit Singleton()
      : dependency1_( D1::instance() )
   {
      CHECK_INHERITANCE_RELATIONSHIP( SingletonType, T );
      CHECK_INHERITANCE_RELATIONSHIP( typename D1::SingletonType, D1 );
      DETECT_CYCLIC_LIFETIME_DEPENDENCY( D1 );
   }

   Singleton( const Singleton& ) = default;
   Singleton& operator=( const Singleton& ) = default;
   ~Singleton() = default;

 public:
   static std::shared_ptr<T> instance()
   {
      static std::shared_ptr<T> object( new T() );
      return object;
   }

 private:
   std::shared_ptr<D1> dependency1_;
};


template< typename T >  // Type of the singleton (CRTP pattern)
class Singleton<T,NullType,NullType,NullType>
{
 public:
   using SingletonType = Singleton<T,NullType,NullType,NullType>;
   using Dependencies = NullType;

 protected:
   explicit Singleton()
   {
      CHECK_INHERITANCE_RELATIONSHIP( SingletonType, T );
   }

   Singleton( const Singleton& ) = default;
   Singleton& operator=( const Singleton& ) = default;
   ~Singleton() = default;

 public:
   static std::shared_ptr<T> instance()
   {
      static std::shared_ptr<T> object( new T() );
      return object;
   }
};


//--Logger.cpp-------------------------------------------------------------------------------------

class Logger : private Singleton<Logger>
{
 public:
   using Singleton<Logger>::instance;

 private:
   Logger() = default;

   BEFRIEND_SINGLETON;
};


//--MySingleton.cpp-------------------------------------------------------------------------------------

class MySingleton : private Singleton<MySingleton,Logger>
{
 public:
   using Singleton<MySingleton,Logger>::instance;

 private:
   MySingleton() = default;

   BEFRIEND_SINGLETON;
};




//--Main.cpp---------------------------------------------------------------------------------------

int main()
{
   auto mysingleton = MySingleton::instance();

   return EXIT_SUCCESS;
}

