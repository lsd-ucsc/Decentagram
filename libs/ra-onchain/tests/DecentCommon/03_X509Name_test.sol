// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";

// This import is required to use custom transaction context
// Although it may fail compilation in 'Solidity Compiler' plugin
// But it will work fine in 'Solidity Unit Testing' plugin
import "remix_accounts.sol";


import {X509Name} from "../../contracts/X509Name.sol";
import {X509CertNodes} from "../../contracts/X509CertNodes.sol";

import {TestCerts} from "../TestCerts.sol";


// File name has to end with '_test.sol', this file can contain more than one testSuite contracts
contract X509CertName_testSuite {
    using X509Name for X509Name.CertNameObj;
    using X509CertNodes for X509CertNodes.CertNodesObj;

    /// 'beforeAll' runs before all other tests
    /// More special functions are: 'beforeEach', 'beforeAll', 'afterEach' & 'afterAll'
    function beforeAll() public {
    }

    function loadNameTest() public {
        bytes memory certDer = TestCerts.IAS_ROOT_CERT_DER;

        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(certDer);

        {
            X509Name.CertNameObj memory name;
            name.loadName(
                certDer,
                certNodes.tbs.issuerNode,
                certNodes.tbs.validityNode
            );
            Assert.equal(
                keccak256(bytes(name.cn)),
                keccak256(bytes(TestCerts.IAS_ROOT_CERT_NAME_CN)),
                "CN not match"
            );
            Assert.equal(
                keccak256(bytes(name.country)),
                keccak256(bytes(TestCerts.IAS_ROOT_CERT_NAME_C)),
                "C not match"
            );
            Assert.equal(
                keccak256(bytes(name.local)),
                keccak256(bytes(TestCerts.IAS_ROOT_CERT_NAME_L)),
                "L not match"
            );
            Assert.equal(
                keccak256(bytes(name.state)),
                keccak256(bytes(TestCerts.IAS_ROOT_CERT_NAME_ST)),
                "ST not match"
            );
            Assert.equal(
                keccak256(bytes(name.org)),
                keccak256(bytes(TestCerts.IAS_ROOT_CERT_NAME_O)),
                "O not match"
            );
        }

        {
            X509Name.CertNameObj memory name;
            name.loadName(
                certDer,
                certNodes.tbs.subjectNode,
                certNodes.tbs.pubKeyNode
            );
            Assert.equal(
                keccak256(bytes(name.cn)),
                keccak256(bytes(TestCerts.IAS_ROOT_CERT_NAME_CN)),
                "CN not match"
            );
            Assert.equal(
                keccak256(bytes(name.country)),
                keccak256(bytes(TestCerts.IAS_ROOT_CERT_NAME_C)),
                "C not match"
            );
            Assert.equal(
                keccak256(bytes(name.local)),
                keccak256(bytes(TestCerts.IAS_ROOT_CERT_NAME_L)),
                "L not match"
            );
            Assert.equal(
                keccak256(bytes(name.state)),
                keccak256(bytes(TestCerts.IAS_ROOT_CERT_NAME_ST)),
                "ST not match"
            );
            Assert.equal(
                keccak256(bytes(name.org)),
                keccak256(bytes(TestCerts.IAS_ROOT_CERT_NAME_O)),
                "O not match"
            );
        }
    }

    function getCNTest() public {
        bytes memory certDer = TestCerts.IAS_ROOT_CERT_DER;

        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(certDer);

        {
            string memory comName = X509Name.getCN(
                certDer,
                certNodes.tbs.issuerNode,
                certNodes.tbs.validityNode
            );
            Assert.equal(
                keccak256(bytes(comName)),
                keccak256(bytes(TestCerts.IAS_ROOT_CERT_NAME_CN)),
                "CN not match"
            );
        }

        {
            string memory comName = X509Name.getCN(
                certDer,
                certNodes.tbs.subjectNode,
                certNodes.tbs.pubKeyNode
            );
            Assert.equal(
                keccak256(bytes(comName)),
                keccak256(bytes(TestCerts.IAS_ROOT_CERT_NAME_CN)),
                "CN not match"
            );
        }

    }
}
