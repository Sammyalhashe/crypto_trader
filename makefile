BUILD_DIR := build
MAKE := make
MAKE_OPTS := -j16
BUILD_TYPE := Debug
CMAKE_OPTS := -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
EXE := crypto_trader

RUN_CMD:=./${BUILD_DIR}/${EXE}

.PHONY: run
run: build
	${RUN_CMD}

.PHONY: ro
ro:
	${RUN_CMD}

BUILD_CMD:=cd ${BUILD_DIR} && cmake --build . && cd -

.PHONY: build
build: prepare
	${BUILD_CMD}
	
.PHONY: bo
bo:
	${BUILD_CMD}

.PHONY: prepare
prepare: conan
	cd ${BUILD_DIR} && cmake .. ${CMAKE_OPTS} && ln -f compile_commands.json .. && cd -

.PHONY: conan
conan: build_dir_prep
	conan install . --output-folder=${BUILD_DIR} --build=missing -s build_type=${BUILD_TYPE}

.PHONY: build_dir_prep
build_dir_prep:
	mkdir -p ${BUILD_DIR}

.PHONY: clean
clean:
	rm -rf ${BUILD_DIR}

.PHONY: test
test:
	cd ${BUILD_DIR} && ctest && cd -
