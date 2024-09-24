#*******************************************************************************
#*   (c) 2018 - 2023 Zondax AG
#*
#*  Licensed under the Apache License, Version 2.0 (the "License");
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*      http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an "AS IS" BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#********************************************************************************

.PHONY: all deps build clean load delete check_python show_info_recovery_mode

TESTS_ZEMU_DIR?=$(CURDIR)/tests_zemu
TESTS_JS_PACKAGE?=
TESTS_JS_DIR?=

LEDGER_SRC=$(CURDIR)/app
DOCKER_APP_SRC=/app
DOCKER_APP_BIN=$(DOCKER_APP_SRC)/app/bin/app.elf

DOCKER_BOLOS_SDKS = NANOS_SDK
DOCKER_BOLOS_SDKX = NANOX_SDK
DOCKER_BOLOS_SDKS2 = NANOSP_SDK
DOCKER_BOLOS_SDKST = STAX_SDK
DOCKER_BOLOS_SDKFL = FLEX_SDK

TARGET_S = nanos
TARGET_X = nanox
TARGET_S2 = nanos2
TARGET_ST = stax
TARGET_FL = flex

# Note: This is not an SSH key, and being public represents no risk
SCP_PUBKEY=049bc79d139c70c83a4b19e8922e5ee3e0080bb14a2e8b0752aa42cda90a1463f689b0fa68c1c0246845c2074787b649d0d8a6c0b97d4607065eee3057bdf16b83
SCP_PRIVKEY=ff701d781f43ce106f72dc26a46b6a83e053b5d07bb3d4ceab79c91ca822a66b

INTERACTIVE:=$(shell [ -t 0 ] && echo 1)
USERID:=$(shell id -u)
GROUPID:=$(shell id -g)
$(info USERID                : $(USERID))
$(info GROUPID               : $(GROUPID))
$(info TESTS_ZEMU_DIR        : $(TESTS_ZEMU_DIR))
$(info TESTS_JS_DIR          : $(TESTS_JS_DIR))
$(info TESTS_JS_PACKAGE      : $(TESTS_JS_PACKAGE))

DOCKER_IMAGE_ZONDAX=zondax/ledger-app-builder:ledger-ec93499de7f17076ee90caaca5953fdb9d3daf6c
DOCKER_IMAGE_LEDGER=ghcr.io/ledgerhq/ledger-app-builder/ledger-app-builder:latest

ifdef INTERACTIVE
INTERACTIVE_SETTING:="-i"
TTY_SETTING:="-t"
else
INTERACTIVE_SETTING:=
TTY_SETTING:=
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	NPROC=$(shell nproc)
endif
ifeq ($(UNAME_S),Darwin)
	NPROC=$(shell sysctl -n hw.physicalcpu)
endif

define run_docker
	docker run $(TTY_SETTING) $(INTERACTIVE_SETTING) --rm \
	-e SCP_PRIVKEY=$(SCP_PRIVKEY) \
	-e SDK_VARNAME=$(1) \
	-e TARGET=$(2) \
	-u $(USERID):$(GROUPID) \
	-v $(shell realpath .):/app \
	-e SUPPORT_SR25519=$(SUPPORT_SR25519) \
	-e SUBSTRATE_PARSER_FULL=$(SUBSTRATE_PARSER_FULL) \
	-e DISABLE_PREVIOUS=$(DISABLE_PREVIOUS) \
	-e DISABLE_CURRENT=$(DISABLE_CURRENT) \
	-e COIN=$(COIN) \
	-e APP_TESTING=$(APP_TESTING) \
	-e PRODUCTION_BUILD=$(PRODUCTION_BUILD) \
	$(DOCKER_IMAGE_ZONDAX) "$(3)"
endef

define run_docker_ledger
	docker run $(TTY_SETTING) $(INTERACTIVE_SETTING) --rm \
	-v $(shell pwd):/app \
	$(DOCKER_IMAGE_LEDGER) "$(1)"
