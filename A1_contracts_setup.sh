#!/bin/bash


NC='\033[0m'
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'


set -e


echo ""
echo ""
echo -e "${GREEN}=========================================================${NC}"
echo -e "${GREEN}Building PubSub contracts...${NC}"
make -C pubsub-onchain
echo -e "${GREEN}PubSub contracts built.${NC}"
echo -e "${GREEN}=========================================================${NC}"

echo ""
echo ""
echo -e "${GREEN}=========================================================${NC}"
echo -e "${GREEN}Building Revoker contracts...${NC}"
make -C revoker-onchain
echo -e "${GREEN}Revoker contracts built.${NC}"
echo -e "${GREEN}=========================================================${NC}"

echo ""
echo ""
echo -e "${GREEN}=========================================================${NC}"
echo -e "${GREEN}Building End2EndLatency tests contracts...${NC}"
make -C Revoker/tests/End2EndLatency/contracts/
echo -e "${GREEN}End2EndLatency tests contracts built.${NC}"
echo -e "${GREEN}=========================================================${NC}"


echo ""
echo ""
echo -e "${GREEN}=========================================================${NC}"
echo -e "${GREEN}Setting up Python virtual environment...${NC}"
if [ -d "venv" ]; then
	echo -e "${YELLOW}venv directory already exists. Skipping creation.${NC}"
else
	echo -e "${GREEN}Creating venv directory...${NC}"
	python3 -m venv venv
fi
./venv/bin/python3 -m pip install --upgrade pip
echo -e "${GREEN}Python virtual environment setup complete.${NC}"
echo -e "${GREEN}=========================================================${NC}"


echo ""
echo ""
echo -e "${GREEN}=========================================================${NC}"
echo -e "${GREEN}Installing Python requirements for PubSub contracts gas eval...${NC}"
./venv/bin/python3 -m pip install --requirement pubsub-onchain/utils/gas_cost_eval_requirements.txt
echo -e "${GREEN}Python requirements for PubSub contracts gas eval installed.${NC}"
echo -e "${GREEN}=========================================================${NC}"


echo ""
echo ""
echo -e "${GREEN}=========================================================${NC}"
echo -e "${GREEN}Installing Python requirements for Revoker contracts gas eval...${NC}"
./venv/bin/python3 -m pip install --requirement revoker-onchain/utils/gas_cost_eval_requirements.txt
echo -e "${GREEN}Python requirements for Revoker contracts gas eval installed.${NC}"
echo -e "${GREEN}=========================================================${NC}"


echo ""
echo ""
echo -e "${GREEN}####################${NC}"
echo -e "${GREEN}Contracts setup complete.${NC}"
echo -e "${GREEN}####################${NC}"
echo ""
