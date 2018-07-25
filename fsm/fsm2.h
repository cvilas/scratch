#ifndef FSM2_H
#define FSM2_H

#include <memory>
#include <vector>
#include <algorithm>
#include <future>

using FsmSignal = std::string;

struct FsmTransition
{
  std::string current_state;
  std::string new_state;
  FsmSignal signal;
};

class Fsm;

class FsmContext
{
public:
  FsmContext();
  void setTopLevelFsm(Fsm* top);
  void trigger(const FsmSignal& signal);
private:
  Fsm* top_level_fsm_;
};

class Fsm
{
public:
  Fsm(FsmContext& ctx, const std::string& name);
  virtual ~Fsm();
  virtual void onEntry();
  virtual void onExit();
  void addState(std::shared_ptr<Fsm> state);
  void addTransitionRule(const std::string& state, const FsmSignal& signal, const std::string& new_state);
  void initialise(const std::string& initial_state);
  bool trigger(const FsmSignal& signal);
  std::shared_ptr<Fsm> getCurrentState();
  FsmContext& getContext();
  std::string name();
private:
  FsmContext& context_;
  std::string name_;
  std::vector<std::shared_ptr<Fsm>> states_;
  std::vector<FsmTransition> transitions_;
  std::shared_ptr<Fsm> current_state_;
  std::vector<std::future<void>> pending_futs_;
};


#endif // FMS2_H