endef

all:
	@$(MAKE) clean
	@$(MAKE) buildS2
	@$(MAKE) buildST
	@$(MAKE) buildFL

.PHONY: check_python
check_python:
	@python -c 'import sys; sys.exit(3-sys.version_info.major)' || (echo "The python command does not point to Python 3"; exit 1)

.PHONY: deps
deps: check_python
	@echo "Install dependencies"
	$(CURDIR)/deps/ledger-zxlib/scripts/install_deps.sh

.PHONY: convert_icon
convert_icon:
	@convert $(LEDGER_SRC)/tmp.gif -monochrome -size 16x16 -depth 1 $(LEDGER_SRC)/nanos_icon.gif
	@convert $(LEDGER_SRC)/nanos_icon.gif -crop 14x14+1+1 +repage -negate $(LEDGER_SRC)/nanox_icon.gif

.PHONY: buildS
buildS:
	TARGET_NAME=TARGET_NANOS make -C app buildS

.PHONY: buildX
buildX:
	TARGET_NAME=TARGET_NANOX make -C app buildX

.PHONY: buildS2
buildS2:
	TARGET_NAME=TARGET_NANOS2 make -C app buildS2

.PHONY: buildST
buildST:
	TARGET_NAME=TARGET_STAX make -C app buildST

.PHONY: buildFL
buildFL:
	TARGET_NAME=TARGET_FLEX make -C app buildFL

.PHONY: clean_output
clean_output:
	TARGET_NAME=TARGET_NANOS2 make -C app clean_output

.PHONY: clean_build
clean_build:
	TARGET_NAME=TARGET_NANOS2 make -C app clean_build

.PHONY: clean
clean: clean_output clean_build

.PHONY: ledger-setup
ledger-setup:
	cargo ledger setup

.PHONY: format
format:
	TARGET_NAME=TARGET_NANOS2 make -C app format

# To be run on linux only
.PHONY: lint-all
lint-all: lint-nanosplus lint-flex lint-stax lint-nanox

.PHONY: lint-nanosplus
lint-nanosplus:
	TARGET_NAME=TARGET_NANOS2 make -C app lint-nanosplus

.PHONY: lint-nanox
lint-nanox:
	TARGET_NAME=TARGET_NANOX make -C app lint-nanox

.PHONY: lint-flex
lint-flex:
	TARGET_NAME=TARGET_FLEX make -C app lint-flex

.PHONY: lint-stax
lint-stax:
	TARGET_NAME=TARGET_STAX make -C app lint-stax


# To be run on linux only
.PHONY: lint-fix-all
lint-fix-all: lint-nanosplus-fix lint-flex-fix lint-stax-fix lint-nanox-fix

.PHONY: lint-nanosplus-fix
lint-nanosplus-fix:
	TARGET_NAME=TARGET_NANOS2 make -C app lint-nanosplus-fix

.PHONY: lint-nanox-fix
lint-nanox-fix:
	TARGET_NAME=TARGET_NANOX make -C app lint-nanox-fix

.PHONY: lint-flex-fix
lint-flex-fix:
	TARGET_NAME=TARGET_FLEX make -C app lint-flex-fix

.PHONY: lint-stax-fix
lint-stax-fix:
	TARGET_NAME=TARGET_STAX make -C app lint-stax-fix

.PHONY: listvariants
listvariants:
	$(call run_docker,$(DOCKER_BOLOS_SDKS2),$(TARGET_S2),make listvariants)

.PHONY: shellS
shellS:
	$(call run_docker,$(DOCKER_BOLOS_SDKS) -t,$(TARGET_S),bash)

.PHONY: shellX
shellX:
	$(call run_docker,$(DOCKER_BOLOS_SDKX) -t,$(TARGET_X),bash)

.PHONY: shellS2
shellS2:
	$(call run_docker,$(DOCKER_BOLOS_SDKS2) -t,$(TARGET_S2),bash)

