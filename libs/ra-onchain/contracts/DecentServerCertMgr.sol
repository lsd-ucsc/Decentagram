// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;


import {DecentServerCert} from "./DecentServerCert.sol";
import {X509CertNodes} from "./X509CertNodes.sol";


contract DecentServerCertMgr {

    //===== Member variables =====

    address private m_iasRepCertMgrAddr;

    mapping(bytes32 => bool) private m_quoteStatusMap;

    mapping(address => DecentServerCert.DecentServerCertObj)
        private m_serverCerts;

    //===== Constructor =====

    constructor(address iasRepCertMgrAddr) {
        m_iasRepCertMgrAddr = iasRepCertMgrAddr;

        m_quoteStatusMap[keccak256("OK")] = true;
        m_quoteStatusMap[keccak256("GROUP_OUT_OF_DATE")] = true;
        m_quoteStatusMap[keccak256("CONFIGURATION_NEEDED")] = true;
        m_quoteStatusMap[keccak256("SW_HARDENING_NEEDED")] = true;
        m_quoteStatusMap[keccak256("CONFIGURATION_AND_SW_HARDENING_NEEDED")] = true;
    }

    //===== Functions =====

    /**
     * Verify Decent Server certificate, with a pre-populated CertNodesObj
     * @param certNodes An populated CertNodesObj instance
     * @param certDer Decent Server certificate in DER format
     */
    function verifyCertWithNodes(
        X509CertNodes.CertNodesObj memory certNodes,
        bytes memory certDer
    )
        external
    {
        DecentServerCert.DecentServerCertObj memory cert;
        DecentServerCert.loadCert(
            cert,
            certNodes,
            certDer,
            m_iasRepCertMgrAddr,
            m_quoteStatusMap
        );
        m_serverCerts[cert.serverKeyAddr] = cert;
    }

    /**
     * Verify Decent Server certificate
     * @param certDer Decent Server certificate in DER format
     */
    function verifyCert(bytes memory certDer) external
    {
        DecentServerCert.DecentServerCertObj memory cert;
        DecentServerCert.loadCert(
            cert,
            certDer,
            m_iasRepCertMgrAddr,
            m_quoteStatusMap
        );
        m_serverCerts[cert.serverKeyAddr] = cert;
    }

    /**
     * Check if a Decent Server certificate has been verified or not
     * @param svrKeyAddr Address derived from the Decent Server public key
     * @return bytes32 Enclave hash of the Decent Server,
     *                 or 32 bytes of zeros if not verified
     */
    function isDecentServer(address svrKeyAddr)
        external
        view
        returns (bytes32)
    {
        DecentServerCert.DecentServerCertObj storage cert =
            m_serverCerts[svrKeyAddr];

        return cert.isVerified ?
            cert.enclaveHash :
            bytes32(0);
    }

}
