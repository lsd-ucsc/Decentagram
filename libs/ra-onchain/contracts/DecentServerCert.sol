// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;


import {Asn1Decode} from "../libs/asn1-decode/Asn1Decode.sol";
import {Base64} from "../libs/base64/base64.sol";
import {BytesUtils} from "../libs/ens-contracts/BytesUtils.sol";
import {RLPReader} from "../libs/Solidity-RLP/contracts/RLPReader.sol";

import {IASReportCert} from "./IASReportCert.sol";
import {Interface_IASReportCertMgr} from "./Interface_IASReportCertMgr.sol";
import {LibSecp256k1Sha256} from "./LibSecp256k1Sha256.sol";
import {OIDs} from "./Constants.sol";
import {X509CertNodes} from "./X509CertNodes.sol";
import {X509Extension} from "./X509Extension.sol";


library DecentServerCert {

    using Asn1Decode for bytes;
    using BytesUtils for bytes;
    using LibSecp256k1Sha256 for X509CertNodes.CertNodesObj;
    using LibSecp256k1Sha256 for X509CertNodes.CertTbsNodesObj;
    using RLPReader for RLPReader.RLPItem;
    using X509CertNodes for X509CertNodes.CertNodesObj;

    //===== constants =====

    uint256 constant EPID_QUOTE_ENCL_HASH_POS = 112;
    uint256 constant EPID_QUOTE_REP_DATA_POS  = 368;

    //===== structs =====

    struct DecentServerCertObj {
        bool isVerified;
        address serverKeyAddr;
        bytes32 enclaveHash;
    }

    //===== functions =====

    function strEqual(
        bytes memory strA,
        uint256 startPos,
        bytes memory strB
    )
        internal
        pure
        returns (bool)
    {
        if (strA.length <= startPos + strB.length) {
            return false;
        }
        for (uint256 i = 0; i < strB.length; i++) {
            if (strA[startPos + i] != strB[i]) {
                return false;
            }
        }
        return true;
    }

    function strFind(
        bytes memory strA,
        uint256 startPos,
        bytes memory strB
    )
        internal
        pure
        returns (uint256)
    {
        require(startPos < strA.length, "Invalid startPos");

        for (uint256 i = startPos; i < strA.length; i++) {
            if (strA[i] == strB[0] && strEqual(strA, i, strB)) {
                return i;
            }
        }
        return strA.length;
    }

    function jsonSimpleReadValPos(
        bytes memory jsonStr,
        bytes memory quatedKey
    )
        internal
        pure
        returns (uint256 pos, uint256 len)
    {
        pos = strFind(jsonStr, 0, quatedKey);
        require(pos < jsonStr.length, "Invalid JSON");

        pos += quatedKey.length;
        pos = strFind(jsonStr, pos, "\""); // find first quote
        require(pos < jsonStr.length, "Invalid JSON");

        pos += 1; // one char after begining quote

        uint256 endPos = strFind(jsonStr, pos, "\""); // find second quote
        require(endPos < jsonStr.length, "Invalid JSON");

        len = endPos - pos;
    }

    function verifyEPIDReportCert(
        bytes memory repCertDer,
        address iasRepCertMgrAddr
    )
        internal
        returns (bytes32 repKeyId)
    {
        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(repCertDer);

        repKeyId =
            IASReportCert.extractReportKeyIdFromCert(certNodes, repCertDer);
        (bool isVerified, bool isExpired) =
            Interface_IASReportCertMgr(iasRepCertMgrAddr).isCertVerified(repKeyId);
        if (!isVerified || !isExpired) {
            Interface_IASReportCertMgr(iasRepCertMgrAddr).verifyCertWithNodes(
                certNodes,
                repCertDer
            );
        }
    }

    function verifyEPIDAttestRep(
        bytes memory stdRepData,
        bytes memory repJson,
        bytes memory keyRing,
        mapping(bytes32 => bool) storage quoteStatusMap
    )
        internal
        view
        returns (bytes32 enclaveHash)
    {
        (uint256 valPos, uint256 valLen) =
            jsonSimpleReadValPos(repJson, "\"isvEnclaveQuoteStatus\"");
        bytes memory quoteStatus = repJson.substringFast(valPos, valLen);
        require(quoteStatusMap[keccak256(quoteStatus)], "Invalid quote status");

        (valPos, valLen) =
            jsonSimpleReadValPos(repJson, "\"isvEnclaveQuoteBody\"");
        string memory quoteBodyB64 = repJson.substrstringFast(valPos, valLen);
        bytes memory quoteBody = Base64.decode(quoteBodyB64);

        enclaveHash = quoteBody.readBytes32(EPID_QUOTE_ENCL_HASH_POS);
        bytes32 repData = quoteBody.readBytes32(EPID_QUOTE_REP_DATA_POS);

        require(
            repData ==
            sha256(
                abi.encodePacked(stdRepData.readBytes32(0), bytes32(0), sha256(keyRing))
            ),
            "Invalid rep data"
        );
    }

    function verifyEPIDAttestRepSet(
        DecentServerCertObj memory cert,
        bytes memory stdRepData,
        bytes memory attestRepRlp,
        bytes memory keyRing,
        address iasRepCertMgrAddr,
        mapping(bytes32 => bool) storage quoteStatusMap
    )
        internal
    {
        // [[cert1, cert2, ...], JSON dict, signature]
        RLPReader.RLPItem[] memory rep = RLPReader.toRlpItem(attestRepRlp).toList();
        RLPReader.RLPItem[] memory repCerts = rep[0].toList();
        require(repCerts.length == 1, "multiple rep cert not supported");

        // checking IAS report certificate
        bytes memory repCertDer = repCerts[0].toBytes();
        bytes32 repKeyId = verifyEPIDReportCert(repCertDer, iasRepCertMgrAddr);

        // checking IAS report signature
        bytes memory repJson = rep[1].toBytes();
        require(
            Interface_IASReportCertMgr(iasRepCertMgrAddr).verifySign(
            repKeyId,
            sha256(repJson),
            rep[2].toBytes() // repSig
            ),
            "Invalid IAS report sign"
        );

        // checking IAS report JSON
        cert.enclaveHash = verifyEPIDAttestRep(
            stdRepData,
            repJson,
            keyRing,
            quoteStatusMap
        );
    }

    function extractDecentServerKey(
        DecentServerCertObj memory cert,
        X509CertNodes.CertNodesObj memory certNodes,
        bytes memory certDer,
        bytes memory keyRing
    )
        internal
        pure
    {
        bytes memory pubKeyDer = certDer.allBytesAt(certNodes.tbs.pubKeyNode);
        require(
            keyRing.contains(sha256(pubKeyDer)),
            "Decent Svr Key not in keyRing"
        );

        (bytes32 keyId, bytes32 curveId) =
            certNodes.tbs.extractPubKeyAlg(certDer);
        require(
            keyId == OIDs.OID_KEY_EC_PUBLIC,
            "Unsupported Key type"
        );
        require(
            curveId == OIDs.OID_KEY_EC_SECP256K1,
            "Unsupported curve"
        );

        cert.serverKeyAddr =
            LibSecp256k1Sha256.pubKeyBytesToAddr(
                certNodes.tbs.extractPubKeyBytes(certDer)
            );
    }

    function verifySelfSign(
        DecentServerCertObj memory cert,
        X509CertNodes.CertNodesObj memory certNodes,
        bytes memory certDer
    )
        internal
        pure
    {
        (bytes32 signR, bytes32 signS) =
            certNodes.extractSignRS(certDer);
        require(
            LibSecp256k1Sha256.verifySignMsg(
                cert.serverKeyAddr,
                certDer.allBytesAt(certNodes.tbs.root),
                signR,
                signS
            ),
            "Invalid self-signed cert"
        );
    }

    function loadCert(
        DecentServerCertObj memory cert,
        X509CertNodes.CertNodesObj memory certNodes,
        bytes memory certDer,
        address iasRepCertMgrAddr,
        mapping(bytes32 => bool) storage quoteStatusMap
    )
        internal
    {
        // Check signature algorithm
        bytes32 algType;
        // algType = certDer.bytes32At(
        //     certDer.firstChildOf(certNodes.tbs.algTypeNode)
        // );
        // require(algType == OIDs.OID_ALG_ECDSA_SHA_256, "alg type mismatch");
        algType = certDer.bytes32At(
            certDer.firstChildOf(certNodes.algTypeNode)
        );
        require(algType == OIDs.OID_ALG_ECDSA_SHA_256, "alg type mismatch");

        // extracting extensions
        X509Extension.ExtEntry[] memory extEntries =
            new X509Extension.ExtEntry[](5);
        extEntries[0].extnID = OIDs.OID_DECENT_EXT_VER;
        extEntries[1].extnID = OIDs.OID_DECENT_PLATFORM_ID;
        extEntries[2].extnID = OIDs.OID_DECENT_HASHED_KEYS;
        extEntries[3].extnID = OIDs.OID_DECENT_STD_REP_DATA;
        extEntries[4].extnID = OIDs.OID_DECENT_ATTESTATION;

        X509Extension.extractNeededExtensions(
            certDer,
            certNodes.tbs.extNode,
            certNodes.algTypeNode,
            extEntries
        );

        // Decent Cert version
        require(
            extEntries[0].isParsed &&
            extEntries[0].extnValue.length == 1 &&
            uint8(extEntries[0].extnValue[0]) == 49, // '1' == 49
            "Unsupported Decent ver"
        );
        require(extEntries[1].isParsed, "Platform ID not found");

        // Decent Keyring
        require(extEntries[2].isParsed, "keyRing not found");
        bytes memory keyRing = extEntries[2].extnValue;

        // Platform specific verification
        if (keccak256(extEntries[1].extnValue) == keccak256("SGX_EPID")) {

            require(extEntries[3].isParsed, "stdRepData not found");
            require(extEntries[4].isParsed, "attestRep not found");

            verifyEPIDAttestRepSet(
                cert,
                extEntries[3].extnValue, // stdRepData
                extEntries[4].extnValue, // attestRep
                keyRing,
                iasRepCertMgrAddr,
                quoteStatusMap
            );
        }
        else {
            require(false, "Unsupported platform");
        }

        // Decent server public key
        extractDecentServerKey(cert, certNodes, certDer, keyRing);

        // OPTIONAL: verify self-signed cert
        // verifySelfSign(cert, certNodes, certDer);

        // finished verification
        cert.isVerified = true;
    }

    function loadCert(
        DecentServerCertObj memory cert,
        bytes memory certDer,
        address iasRepCertMgrAddr,
        mapping(bytes32 => bool) storage quoteStatusMap
    )
        internal
    {
        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(certDer);
        loadCert(cert, certNodes, certDer, iasRepCertMgrAddr, quoteStatusMap);
    }

}
