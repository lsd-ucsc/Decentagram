// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";


import {LibRsaSha256} from "../../libs/sig-verify-algs/LibRsaSha256.sol";
import {TestCerts} from "../TestCerts.sol";


contract LibRsaSha256_proxy {

    function extractComponentsTest() external {
        (bytes memory mod, bytes memory exp) =
            LibRsaSha256.extractKeyComponents(TestCerts.IAS_ROOT_CERT_KEY_DER);

        Assert.equal(
            keccak256(TestCerts.IAS_ROOT_CERT_KEY_MOD),
            keccak256(mod),
            "modulus not equal"
        );
        Assert.equal(
            keccak256(TestCerts.IAS_ROOT_CERT_KEY_EXP),
            keccak256(exp),
            "exponent not equal"
        );
    }

    function verifyWithComponentsTest() external {
        bool result =
            LibRsaSha256.verifyWithComponents(
                TestCerts.IAS_ROOT_CERT_KEY_MOD,
                TestCerts.IAS_ROOT_CERT_KEY_EXP,
                TestCerts.IAS_ROOT_CERT_HASH,
                TestCerts.IAS_ROOT_CERT_SIGN
            );

        Assert.ok(result, "signature not verified");
    }

    function verifySignMsgTest() external {
        bool result =
            LibRsaSha256.verifySignMsg(
                TestCerts.IAS_ROOT_CERT_KEY_DER,
                TestCerts.IAS_ROOT_CERT_TBS,
                TestCerts.IAS_ROOT_CERT_SIGN
            );

        Assert.ok(result, "signature not verified");
    }
}
