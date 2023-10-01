wemo = {
  platform = {
    can_bus_main=can0,
    can_bus_aux=can1,
    enable_lift=yes,
    hmi={
      buzzer=on
    },
    position_hold_monitor={
      tolerance_meters=0.2,
      tolerance_radians=0.4
    }
  }
}

print("hold monitor tolerance is "..
      wemo.platform.position_hold_monitor.tolerance_meters..
      " meters and "..
      wemo.platform.position_hold_monitor.tolerance_radians..
      " radians")
