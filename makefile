BUILD_DIR := build
MAKE := make
MAKE_OPTS := -j16
EXE := crypto_trader

.PHONY: run
run: build
	./${BUILD_DIR}/${EXE}

.PHONY: build
build: prepare
	pushd ${BUILD_DIR} && ${MAKE} ${MAKE_OPTS} && popd

.PHONY: prepare
prepare:
	mkdir -p ${BUILD_DIR} && pushd ${BUILD_DIR} && cmake .. && popd

.PHONY: clean
clean:
	rm -rf ${BUILD_DIR}

