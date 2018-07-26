#ifndef FSM2_H
#define FSM2_H

#include <memory>
#include <vector>
#include <algorithm>
#include <future>
#include <deque>
#include <mutex>
#include <condition_variable>

class Fsm;
class FsmState;
struct FsmTransition;
using FsmSignal = std::string;

class FsmState
{
public:
  virtual ~FsmState(){}
  virtual void onEntry() = 0;
  virtual void onExit() = 0;
  std::string getName() const { return name_; }
  Fsm& getFsm() { return fsm_; }
protected:
  FsmState(Fsm& fsm, const std::string& name) : fsm_(fsm), name_(name) {}
protected:
  Fsm& fsm_;
  std::string name_;
};

struct FsmTransition
{
  std::string current_state;
  std::string new_state;
  FsmSignal signal;
};

class Fsm
{
public:
  Fsm();
  ~Fsm();
  void addState(std::shared_ptr<FsmState> state);
  void addTransitionRule(const std::string& from_state, const FsmSignal& signal, const std::string& to_state);
  void initialise(const std::string& initial_state);
  void trigger(const FsmSignal& signal);
  bool isTriggerPending() const;
  const std::shared_ptr<FsmState>& getCurrentState() const;
private:
  bool stateExists(const std::string& name);
  bool transitionExists(const std::string& state_name, const FsmSignal& signal);
  void triggerProcessingLoop();
  void switchState(const FsmSignal& signal);
private:
  std::vector<std::shared_ptr<FsmState>> states_;
  std::vector<FsmTransition> transitions_;
  std::shared_ptr<FsmState> current_state_;

  mutable std::mutex trigger_guard_;
  bool exit_trigger_processor_;
  std::condition_variable trigger_condition_;
  std::deque<FsmSignal> trigger_queue_;
  std::future<void> trigger_processor_result_;
};


#endif // FMS2_H
