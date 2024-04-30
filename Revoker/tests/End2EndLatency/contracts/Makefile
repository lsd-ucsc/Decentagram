SOLC_VERSION   := v0.8.21
MKFILE_PATH    := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR    := $(dir $(MKFILE_PATH))
SOLC_BIN       := $(CURRENT_DIR)/build/solc-static-linux
BUILD_DIR      := $(CURRENT_DIR)/build

SOLC_BIN     := $(BUILD_DIR)/solc-static-linux
OPTIMIZE_RUN := 200
SOLC_FLAGS   := --optimize --optimize-runs $(OPTIMIZE_RUN) \
				--revert-strings strip \
				--via-ir \
				--overwrite \
				--base-path $(CURRENT_DIR) \
				--output-dir $(BUILD_DIR)/

CONTRACTS   := \
	HybridSubscriber \
	Oracle


all: $(SOLC_BIN) $(CONTRACTS)


$(SOLC_BIN):
	mkdir -p $(dir $(SOLC_BIN)) && \
	curl -fsSL -o $(SOLC_BIN) \
		https://github.com/ethereum/solidity/releases/download/$(SOLC_VERSION)/solc-static-linux \
		&& \
	chmod +x $(SOLC_BIN)


solc_bin: $(SOLC_BIN)

$(BUILD_DIR)/%.bin: %.sol $(SOLC_BIN)
	( \
		$(SOLC_BIN) --bin $(SOLC_FLAGS) $< \
	)


$(BUILD_DIR)/%.abi: %.sol $(SOLC_BIN)
	( \
		$(SOLC_BIN) --abi $(SOLC_FLAGS) $< \
	)


$(CONTRACTS): %: $(BUILD_DIR)/%.abi $(BUILD_DIR)/%.bin

