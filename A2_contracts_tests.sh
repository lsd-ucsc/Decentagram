#!/bin/bash


NC='\033[0m'
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'


set -e

echo ""
echo ""
echo -e "${GREEN}=========================================================${NC}"
echo -e "${GREEN}PubSub contracts gas evaluation...${NC}"
pushd pubsub-onchain
	echo -e "${YELLOW}Running GasCostEvalMultiSubs.py...${NC}"
	../venv/bin/python3 tests/GasCostEvalMultiSubs.py
	echo -e "${YELLOW}Running GasCostEvalMultiPubs.py...${NC}"
	../venv/bin/python3 tests/GasCostEvalMultiPubs.py
	echo -e "${YELLOW}Running GasCostEvalDeploy.py...${NC}"
	../venv/bin/python3 tests/GasCostEvalDeploy.py
	echo -e "${YELLOW}Running GasCostEvalPlot.py...${NC}"
	../venv/bin/python3 tests/GasCostEvalPlot.py
popd
echo -e "${GREEN}PubSub contracts gas evaluation complete.${NC}"
echo -e "${GREEN}=========================================================${NC}"


echo -e "${GREEN}=========================================================${NC}"
echo -e "${GREEN}Revoker contracts gas evaluation...${NC}"
pushd revoker-onchain
	echo -e "${YELLOW}Running GasEvalEnclaveRevoker.py...${NC}"
	../venv/bin/python3 utils/GasEvalEnclaveRevoker.py
	echo -e "${YELLOW}Running GasEvalKeyRevoker.py...${NC}"
	../venv/bin/python3 utils/GasEvalKeyRevoker.py
popd
echo -e "${GREEN}Revoker contracts gas evaluation complete.${NC}"
echo -e "${GREEN}=========================================================${NC}"
