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

import {TestCerts} from "../TestCerts.sol";
import {DecentServerCertCerts_proxy} from "./01_DecentServerCert.sol";


// File name has to end with '_test.sol', this file can contain more than one testSuite contracts
contract DecentServerCert_testSuit {

    //===== member variables =====

    address m_iasRootCertMgrAddr;
    address m_testProxyCAddr;

    /// 'beforeAll' runs before all other tests
    /// More special functions are: 'beforeEach', 'beforeAll', 'afterEach' & 'afterAll'
    function beforeAll() public {
        m_iasRootCertMgrAddr =
            address(new IASRootCertMgr(TestCerts.IAS_ROOT_CERT_DER));
        m_testProxyCAddr = address(new DecentServerCertCerts_proxy());
    }

    function verifySelfSignTest() public {
        try DecentServerCertCerts_proxy(m_testProxyCAddr).verifySelfSignTest() {
            Assert.ok(true, "verifySelfSignTest should not throw");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error - verifySelfSignTest");
        }
    }

    function extractDecentServerKeyTest() public {
        try DecentServerCertCerts_proxy(m_testProxyCAddr).extractDecentServerKeyTest() {
            Assert.ok(true, "extractDecentServerKeyTest should not throw");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error - extractDecentServerKeyTest");
        }
    }

    function loadCertTest() public {
        IASReportCertMgr iasReportCertMgr =
            new IASReportCertMgr(m_iasRootCertMgrAddr);

        try DecentServerCertCerts_proxy(m_testProxyCAddr).loadCertTest(
            address(iasReportCertMgr)
        ) {
            Assert.ok(true, "loadCertTest should not throw");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error - loadCertTest");
        }
    }

}
