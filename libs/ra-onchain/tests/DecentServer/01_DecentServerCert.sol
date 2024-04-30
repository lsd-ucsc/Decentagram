// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;


// This import is automatically injected by Remix
import "remix_tests.sol";

import {BytesUtils} from "../../libs/ens-contracts/BytesUtils.sol";

import {DecentServerCert} from "../../contracts/DecentServerCert.sol";
import {OIDs} from "../../contracts/Constants.sol";
import {X509CertNodes} from "../../contracts/X509CertNodes.sol";

import {TestCerts} from "../TestCerts.sol";


contract DecentServerCertBasics_proxy {

    using DecentServerCert for DecentServerCert.DecentServerCertObj;
    using X509CertNodes for X509CertNodes.CertNodesObj;

    //===== constants =====

    bytes constant REPORT_JSON =
        "{\"nonce\":\"B97DBE6183FC197B27BF5C7D9B0ADEC1\","
        "\"id\":\"150936180791254131159804720163106156575\","
        "\"timestamp\":\"2022-11-12T11:07:28.913701\",\"version\":4,"
        "\"epidPseudonym\":\"SON/Cz7tPAHEF4f7GbK6rDwL/oMXumIuvFn+rFQlST"
        "UZAvIHXOHUnWATyKeCbCXV+0yZ5cCossyjRj4fZm9sAP/NWnbXSSsBhXooOqyN"
        "kIIEKUeNHlC2RSnwc+xuaHUfgpsAedWU+TWkXMe4ag20vjydxW3YcVmbjdl50w"
        "FoEps=\","
        "\"advisoryURL\":\"https://security-center.intel.com\","
        "\"advisoryIDs\":[\"INTEL-SA-00161\",\"INTEL-SA-00219\",\"INTEL"
        "-SA-00289\",\"INTEL-SA-00334\",\"INTEL-SA-00615\"],"
        "\"isvEnclaveQuoteStatus\":\"CONFIGURATION_AND_SW_HARDENING_NEEDED\","
        "\"platformInfoBlob\":\"150200650000080000111102040101070000000"
        "000000000000C00000C000000020000000000000C1EFBA13635781B4B10A5A"
        "FFBBDE9B1B5FF952CA8436B875BD3C6912626B13480C3D23721325EBF61A50"
        "4A2D06656209237077FA11A94D1ADD8620CB385A97539EF\","
        "\"isvEnclaveQuoteBody\":\"AgABAB4MAAAMAAwAAAAAAFhD2LlfDahUgPFu0"
        "pHmSd0AAAAAAAAAAAAAAAAAAAAAExP//wECAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAABwAAAAAAAAAHAAAAAAAAAHSOwEI2yESCn"
        "9UGK7cbCk0frFukTMjags5Icq7P6b3nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAACD1xnnferKFHD2uvYqTXdDA8iZ22kCD5xw7h38CMfOngAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAADODHJLXqNyv6lgKWonfqSvItJAOan/199GJqBIq3jCt"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\"}";

    //===== member variables =====

    mapping(bytes32 => bool) m_quoteStatusMap;

    //===== constructor =====

    constructor() {
        m_quoteStatusMap[keccak256("OK")] = true;
        m_quoteStatusMap[keccak256("CONFIGURATION_NEEDED")] = true;
        m_quoteStatusMap[keccak256("SW_HARDENING_NEEDED")] = true;
        m_quoteStatusMap[keccak256("CONFIGURATION_AND_SW_HARDENING_NEEDED")] = true;
    }

    //===== functions =====

    function strFindTest() public {
        {
            bytes memory str = "ABCDEFG";
            bytes memory key = "CDE";
            uint256 idx = DecentServerCert.strFind(str, 1, key);
            Assert.equal(idx, 2, "strFind failed");
        }
        {
            bytes memory str = REPORT_JSON;

            bytes memory key1 = "\"isvEnclaveQuoteStatus\"";
            bytes memory key2 = "\"isvEnclaveQuoteBody\"";
            uint256 idx1 = DecentServerCert.strFind(str, 0, key1);
            uint256 idx2 = DecentServerCert.strFind(str, 0, key2);
            Assert.equal(idx1, 486, "strFind failed");
            Assert.equal(idx2, 782, "strFind failed");

            idx1 = DecentServerCert.strFind(str, idx1 + key1.length, "\"");
            idx2 = DecentServerCert.strFind(str, idx2 + key2.length, "\"");
            Assert.equal(idx1, 510, "strFind failed");
            Assert.equal(idx2, 804, "strFind failed");
        }
    }

    function jsonSimpleReadValPosTest() public {
        bytes memory str = REPORT_JSON;
        {
            bytes memory key = "\"isvEnclaveQuoteStatus\"";
            (uint256 idx, uint256 len) =
                DecentServerCert.jsonSimpleReadValPos(str, key);

            Assert.equal(idx, 511, "failed to find isvEnclaveQuoteStatus");
            Assert.equal(len, 548-idx, "failed to get len of isvEnclaveQuoteStatus");

            bytes memory val = BytesUtils.substringFast(str, idx, len);
            Assert.equal(
                keccak256(val),
                keccak256("CONFIGURATION_AND_SW_HARDENING_NEEDED"),
                "failed to get isvEnclaveQuoteStatus"
            );
        }
        {
            bytes memory key = "\"isvEnclaveQuoteBody\"";
            (uint256 idx, uint256 len) =
                DecentServerCert.jsonSimpleReadValPos(str, key);

            Assert.equal(idx, 805, "failed to find isvEnclaveQuoteBody");
            Assert.equal(len, 1381-idx, "failed to get len of isvEnclaveQuoteBody");

            bytes memory val = BytesUtils.substringFast(str, idx, len);
            Assert.equal(
                keccak256(val),
                keccak256("AgABAB4MAAAMAAwAAAAAAFhD2LlfDahUgPFu0pHmSd0AAAAAAAAAAAAAAAAAAAAAExP//wECAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABwAAAAAAAAAHAAAAAAAAAHSOwEI2yESCn9UGK7cbCk0frFukTMjags5Icq7P6b3nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACD1xnnferKFHD2uvYqTXdDA8iZ22kCD5xw7h38CMfOngAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADODHJLXqNyv6lgKWonfqSvItJAOan/199GJqBIq3jCtAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"),
                "failed to get isvEnclaveQuoteBody"
            );
        }
    }

    function verifyEPIDAttestationRepTest(
        address iasRepCertMgrAddr
    ) public {
        DecentServerCert.DecentServerCertObj memory cert;
        cert.verifyEPIDAttestRepSet(
            TestCerts.DECENT_SVR_CERT_STD_REP_DATA,
            TestCerts.DECENT_SVR_CERT_ATT_REP_RLP,
            TestCerts.DECENT_SVR_CERT_KEYRING,
            iasRepCertMgrAddr,
            m_quoteStatusMap
        );
        Assert.equal(
            cert.enclaveHash,
            TestCerts.DECENT_SVR_CERT_ENCL_HASH,
            "enclaveHash mismatch"
        );
    }

}


