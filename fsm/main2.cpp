#include "fsm2.h"

#include <thread>
#include <iostream>

/// do nothing state
class IdleState : public Fsm
{
public:
  IdleState(FsmContext& ctx) : Fsm(ctx, "idle") {}
  ~IdleState(){}
};

/// powering up
class PowerUpState : public Fsm
{
public:
  PowerUpState(FsmContext& ctx) : Fsm(ctx, "power_up") {}
  ~PowerUpState(){}
  virtual void onEntry()
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    getContext().trigger("maintain_speed");
  }
};

/// powering down
class PowerDownState : public Fsm
{
public:
  PowerDownState(FsmContext& ctx) : Fsm(ctx, "power_down") {}
  ~PowerDownState(){}
  virtual void onEntry()
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    getContext().trigger("has_shutdown");
  }
};

/// maintain speed
class SpeedControlState : public Fsm
{
public:
  SpeedControlState(FsmContext& ctx) : Fsm(ctx,"speed_control") {}
  ~SpeedControlState(){}
  virtual void onEntry()
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    getContext().trigger("off");
  }
};

/// The automatic motor controller
class MotorController
{
public:
  MotorController() : controller_fsm_(std::make_shared<Fsm>(ctx_, "motor_controller"))
  {
    controller_fsm_->addState(std::make_shared<IdleState>(ctx_));
    controller_fsm_->addState(std::make_shared<PowerUpState>(ctx_));
    controller_fsm_->addState(std::make_shared<PowerDownState>(ctx_));
    controller_fsm_->addState(std::make_shared<SpeedControlState>(ctx_));

    controller_fsm_->addTransitionRule("idle", "on", "power_up");
    controller_fsm_->addTransitionRule("power_up", "maintain_speed", "speed_control");
    controller_fsm_->addTransitionRule("speed_control", "off", "power_down");
    controller_fsm_->addTransitionRule("power_up", "off", "power_down");
    controller_fsm_->addTransitionRule("power_down", "on", "power_up");
    controller_fsm_->addTransitionRule("power_down", "has_shutdown", "idle");

    ctx_.setTopLevelFsm(controller_fsm_.get());
    controller_fsm_->initialise("idle");
  }

  ~MotorController() {}

  void trigger(FsmSignal signal)
  {
    ctx_.trigger(signal);
  }

  std::string getCurrentState() const
  {
    return controller_fsm_->getCurrentState()->name();
  }

private:
  FsmContext ctx_;
  std::shared_ptr<Fsm> controller_fsm_;
};

//=====================================================================================================================
int main(int argc, char** argv)
//=====================================================================================================================
{
  MotorController controller;
  std::cout << "\r" << controller.getCurrentState() << std::flush;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  controller.trigger("on");

  while(1)
  {
    std::cout << "\r" << controller.getCurrentState()<< std::flush;
  }
}
