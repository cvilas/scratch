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

#include <cstdlib>
#include <algorithm>
#include <functional>
#include <list>
#include <iostream>
#include <stdexcept>
#include <vector>


//--Observer.h-------------------------------------------------------------------------------------

class Observer
{
 public:
   explicit Observer( int id );

   void update();

 private:
   virtual void doUpdate() = 0;

   int id_;
};


//--Observer.cpp-----------------------------------------------------------------------------------

Observer::Observer( int id )
   : id_( id )
{}

void Observer::update()
{
   std::cout << " Updating observer " << id_ << "\n";
   doUpdate();
}


//--Subject.h--------------------------------------------------------------------------------------

class Subject
{
 public:
   explicit Subject();

   void attach( Observer* observer );
   void detach( Observer* observer );

 protected:
   void notifyAllObservers();

 private:
   std::vector<Observer*> observers_;
};


//--Subject.cpp------------------------------------------------------------------------------------

Subject::Subject()
{}

void Subject::attach( Observer* observer )
{
   auto const pos = std::find( observers_.begin(), observers_.end(), observer );

   if( pos != observers_.end() )
      throw std::logic_error( "Same observer registered multiple times" );

   observers_.push_back( observer );
}

void Subject::detach( Observer* observer )
{
   auto const pos = std::find( observers_.begin(), observers_.end(), observer );

   if( pos == observers_.end() )
      throw std::logic_error( "Observer couldn't be detached" );

   observers_.erase( pos );
}

void Subject::notifyAllObservers()
{
   for( auto observer : observers_ ) {
      observer->update();
   }
}


//--ConcreteObserver.h-----------------------------------------------------------------------------

class ConcreteObserver : public Observer
{
 public:
   explicit ConcreteObserver( int id );

 private:
   void doUpdate() override;
};

//--ConcreteObserver.cpp---------------------------------------------------------------------------

ConcreteObserver::ConcreteObserver( int id )
   : Observer( id )
{}

void ConcreteObserver::doUpdate()
{
   std::cout << " Updating concrete observer...\n";
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

   subject1.attach( &observer1 );
   subject1.attach( &observer2 );

   subject2.attach( &observer2 );
   subject2.attach( &observer3 );

   std::cout << "\n Running 'subject1.doSomething()'...\n";
   subject1.doSomething();

   std::cout << "\n Running 'subject2.doSomething()'...\n";
   subject2.doSomething();

   subject1.detach( &observer1 );
   subject1.detach( &observer2 );

   subject2.detach( &observer2 );
   subject2.detach( &observer3 );

   return EXIT_SUCCESS;
}
