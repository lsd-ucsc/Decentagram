// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;


import {LibRsaSha256} from "../libs/sig-verify-algs/LibRsaSha256.sol";

import {Interface_IASRootCertMgr} from "./Interface_IASRootCertMgr.sol";
import {IASReportCert} from "./IASReportCert.sol";
import {X509CertNodes} from "./X509CertNodes.sol";


contract IASReportCertMgr {

    //===== Member variables =====

    address private m_rootCertMgrAddr;
    mapping(bytes32 => IASReportCert.IASReportCertObj) private m_certs;

    //===== Constructor =====

    /**
     * @param rootCertMgrAddr Address of IAS root certificate manager contract
     */
    constructor(address rootCertMgrAddr) {
        m_rootCertMgrAddr = rootCertMgrAddr;

        // ensure that the root CA cert is not expired
        Interface_IASRootCertMgr(m_rootCertMgrAddr).requireValidity();
    }

    //===== Functions =====

    /**
     * Verify IAS report signing certificate, with a pre-populated CertNodesObj
     * instance, and store it if valid
     * @param certNodes An populated CertNodesObj instance
     * @param certDer IAS report signing certificate in DER format
     */
    function verifyCertWithNodes(
        X509CertNodes.CertNodesObj calldata certNodes,
        bytes memory certDer
    ) external {
        // ensure that the root CA cert is not expired
        Interface_IASRootCertMgr(m_rootCertMgrAddr).requireValidity();

        // gets root CA public key
        (bytes memory rootPubKeyMod, bytes memory rootPubKeyExp) =
            Interface_IASRootCertMgr(m_rootCertMgrAddr).getPubKey();

        // loads report signing certificate
        // this will throw if the certificate is not valid
        IASReportCert.IASReportCertObj memory cert =
            IASReportCert.loadCert(
                certNodes,
                certDer,
                rootPubKeyMod,
                rootPubKeyExp
            );

        // store the certificate
        m_certs[cert.keyId] = cert;
    }

    /**
     * Verify IAS report signing certificate, and store it if valid
     * @param certDer IAS report signing certificate in DER format
     */
    function verifyCert(
        bytes memory certDer
    ) external {
        X509CertNodes.CertNodesObj memory certNodes;
        X509CertNodes.loadCertNodes(certNodes, certDer);
        this.verifyCertWithNodes(certNodes, certDer);
    }

    /**
     * Check if an IAS report signing certificate has already been verified
     * and check if it is expired
     * @param reportKeyId Report key ID, which is the keccak256 hash of the
     *                    report signing certificate's public key in DER format
     * @return isVerified Whether the certificate has been verified
     * @return isValid Whether the certificate is valid (not expired)
     */
    function isCertVerified(bytes32 reportKeyId)
        external
        view
        returns (bool, bool)
    {
        IASReportCert.IASReportCertObj storage cert = m_certs[reportKeyId];

        return
            cert.isVerified ? ( // verified; check if expired
                true,
                (block.timestamp < cert.notAfter)
            ) : ( // not verified
                false,
                false
            );
    }

    /**
     * Get the public key of an IAS report signing certificate that has already
     * been verified
     * @param reportKeyId Report key ID, which is the keccak256 hash of the
     *                    report signing certificate's public key in DER format
     */
    function getPubKey(bytes32 reportKeyId)
        external
        view
        returns (bytes memory, bytes memory)
    {
        IASReportCert.IASReportCertObj storage cert = m_certs[reportKeyId];

        require(cert.isVerified, "report cert not verified");
        require(block.timestamp < cert.notAfter, "report cert expired");

        return (cert.pubKeyMod, cert.pubKeyExp);
    }

    /**
     * Verify the signature over a IAS report using the public key specified by
     * the report key ID
     * @param reportKeyId Report key ID, which is the keccak256 hash of the
     *                    report signing certificate's public key in DER format
     * @param repHash Hash of the report to be verified
     * @param signature Signature that signs over the report
     * @return bool Whether the signature is valid
     */
    function verifySign(
        bytes32 reportKeyId,
        bytes32 repHash,
        bytes memory signature
    )
        external
        view
        returns (bool)
    {
        IASReportCert.IASReportCertObj storage cert = m_certs[reportKeyId];

        require(cert.isVerified, "report cert not verified");
        require(block.timestamp < cert.notAfter, "report cert expired");

        return LibRsaSha256.verifyWithComponents(
            cert.pubKeyMod,
            cert.pubKeyExp,
            repHash,
            signature
        );
    }

}
