// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;


import {DecentAppCert} from "../contracts/DecentAppCert.sol";
import {DecentCertChain} from "../contracts/DecentCertChain.sol";
import {
    Interface_DecentServerCertMgr
} from "../contracts/Interface_DecentServerCertMgr.sol";


contract HelloWorldApp {

    using DecentAppCert for DecentAppCert.DecentApp;

    //===== member variables =====

    address m_decentSvrMgr;
    DecentAppCert.DecentApp m_appCert;

    //===== constructor =====

    constructor(address decentSvrMgr) {
        m_decentSvrMgr = decentSvrMgr;
    }

    //===== functions =====

    function loadAppCert(
        address decentSvrKeyAddr,
        bytes memory certDer
    ) external {
        DecentAppCert.DecentApp memory appCert;
        appCert.loadCert(certDer, m_decentSvrMgr, decentSvrKeyAddr);
        m_appCert = appCert;
    }

    function verifyCertChain(
        bytes memory svrCertDer,
        bytes memory appCertDer
    ) external {
        DecentAppCert.DecentApp memory appCert;
        DecentCertChain.verifyCertChain(
            appCert,
            m_decentSvrMgr,
            svrCertDer,
            appCertDer
        );
        m_appCert = appCert;
    }

}
