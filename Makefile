#
# Copyright (c) Flyover Games, LLC
#

SHELL := /usr/bin/env bash

__default__: help

help:
	@echo done $@

GYP ?= gyp
gyp:
	$(GYP) --depth=. -f xcode -DOS=ios --generator-output=./node-sdl2_ttf-ios node-sdl2_ttf.gyp
	$(GYP) --depth=. -f xcode -DOS=osx --generator-output=./node-sdl2_ttf-osx node-sdl2_ttf.gyp
