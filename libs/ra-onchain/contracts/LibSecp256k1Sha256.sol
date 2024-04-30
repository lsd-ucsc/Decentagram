// SPDX-License-Identifier: MIT
pragma solidity >=0.8.0 <0.9.0;


import {Asn1Decode} from "../libs/asn1-decode/Asn1Decode.sol";
import {BytesUtils} from "../libs/ens-contracts/BytesUtils.sol";

import {X509CertNodes} from "./X509CertNodes.sol";


library LibSecp256k1Sha256 {

    using Asn1Decode for bytes;
    using BytesUtils for bytes;

    //===== Functions =====

    function extractPubKeyAlg(
        X509CertNodes.CertTbsNodesObj memory certTbsNodes,
        bytes memory certDer
    )
        internal
        pure
        returns (bytes32 keyId, bytes32 curveId)
    {
        uint256 algNode = certDer.firstChildOf(certTbsNodes.pubKeyNode);
        uint256 keyIdNode = certDer.firstChildOf(algNode);
        uint256 curveIdNode = certDer.nextSiblingOf(keyIdNode);
        keyId = certDer.bytes32At(keyIdNode);
        curveId = certDer.bytes32At(curveIdNode);
    }

    function extractSignRS(
        X509CertNodes.CertNodesObj memory certNodes,
        bytes memory certDer
    )
        internal
        pure
        returns (bytes32 r, bytes32 s)
    {
        bytes memory signBytes = certDer.bitstringAt(certNodes.sigNode);
        uint256 rNode = signBytes.firstChildOf(signBytes.root());
        uint256 sNode = signBytes.nextSiblingOf(rNode);
        bytes memory rBytes = signBytes.uintBytesAt(rNode);
        bytes memory sBytes = signBytes.uintBytesAt(sNode);
        r = rBytes.readBytes32(0);
        s = sBytes.readBytes32(0);
    }

    function extractPubKeyBytes(
        X509CertNodes.CertTbsNodesObj memory certTbsNodes,
        bytes memory certDer
    )
        internal
        pure
        returns (bytes memory pubKeyBytes)
    {
        uint256 pubKeyNode = certDer.nextSiblingOf(
            certDer.firstChildOf(certTbsNodes.pubKeyNode)
        );
        pubKeyBytes = certDer.bitstringAt(pubKeyNode);

        // we only supports ECP_PF_UNCOMPRESSED format
        // which is 0x04 || x || y
        require(
            (pubKeyBytes.length == 65) && (pubKeyBytes[0] == 0x04),
            "pkey isn't uncompressed format"
        );

        pubKeyBytes = pubKeyBytes.substringFast(1, pubKeyBytes.length - 1);
    }

    function determineRecoverId(
        bool isCompressed,
        bool isXHigherThanOrder,
        bool isYOdd
    )
        internal
        pure
        returns (uint8 recoverId)
    {
        // - Uncompressed format
        //   - X is lower than the curve order
        //     - Y is even - 27
        //     - Y is odd  - 28
        //   - X is higher than the curve order
        //     - Y is even - 29
        //     - Y is odd  - 30
        // - Compressed format
        //   - X is lower than the curve order
        //     - Y is even - 31
        //     - Y is odd  - 32
        //   - X is higher than the curve order
        //     - Y is even - 33
        //     - Y is odd  - 34
        // source: https://bitcoin.stackexchange.com/questions/38351/ecdsa-v-r-s-what-is-v
        return
            (isCompressed ? 31 : 27) +
            (isXHigherThanOrder ? 2 : 0) +
            (isYOdd ? 1 : 0);
    }

    function pubKeyBytesToAddr(
        bytes memory pubKeyBytes
    )
        internal
        pure
        returns (address addr)
    {
        require(pubKeyBytes.length == 64, "len(pkey)!=64");
        addr = address(uint160(uint256(keccak256(pubKeyBytes))));
    }

    function verifySignMsg(
        address signerAddr,
        bytes memory msgBytes,
        bytes32 r,
        bytes32 s
    )
        internal
        pure
        returns (bool)
    {
        // we have to try 27 and then 28, because it's depending on the R.Y's
        // parity bit, which we don't have access to at this point
        // (R.Y is the randome point R's Y coordinate value, where the R is
        // randomly generated during the signing process; the X coordinate is
        // the r)
        return
            (ecrecover(sha256(msgBytes), 27, r, s) == signerAddr) ||
            (ecrecover(sha256(msgBytes), 28, r, s) == signerAddr);
    }

    function verifySignHash(
        address signerAddr,
        bytes32 msgHash,
        bytes32 r,
        bytes32 s
    )
        internal
        pure
        returns (bool)
    {
        return
            (ecrecover(msgHash, 27, r, s) == signerAddr) ||
            (ecrecover(msgHash, 28, r, s) == signerAddr);
    }

}
