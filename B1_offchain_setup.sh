#!/bin/bash


set -e


NC='\033[0m'
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'


echo ""
echo ""
echo -e "${GREEN}=========================================================${NC}"
echo -e "${GREEN}Building off-chain broker${NC}"
cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release -B Ethereum/build -S Ethereum
cmake --build Ethereum/build --config Release
echo -e "${GREEN}off-chain broker build complete.${NC}"
echo -e "${GREEN}=========================================================${NC}"


echo ""
echo ""
echo -e "${GREEN}=========================================================${NC}"
echo -e "${GREEN}Building off-chain revoker${NC}"
cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release -B Revoker/build -S Revoker
cmake --build Revoker/build --config Release
echo -e "${GREEN}off-chain revoker build complete.${NC}"
echo -e "${GREEN}=========================================================${NC}"
