// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";

// This import is required to use custom transaction context
// Although it may fail compilation in 'Solidity Compiler' plugin
// But it will work fine in 'Solidity Unit Testing' plugin
import "remix_accounts.sol";


import {IASRootCertMgr} from "../../contracts/IASRootCertMgr.sol";
import {TestCerts} from "../TestCerts.sol";


// File name has to end with '_test.sol', this file can contain more than one testSuite contracts
contract IASRootCertMgr_testSuit {

    /// 'beforeAll' runs before all other tests
    /// More special functions are: 'beforeEach', 'beforeAll', 'afterEach' & 'afterAll'
    function beforeAll() public {
    }

    function constructTest() public {
        try new IASRootCertMgr(TestCerts.IAS_ROOT_CERT_DER)
            returns (IASRootCertMgr iasRootCertMgr)
        {
            // getters
            Assert.equal(
                iasRootCertMgr.getNotAfter(),
                TestCerts.IAS_ROOT_CERT_NOT_AFTER,
                "notAfter not match"
            );

            iasRootCertMgr.requireValidity();

            (bytes memory pubKeyMod, bytes memory pubKeyExp) =
                iasRootCertMgr.getPubKey();

            Assert.equal(
                keccak256(pubKeyMod),
                keccak256(TestCerts.IAS_ROOT_CERT_KEY_MOD),
                "pubKey mod not match"
            );
            Assert.equal(
                keccak256(pubKeyExp),
                keccak256(TestCerts.IAS_ROOT_CERT_KEY_EXP),
                "pubKey exp not match"
            );
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unknown error");
        }
    }
}
