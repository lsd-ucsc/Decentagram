// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";

// This import is required to use custom transaction context
// Although it may fail compilation in 'Solidity Compiler' plugin
// But it will work fine in 'Solidity Unit Testing' plugin
import "remix_accounts.sol";


import {BytesUtils_proxy} from "./BytesUtils.sol";


// File name has to end with '_test.sol', this file can contain more than one testSuite contracts
contract BytesUtils_testSuite {

    /// 'beforeAll' runs before all other tests
    /// More special functions are: 'beforeEach', 'beforeAll', 'afterEach' & 'afterAll'
    function beforeAll() public {
    }

    function testSubstringSafe() public {
        BytesUtils_proxy testProxy = new BytesUtils_proxy();
        try testProxy.testSubstringSafe() {
            Assert.ok(true, "substringSafe success");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory lowLevelData) {
            Assert.ok(false, "unexpected error - substringSafe");
        }
    }

    function testSubstringFast() public {
        BytesUtils_proxy testProxy = new BytesUtils_proxy();
        try testProxy.testSubstringFast() {
            Assert.ok(true, "substringFast success");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory lowLevelData) {
            Assert.ok(false, "unexpected error - substringFast");
        }
    }

    function testSubstringUnsafe() public {
        BytesUtils_proxy testProxy = new BytesUtils_proxy();
        try testProxy.testSubstringUnsafe() {
            Assert.ok(true, "substringUnsafe success");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory lowLevelData) {
            Assert.ok(false, "unexpected error - substringUnsafe");
        }
    }

    function testSubstrstringFast() public {
        BytesUtils_proxy testProxy = new BytesUtils_proxy();
        try testProxy.testSubstrstringFast() {
            Assert.ok(true, "substrstringFast success");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory lowLevelData) {
            Assert.ok(false, "unexpected error - substrstringFast");
        }
    }

    function concatTest() public {
        BytesUtils_proxy testProxy = new BytesUtils_proxy();
        try testProxy.concatTest() {
            Assert.ok(true, "concatTest success");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory lowLevelData) {
            Assert.ok(false, "unexpected error - concatTest");
        }
    }

    function containsTest() public {
        BytesUtils_proxy testProxy = new BytesUtils_proxy();
        try testProxy.containsTest() {
            Assert.ok(true, "containsTest success");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch (bytes memory lowLevelData) {
            Assert.ok(false, "unexpected error - containsTest");
        }
    }

}
