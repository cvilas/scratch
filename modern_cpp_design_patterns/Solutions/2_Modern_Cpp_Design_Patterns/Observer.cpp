/**************************************************************************************************
*
* \file Observer.cpp
* \brief C++ Training - Example for the Observer Design Pattern
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
* Task: Refactor the classical Observer solution by means of the Command pattern. Note that
*       it should still be possible to detach observers from their subjects.
*
**************************************************************************************************/

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <list>
#include <iostream>
#include <stdexcept>
#include <vector>


//--Callback.h-------------------------------------------------------------------------------------

using Callback = std::function<void()>;


//--Subject.h--------------------------------------------------------------------------------------

class Subject
{
 public:
   using ID = std::list<Callback>::const_iterator;

   explicit Subject();

   ID   attach( Callback callback );
   void detach( ID id );

 protected:
   void notifyAllObservers();

 private:
   std::list<Callback> callbacks_;
};


//--Subject.cpp------------------------------------------------------------------------------------

Subject::Subject()
{}

Subject::ID Subject::attach( Callback callback )
{
   return callbacks_.insert( callbacks_.end(), callback );
}

void Subject::detach( ID id )
{
   callbacks_.erase( id );
}

void Subject::notifyAllObservers()
{
   for( auto callback : callbacks_ ) {
      callback();
   }
}


//--ConcreteObserver.h-----------------------------------------------------------------------------

class ConcreteObserver
{
 public:
   explicit ConcreteObserver( int id );

   void update();

 private:
   int id_;
};

//--ConcreteObserver.cpp---------------------------------------------------------------------------

ConcreteObserver::ConcreteObserver( int id )
   : id_( id )
{}

void ConcreteObserver::update()
{
   std::cout << " Updating concrete observer " << id_ << "...\n";
}


//--ConcreteSubject.h------------------------------------------------------------------------------

class ConcreteSubject : public Subject
{
 public:
   explicit ConcreteSubject();

   void doSomething();
};


//--ConcreteSubject.cpp----------------------------------------------------------------------------

ConcreteSubject::ConcreteSubject()
   : Subject()
{}

void ConcreteSubject::doSomething()
{
   // Running some algorithm, then notify all observers

   notifyAllObservers();
}


//--Main.cpp---------------------------------------------------------------------------------------

int main()
{
   ConcreteSubject subject1;
   ConcreteSubject subject2;

   ConcreteObserver observer1( 1 );
   ConcreteObserver observer2( 2 );
   ConcreteObserver observer3( 3 );

   auto const id1 = subject1.attach( [&observer1](){ observer1.update(); } );
   auto const id2 = subject1.attach( [&observer2](){ observer2.update(); } );

   auto const id3 = subject2.attach( [&observer2](){ observer2.update(); } );
   auto const id4 = subject2.attach( [&observer3](){ observer3.update(); } );

   std::cout << "\n Running 'subject1.doSomething()'...\n";
   subject1.doSomething();

   std::cout << "\n Running 'subject2.doSomething()'...\n";
   subject2.doSomething();

   subject1.detach( id1 );
   subject1.detach( id2 );

   subject2.detach( id3 );
   subject2.detach( id4 );

   return EXIT_SUCCESS;
}
