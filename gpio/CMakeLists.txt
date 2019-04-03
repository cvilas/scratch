cmake_minimum_required(VERSION 2.8)
project(gpio)

add_executable(lsgpio lsgpio.c gpio-utils.c gpio-utils.h)
add_executable(gpio-event-mon gpio-event-mon.c gpio-utils.c gpio-utils.h)
add_executable(gpio-hammer gpio-hammer.c gpio-utils.c gpio-utils.h)
