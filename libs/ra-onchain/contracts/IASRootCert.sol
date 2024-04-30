// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;


import {Asn1Decode} from "../libs/asn1-decode/Asn1Decode.sol";
import {LibRsaSha256} from "../libs/sig-verify-algs/LibRsaSha256.sol";

import {OIDs, Names} from "./Constants.sol";
import {X509CertNodes} from "./X509CertNodes.sol";
import {X509Name} from "./X509Name.sol";
import {X509Timestamp} from "./X509Timestamp.sol";


library IASRootCert {

    using Asn1Decode for bytes;
    using X509CertNodes for X509CertNodes.CertNodesObj;
    using X509CertNodes for X509CertNodes.CertTbsNodesObj;
    using X509Timestamp for X509CertNodes.CertTbsNodesObj;

    //===== structs =====

    struct IASRootCertObj {
        bool  isVerified;
        bytes pubKeyMod;
        bytes pubKeyExp;
        uint256 notAfter;
    }

    //===== functions =====

    /**
     * Read in an IAS root certificate, verify its validity, and populate the
     * given IASRootCertObj instance.
     * @param cert An empty IASRootCertObj instance, which will be populated
     *             during the process
     * @param certDer IAS root certificate in DER format
     */
    function loadCert(
        IASRootCertObj memory cert,
        bytes memory certDer
    )
        internal
        view
    {
        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(certDer);

        // Check signature algorithm
        bytes32 algType = certDer.bytes32At(
            certDer.firstChildOf(certNodes.tbs.algTypeNode)
        );
        require(algType == OIDs.OID_ALG_RSA_SHA_256, "alg type not match");
        algType = certDer.bytes32At(
            certDer.firstChildOf(certNodes.algTypeNode)
        );
        require(algType == OIDs.OID_ALG_RSA_SHA_256, "alg type not match");

        // Check issuer common name
        string memory comName = X509Name.getCN(
            certDer,
            certNodes.tbs.issuerNode,
            certNodes.tbs.validityNode
        );
        require(
            keccak256(bytes(comName)) == Names.HASH_IAS_ROOT_CERT_CN,
            "issuer CN not match"
        );

        // Check subject common name
        comName = X509Name.getCN(
            certDer,
            certNodes.tbs.subjectNode,
            certNodes.tbs.pubKeyNode
        );
        require(
            keccak256(bytes(comName)) == Names.HASH_IAS_ROOT_CERT_CN,
            "subject CN not match"
        );

        // Check validity
        (uint256 notBefore, uint256 notAfter) =
            certNodes.tbs.getValidityTimestamps(certDer);
        require(notBefore <= block.timestamp, "cert not valid yet");
        require(block.timestamp < notAfter, "cert expired");
        cert.notAfter = notAfter;

        // Store public key
        (cert.pubKeyMod, cert.pubKeyExp) =
            LibRsaSha256.extractKeyComponents(
                certDer.allBytesAt(certNodes.tbs.pubKeyNode)
            );

        // Check signature
        bytes memory sigValue = certDer.bitstringAt(certNodes.sigNode);
        bytes memory tbsBytes = certDer.allBytesAt(certNodes.tbs.root);
        cert.isVerified = LibRsaSha256.verifyWithComponents(
            cert.pubKeyMod,
            cert.pubKeyExp,
            sha256(tbsBytes),
            sigValue
        );
        require(cert.isVerified, "invalid signature");
    }

    function loadCert(
        bytes memory certDer
    )
        internal
        view
        returns (IASRootCertObj memory cert)
    {
        loadCert(cert, certDer);
    }
}
