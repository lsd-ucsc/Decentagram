// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;


// This import is automatically injected by Remix
import "remix_tests.sol";

import {OIDs} from "../../contracts/Constants.sol";
import {X509CertNodes} from "../../contracts/X509CertNodes.sol";
import {X509Extension} from "../../contracts/X509Extension.sol";

import {TestCerts} from "../TestCerts.sol";


contract X509Extension_proxy {

    using X509CertNodes for X509CertNodes.CertNodesObj;

    function findIdxOfExtEntryTest() public {
        X509Extension.ExtEntry[] memory extEntries =
            new X509Extension.ExtEntry[](3);

        extEntries[0].extnID = OIDs.OID_DECENT_EXT_VER;
        extEntries[1].extnID = OIDs.OID_DECENT_PLATFORM_ID;
        extEntries[2].extnID = OIDs.OID_DECENT_HASHED_KEYS;

        uint256 idx;

        idx = X509Extension.findIdxOfExtEntry(
            OIDs.OID_DECENT_HASHED_KEYS,
            extEntries
        );
        Assert.equal(idx, 2, "wrong index");

        idx = X509Extension.findIdxOfExtEntry(
            OIDs.OID_DECENT_PLATFORM_ID,
            extEntries
        );
        Assert.equal(idx, 1, "wrong index");

        idx = X509Extension.findIdxOfExtEntry(
            OIDs.OID_DECENT_EXT_VER,
            extEntries
        );
        Assert.equal(idx, 0, "wrong index");

        idx = X509Extension.findIdxOfExtEntry(
            OIDs.OID_DECENT_STD_REP_DATA,
            extEntries
        );
        Assert.equal(idx, extEntries.length, "wrong index");

        idx = X509Extension.findIdxOfExtEntry(
            OIDs.OID_NAME_CN,
            extEntries
        );
        Assert.equal(idx, extEntries.length, "wrong index");
    }

    function extractNeededExtensionsTest() public {
        X509Extension.ExtEntry[] memory extEntries =
            new X509Extension.ExtEntry[](3);

        extEntries[0].extnID = OIDs.OID_DECENT_EXT_VER;
        extEntries[1].extnID = OIDs.OID_DECENT_PLATFORM_ID;
        extEntries[2].extnID = OIDs.OID_NAME_CN;

        bytes memory certDer = TestCerts.DECENT_SVR_CERT_DER;

        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(certDer);

        X509Extension.extractNeededExtensions(
            certDer,
            certNodes.tbs.extNode,
            certNodes.algTypeNode,
            extEntries
        );

        Assert.equal(extEntries[0].isParsed, true, "ver not parsed");
        Assert.equal(extEntries[0].isCritical, false, "ver critical");
        Assert.equal(
            keccak256(extEntries[0].extnValue),
            keccak256("1"),
            "wrong ver value"
        );

        Assert.equal(extEntries[1].isParsed, true, "platId not parsed");
        Assert.equal(extEntries[1].isCritical, false, "platId critical");
        Assert.equal(
            keccak256(extEntries[1].extnValue),
            keccak256("SGX_EPID"),
            "wrong platId value"
        );

        Assert.equal(extEntries[2].isParsed, false, "cn parsed");
    }

}