.PHONY: shellST
shellST:
	$(call run_docker,$(DOCKER_BOLOS_SDKST) -t,$(TARGET_ST),bash)

.PHONY: shellFL
shellFL:
	$(call run_docker,$(DOCKER_BOLOS_SDKFL) -t,$(TARGET_FL),bash)

.PHONY: loadS
loadS:
	${LEDGER_SRC}/pkg/installer_s.sh load

.PHONY: deleteS
deleteS:
	${LEDGER_SRC}/pkg/installer_s.sh delete

.PHONY: loadS2
loadS2:
	${LEDGER_SRC}/pkg/installer_s2.sh load

.PHONY: deleteS2
deleteS2:
	${LEDGER_SRC}/pkg/installer_s2.sh delete

.PHONY: loadST
loadST:
	${LEDGER_SRC}/pkg/installer_stax.sh load

.PHONY: deleteST
deleteST:
	${LEDGER_SRC}/pkg/installer_stax.sh delete

.PHONY: loadFL
loadFL:
	${LEDGER_SRC}/pkg/installer_flex.sh load

.PHONY: deleteFL
deleteFL:
	${LEDGER_SRC}/pkg/installer_flex.sh delete

.PHONY: sizeS
sizeS:
	$(CURDIR)/deps/ledger-zxlib/scripts/getSize.py nanos

.PHONY: sizeS2
sizeS2:
	$(CURDIR)/deps/ledger-zxlib/scripts/getSize.py nanos2

.PHONY: sizeX
sizeX:
	$(CURDIR)/deps/ledger-zxlib/scripts/getSize.py nanox

.PHONY: sizeST
sizeST:
	$(CURDIR)/deps/ledger-zxlib/scripts/getSize.py stax

.PHONY: sizeFL
sizeFL:
	$(CURDIR)/deps/ledger-zxlib/scripts/getSize.py flex

.PHONY: show_info_recovery_mode
show_info_recovery_mode:
	@echo "This command requires a Ledger Nano S in recovery mode. To go into recovery mode, follow:"
	@echo " 1. Settings -> Device -> Reset all and confirm"
	@echo " 2. Unplug device, press and hold the right button, plug-in again"
	@echo " 3. Navigate to the main menu"
	@echo "If everything was correct, no PIN needs to be entered."

# This target will initialize the device with the integration testing mnemonic
.PHONY: dev_init
dev_init: show_info_recovery_mode
	@echo "Initializing device with test mnemonic! WARNING TAKES 2 MINUTES AND REQUIRES RECOVERY MODE"
	@python -m ledgerblue.hostOnboard --apdu --id 0 --prefix "" --passphrase "" --pin 5555 --words "equip will roof matter pink blind book anxiety banner elbow sun young"

# This target will initialize the device with the secondary integration testing mnemonic (Bob)
.PHONY: dev_init_secondary
dev_init_secondary: check_python show_info_recovery_mode
	@echo "Initializing device with secondary test mnemonic! WARNING TAKES 2 MINUTES AND REQUIRES RECOVERY MODE"
	@python -m ledgerblue.hostOnboard --apdu --id 0 --prefix "" --passphrase "" --pin 5555 --words "elite vote proof agree february step sibling sand grocery axis false cup"

# This target will setup a custom developer certificate
.PHONY: dev_ca
dev_ca: check_python
	@python -m ledgerblue.setupCustomCA --targetId 0x31100004 --public $(SCP_PUBKEY) --name zondax

.PHONY: dev_ca_delete
dev_ca_delete: check_python
	@python -m ledgerblue.resetCustomCA --targetId 0x31100004

# This target will setup a custom developer certificate
.PHONY: dev_caS2
dev_caS2: check_python
	@python -m ledgerblue.setupCustomCA --targetId 0x33100004 --public $(SCP_PUBKEY) --name zondax

