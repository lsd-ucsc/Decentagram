// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";

// This import is required to use custom transaction context
// Although it may fail compilation in 'Solidity Compiler' plugin
// But it will work fine in 'Solidity Unit Testing' plugin
import "remix_accounts.sol";


import {IASReportCert} from "../../contracts/IASReportCert.sol";
import {X509CertNodes} from "../../contracts/X509CertNodes.sol";

import {TestCerts} from "../TestCerts.sol";
import {IASReportCert_proxy} from "./03_IASReportCert.sol";


// File name has to end with '_test.sol', this file can contain more than one testSuite contracts
contract IASReportCert_testSuit {

    using X509CertNodes for X509CertNodes.CertNodesObj;

    /// 'beforeAll' runs before all other tests
    /// More special functions are: 'beforeEach', 'beforeAll', 'afterEach' & 'afterAll'
    function beforeAll() public {
    }

    function extractReportKeyIdFromCertTest() public {
        X509CertNodes.CertNodesObj memory certNodes;

        bytes32 keyId = IASReportCert.extractReportKeyIdFromCert(
            certNodes,
            TestCerts.IAS_REPORT_CERT_DER
        );

        Assert.equal(certNodes.tbs.version, 2, "cert nodes not popluated");
        Assert.equal(
            keyId,
            keccak256(TestCerts.IAS_REPORT_CERT_KEY_DER),
            "keyId not match"
        );
    }

    function loadCertTest() public {
        bytes memory reportCertDer = TestCerts.IAS_REPORT_CERT_DER;

        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(reportCertDer);

        IASReportCert_proxy proxy = new IASReportCert_proxy();
        IASReportCert.IASReportCertObj memory cert = proxy.loadCertTest(
            certNodes,
            reportCertDer,
            TestCerts.IAS_ROOT_CERT_KEY_MOD,
            TestCerts.IAS_ROOT_CERT_KEY_EXP
        );

        Assert.ok(cert.isVerified, "cert not verified");
        Assert.equal(
            cert.keyId,
            keccak256(TestCerts.IAS_REPORT_CERT_KEY_DER),
            "keyId not match"
        );
        Assert.equal(
            keccak256(cert.pubKeyMod),
            keccak256(TestCerts.IAS_REPORT_CERT_KEY_MOD),
            "key mod not match"
        );
        Assert.equal(
            keccak256(cert.pubKeyExp),
            keccak256(TestCerts.IAS_REPORT_CERT_KEY_EXP),
            "key exp not match"
        );
        Assert.equal(
            cert.notAfter,
            TestCerts.IAS_REPORT_CERT_NOT_AFTER,
            "notAfter not match"
        );
    }

    function loadCertInvalidSignTest() public {
        bytes memory reportCertDer = TestCerts.IAS_REPORT_CERT_DER;

        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(reportCertDer);

        IASReportCert_proxy proxy = new IASReportCert_proxy();
        try proxy.loadCertTest(
            certNodes,
            reportCertDer,
            TestCerts.IAS_REPORT_CERT_KEY_MOD,
            TestCerts.IAS_REPORT_CERT_KEY_EXP
        ) {
            Assert.ok(false, "cert shoud not be verified");
        } catch Error(string memory reason) {
            Assert.equal(
                reason,
                "invalid signature",
                reason
            );
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error");
        }
    }
}
