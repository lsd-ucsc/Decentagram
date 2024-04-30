// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";

// This import is required to use custom transaction context
// Although it may fail compilation in 'Solidity Compiler' plugin
// But it will work fine in 'Solidity Unit Testing' plugin
import "remix_accounts.sol";


import {DecentAppCert} from "../../contracts/DecentAppCert.sol";
import {DecentCertChain} from "../../contracts/DecentCertChain.sol";
// import {DecentServerCertMgr} from "../../contracts/DecentServerCertMgr.sol";
// import {IASReportCertMgr} from "../../contracts/IASReportCertMgr.sol";
// import {IASRootCertMgr} from "../../contracts/IASRootCertMgr.sol";

import {TestCerts} from "../TestCerts.sol";


// File name has to end with '_test.sol', this file can contain more than one testSuite contracts
contract DecentCertChain_testSuit {

    //===== member variables =====

    address m_decentSvrCertMgrAddr;

    //===== functions =====

    /// 'beforeAll' runs before all other tests
    /// More special functions are: 'beforeEach', 'beforeAll', 'afterEach' & 'afterAll'
    function beforeAll() public {
        m_decentSvrCertMgrAddr =
            // address(new DecentServerCertMgr(
            //     address(new IASReportCertMgr(
            //         address(new IASRootCertMgr(TestCerts.IAS_ROOT_CERT_DER))
            //     ))
            // ));
            0xaa96CB8107828e584C4FbC37a41754333DfFD206;
    }

    function verifyCertChainTest() public {
        DecentAppCert.DecentApp memory appCert;
        DecentCertChain.verifyCertChain(
            appCert,
            m_decentSvrCertMgrAddr,
            TestCerts.DECENT_SVR_CERT_DER,
            TestCerts.DECENT_APP_CERT_DER
        );

        Assert.ok(
            appCert.isVerified,
            "app cert should be verified"
        );

        Assert.equal(
            appCert.issuerKeyAddr,
            TestCerts.DECENT_SVR_CERT_KEY_ADDR,
            "issuer key address mismatch"
        );
        Assert.equal(
            appCert.issuerEnclaveHash,
            TestCerts.DECENT_SVR_CERT_ENCL_HASH,
            "issuer enclave hash mismatch"
        );

        Assert.equal(
            appCert.appKeyAddr,
            TestCerts.DECENT_APP_CERT_KEY_ADDR,
            "app key address mismatch"
        );
        Assert.equal(
            appCert.appEnclaveHash,
            TestCerts.DECENT_APP_CERT_ENCL_HASH,
            "appEnclaveHash mismatch"
        );
        Assert.equal(
            keccak256(appCert.appPlatform),
            keccak256("SGX_EPID"),
            "appPlatform mismatch"
        );
        Assert.equal(
            keccak256(appCert.appAuthList),
            keccak256(TestCerts.DECENT_APP_CERT_AUTHLIST),
            "appAuthList mismatch"
        );
    }

}
