// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;


import {Asn1Decode} from "../libs/asn1-decode/Asn1Decode.sol";
import {LibRsaSha256} from "../libs/sig-verify-algs/LibRsaSha256.sol";

import {OIDs, Names} from "./Constants.sol";
import {X509CertNodes} from "./X509CertNodes.sol";
import {X509Name} from "./X509Name.sol";
import {X509Timestamp} from "./X509Timestamp.sol";


library IASReportCert {

    using Asn1Decode for bytes;
    using X509CertNodes for X509CertNodes.CertNodesObj;
    using X509CertNodes for X509CertNodes.CertTbsNodesObj;
    using X509Timestamp for X509CertNodes.CertTbsNodesObj;

    //===== constants =====

    //===== structs =====

    struct IASReportCertObj {
        bool  isVerified;
        bytes32 keyId;
        bytes pubKeyMod;
        bytes pubKeyExp;
        uint256 notAfter;
    }

    //===== functions =====

    /**
     * Extract report key ID from IAS report signing certificate
     * @param certNodes An empty CertNodesObj instance, which will be populated
     *                  during the process
     * @param certDer IAS report signing certificate in DER format
     */
    function extractReportKeyIdFromCert(
        X509CertNodes.CertNodesObj memory certNodes,
        bytes memory certDer
    )
        internal
        pure
        returns (bytes32)
    {
        certNodes.loadCertNodes(certDer);

        bytes memory pubKeyDer = certDer.allBytesAt(certNodes.tbs.pubKeyNode);

        return keccak256(pubKeyDer);
    }

    function loadCert(
        IASReportCertObj memory cert,
        X509CertNodes.CertNodesObj calldata certNodes,
        bytes memory certDer,
        bytes memory caKeyMod,
        bytes memory caKeyExp
    )
        internal
        view
    {
        // Check signature algorithm
        bytes32 algType;
        // algType = certDer.bytes32At(
        //     certDer.firstChildOf(certNodes.tbs.algTypeNode)
        // );
        // require(algType == OIDs.OID_ALG_RSA_SHA_256, "alg type not match");
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

        // Check validity
        (uint256 notBefore, uint256 notAfter) =
            certNodes.tbs.getValidityTimestamps(certDer);
        require(notBefore <= block.timestamp, "cert not valid yet");
        require(block.timestamp < notAfter, "cert expired");
        cert.notAfter = notAfter;

        // Store public key
        bytes memory pubKeyDer = certDer.allBytesAt(certNodes.tbs.pubKeyNode);
        cert.keyId = keccak256(pubKeyDer);
        (cert.pubKeyMod, cert.pubKeyExp) =
            LibRsaSha256.extractKeyComponents(pubKeyDer);

        // Check signature
        bytes memory sigValue = certDer.bitstringAt(certNodes.sigNode);
        bytes32 tbsHash = sha256(certDer.allBytesAt(certNodes.tbs.root));
        cert.isVerified = LibRsaSha256.verifyWithComponents(
            caKeyMod,
            caKeyExp,
            tbsHash,
            sigValue
        );
        require(cert.isVerified, "invalid signature");
    }

    function loadCert(
        X509CertNodes.CertNodesObj calldata certNodes,
        bytes memory certDer,
        bytes memory caKeyMod,
        bytes memory caKeyExp
    )
        internal
        view
        returns (IASReportCertObj memory cert)
    {
        loadCert(cert, certNodes, certDer, caKeyMod, caKeyExp);
    }

}
