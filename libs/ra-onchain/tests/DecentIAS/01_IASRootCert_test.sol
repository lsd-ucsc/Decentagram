// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";

// This import is required to use custom transaction context
// Although it may fail compilation in 'Solidity Compiler' plugin
// But it will work fine in 'Solidity Unit Testing' plugin
import "remix_accounts.sol";


import {IASRootCert} from "../../contracts/IASRootCert.sol";
import {Names} from "../../contracts/Constants.sol";

import {TestCerts} from "../TestCerts.sol";


contract IASRootCert_testSuit {

    using IASRootCert for IASRootCert.IASRootCertObj;

    /// 'beforeAll' runs before all other tests
    /// More special functions are: 'beforeEach', 'beforeAll', 'afterEach' & 'afterAll'
    function beforeAll() public {
    }

    function constantTest() public {
        Assert.equal(
            keccak256(bytes(Names.IAS_ROOT_CERT_CN)),
            keccak256(bytes(TestCerts.IAS_ROOT_CERT_NAME_CN)),
            "IAS_ROOT_CERT_CN not match"
        );
        Assert.equal(
            Names.HASH_IAS_ROOT_CERT_CN,
            keccak256(bytes(TestCerts.IAS_ROOT_CERT_NAME_CN)),
            "HASH_IAS_ROOT_CERT_CN not match"
        );
    }

    function loadCertTest() public {
        {
            IASRootCert.IASRootCertObj memory cert;
            cert.loadCert(TestCerts.IAS_ROOT_CERT_DER);

            Assert.lesserThan(block.timestamp, cert.notAfter, "cert expired");

            Assert.equal(
                keccak256(cert.pubKeyMod),
                keccak256(TestCerts.IAS_ROOT_CERT_KEY_MOD),
                "pubKey mod not match"
            );
            Assert.equal(
                keccak256(cert.pubKeyExp),
                keccak256(TestCerts.IAS_ROOT_CERT_KEY_EXP),
                "pubKey exp not match"
            );
        }

        {
            IASRootCert.IASRootCertObj memory cert =
                IASRootCert.loadCert(TestCerts.IAS_ROOT_CERT_DER);

            Assert.lesserThan(block.timestamp, cert.notAfter, "cert expired");

            Assert.equal(
                keccak256(cert.pubKeyMod),
                keccak256(TestCerts.IAS_ROOT_CERT_KEY_MOD),
                "pubKey mod not match"
            );
            Assert.equal(
                keccak256(cert.pubKeyExp),
                keccak256(TestCerts.IAS_ROOT_CERT_KEY_EXP),
                "pubKey exp not match"
            );
        }
    }
}
