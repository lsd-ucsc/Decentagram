// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";


import {Asn1Decode} from "../../libs/asn1-decode/Asn1Decode.sol";

import {LibSecp256k1Sha256} from "../../contracts/LibSecp256k1Sha256.sol";
import {OIDs} from "../../contracts/Constants.sol";
import {TestCerts} from "../TestCerts.sol";
import {X509CertNodes} from "../../contracts/X509CertNodes.sol";


contract LibSecp256k1Sha256_proxy {

    using Asn1Decode for bytes;

    using LibSecp256k1Sha256 for X509CertNodes.CertNodesObj;
    using LibSecp256k1Sha256 for X509CertNodes.CertTbsNodesObj;
    using X509CertNodes for X509CertNodes.CertNodesObj;

    bytes constant TEST_ECKEY_01_BYTES =
        hex"2c72f7570678570abbffc5f50b16061715cacdb156638e7130c3a3ad315849fc"
        hex"d5c7be532bb1f6d2b000108c99eeb76122c2ab2bb4b337f29693adf9b946bb44";
    address constant TEST_ECKEY_01_ADDR =
        address(uint160(uint256(keccak256(TEST_ECKEY_01_BYTES))));

    bytes constant TEST_ECKEY_02_BYTES =
        hex"c6c60341a8b295cf04380fb98ac13e4222d7deaeefccfcd98c4ddbff95f21370"
        hex"723b538b0283831a1b0e289ab6c2cd49715f2abc9875755887cc43dc5daeaaa6";
    address constant TEST_ECKEY_02_ADDR =
        address(uint160(uint256(keccak256(TEST_ECKEY_02_BYTES))));

    bytes constant TEST_ECKEY_03_BYTES =
        hex"cb1dc12b332ea373d75496dc608c2d5b0f37ee4fbccd5a602b143c228d15047c"
        hex"4f6b6cbb158c4600d84031274376059cf607713d299080bc09d55d9c81f31157";
    address constant TEST_ECKEY_03_ADDR =
        address(uint160(uint256(keccak256(TEST_ECKEY_03_BYTES))));

    function extractPubKeyAlgTest() public {
        bytes memory certDer = TestCerts.DECENT_SVR_CERT_DER;

        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(certDer);

        (bytes32 keyId, bytes32 curveId) =
            certNodes.tbs.extractPubKeyAlg(certDer);

        Assert.equal(
            keyId,
            OIDs.OID_KEY_EC_PUBLIC,
            "keyId!=OID_KEY_EC_PUBLIC"
        );
        Assert.equal(
            curveId,
            OIDs.OID_KEY_EC_SECP256K1,
            "curveId!=OID_KEY_EC_SECP256K1"
        );
    }

    function extractSignRSTest() public {
        bytes memory certDer = TestCerts.DECENT_SVR_CERT_DER;

        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(certDer);

        (bytes32 r, bytes32 s) =
            certNodes.extractSignRS(certDer);

        Assert.equal(
            r,
            TestCerts.DECENT_SVR_CERT_SIGN_R,
            "r!=DECENT_SVR_CERT_SIGN_R"
        );
        Assert.equal(
            s,
            TestCerts.DECENT_SVR_CERT_SIGN_S,
            "s!=DECENT_SVR_CERT_SIGN_S"
        );
    }

    function extractPubKeyBytesTest() public {
        bytes memory certDer = TestCerts.DECENT_SVR_CERT_DER;

        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(certDer);

        bytes memory pubKeyBytes =
            certNodes.tbs.extractPubKeyBytes(certDer);

        Assert.equal(
            pubKeyBytes.length,
            64,
            "pubKeyBytes.length!=64"
        );
        Assert.equal(
            keccak256(pubKeyBytes),
            keccak256(TestCerts.DECENT_SVR_CERT_KEY_BYTES),
            "pubKeyBytes!=DECENT_SVR_CERT_KEY_BYTES"
        );
    }

    function pubKeyBytesToAddrTest() public {
        bytes memory certDer = TestCerts.DECENT_SVR_CERT_DER;

        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(certDer);

        bytes memory pubKeyBytes =
            certNodes.tbs.extractPubKeyBytes(certDer);

        address addr =
            LibSecp256k1Sha256.pubKeyBytesToAddr(pubKeyBytes);

        Assert.equal(
            addr, TestCerts.DECENT_SVR_CERT_KEY_ADDR,
            "addr!=DECENT_SVR_CERT_KEY_ADDR"
        );

        // Test EC key 01
        addr = LibSecp256k1Sha256.pubKeyBytesToAddr(TEST_ECKEY_01_BYTES);
        Assert.equal(addr, TEST_ECKEY_01_ADDR, "addr!=TEST_ECKEY_01_ADDR");

        // Test EC key 02
        addr = LibSecp256k1Sha256.pubKeyBytesToAddr(TEST_ECKEY_02_BYTES);
        Assert.equal(addr, TEST_ECKEY_02_ADDR, "addr!=TEST_ECKEY_02_ADDR");

        // Test EC key 03
        addr = LibSecp256k1Sha256.pubKeyBytesToAddr(TEST_ECKEY_03_BYTES);
        Assert.equal(addr, TEST_ECKEY_03_ADDR, "addr!=TEST_ECKEY_03_ADDR");
    }

    function verifySignMsgTest() public {
        // OK
        {
            bytes memory certDer = TestCerts.DECENT_SVR_CERT_DER;

            X509CertNodes.CertNodesObj memory certNodes;
            certNodes.loadCertNodes(certDer);

            address addr = TestCerts.DECENT_SVR_CERT_KEY_ADDR;
            bytes32 r = TestCerts.DECENT_SVR_CERT_SIGN_R;
            bytes32 s = TestCerts.DECENT_SVR_CERT_SIGN_S;

            bytes memory tbsBytes = certDer.allBytesAt(certNodes.tbs.root);
            bytes32 tbsHash = sha256(tbsBytes);

            Assert.equal(
                ecrecover(tbsHash, 28, r, s),
                addr,
                "ecrecover()!=addr"
            );
            Assert.ok(
                LibSecp256k1Sha256.verifySignMsg(addr, tbsBytes, r, s),
                "verifySignMsg!=true"
            );
            Assert.ok(
                LibSecp256k1Sha256.verifySignHash(addr, tbsHash, r, s),
                "verifySignHash!=true"
            );
        }

        // OK
        {
            bytes memory certDer = TestCerts.DECENT_APP_CERT_DER;

            X509CertNodes.CertNodesObj memory certNodes;
            certNodes.loadCertNodes(certDer);

            address addr = TestCerts.DECENT_SVR_CERT_KEY_ADDR;
            bytes32 r = TestCerts.DECENT_APP_CERT_SIGN_R;
            bytes32 s = TestCerts.DECENT_APP_CERT_SIGN_S;

            bytes memory tbsBytes = certDer.allBytesAt(certNodes.tbs.root);
            bytes32 tbsHash = sha256(tbsBytes);

            Assert.equal(
                ecrecover(tbsHash, 27, r, s),
                addr,
                "ecrecover()!=addr"
            );
            Assert.ok(
                LibSecp256k1Sha256.verifySignMsg(addr, tbsBytes, r, s),
                "verifySignMsg!=true"
            );
            Assert.ok(
                LibSecp256k1Sha256.verifySignHash(addr, tbsHash, r, s),
                "verifySignHash!=true"
            );
        }

        // should fail
        {
            bytes memory certDer = TestCerts.DECENT_APP_CERT_DER;

            X509CertNodes.CertNodesObj memory certNodes;
            certNodes.loadCertNodes(certDer);

            address addr = TestCerts.DECENT_APP_CERT_KEY_ADDR;
            bytes32 r = TestCerts.DECENT_APP_CERT_SIGN_R;
            bytes32 s = TestCerts.DECENT_APP_CERT_SIGN_S;

            bytes memory tbsBytes = certDer.allBytesAt(certNodes.tbs.root);
            bytes32 tbsHash = sha256(tbsBytes);

            Assert.ok(
                !LibSecp256k1Sha256.verifySignMsg(addr, tbsBytes, r, s),
                "verifySignMsg==true"
            );
            Assert.ok(
                !LibSecp256k1Sha256.verifySignHash(addr, tbsHash, r, s),
                "verifySignHash==true"
            );
        }
    }

    function verifySignHashGasEval() public {
        {
            address addr = TestCerts.DECENT_SVR_CERT_KEY_ADDR;

            bytes32 r = TestCerts.DECENT_SVR_CERT_SIGN_R;
            bytes32 s = TestCerts.DECENT_SVR_CERT_SIGN_S;
            bytes32 tbsHash = TestCerts.DECENT_SVR_CERT_HASH;
            // recovery ID is 28

            bool res;
            uint256 gasUsed = gasleft();
            res = LibSecp256k1Sha256.verifySignHash(addr, tbsHash, r, s);
            gasUsed = gasUsed - gasleft();
            Assert.ok(res, "verifySignHash!=true");
            // Assert.equal(gasUsed, 6796, "gasUsed");
        }

        {
            address addr = TestCerts.DECENT_SVR_CERT_KEY_ADDR;

            bytes32 r = TestCerts.DECENT_APP_CERT_SIGN_R;
            bytes32 s = TestCerts.DECENT_APP_CERT_SIGN_S;
            bytes32 tbsHash = TestCerts.DECENT_APP_CERT_HASH;
            // recovery ID is 27

            bool res;
            uint256 gasUsed = gasleft();
            res = LibSecp256k1Sha256.verifySignHash(addr, tbsHash, r, s);
            gasUsed = gasUsed - gasleft();
            Assert.ok(res, "verifySignHash!=true");
            // Assert.equal(gasUsed, 3431, "gasUsed");
        }
    }

}