contract DecentServerCertCerts_proxy {

    using DecentServerCert for DecentServerCert.DecentServerCertObj;
    using X509CertNodes for X509CertNodes.CertNodesObj;

    //===== constants =====

    //===== member variables =====

    mapping(bytes32 => bool) m_quoteStatusMap;

    //===== constructor =====

    constructor() {
        m_quoteStatusMap[keccak256("OK")] = true;
        m_quoteStatusMap[keccak256("CONFIGURATION_NEEDED")] = true;
        m_quoteStatusMap[keccak256("SW_HARDENING_NEEDED")] = true;
        m_quoteStatusMap[keccak256("CONFIGURATION_AND_SW_HARDENING_NEEDED")] = true;
    }

    //===== functions =====

    function verifySelfSignTest() public {
        bytes memory certDer = TestCerts.DECENT_SVR_CERT_DER;
        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(certDer);

        DecentServerCert.DecentServerCertObj memory cert;
        cert.serverKeyAddr = TestCerts.DECENT_SVR_CERT_KEY_ADDR;

        cert.verifySelfSign(certNodes, certDer);
    }

    function extractDecentServerKeyTest() public {
        bytes memory certDer = TestCerts.DECENT_SVR_CERT_DER;
        X509CertNodes.CertNodesObj memory certNodes;
        certNodes.loadCertNodes(certDer);

        DecentServerCert.DecentServerCertObj memory cert;
        DecentServerCert.extractDecentServerKey(
            cert,
            certNodes,
            certDer,
            TestCerts.DECENT_SVR_CERT_KEYRING
        );

        Assert.equal(
            cert.serverKeyAddr,
            TestCerts.DECENT_SVR_CERT_KEY_ADDR,
            "serverKeyAddr mismatch"
        );
    }

    function loadCertTest(
        address iasRepCertMgrAddr
    ) public {
        bytes memory certDer = TestCerts.DECENT_SVR_CERT_DER;

        DecentServerCert.DecentServerCertObj memory cert;
        cert.loadCert(certDer, iasRepCertMgrAddr, m_quoteStatusMap);

        Assert.ok(
            cert.isVerified,
            "Decent server cert is not verified"
        );
        Assert.equal(
            cert.enclaveHash,
            TestCerts.DECENT_SVR_CERT_ENCL_HASH,
            "enclaveHash mismatch"
        );
        Assert.equal(
            cert.serverKeyAddr,
            TestCerts.DECENT_SVR_CERT_KEY_ADDR,
            "serverKeyAddr mismatch"
        );
    }

}
