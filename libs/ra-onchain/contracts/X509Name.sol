// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;


import {Asn1Decode} from "../libs/asn1-decode/Asn1Decode.sol";

import {OIDs} from "./Constants.sol";


library X509Name {

    using Asn1Decode for bytes;

    //===== structs =====
    struct CertNameObj {
        string cn;
        string country;
        string local;
        string state;
        string org;
        string orgUnit;
    }

    //===== functions =====

    function loadName(
        CertNameObj memory name,
        bytes memory certDer,
        uint256 nameNode,
        uint256 endNode,
        bool readComName,
        bool readCountry,
        bool readLocal,
        bool readState,
        bool readOrg,
        bool readOrgUnit
    )
        internal
        pure
    {
        require(nameNode < endNode, "invalid node ptr");

        uint256 relDistNameNode = certDer.firstChildOf(nameNode);
        while (relDistNameNode < endNode)
        {
            uint256 oidNode = certDer.firstChildOf(relDistNameNode);
            oidNode = certDer.firstChildOf(oidNode);

            bytes32 oid = certDer.bytes32At(oidNode);
            if (readComName && oid == OIDs.OID_NAME_CN)
            {
                name.cn = certDer.stringAt(
                    certDer.nextSiblingOf(oidNode)
                );
            }
            else if (readCountry && oid == OIDs.OID_NAME_COUNTRY)
            {
                name.country = certDer.stringAt(
                    certDer.nextSiblingOf(oidNode)
                );
            }
            else if (readLocal && oid == OIDs.OID_NAME_LOCAL)
            {
                name.local = certDer.stringAt(
                    certDer.nextSiblingOf(oidNode)
                );
            }
            else if (readState && oid == OIDs.OID_NAME_STATE)
            {
                name.state = certDer.stringAt(
                    certDer.nextSiblingOf(oidNode)
                );
            }
            else if (readOrg && oid == OIDs.OID_NAME_ORG)
            {
                name.org = certDer.stringAt(
                    certDer.nextSiblingOf(oidNode)
                );
            }
            else if (readOrgUnit && oid == OIDs.OID_NAME_ORG_UNIT)
            {
                name.orgUnit = certDer.stringAt(
                    certDer.nextSiblingOf(oidNode)
                );
            }
            else
            {
                // ignore
            }

            relDistNameNode = certDer.nextSiblingOf(relDistNameNode);
        }
    }

    function loadName(
        CertNameObj memory name,
        bytes memory certDer,
        uint256 nameNode,
        uint256 endNode
    )
        internal
        pure
    {
        loadName(
            name,
            certDer,
            nameNode,
            endNode,
            true, // readComName
            true, // readCountry
            true, // readLocal
            true, // readState
            true, // readOrg
            true  // readOrgUnit
        );
    }

    function getCN(
        bytes memory certDer,
        uint256 nameNode,
        uint256 endNode
    )
        internal
        pure
        returns (string memory)
    {
        CertNameObj memory name;
        loadName(
            name,
            certDer,
            nameNode,
            endNode,
            true,  // readComName
            false, // readCountry
            false, // readLocal
            false, // readState
            false, // readOrg
            false  // readOrgUnit
        );
        return name.cn;
    }
}
