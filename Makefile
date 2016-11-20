all: install

install-log: build
	pebble install --emulator aplite --log

install: build
	pebble install --emulator aplite

build: src/* resources/*
	pebble build
