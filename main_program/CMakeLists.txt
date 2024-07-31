#
# Copyright (c) 2018 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sc_test)

# NORDIC SDK APP START
FILE(GLOB app_sources src/*.c ../dev_fw/src/*.c  ../dev_fw/dev_h/nRF52840_pin_def.h ../dev_fw/inc/*.h )

target_sources(app PRIVATE ${app_sources})

zephyr_include_directories( ${APPLICATION_SOURCE_DIR}/../dev_fw/inc ${APPLICATION_SOURCE_DIR}/../dev_fw/dev_h ${APPLICATION_SOURCE_DIR}/../test )
