package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"math"
	"net/http"
	"os"
	"time"

	"github.com/ethereum/go-ethereum/common"
	"github.com/ethereum/go-ethereum/core/types"
	"github.com/ethereum/go-ethereum/rlp"
	"github.com/ethereum/go-ethereum/trie"
)

const (
	COMPONENT_CONFIG_PATH = "../geth-enclave-throughput-eval/components_config.json"
	START_BLOCK           = 8875000
	END_BLOCK             = 8880000
)

var (
	RECEIPT_RATE = [...]float64{
		0.00,
		0.10,
		0.20,
		0.30,
		0.40,
		0.50,
		0.60,
		0.70,
		0.80,
		0.90,
		1.00,
	}
	LOCAL_ENDPOINT = "http://localhost:8546"
)

type GethConfig struct {
	Protocol string `json:"Protocol"`
	Host     string `json:"Host"`
	Port     int    `json:"Port"`
}

type ComponentConfig struct {
	GethCfg GethConfig `json:"Geth"`
}

type PayloadBlock struct {
	Method  string `json:"method"`
	Params  []any  `json:"params"`
	ID      int    `json:"id"`
	Jsonrpc string `json:"jsonrpc"`
}

func PanicIfError(err error) {
	if err != nil {
		panic(err)
	}
}

func ReadComponentConfig() *ComponentConfig {
	// read config file
	configFile, err := os.Open(COMPONENT_CONFIG_PATH)
	PanicIfError(err)
	defer configFile.Close()

	// decode json config file
	var config ComponentConfig
	jsonParser := json.NewDecoder(configFile)
	err = jsonParser.Decode(&config)
	PanicIfError(err)

	return &config
}

func HexStrToBytes(hexStr string) []byte {
	hexStr = hexStr[2:]
	resBytes := common.FromHex(hexStr)

	return resBytes
}

func ExecuteRequest(requestBody *bytes.Reader) map[string]interface{} {
	req, _ := http.NewRequest("POST", LOCAL_ENDPOINT, requestBody)
	req.Header.Set("Content-Type", "application/json")
	resp, _ := http.DefaultClient.Do(req)

	// check if response is valid
	if resp.StatusCode != 200 {
		panic(fmt.Sprintf("Response status code: %d", resp.StatusCode))
	}
	defer resp.Body.Close()

	// decode response into a map to extract result
	var RlpResp map[string]interface{}
	err := json.NewDecoder(resp.Body).Decode(&RlpResp)
	PanicIfError(err)

	return RlpResp
}

func GenPayload(method string, blockNumHash string) *bytes.Reader {
	data := PayloadBlock{
		Method:  method,
		Params:  []any{blockNumHash},
		ID:      1,
		Jsonrpc: "2.0",
	}
	payloadBytes, _ := json.Marshal(data)
	payload := bytes.NewReader(payloadBytes)
	return payload
}

func GetRawHeader(blockNum uint64) []byte {
	payload := GenPayload("debug_getRawHeader", fmt.Sprintf("0x%x", blockNum))
	headerRlp := ExecuteRequest(payload)
	headerBytes := HexStrToBytes(headerRlp["result"].(string))

	return headerBytes
}

func GetHeader(blockNum uint64) *types.Header {
	headerRlp := GetRawHeader(blockNum)

	EthHeader := new(types.Header)

	// decode header rlp
	if err := rlp.Decode(bytes.NewReader(headerRlp), &EthHeader); err != nil {
		PanicIfError(err)
	}

	return EthHeader
}

func GetRawReceipts(blockNum uint64) []interface{} {
	payload := GenPayload("debug_getRawReceipts", fmt.Sprintf("0x%x", blockNum))
	receiptRlp := ExecuteRequest(payload)
	receiptsRlp := receiptRlp["result"].([]interface{})
	numReceipts := len(receiptsRlp)
	for i := 0; i < numReceipts; i++ {
		receiptsRlp[i] = HexStrToBytes(receiptsRlp[i].(string))
	}

	return receiptsRlp
}

func GetReceipts(blockNum uint64) []*types.Receipt {
	receiptsRlp := GetRawReceipts(blockNum)

	numReceipts := len(receiptsRlp)
	receipts := make([]*types.Receipt, numReceipts)

	for i := 0; i < numReceipts; i++ {
		receipt := new(types.Receipt)
		receiptsBytes := receiptsRlp[i].([]byte)

		if err := receipt.UnmarshalBinary(receiptsBytes); err != nil {
			PanicIfError(err)
		}

		receipts[i] = receipt
	}
	return receipts
}

func RunTest(
	startBlock uint64,
	endBlock uint64,
	receiptRate float64,
) {
	receiptLimit := uint8(math.MaxUint8 * receiptRate)
	var lastByte uint8
	var numReceiptsRead uint64

	startTime := time.Now()
	for blockNum := startBlock; blockNum < endBlock; blockNum++ {
		// get header
		header := GetHeader(blockNum)
		headerHash := header.Hash()

		// determine if we need to get receipts
		lastByte = headerHash[len(headerHash)-1]
		if lastByte < receiptLimit || receiptLimit == 255 {
			// get receipts
			receipts := GetReceipts(blockNum)

			// verify receipt root hash
			hasher := trie.NewStackTrie(nil)
			receiptRootHash := types.DeriveSha(types.Receipts(receipts), hasher)
			if receiptRootHash != header.ReceiptHash {
				panic("Receipt root hash mismatch")
			}

			// increment num receipts read
			numReceiptsRead++
		}
	}
	endTime := time.Now()
	elapsedTime := uint64(endTime.Sub(startTime).Seconds())
	numBlocks := endBlock - startBlock
	throughput := float64(numBlocks) / float64(elapsedTime)

	fmt.Println("Receipt %:  ", receiptRate*100, "%")
	fmt.Println("Pushed:     ", numBlocks, "blocks")
	fmt.Println("Took:       ", elapsedTime, "seconds")
	fmt.Println("Throughput: ", throughput, "blocks / second")
	fmt.Println("Read:       ", numReceiptsRead, "receipts")
}

func main() {
	// read config file
	config := ReadComponentConfig()

	// set local endpoint
	localEndpoint := fmt.Sprintf(
		"%s://%s:%d",
		config.GethCfg.Protocol,
		config.GethCfg.Host,
		config.GethCfg.Port,
	)
	LOCAL_ENDPOINT = localEndpoint
	fmt.Println("Local endpoint: ", LOCAL_ENDPOINT)

	for _, receiptRate := range RECEIPT_RATE {
		RunTest(
			START_BLOCK,
			END_BLOCK,
			receiptRate,
		)
	}
}
