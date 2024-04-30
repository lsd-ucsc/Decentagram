// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";

// This import is required to use custom transaction context
// Although it may fail compilation in 'Solidity Compiler' plugin
// But it will work fine in 'Solidity Unit Testing' plugin
import "remix_accounts.sol";


import {IASReportCertMgr} from "../../contracts/IASReportCertMgr.sol";
import {IASRootCertMgr} from "../../contracts/IASRootCertMgr.sol";
import {X509CertNodes} from "../../contracts/X509CertNodes.sol";

import {TestCerts} from "../TestCerts.sol";


// File name has to end with '_test.sol', this file can contain more than one testSuite contracts
contract IASReportCertMgr_testSuit {

    using X509CertNodes for X509CertNodes.CertNodesObj;

    //===== member variables =====

    address m_iasRootCertMgrAddr;

    //===== functions =====

    /// 'beforeAll' runs before all other tests
    /// More special functions are: 'beforeEach', 'beforeAll', 'afterEach' & 'afterAll'
    function beforeAll() public {
        m_iasRootCertMgrAddr =
            address(new IASRootCertMgr(TestCerts.IAS_ROOT_CERT_DER));
    }

    function notVerifiedCertGettersTest() public {
        IASReportCertMgr iasReportCertMgr =
            new IASReportCertMgr(m_iasRootCertMgrAddr);

        bytes32 repKeyId = keccak256(TestCerts.IAS_REPORT_CERT_KEY_DER);

        try iasReportCertMgr.isCertVerified(repKeyId)
            returns (bool verified, bool notExpired)
        {
            Assert.ok(!verified, "cert should not be verified");
            Assert.ok(!notExpired, "cert should be expired");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error");
        }

        try iasReportCertMgr.getPubKey(repKeyId)
            returns (bytes memory mod, bytes memory exp)
        {
            Assert.ok(false, "should not get pub key");
        } catch Error(string memory reason) {
            Assert.equal(reason, "report cert not verified", reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error");
        }
    }

    function verifyCertWithNodesTest() public {
        IASReportCertMgr iasReportCertMgr =
            new IASReportCertMgr(m_iasRootCertMgrAddr);

        bytes memory reportCertDer = TestCerts.IAS_REPORT_CERT_DER;

        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(reportCertDer);

        try iasReportCertMgr.verifyCertWithNodes(
            certNodes,
            reportCertDer
        ) {
            Assert.ok(true, "cert verified");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error");
        }

        bytes32 repKeyId = keccak256(TestCerts.IAS_REPORT_CERT_KEY_DER);

        // getters

        try iasReportCertMgr.isCertVerified(repKeyId)
            returns (bool verified, bool notExpired)
        {
            Assert.ok(verified, "cert not verified");
            Assert.ok(notExpired, "cert expired");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error");
        }

        try iasReportCertMgr.getPubKey(repKeyId)
            returns (bytes memory mod, bytes memory exp)
        {
            Assert.equal(
                keccak256(mod),
                keccak256(TestCerts.IAS_REPORT_CERT_KEY_MOD),
                "pub key mod not match"
            );
            Assert.equal(
                keccak256(exp),
                keccak256(TestCerts.IAS_REPORT_CERT_KEY_EXP),
                "pub key exp not match"
            );
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error");
        }
    }

    function verifyCertTest() public {
        IASReportCertMgr iasReportCertMgr =
            new IASReportCertMgr(m_iasRootCertMgrAddr);

        bytes memory reportCertDer = TestCerts.IAS_REPORT_CERT_DER;

        try iasReportCertMgr.verifyCert(reportCertDer) {
            Assert.ok(true, "cert verified");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error");
        }

        bytes32 repKeyId = keccak256(TestCerts.IAS_REPORT_CERT_KEY_DER);

        // getters

        try iasReportCertMgr.isCertVerified(repKeyId)
            returns (bool verified, bool notExpired)
        {
            Assert.ok(verified, "cert not verified");
            Assert.ok(notExpired, "cert expired");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error");
        }

        try iasReportCertMgr.getPubKey(repKeyId)
            returns (bytes memory mod, bytes memory exp)
        {
            Assert.equal(
                keccak256(mod),
                keccak256(TestCerts.IAS_REPORT_CERT_KEY_MOD),
                "pub key mod not match"
            );
            Assert.equal(
                keccak256(exp),
                keccak256(TestCerts.IAS_REPORT_CERT_KEY_EXP),
                "pub key exp not match"
            );
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error");
        }
    }

    function verifySignTest() public {
        IASReportCertMgr iasReportCertMgr =
            new IASReportCertMgr(m_iasRootCertMgrAddr);

        bytes memory reportCertDer = TestCerts.IAS_REPORT_CERT_DER;

        try iasReportCertMgr.verifyCert(reportCertDer) {
            Assert.ok(true, "cert verified");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error");
        }

        bytes32 repKeyId = keccak256(TestCerts.IAS_REPORT_CERT_KEY_DER);

        try iasReportCertMgr.verifySign(
            repKeyId,
            sha256(TestCerts.DECENT_SVR_CERT_ATT_REP_JSON),
            TestCerts.DECENT_SVR_CERT_ATT_REP_SIGN
        ) returns (bool vrfyRes) {
            Assert.ok(vrfyRes, "sign not verified");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error");
        }
    }

}
