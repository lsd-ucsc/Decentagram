// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";

// This import is required to use custom transaction context
// Although it may fail compilation in 'Solidity Compiler' plugin
// But it will work fine in 'Solidity Unit Testing' plugin
import "remix_accounts.sol";


import {BytesUtils} from "../../libs/ens-contracts/BytesUtils.sol";
import {RSAVerify} from "../../libs/ens-contracts/RSAVerify.sol";

import {LibRsaSha256_proxy} from "./01_LibRsaSha256.sol";
import {TestCerts} from "../TestCerts.sol";


// File name has to end with '_test.sol', this file can contain more than one testSuite contracts
contract LibRsaSha256_testSuite {

    using BytesUtils for bytes;

    //===== member variables =====

    address m_testProxyAddr;

    //===== functions =====

    /// 'beforeAll' runs before all other tests
    /// More special functions are: 'beforeEach', 'beforeAll', 'afterEach' & 'afterAll'
    function beforeAll() public {
        m_testProxyAddr = address(new LibRsaSha256_proxy());
    }

    function extractComponentsTest() public {
        try LibRsaSha256_proxy(m_testProxyAddr).extractComponentsTest() {
            Assert.ok(true, "extractComponentsTest should pass");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch {
            Assert.ok(false, "unexpected error - extractComponentsTest");
        }
    }

    function verifyWithComponentsTest() public {
        try LibRsaSha256_proxy(m_testProxyAddr).verifyWithComponentsTest() {
            Assert.ok(true, "verifyWithComponentsTest should pass");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch {
            Assert.ok(false, "unexpected error - verifyWithComponentsTest");
        }
    }

    function verifySignMsgTest() public {
        try LibRsaSha256_proxy(m_testProxyAddr).verifySignMsgTest() {
            Assert.ok(true, "verifySignMsgTest should pass");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch {
            Assert.ok(false, "unexpected error - verifySignMsgTest");
        }
    }

    function rsarecoverGasEval() public {
        // 3072 bits
        {
            bytes memory mod = TestCerts.IAS_ROOT_CERT_KEY_MOD;
            bytes memory exp = TestCerts.IAS_ROOT_CERT_KEY_EXP;
            bytes32 msgHash = TestCerts.IAS_ROOT_CERT_HASH;
            bytes memory sign = TestCerts.IAS_ROOT_CERT_SIGN;

            bytes memory actHash;
            uint256 gasUsed = gasleft();
            actHash = RSAVerify.rsarecover(mod, exp, sign);
            gasUsed = gasUsed - gasleft();

            Assert.equal(
                actHash.readBytes32(actHash.length - 32),
                msgHash,
                "hash not match"
            );
            // Assert.equal(gasUsed, 15064, "rsarecover gasUsed");
        }

        // 2048 bits
        {
            bytes memory mod = TestCerts.IAS_REPORT_CERT_KEY_MOD;
            bytes memory exp = TestCerts.IAS_REPORT_CERT_KEY_EXP;
            bytes32 msgHash = sha256(TestCerts.DECENT_SVR_CERT_ATT_REP_JSON);
            bytes memory sign = TestCerts.DECENT_SVR_CERT_ATT_REP_SIGN;

            bytes memory actHash;
            uint256 gasUsed = gasleft();
            actHash = RSAVerify.rsarecover(mod, exp, sign);
            gasUsed = gasUsed - gasleft();

            Assert.equal(
                actHash.readBytes32(actHash.length - 32),
                msgHash,
                "hash not match"
            );
            // Assert.equal(gasUsed, 7555, "rsarecover gasUsed");
        }
    }

}
