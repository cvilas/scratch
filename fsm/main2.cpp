#include "fsm2.h"

#include <thread>
#include <iostream>
#include <csignal>

/// do nothing state
class IdleState : public FsmState
{
public:
  IdleState(Fsm& fsm) : FsmState(fsm, "idle") {}
  ~IdleState(){}
  void onEntry() final
  {
    std::cout << "[" << getName() << "::onEntry]\n";
  }
  void onExit() final
  {
    std::cout << "[" << getName() << "::onExit]\n";
  }
};

/// powering up
class PowerUpState : public FsmState
{
public:
  PowerUpState(Fsm& ctx) : FsmState(ctx, "power_up") {}
  ~PowerUpState(){}
  virtual void onEntry()
  {
    std::cout << "[" << getName() << "::onEntry] entered\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    getFsm().raiseSignal("maintain_speed");
    std::cout << "[" << getName() << "::onEntry] exited\n";
  }
  void onExit() final
  {
    std::cout << "[" << getName() << "::onExit]\n";
  }
};

/// powering down
class PowerDownState : public FsmState
{
public:
  PowerDownState(Fsm& ctx) : FsmState(ctx, "power_down") {}
  ~PowerDownState(){}
  virtual void onEntry()
  {
    std::cout << "[" << getName() << "::onEntry] entered\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    getFsm().raiseSignal("has_shutdown");
    std::cout << "[" << getName() << "::onEntry] exited\n";
  }
  void onExit() final
  {
    std::cout << "[" << getName() << "::onExit]\n";
  }
};

/// maintain speed
class SpeedControlState : public FsmState
{
public:
  SpeedControlState(Fsm& ctx) : FsmState(ctx,"speed_control") {}
  ~SpeedControlState(){}
  virtual void onEntry()
  {
    std::cout << "[" << getName() << "::onEntry] entered\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    getFsm().raiseSignal("off");
    std::cout << "[" << getName() << "::onEntry] exited\n";
  }
  void onExit() final
  {
    std::cout << "[" << getName() << "::onExit]\n";
  }
};

/// The automatic motor controller
class MotorController
{
public:
  MotorController()
  {
    controller_fsm_.addState(std::make_shared<IdleState>(controller_fsm_));
    controller_fsm_.addState(std::make_shared<PowerUpState>(controller_fsm_));
    controller_fsm_.addState(std::make_shared<PowerDownState>(controller_fsm_));
    controller_fsm_.addState(std::make_shared<SpeedControlState>(controller_fsm_));

    controller_fsm_.addTransitionRule("idle", "on", "power_up");
    controller_fsm_.addTransitionRule("power_up", "maintain_speed", "speed_control");
    controller_fsm_.addTransitionRule("speed_control", "off", "power_down");
    controller_fsm_.addTransitionRule("power_up", "off", "power_down");
    controller_fsm_.addTransitionRule("power_down", "on", "power_up");
    controller_fsm_.addTransitionRule("power_down", "has_shutdown", "idle");

    controller_fsm_.initialise("idle");
  }

  ~MotorController()
  {
    std::cout << "State while exiting: " << getCurrentState() << "\n" << std::flush;
    controller_fsm_.raiseSignal("off");
    std::cout << "Waiting for pending triggers to flush..\n" << std::flush;
    while(controller_fsm_.isTransitionPending())
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "No pending triggers\n" << std::flush;
    std::cout << "Waiting for \"idle\" state..\n" << std::flush;
    while(getCurrentState() != "idle")
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "State \"idle\" reached\n" << std::flush;
  }

  void trigger(FsmSignal signal)
  {
    controller_fsm_.raiseSignal(signal);
  }

  std::string getCurrentState() const
  {
    return controller_fsm_.getCurrentState()->getName();
  }

private:
  Fsm controller_fsm_;
};

static int s_stopFlag = 0;

//---------------------------------------------------------------------------------------------------------------------
static void onSignal(int signum)
//---------------------------------------------------------------------------------------------------------------------
{
  (void)signum;
  s_stopFlag = 1;
}

//=====================================================================================================================
int main(int argc, char** argv)
//=====================================================================================================================
{
  signal(SIGINT, &onSignal);

  MotorController controller;
  std::cout << "Starting state: " << controller.getCurrentState() << "\n" << std::flush;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  controller.trigger("on");

  while(0 == s_stopFlag)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
