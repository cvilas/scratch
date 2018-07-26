#include "fsm2.h"

#include <sstream>

//======================================================================================================================
Fsm::Fsm() : exit_trigger_processor_(false)
//======================================================================================================================
{
  trigger_processor_result_ = std::async(std::launch::async, [this](){this->triggerProcessingLoop();});
}

//----------------------------------------------------------------------------------------------------------------------
Fsm::~Fsm()
//----------------------------------------------------------------------------------------------------------------------
{
  exit_trigger_processor_ = true;
  trigger_condition_.notify_all();
  if (trigger_processor_result_.valid())
  {
   trigger_processor_result_.wait();
  }
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::addState(std::shared_ptr<FsmState> state)
//----------------------------------------------------------------------------------------------------------------------
{
  states_.emplace_back(std::move(state));
}

//----------------------------------------------------------------------------------------------------------------------
bool Fsm::stateExists(const std::string& name)
//----------------------------------------------------------------------------------------------------------------------
{
  return states_.end() != std::find_if(states_.begin(), states_.end(), [&name](const std::shared_ptr<FsmState>& s){return s->getName() == name; });
}

//----------------------------------------------------------------------------------------------------------------------
bool Fsm::transitionExists(const std::string& state_name, const FsmSignal& signal)
//----------------------------------------------------------------------------------------------------------------------
{
  return transitions_.end() != std::find_if(transitions_.begin(), transitions_.end(), [&state_name, &signal](const FsmTransition& t)
  {return ((t.current_state == state_name) && (t.signal == signal)); });
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::addTransitionRule(const std::string& from_state, const FsmSignal& signal, const std::string& to_state)
//----------------------------------------------------------------------------------------------------------------------
{
  if(!stateExists(from_state))
  {
    std::stringstream str;
    str << "'From' state " << from_state << " does not exit";
    throw std::runtime_error(str.str());
  }
  if(!stateExists(to_state))
  {
    std::stringstream str;
    str << "'To' state " << to_state << " does not exit";
    throw std::runtime_error(str.str());
  }
  if(transitionExists(from_state, signal))
  {
    std::stringstream str;
    str << "Transition from " << from_state << " already exists for " << signal;
    throw std::runtime_error(str.str());
  }
  transitions_.push_back({from_state, to_state, signal});
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::initialise(const std::string& initial_state)
//----------------------------------------------------------------------------------------------------------------------
{
  const auto it_end = states_.end();
  if(current_state_ != nullptr)
  {
    current_state_->onExit();
  }
  auto it = std::find_if(states_.begin(), states_.end(), [&initial_state](const std::shared_ptr<FsmState>& state){return state->getName() == initial_state; });
  if(it == it_end)
  {
    std::stringstream str;
    str << "State " << initial_state << " does not exist";
    throw std::runtime_error(str.str());
  }
  current_state_ = *it;
  current_state_->onEntry();
}

//----------------------------------------------------------------------------------------------------------------------
const std::shared_ptr<FsmState>& Fsm::getCurrentState() const
//----------------------------------------------------------------------------------------------------------------------
{
  return current_state_;
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::trigger(const std::string& signal)
//----------------------------------------------------------------------------------------------------------------------
{
  std::lock_guard<std::mutex> lk(trigger_guard_);
  trigger_queue_.push_back(signal);
  trigger_condition_.notify_one();
}

//----------------------------------------------------------------------------------------------------------------------
bool Fsm::isTriggerPending() const
//----------------------------------------------------------------------------------------------------------------------
{
  std::lock_guard<std::mutex> lk(trigger_guard_);
  return !trigger_queue_.empty();
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::switchState(const FsmSignal& signal)
//----------------------------------------------------------------------------------------------------------------------
{
  // find a valid transition
  const auto it = std::find_if(transitions_.begin(), transitions_.end(), [this, signal](const FsmTransition& t)
  {
    return (t.current_state == current_state_->getName()) && (t.signal == signal);
  });

  if(it == transitions_.end())
  {
    //std::cout << "No transition from " << current_state_->getName() << " for signal " << signal << ". Ignored.\n";
    return;
  }

  // find next state
  auto next_state_it = std::find_if(states_.begin(), states_.end(), [it](const std::shared_ptr<FsmState>& state)
  {
    return state->getName() == (*it).new_state;
  });

  // exit current state and bring up new state
  if(current_state_ != nullptr)
  {
    current_state_->onExit();
  }
  current_state_ = *next_state_it;
  current_state_->onEntry();
}

//----------------------------------------------------------------------------------------------------------------------
void Fsm::triggerProcessingLoop()
//----------------------------------------------------------------------------------------------------------------------
{
  while(1)
  {
    std::unique_lock<std::mutex> lk(trigger_guard_);
    trigger_condition_.wait(lk);

    while(!trigger_queue_.empty())
    {
      const auto trigger = trigger_queue_.front();
      trigger_queue_.pop_front();

      lk.unlock();
      switchState(trigger);
      lk.lock();
    }

    if(exit_trigger_processor_)
    {
      return;
    }
  }
}
