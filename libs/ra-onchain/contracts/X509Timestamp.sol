// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;


import {Asn1Decode} from "../libs/asn1-decode/Asn1Decode.sol";
import {X509Parser} from "../libs/x509-forest-of-trust/X509Parser.sol";

import {X509CertNodes} from "./X509CertNodes.sol";


library X509Timestamp {

    using Asn1Decode for bytes;

    //===== functions =====

    function getValidityTimestamps(
        X509CertNodes.CertTbsNodesObj memory tbsNodes,
        bytes memory certDer
    )
        internal
        pure
        returns (uint256 notBefore, uint256 notAfter)
    {
        uint256 notBeforeNode = certDer.firstChildOf(tbsNodes.validityNode);
        uint256 notAfterNode = certDer.nextSiblingOf(notBeforeNode);

        notBefore =
            X509Parser.toTimestamp(certDer.bytesAt(notBeforeNode));

        notAfter =
            X509Parser.toTimestamp(certDer.bytesAt(notAfterNode));
    }

}