.PHONY: dev_ca_deleteS2
dev_ca_deleteS2: check_python
	@python -m ledgerblue.resetCustomCA --targetId 0x33100004

# TODO: verify that targetId is correct and if it works on a real device
# This target will setup a custom developer certificate
.PHONY: dev_caST
dev_caST: check_python
	@python -m ledgerblue.setupCustomCA --targetId 0x33200004 --public $(SCP_PUBKEY) --name zondax

.PHONY: dev_ca_deleteST
dev_ca_deleteST: check_python
	@python -m ledgerblue.resetCustomCA --targetId 0x33200004

# TODO: complete with Flex targetId
# This target will setup a custom developer certificate
.PHONY: dev_caFL
dev_caFL: check_python
	@python -m ledgerblue.setupCustomCA --targetId 0x33200004 --public $(SCP_PUBKEY) --name zondax

.PHONY: dev_ca_deleteFL
dev_ca_deleteFL: check_python
	@python -m ledgerblue.resetCustomCA --targetId 0x33200004

.PHONY: zemu_install_js_link
ifeq ($(TESTS_JS_DIR),)
zemu_install_js_link:
	@echo "No local package defined"
else
zemu_install_js_link:
	# First unlink everything
	cd $(TESTS_JS_DIR) && yarn unlink || true
	cd $(TESTS_ZEMU_DIR) && yarn unlink $(TESTS_JS_PACKAGE) || true
	# Now build and link
	cd $(TESTS_JS_DIR) && yarn install && yarn build && yarn link || true
	cd $(TESTS_ZEMU_DIR) && yarn link $(TESTS_JS_PACKAGE) && yarn install || true
	@echo
	# List linked packages
	@echo
	@cd $(TESTS_ZEMU_DIR) && ( ls -l node_modules ; ls -l node_modules/@* ) | grep ^l || true
	@echo
endif

.PHONY: zemu_install
zemu_install: zemu_install_js_link
	# and now install everything
	cd $(TESTS_ZEMU_DIR) && yarn install

.PHONY: zemu
zemu:
	cd $(TESTS_ZEMU_DIR)/tools && node debug.mjs $(COIN)

.PHONY: zemu_val
zemu_val:
	cd $(TESTS_ZEMU_DIR)/tools && node debug_val.mjs

.PHONY: zemu_debug
zemu_debug:
	cd $(TESTS_ZEMU_DIR)/tools && node debug.mjs $(COIN) debug

########################## TEST Section ###############################

.PHONY: zemu_test
zemu_test:
	cd $(TESTS_ZEMU_DIR) && yarn test$(COIN)

.PHONY: rust_test
rust_test:
	cd app/rust && cargo test

.PHONY: cpp_test
cpp_test:
	mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make
	cd build && GTEST_COLOR=1 ASAN_OPTIONS=detect_leaks=0 ctest -VV

########################## FUZZING Section ###############################

.PHONY: fuzz_build
fuzz_build:
	cmake -B build -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DENABLE_FUZZING=1 -DENABLE_SANITIZERS=1 .
	make -C build

.PHONY: fuzz
fuzz: fuzz_build
	./fuzz/run-fuzzers.py

.PHONY: fuzz_crash
fuzz_crash: FUZZ_LOGGING=1
fuzz_crash: fuzz_build
	./fuzz/run-fuzz-crashes.py

.PHONY: shell
shell:
	poetry install --no-root && poetry shell

ts_upgrade:
	if [ -d js ]; then cd js && bun run upgrade; fi
	if [ -d tests_zemu ]; then cd tests_zemu && bun run upgrade; fi

ts_format:
	if [ -d js ]; then cd js && bun run format; fi
	if [ -d tests_zemu ]; then cd tests_zemu && bun run format; fi

ts_lint:
	if [ -d js ]; then cd js && bun run lint; fi
	if [ -d tests_zemu ]; then cd tests_zemu && bun run lint; fi
