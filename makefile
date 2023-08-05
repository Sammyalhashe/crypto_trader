BUILD_DIR := build
MAKE := make
MAKE_OPTS := -j16
BUILD_TYPE := Debug
CMAKE_OPTS := -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
EXE := crypto_trader

.PHONY: run
run: build
	./${BUILD_DIR}/${EXE}

.PHONY: build
build: prepare
	pushd ${BUILD_DIR} && cmake --build . && popd

.PHONY: prepare
prepare: conan
	pushd ${BUILD_DIR} && cmake .. ${CMAKE_OPTS} && ln -f compile_commands.json .. && popd

.PHONY: conan
conan: build_dir_prep
	conan install . --output-folder=${BUILD_DIR} --build=missing -s build_type=${BUILD_TYPE}

.PHONY: build_dir_prep
build_dir_prep:
	mkdir -p ${BUILD_DIR}

.PHONY: clean
clean:
	rm -rf ${BUILD_DIR}

