/**************************************************************************************************
*
* \file StateMachine.cpp
* \brief C++ Training - Example for the implementation of a state machine with std::variant
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
* Task: Modify the given state machine implementation such that it is possible to reset a
*       given state machine to the 'Start' state.
*
**************************************************************************************************/

#include <cstdlib>
#include <iostream>
#include <variant>


struct Start {};
struct Ready {};
struct Active {};
struct End {};


using StateMachine = std::variant<Start,Ready,Active,End>;


struct Prepare
{
   void operator()( Start  ) { puts( "Transition to the 'Ready' state" ); sm_ = Ready{}; }
   void operator()( Ready  ) { puts( "Staying in the 'Ready' state" ); }
   void operator()( Active ) { puts( "Staying in the 'Active' state" ); }
   void operator()( End    ) { puts( "Staying in the 'End' state" ); }

   StateMachine& sm_;
};


struct Go
{
   void operator()( Start  ) { puts( "Staying in the 'Start' state" ); }
   void operator()( Ready  ) { puts( "Transition to the 'Active' state" ); sm_ = Active{}; }
   void operator()( Active ) { puts( "Staying in the 'Active' state" ); }
   void operator()( End    ) { puts( "Staying in the 'End' state" ); }

   StateMachine& sm_;
};


struct Stop
{
   void operator()( Start  ) { puts( "Transition to the 'End' state" ); sm_ = End{}; }
   void operator()( Ready  ) { puts( "Transition to the 'End' state" ); sm_ = End{}; }
   void operator()( Active ) { puts( "Transition to the 'End' state" ); sm_ = End{}; }
   void operator()( End    ) { puts( "Staying in the 'End' state" ); }

   StateMachine& sm_;
};


template< typename Signal >
void signal( StateMachine& sm )
{
   std::visit( Signal{ sm }, sm );
}


int main()
{
   StateMachine sm{};

   signal<Go>( sm );
   signal<Prepare>( sm );
   signal<Prepare>( sm );
   signal<Go>( sm );
   signal<Go>( sm );
   signal<Stop>( sm );

   return EXIT_SUCCESS;
}
