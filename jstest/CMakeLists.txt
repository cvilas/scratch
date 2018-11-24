cmake_minimum_required(VERSION 2.8)
project(jstest)

add_executable(jstest jstest.c axbtnmap.c axbtnmap.h)
add_executable(evtest_old evtest_old.c)
add_executable(evtest_new evtest_new.c)
add_executable(evdev_joystick evdev_joystick.c)
