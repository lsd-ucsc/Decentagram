// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;


import {Asn1Decode, NodePtr} from "../libs/asn1-decode/Asn1Decode.sol";


library X509Extension {

    using Asn1Decode for bytes;
    using NodePtr for uint256;

    //===== structs =====
    struct ExtEntry {
        // for parsing
        // bool isNeeded;    // true if this entry should be parsed
        bool isParsed;    // true if this entry is parsed
        // uint256 nodePtr;  // node pointer of this entry

        // for storing
        bytes32 extnID;     // extension ID (OID) of this entry
        bool    isCritical; // true if this extension is critical
        bytes   extnValue;  // extension value of this entry
    }

    //===== functions =====

    function findIdxOfExtEntry(
        bytes32 extnID,
        ExtEntry[] memory extEntries
    )
        internal
        pure
        returns (uint256)
    {
        for (uint256 i = 0; i < extEntries.length; i++)
        {
            if (extEntries[i].extnID == extnID)
            {
                return i;
            }
        }
        return extEntries.length;
    }

    function extractOneExtension(
        bytes memory certDer,
        uint256 extnIDNode,
        ExtEntry memory extEntry
    )
        internal
        pure
    {
        // make sure this entry is unique in the given DER
        require(!extEntry.isParsed, "duplicated ext entry");

        uint256 nextNode = certDer.nextSiblingOf(extnIDNode);
        if (certDer[nextNode.ixs()] == 0x01)
        {
            // this extension has a critical field
            // 0x00: false, 0xFF: true
            extEntry.isCritical = (certDer[nextNode.ixf()] != 0x00);
            // extnValue should be the next node
            nextNode = certDer.nextSiblingOf(nextNode);
        }
        extEntry.extnValue = certDer.bytesAt(nextNode);

        // finish parsing this entry
        extEntry.isParsed = true;
    }

    function extractNeededExtensions(
        bytes memory certDer,
        uint256 extNode,
        uint256 endNode,
        ExtEntry[] memory extEntries
    )
        internal
        pure
    {
        extNode = certDer.firstChildOf(
            certDer.firstChildOf(extNode)
        );
        while (extNode < endNode)
        {
            uint256 extnIDNode = certDer.firstChildOf(extNode);
            bytes32 extnID = certDer.bytes32At(extnIDNode);

            // check if this extension is needed to be parsed
            uint256 idx = findIdxOfExtEntry(extnID, extEntries);
            if (idx < extEntries.length)
            {
                // extract this extension
                extractOneExtension(certDer, extnIDNode, extEntries[idx]);
            }

            // move to next extension
            extNode = certDer.nextSiblingOf(extNode);
        }
    }

}
