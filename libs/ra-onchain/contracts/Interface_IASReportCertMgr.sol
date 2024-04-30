// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;


import {X509CertNodes} from "./X509CertNodes.sol";


interface Interface_IASReportCertMgr {

    /**
     * Verify IAS report signing certificate, with a pre-populated CertNodesObj
     * instance, and store it if valid
     * @param certNodes An populated CertNodesObj instance
     * @param certDer IAS report signing certificate in DER format
     */
    function verifyCertWithNodes(
        X509CertNodes.CertNodesObj calldata certNodes,
        bytes memory certDer
    ) external;

    /**
     * Verify IAS report signing certificate, and store it if valid
     * @param certDer IAS report signing certificate in DER format
     */
    function verifyCert(
        bytes memory certDer
    ) external;

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
        returns (bool, bool);

    /**
     * Get the public key of an IAS report signing certificate that has already
     * been verified
     * @param reportKeyId Report key ID, which is the keccak256 hash of the
     *                    report signing certificate's public key in DER format
     */
    function getPubKey(bytes32 reportKeyId)
        external
        view
        returns (bytes memory, bytes memory);

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
        returns (bool);

}
