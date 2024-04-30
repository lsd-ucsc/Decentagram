// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";

// This import is required to use custom transaction context
// Although it may fail compilation in 'Solidity Compiler' plugin
// But it will work fine in 'Solidity Unit Testing' plugin
import "remix_accounts.sol";


import {DecentServerCert} from "../../contracts/DecentServerCert.sol";
import {DecentServerCertMgr} from "../../contracts/DecentServerCertMgr.sol";
import {IASReportCertMgr} from "../../contracts/IASReportCertMgr.sol";
import {IASRootCertMgr} from "../../contracts/IASRootCertMgr.sol";
import {X509CertNodes} from "../../contracts/X509CertNodes.sol";

import {TestCerts} from "../TestCerts.sol";


// File name has to end with '_test.sol', this file can contain more than one testSuite contracts
contract DecentServerCertMgr_testSuit {

    using X509CertNodes for X509CertNodes.CertNodesObj;

    //===== member variables =====

    address m_iasRootCertMgrAddr;
    address m_iasReportCertMgrAddr;

    //===== functions =====

    /// 'beforeAll' runs before all other tests
    /// More special functions are: 'beforeEach', 'beforeAll', 'afterEach' & 'afterAll'
    function beforeAll() public {
        m_iasRootCertMgrAddr =
            address(new IASRootCertMgr(TestCerts.IAS_ROOT_CERT_DER));

        IASReportCertMgr reportCertMgr =
            new IASReportCertMgr(m_iasRootCertMgrAddr);
        reportCertMgr.verifyCert(TestCerts.IAS_REPORT_CERT_DER);

        m_iasReportCertMgrAddr = address(reportCertMgr);
    }

    function verifyCertWithNodesTest() public {
        bytes memory certDer = TestCerts.DECENT_SVR_CERT_DER;

        DecentServerCertMgr decentServerCertMgr =
            new DecentServerCertMgr(m_iasReportCertMgrAddr);

        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(certDer);

        try decentServerCertMgr.verifyCertWithNodes(certNodes, certDer) {
            Assert.ok(true, "cert should be verified");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error");
        }

        bytes32 encHash =
            decentServerCertMgr.isDecentServer(TestCerts.DECENT_SVR_CERT_KEY_ADDR);

        Assert.equal(
            encHash,
            TestCerts.DECENT_SVR_CERT_ENCL_HASH,
            "encHash not match"
        );
    }

    function verifyCertTest() public {
        bytes memory certDer = TestCerts.DECENT_SVR_CERT_DER;

        DecentServerCertMgr decentServerCertMgr =
            new DecentServerCertMgr(m_iasReportCertMgrAddr);

        try decentServerCertMgr.verifyCert(certDer) {
            Assert.ok(true, "cert should be verified");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error");
        }

        bytes32 encHash =
            decentServerCertMgr.isDecentServer(TestCerts.DECENT_SVR_CERT_KEY_ADDR);

        Assert.equal(
            encHash,
            TestCerts.DECENT_SVR_CERT_ENCL_HASH,
            "encHash not match"
        );
    }

    function verifyInvalidCertTest() public {
        bytes memory certDer = TestCerts.DECENT_SVR_CERT_DER;

        // change something in the attestation report JSON
        (uint256 valPos, uint256 valLen) =
            DecentServerCert.jsonSimpleReadValPos(
                certDer,
                "\"isvEnclaveQuoteStatus\""
            );
        certDer[valPos] = "0";

        DecentServerCertMgr decentServerCertMgr =
            new DecentServerCertMgr(m_iasReportCertMgrAddr);

        try decentServerCertMgr.verifyCert(certDer) {
            Assert.ok(false, "cert should not be verified");
        } catch Error(string memory reason) {
            Assert.equal(reason, "Invalid IAS report sign", reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error");
        }

        bytes32 encHash =
            decentServerCertMgr.isDecentServer(TestCerts.DECENT_SVR_CERT_KEY_ADDR);

        Assert.equal(encHash, bytes32(0), "encHash not match");
    }

    function getterTest() public {
        address someAddr = address(this);

        DecentServerCertMgr decentServerCertMgr =
            new DecentServerCertMgr(m_iasReportCertMgrAddr);

        bytes32 encHash = decentServerCertMgr.isDecentServer(someAddr);

        Assert.equal(encHash, bytes32(0), "encHash not match");
    }

}
