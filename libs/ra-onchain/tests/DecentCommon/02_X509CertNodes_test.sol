// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";

// This import is required to use custom transaction context
// Although it may fail compilation in 'Solidity Compiler' plugin
// But it will work fine in 'Solidity Unit Testing' plugin
import "remix_accounts.sol";


import {Asn1Decode, NodePtr} from "../../libs/asn1-decode/Asn1Decode.sol";

import {X509CertNodes} from "../../contracts/X509CertNodes.sol";
import {X509Timestamp} from "../../contracts/X509Timestamp.sol";
import {OIDs} from "../../contracts/Constants.sol";

import {TestCerts} from "../TestCerts.sol";
import {X509CertNodes_utils} from "./02_X509CertNodes.sol";


contract X509CertNodes_testSuit {

    using Asn1Decode for bytes;
    using X509CertNodes for X509CertNodes.CertTbsNodesObj;
    using X509CertNodes for X509CertNodes.CertNodesObj;
    using X509Timestamp for X509CertNodes.CertTbsNodesObj;

    /// 'beforeAll' runs before all other tests
    /// More special functions are: 'beforeEach', 'beforeAll', 'afterEach' & 'afterAll'
    function beforeAll() public {
    }

    function loadTbsNodesTest() public {
        bytes memory certDer = TestCerts.IAS_ROOT_CERT_DER;

        uint256 certRootNode = certDer.root();

        // tbsCertificate
        uint256 tbsNode = certDer.firstChildOf(certRootNode);

        X509CertNodes.CertTbsNodesObj memory tbsNodes;
        // uint256 loadTbsGasUsed = gasleft();
        tbsNodes.loadTbsNodes(certDer, tbsNode);
        // loadTbsGasUsed -= gasleft();
        // Assert.equal(loadTbsGasUsed, 9330, "");
        Assert.equal(tbsNodes.root, tbsNode, "tbs node ptr not match");

        // version
        Assert.equal(tbsNodes.version, 2, "version must be 2");

        // serialNumber
        Assert.equal(
            uint8(certDer[NodePtr.ixs(tbsNodes.serialNode)]),
            uint8(0x02),
            "serial node tag not match"
        );
        uint256 serial = certDer.uintAt(tbsNodes.serialNode);
        Assert.equal(serial, 15062137621417537684, "serial num not match");

        // signature
        bytes32 algType = certDer.bytes32At(
            certDer.firstChildOf(tbsNodes.algTypeNode)
        );
        Assert.equal(algType, OIDs.OID_ALG_RSA_SHA_256, "alg type not match");

        // issuer
        Assert.lesserThan(
            tbsNodes.issuerNode,
            tbsNodes.validityNode,
            "issuer node ptr out of order"
        );

        // validity
        uint256 notBeforeNode = certDer.firstChildOf(tbsNodes.validityNode);
        uint256 notAfterNode = certDer.nextSiblingOf(notBeforeNode);
        bytes memory notBefore = certDer.bytesAt(notBeforeNode);
        bytes memory notAfter = certDer.bytesAt(notAfterNode);
        Assert.equal(notBefore.length, 13, "notBefore length not match");
        Assert.equal(notAfter.length, 15, "notAfter length not match");
        Assert.equal(
            keccak256(notBefore),
            keccak256(hex"3136313131343135333733315A"),
            "notBefore not match"
        );
        Assert.equal(
            keccak256(notBefore),
            keccak256("161114153731Z"),
            "notBefore not match"
        );
        Assert.equal(
            keccak256(notAfter),
            keccak256(hex"32303439313233313233353935395A"),
            "notAfter not match"
        );
        Assert.equal(
            keccak256(notAfter),
            keccak256("20491231235959Z"),
            "notAfter not match"
        );

        // subject
        Assert.lesserThan(
            tbsNodes.subjectNode,
            tbsNodes.pubKeyNode,
            "subject node ptr out of order"
        );

        // subjectPublicKeyInfo
        bytes memory pubKeyInfo = certDer.allBytesAt(tbsNodes.pubKeyNode);
        Assert.equal(pubKeyInfo.length, 422, "pubKeyInfo length not match");
        Assert.equal(
            keccak256(pubKeyInfo),
            keccak256(TestCerts.IAS_ROOT_CERT_KEY_DER),
            "pubKeyInfo not match"
        );
    }

    function loadCertNodesTest() public {
        bytes memory certDer = TestCerts.IAS_ROOT_CERT_DER;

        X509CertNodes.CertNodesObj memory certNodes;
        // uint256 loadCertGasUsed = gasleft();
        certNodes.loadCertNodes(certDer);
        // loadCertGasUsed -= gasleft();
        // Assert.equal(loadCertGasUsed, 13792, "");

        // signatureAlgorithm
        bytes32 algType = certDer.bytes32At(
            certDer.firstChildOf(certNodes.algTypeNode)
        );
        Assert.equal(algType, OIDs.OID_ALG_RSA_SHA_256, "alg type not match");

        // signatureValue
        bytes memory sigValue = certDer.bitstringAt(certNodes.sigNode);
        Assert.equal(
            sigValue.length,
            TestCerts.IAS_ROOT_CERT_SIGN.length,
            "sigValue length not match"
        );
        Assert.equal(
            keccak256(sigValue),
            keccak256(TestCerts.IAS_ROOT_CERT_SIGN),
            "sigValue not match"
        );
    }

    function getValidityTimestampsTest() public {
        bytes memory certDer = TestCerts.IAS_ROOT_CERT_DER;

        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(certDer);

        (uint256 notBefore, uint256 notAfter) =
            certNodes.tbs.getValidityTimestamps(certDer);
        Assert.equal(notBefore, 1479137851, "notBefore not match");
        Assert.equal(notAfter,  2524607999, "notAfter not match");
    }

    function gasEval() public {
        X509CertNodes_utils u = new X509CertNodes_utils();

        bytes memory certDer = TestCerts.IAS_ROOT_CERT_DER;
        uint256 gasUsed;
        uint256 retVal;

        X509CertNodes.CertNodesObj memory certNodes1;
        gasUsed = gasleft();
        certNodes1.loadCertNodes(TestCerts.IAS_ROOT_CERT_DER);
        gasUsed -= gasleft();
        // Assert.equal(gasUsed, 14118, "gasUsed");

        X509CertNodes.CertNodesObj memory certNodes2;
        gasUsed = gasleft();
        certNodes2.loadCertNodes(certDer);
        gasUsed -= gasleft();
        // Assert.equal(gasUsed, 13792, "gasUsed");

        gasUsed = gasleft();
        retVal = u.certNodesMem(certNodes1);
        gasUsed -= gasleft();
        Assert.equal(retVal, 1, "retVal!=1");
        //Assert.equal(gasUsed, 1642, "gasUsed");

        gasUsed = gasleft();
        retVal = u.certNodesCall(certNodes1);
        gasUsed -= gasleft();
        Assert.equal(retVal, 1, "retVal!=1");
        // Assert.equal(gasUsed, 1190, "gasUsed");
    }
}
