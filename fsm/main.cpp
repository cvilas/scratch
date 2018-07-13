#include "fsm.h"
#include <iostream>

/// do nothing state
class IdleState : public Fsm
{
public:
  IdleState() : Fsm("idle") {}
  virtual ~IdleState() = default;
};

/// powering up
class PowerUpState : public Fsm
{
public:
  PowerUpState() : Fsm("power_up") {}
  virtual ~PowerUpState() = default;
  virtual void onEntry()
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    FsmContext::raiseEvent("maintain_speed");
  }
};

/// powering down
class PowerDownState : public Fsm
{
public:
  PowerDownState() : Fsm("power_down") {}
  virtual ~PowerDownState() = default;
  virtual void onEntry()
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    FsmContext::raiseEvent("has_shutdown");
  }
};

/// maintain speed
class SpeedControlState : public Fsm
{
public:
  SpeedControlState() : Fsm("speed_control") {}
  virtual ~SpeedControlState() = default;
  virtual void onEntry()
  {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    FsmContext::raiseEvent("off");
  }
};

/// The automatic motor controller
class MotorController : public Fsm
{
public:
  MotorController() : Fsm("motor_controller")
  {
    init(&idle_);
    addTransition(&idle_, &powerUp_, "on");
    addTransition(&powerUp_, &speedCtrl_, "maintain_speed");
    addTransition(&speedCtrl_, &powerDn_, "off");
    addTransition(&powerUp_, &powerDn_, "off");
    addTransition(&powerDn_, &powerUp_, "on");
    addTransition(&powerDn_, &idle_, "has_shutdown");
  }
  virtual ~MotorController() = default;

private:
  IdleState idle_;
  PowerUpState powerUp_;
  PowerDownState powerDn_;
  SpeedControlState speedCtrl_;
};

//=====================================================================================================================
int main(int argc, char** argv)
//=====================================================================================================================
{
  MotorController fsm;
  FsmContext::setTopLevelFsm(&fsm);
  FsmContext::raiseEvent("on");

  while(1)
  {
    std::stringstream ss;
    fsm.print(ss);
    std::cout << ss.str() << std::endl;
  }
}
