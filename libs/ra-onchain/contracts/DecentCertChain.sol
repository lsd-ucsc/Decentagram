// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;


import {DecentAppCert} from "./DecentAppCert.sol";
import {
    Interface_DecentServerCertMgr
} from "./Interface_DecentServerCertMgr.sol";
import {LibSecp256k1Sha256} from "./LibSecp256k1Sha256.sol";
import {X509CertNodes} from "./X509CertNodes.sol";


library DecentCertChain {

    using DecentAppCert for DecentAppCert.DecentApp;
    using LibSecp256k1Sha256 for X509CertNodes.CertTbsNodesObj;
    using X509CertNodes for X509CertNodes.CertNodesObj;

    //===== functions =====

    /**
     * Verify a Decent App certificate chain, containing:
     * IAS Report Cert -> Decent Server Cert -> Decent App Cert
     * @param appCert DecentApp struct to be filled
     * @param decentSvrCertMgrAddr Address of DecentServerCertMgr contract
     * @param svrCertDer Decent Server certificate in DER format
     * @param appCertDer Decent App certificate in DER format
     */
    function verifyCertChain(
        DecentAppCert.DecentApp memory appCert,
        address decentSvrCertMgrAddr,
        bytes memory svrCertDer,
        bytes memory appCertDer
    )
        internal
    {
        // get Decent Server key address
        X509CertNodes.CertNodesObj memory svrCertNodes;
        svrCertNodes.loadCertNodes(svrCertDer);

        bytes memory svrPubKeyBytes =
            svrCertNodes.tbs.extractPubKeyBytes(svrCertDer);
        address svrKeyAddr =
            LibSecp256k1Sha256.pubKeyBytesToAddr(svrPubKeyBytes);

        // check if Decent Server certificate has been verified
        bytes32 svrEnclaveHash =
            Interface_DecentServerCertMgr(decentSvrCertMgrAddr)
                .isDecentServer(svrKeyAddr);

        if (svrEnclaveHash == bytes32(0)) {
            // Decent Server certificate has not been verified
            // verify Decent Server certificate
            Interface_DecentServerCertMgr(decentSvrCertMgrAddr)
                .verifyCertWithNodes(svrCertNodes, svrCertDer);

            svrEnclaveHash =
                Interface_DecentServerCertMgr(decentSvrCertMgrAddr)
                    .isDecentServer(svrKeyAddr);
        }

        // verify Decent App certificate
        appCert.issuerEnclaveHash = svrEnclaveHash;
        appCert.issuerKeyAddr = svrKeyAddr;
        appCert.loadCert(appCertDer);
    }

}
