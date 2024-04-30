// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;


// This import is automatically injected by Remix
import "remix_tests.sol";

import {DecentAppCert} from "../../contracts/DecentAppCert.sol";
// import {DecentServerCertMgr} from "../../contracts/DecentServerCertMgr.sol";
// import {IASReportCertMgr} from "../../contracts/IASReportCertMgr.sol";
// import {IASRootCertMgr} from "../../contracts/IASRootCertMgr.sol";
import {
    Interface_DecentServerCertMgr
} from "../../contracts/Interface_DecentServerCertMgr.sol";
import {X509CertNodes} from "../../contracts/X509CertNodes.sol";

import {TestCerts} from "../TestCerts.sol";


contract DecentAppCert_proxy {

    using DecentAppCert for DecentAppCert.DecentApp;
    using X509CertNodes for X509CertNodes.CertNodesObj;

    //===== member variables =====

    address m_decentSvrCertMgrAddr;

    //===== constructor =====

    constructor() {
        m_decentSvrCertMgrAddr =
            // address(new DecentServerCertMgr(
            //     address(new IASReportCertMgr(
            //         address(new IASRootCertMgr(TestCerts.IAS_ROOT_CERT_DER))
            //     ))
            // ));
            0xD9eC9E840Bb5Df076DBbb488d01485058f421e58;
        Interface_DecentServerCertMgr(m_decentSvrCertMgrAddr).verifyCert(
            TestCerts.DECENT_SVR_CERT_DER
        );
    }

    //===== functions =====

    function loadCertWithAddrTest() public {
        DecentAppCert.DecentApp memory decentApp;
        decentApp.loadCert(
            TestCerts.DECENT_APP_CERT_DER,
            m_decentSvrCertMgrAddr,
            TestCerts.DECENT_SVR_CERT_KEY_ADDR
        );

        Assert.equal(
            decentApp.appKeyAddr,
            TestCerts.DECENT_APP_CERT_KEY_ADDR,
            "appKeyAddr should be equal"
        );
        Assert.equal(
            keccak256(decentApp.appPlatform),
            keccak256("SGX_EPID"),
            "appPlatform not match"
        );
        Assert.equal(
            decentApp.appEnclaveHash,
            TestCerts.DECENT_APP_CERT_ENCL_HASH,
            "appEnclaveHash not match"
        );
        Assert.equal(
            keccak256(decentApp.appAuthList),
            keccak256(TestCerts.DECENT_APP_CERT_AUTHLIST),
            "appAuthList not match"
        );
    }

    function loadCertWithWrongAddrTest() public {
        bytes memory appCertDer = TestCerts.DECENT_APP_CERT_DER;

        DecentAppCert.DecentApp memory decentApp;

        decentApp.loadCert(
            appCertDer,
            m_decentSvrCertMgrAddr,
            address(this)
        );
    }

}
