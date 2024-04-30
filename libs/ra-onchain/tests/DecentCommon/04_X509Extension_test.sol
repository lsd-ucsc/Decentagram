// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";

// This import is required to use custom transaction context
// Although it may fail compilation in 'Solidity Compiler' plugin
// But it will work fine in 'Solidity Unit Testing' plugin
import "remix_accounts.sol";


import {X509Extension} from "../../contracts/X509Extension.sol";
import {X509Extension_proxy} from "./04_X509Extension.sol";


// File name has to end with '_test.sol', this file can contain more than one testSuite contracts
contract X509Extension_testSuit {

    using X509Extension for X509Extension.ExtEntry;

    /// 'beforeAll' runs before all other tests
    /// More special functions are: 'beforeEach', 'beforeAll', 'afterEach' & 'afterAll'
    function beforeAll() public {
    }

    function findIdxOfExtEntryTest() public {
        X509Extension_proxy proxy = new X509Extension_proxy();
        try proxy.findIdxOfExtEntryTest() {
            Assert.ok(true, "findIdxOfExtEntryTest should not throw");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error - findIdxOfExtEntryTest");
        }
    }

    function extractNeededExtensionsTest() public {
        X509Extension_proxy proxy = new X509Extension_proxy();
        try proxy.extractNeededExtensionsTest() {
            Assert.ok(true, "extractNeededExtensionsTest should not throw");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory /*lowLevelData*/) {
            Assert.ok(false, "unexpected error - extractNeededExtensionsTest");
        }
    }

}
