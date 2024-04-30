// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";

// This import is required to use custom transaction context
// Although it may fail compilation in 'Solidity Compiler' plugin
// But it will work fine in 'Solidity Unit Testing' plugin
import "remix_accounts.sol";


import {RLPReader} from "../../libs/Solidity-RLP/contracts/RLPReader.sol";

import {TestCerts} from "../TestCerts.sol";


contract RLPTest {

    using RLPReader for RLPReader.RLPItem;

    function beforeAll () public {
    }

    function rlpTest1() public {
        uint i = 1337;
        bytes memory rlpBytes = abi.encodePacked(i);
        RLPReader.RLPItem memory item = RLPReader.toRlpItem(rlpBytes);
        uint val = RLPReader.toUint(item);

        Assert.equal(val, uint(1337), "rlp value not equal");
    }

    function decentAttestRepSetTest() public {
        bytes memory rlpBytes = TestCerts.DECENT_SVR_CERT_ATT_REP_RLP;

        bytes memory iasRepCert = TestCerts.IAS_REPORT_CERT_DER;
        bytes memory iasRep = TestCerts.DECENT_SVR_CERT_ATT_REP_JSON;
        bytes memory iasSig = TestCerts.DECENT_SVR_CERT_ATT_REP_SIGN;

        RLPReader.RLPItem[] memory rep = RLPReader.toRlpItem(rlpBytes).toList();

        RLPReader.RLPItem[] memory repCerts = rep[0].toList();
        Assert.equal(repCerts.length, 1, "wrong number of items");

        bytes memory repCertDer = repCerts[0].toBytes();
        Assert.equal(
            keccak256(repCertDer),
            keccak256(iasRepCert),
            "wrong IAS report cert DER"
        );

        bytes memory repJson = rep[1].toBytes();
        Assert.equal(
            keccak256(repJson),
            keccak256(iasRep),
            "wrong IAS report JSON"
        );

        bytes memory repSig = rep[2].toBytes();
        Assert.equal(
            keccak256(repSig),
            keccak256(iasSig),
            "wrong IAS report signature"
        );
    }

}
