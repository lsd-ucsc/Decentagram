# Off-chain Revocation List Monitor

## Requirements

- The code has been built and tested on Ubuntu 22.04 LTS
  - Including `build-essential` and `cmake` packages
- The following requirements for Enclave development are also needed:
  - Intel SGX SDK
  - Intel SGX Driver
  - Intel SGX PSW, including
    - libsgx-enclave-common
    - libsgx-enclave-common-dbgsym
    - libsgx-urts
    - libsgx-urts-dbgsym
    - libsgx-epid
    - libsgx-uae-service

## How to build

```sh
cmake -B build -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
