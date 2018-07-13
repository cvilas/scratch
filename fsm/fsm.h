#include <sstream>
#include <deque>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>

//References:
//* https://www.codeproject.com/Articles/1087619/State-Machine-Design-in-Cplusplus
//* https://github.com/digint/tinyfsm
//* https://codereview.stackexchange.com/questions/40686/state-pattern-c-template

class Fsm;
class FsmContext;
using FsmEvent = std::string;

struct FsmTransition
{
  Fsm* pCurrent;
  Fsm* pNext;
  FsmEvent event;
};

using FsmTransitionTable = std::vector<FsmTransition>;


/// Fsm implements a state and is a state machine by itself
class Fsm
{
  friend class FsmContext;

public:
  Fsm(const std::string& name) : name_(name), pCurrent_(nullptr) {}

  virtual ~Fsm() = default;

  std::string name() const { return name_; }

  virtual void init(Fsm* initState)
  {
    pCurrent_ = initState;
    pCurrent_->onEntry();
  }

  virtual void onEntry() {}

  virtual void onExit() {}

  void addTransition(Fsm* pCurrent, Fsm* pNext, FsmEvent event)
  {
    transitions_.push_back({pCurrent, pNext, event});
  }

  void print(std::stringstream& ss, int indent = 0)
  {
    ss << "-" << name_;
    if(pCurrent_)
    {
      indent += ss.str().length();
      ss << "+" << std::endl;
      ss << std::string(indent, ' ');
      pCurrent_->print(ss, indent);
    }
    else
    {
      ss << std::endl;
    }
  }

private:
  FsmTransitionTable::const_iterator find(Fsm* pCurrent, FsmEvent e)
  {
    return std::find_if(transitions_.begin(), transitions_.end(), [pCurrent, e](const FsmTransition& transition){ return transition.pCurrent == pCurrent && transition.event == e; });
  }

  bool handleEvent(const FsmEvent& e)
  {
    // make top level Fsm catch the event
    // recurse down to the deepest fsm and handle there first.
    if(pCurrent_ && !pCurrent_->handleEvent(e))
    {
      // deeper fsms didn't handle. handle transition now
      const auto& it = find(pCurrent_, e);
      if(it == transitions_.end())
      {
        return false;
      }

      /// \todo complete currently executing action

      pCurrent_->onExit();
      pCurrent_ = it->pNext;
      if(pCurrent_)
      {
        pCurrent_->onEntry();
      }
      return true;
    }

    return false;
  }
private:
  std::string name_;
  Fsm* pCurrent_;
  FsmTransitionTable transitions_;
};

/// Context in which a state machine is operating
class FsmContext : public std::thread
{
public:
  static void setTopLevelFsm(Fsm* fsm)
  {
    get().pFsm_ = fsm;
  }

  static void raiseEvent(const FsmEvent& ev)
  {
    get().events_.push_back(ev);
  }

  static FsmContext& get()
  {
    static FsmContext ctx;
    return ctx;
  }

private:
  FsmContext() : std::thread(handleEvent), exitFlag_(false) {}
  virtual ~FsmContext() = default;

  static void handleEvent()
  {
    FsmContext& ctx = get();
    while(!ctx.exitFlag_)
    {
      while(!ctx.events_.empty())
      {
        ctx.pFsm_->handleEvent(get().events_.front());
        ctx.events_.pop_front();
      }
    }
  }

private:
  Fsm* pFsm_;
  std::deque<FsmEvent> events_;
  bool exitFlag_;
};
