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
import {DecentServerCertBasics_proxy} from "./01_DecentServerCert.sol";


// File name has to end with '_test.sol', this file can contain more than one testSuite contracts
contract DecentServerCertBasics_testSuit {

    //===== member variables =====

    address m_iasRootCertMgrAddr;
    address m_testProxyBAddr;

    /// 'beforeAll' runs before all other tests
    /// More special functions are: 'beforeEach', 'beforeAll', 'afterEach' & 'afterAll'
    function beforeAll() public {
        m_iasRootCertMgrAddr =
            address(new IASRootCertMgr(TestCerts.IAS_ROOT_CERT_DER));
        m_testProxyBAddr = address(new DecentServerCertBasics_proxy());
    }

    function strFindTest() public {
        try DecentServerCertBasics_proxy(m_testProxyBAddr).strFindTest() {
            Assert.ok(true, "strFindTest should not throw");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error - strFindTest");
        }
    }

    function jsonSimpleReadValPosTest() public {
        try DecentServerCertBasics_proxy(m_testProxyBAddr).jsonSimpleReadValPosTest() {
            Assert.ok(true, "jsonSimpleReadValPosTest should not throw");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error - jsonSimpleReadValPosTest");
        }
    }

    function verifyEPIDAttestationRepTest() public {
        IASReportCertMgr iasReportCertMgr =
            new IASReportCertMgr(m_iasRootCertMgrAddr);

        try DecentServerCertBasics_proxy(m_testProxyBAddr).verifyEPIDAttestationRepTest(
            address(iasReportCertMgr)
        ) {
            Assert.ok(true, "verifyEPIDAttestationRepTest should not throw");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error - verifyEPIDAttestationRepTest");
        }
    }

}
