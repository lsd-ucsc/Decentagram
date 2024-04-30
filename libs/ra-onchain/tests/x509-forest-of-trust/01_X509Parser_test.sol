// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";

// This import is required to use custom transaction context
// Although it may fail compilation in 'Solidity Compiler' plugin
// But it will work fine in 'Solidity Unit Testing' plugin
import "remix_accounts.sol";


import {X509Parser_proxy} from "./01_X509Parser.sol";


// File name has to end with '_test.sol', this file can contain more than one testSuite contracts
contract X509Parser_testSuite {

    /// 'beforeAll' runs before all other tests
    /// More special functions are: 'beforeEach', 'beforeAll', 'afterEach' & 'afterAll'
    function beforeAll() public {
    }

    function toTimestampTest() public {
        X509Parser_proxy proxy = new X509Parser_proxy();
        try proxy.toTimestampTest() {
            Assert.ok(true, "toTimestampTest should pass");
        } catch Error(string memory reason) {
            Assert.ok(false, reason);
        } catch {
            Assert.ok(false, "unexpected error - toTimestampTest");
        }
    }

}
