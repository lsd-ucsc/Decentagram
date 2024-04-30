// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;


import {X509CertNodes} from "./X509CertNodes.sol";


interface Interface_DecentServerCertMgr {

    /**
     * Verify Decent Server certificate, with a pre-populated CertNodesObj
     * @param certNodes An populated CertNodesObj instance
     * @param certDer Decent Server certificate in DER format
     */
    function verifyCertWithNodes(
        X509CertNodes.CertNodesObj memory certNodes,
        bytes memory certDer
    )
        external;

    /**
     * Verify Decent Server certificate
     * @param certDer Decent Server certificate in DER format
     */
    function verifyCert(bytes memory certDer) external;

    /**
     * Check if a Decent Server certificate has been verified or not
     * @param svrKeyAddr Address derived from the Decent Server public key
     * @return bytes32 Enclave hash of the Decent Server,
     *                 or 32 bytes of zeros if not verified
     */
    function isDecentServer(address svrKeyAddr)
        external
        view
        returns (bytes32);

}
