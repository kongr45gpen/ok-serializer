FROM buildpack-deps:bookworm

WORKDIR /opt

RUN apt-get update; \
    apt-get install --yes --no-install-recommends binutils cmake make python3

RUN git clone https://github.com/kongr45gpen/llvm-project.git --depth 1 --branch reflection-17.0.4-1 --single-branch

WORKDIR /opt/llvm-project

RUN cmake -S llvm -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_INCLUDE_BENCHMARKS=Off

RUN cmake --build build -- -j $(nproc)

RUN cmake --install build



