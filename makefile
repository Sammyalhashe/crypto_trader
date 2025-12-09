SUFFIX=
ifneq ($(strip $(SANITIZE)),)
	ifeq ($(strip $(SANITIZE)), $(filter $(strip $(SANITIZE)), thread address ub))
		SUFFIX=_$(SANITIZE)
	endif

	ifneq ($(strip $(SANITIZE)), $(filter $(strip $(SANITIZE)), thread address ub))
        $(error SANITIZE either needs to be thread, address, or ub)
	endif

endif

ifeq ($(strip $(CLANG_FORMAT)),)
	CLANG_FORMAT=clang-format
endif

G=
ifneq ($(strip $(GENERATOR)),)
	G=-G $(GENERATOR)
endif

BUILD_DIR_PREFIX=cmake.bld
BUILD_DIR := $(BUILD_DIR_PREFIX)/$(shell uname)/full$(SUFFIX)

MAKE := make
MAKE_OPTS := -j16
BUILD_TYPE := Debug

sanitize_flag=
ifneq ($(strip $(SUFFIX)),)
	sanitize_flag=-Dcustom_build_type=$(SANITIZE)
endif
CMAKE_OPTS := -DCMAKE_BUILD_TYPE=${BUILD_TYPE} $(sanitize_flag) $(G)

# if a conan build add the toolchain
ifneq ($(strip $(CONAN)),)
	CMAKE_OPTS += -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
	CMAKE_OPTS += -DCONAN_BUILD=1
endif


EXE := crypto_trader

RUN_CMD:=./${BUILD_DIR}/${EXE}


all: build
	${BUILD_CMD}

.PHONY: run
run: build
	${RUN_CMD}

.PHONY: ro
ro:
	${RUN_CMD}

BUILD_CMD:=cmake --build ${BUILD_DIR}

.PHONY: build
build: prepare
	${BUILD_CMD}

.PHONY: bo
bo:
	${BUILD_CMD}

.PHONY: prepare
prepare: conan
	cd ${BUILD_DIR} && cmake ../../.. ${CMAKE_OPTS} && ln -f compile_commands.json ../../.. && cd - && cp config.json ${BUILD_DIR}

.PHONY: conan
conan: build_dir_prep
	@if [ -x "$(command -v conan)" ]; then\
		conan install . --output-folder=${BUILD_DIR} --build=missing -s build_type=${BUILD_TYPE};\
	fi

.PHONY: build_dir_prep
build_dir_prep:
	mkdir -p ${BUILD_DIR}

.PHONY: clean
clean:
	rm -rf ${BUILD_DIR_PREFIX}

.PHONY: test
test:
	cd ${BUILD_DIR} && ctest && cd -

.PHONY: format
format:
	find . -regex '.*\.\(cpp\|hpp\|cu\|c\|h\)' -exec ${CLANG_FORMAT} -style=file -i {} \;

.PHONY: docs
docs:
	cmake --build ${BUILD_DIR} --target doc_doxygen

.PHONY: serve-docs
serve-docs: docs
	@echo "Serving Doxygen documentation locally at http://localhost:8000"
	python3 -m http.server 8000 --directory ${BUILD_DIR}/doc_doxygen/html
