num_cpus = $(shell grep -c "^processor" /proc/cpuinfo)
num_threads = $(shell echo $$(($(num_cpus) + 1)))

all: build-html

.PHONY: build
build: assimp bullet
	mkdir -p build \
	&& cd build \
	&& cmake \
		-DGLM_ROOT_DIR=./extern/glm \
		-DBULLET_ROOT=./extern/bullet3/install \
		.. \
	&& make -j $(num_threads)

.PHONY: build-html
build-html: assimp-js bullet-js
	mkdir -p build-html install-html \
	&& cd build-html \
	&& emconfigure cmake \
		-DGLM_ROOT_DIR=./extern/glm \
		-DCMAKE_INSTALL_PREFIX=../install-html \
    -DEMSCRIPTEN_ENABLED=ON \
		.. \
	&& emmake make -j $(num_threads) \
	&& emmake make install

.PHONY: assimp
assimp:
	cd extern \
	&& mkdir -p assimp-build assimp-install \
	&& cd assimp-build \
	&& cmake \
		-DCMAKE_BUILD_TYPE=debug \
		-DCMAKE_INSTALL_PREFIX=../assimp-install \
		-DASSIMP_BUILD_TESTS=OFF \
		../assimp \
	&& make -j $(num_threads) \
	&& make install

.PHONY: assimp-js
assimp-js:
	cd extern \
	&& mkdir -p assimp-build-js assimp-install-js \
	&& cd assimp-build-js \
	&& emconfigure cmake \
		-DCMAKE_BUILD_TYPE=release \
		-DCMAKE_INSTALL_PREFIX=../assimp-install-js \
		-DASSIMP_BUILD_TESTS=OFF \
		../assimp \
	&& emmake make -j $(num_threads) all install

.PHONY: bullet
bullet:
	cd extern \
	&& mkdir -p bullet3-build bullet3-install \
	&& cd bullet3-build \
	&& cmake \
		-DCMAKE_BUILD_TYPE=debug \
		-DBUILD_DEMOS=OFF \
		-DBUILD_UNIT_TESTS=OFF \
		-DBUILD_CPU_DEMOS=OFF \
		-DBUILD_BULLET2_DEMOS=OFF \
		-DBUILD_EXTRAS=OFF \
		-DCMAKE_INSTALL_PREFIX=../bullet3-install \
		../bullet3 \
	&& make -j $(num_threads) \
	&& make install

.PHONY: bullet-js
bullet-js:
	cd extern \
	&& mkdir -p bullet3-build-js bullet3-install-js \
	&& cd bullet3-build-js \
	&& emconfigure cmake \
		-DCMAKE_BUILD_TYPE=debug \
		-DBUILD_DEMOS=OFF \
		-DBUILD_UNIT_TESTS=OFF \
		-DBUILD_CPU_DEMOS=OFF \
		-DBUILD_BULLET2_DEMOS=OFF \
		-DBUILD_EXTRAS=OFF \
		-DCMAKE_INSTALL_PREFIX=../bullet3-install-js \
		../bullet3 \
	&& emmake make -j $(num_threads) \
	&& emmake make install

# .PHONY: lcms-js
# lcms-js:
# 	cd extern/bullet3 \
# 	&& mkdir -p build-js install-js \
# 	&& cd build-js \
# 	&& emconfigure cmake \
# 		-DCMAKE_BUILD_TYPE=debug \
# 		-DBUILD_DEMOS=OFF \
# 		-DBUILD_UNIT_TESTS=OFF \
# 		-DCMAKE_INSTALL_PREFIX=../install-js \
# 		.. \
# 	&& emmake make -j $(num_threads) \
# 	&& emmake make install

.PHONY: clean
clean:
	rm -rf extern/*{build,install}
	rm -rf extern/*{build,install}-js
	rm -rf build
	rm -rf {build,install}-html

