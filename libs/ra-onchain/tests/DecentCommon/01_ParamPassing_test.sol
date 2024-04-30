// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";

// This import is required to use custom transaction context
// Although it may fail compilation in 'Solidity Compiler' plugin
// But it will work fine in 'Solidity Unit Testing' plugin
import "remix_accounts.sol";


import {LibParamPassing, ContraParamPassing} from "./01_ParamPassing.sol";


// File name has to end with '_test.sol', this file can contain more than one testSuite contracts
contract ParamPassing_testSuite {

    bytes private m_b;

    /// 'beforeAll' runs before all other tests
    /// More special functions are: 'beforeEach', 'beforeAll', 'afterEach' & 'afterAll'
    function beforeAll() public {
        m_b = new bytes(4096);
        m_b[0] = 0x01;
    }

    /**
     * library + memory + internal ==> Pass by reference
     */
    function libInternMemFunc() public {
        LibParamPassing.Struct1 memory s;
        uint256 retVal = LibParamPassing.internMemFunc(s);
        Assert.equal(s.a, 1, "s.a must be 1");
        Assert.equal(s.b, 2, "s.b must be 2");
        Assert.equal(retVal, 3, "retVal must be 3");
    }

    /**
     * library + private ==> Pass by reference (not visible)
     */
    function libPrivMemFunc() public {
        LibParamPassing.Struct1 memory s;
        uint256 retVal = LibParamPassing.privMemFuncProxy(s);
        Assert.equal(s.a, 1, "s.a must be 1");
        Assert.equal(s.b, 2, "s.b must be 2");
        Assert.equal(retVal, 3, "retVal must be 3");
    }

    /**
     * library + external ==> Pass by copy
     */
    function libExternMemFunc() public {
        LibParamPassing.Struct1 memory s;
        uint256 retVal = LibParamPassing.externMemFunc(s);
        Assert.equal(s.a, 0, "s.a must be 0");
        Assert.equal(s.b, 0, "s.b must be 0");
        Assert.equal(retVal, 3, "retVal must be 3");
    }

    /**
     * library + public ==> Pass by copy
     */
    function libPubMemFunc() public {
        LibParamPassing.Struct1 memory s;
        uint256 retVal = LibParamPassing.pubMemFunc(s);
        Assert.equal(s.a, 0, "s.a must be 0");
        Assert.equal(s.b, 0, "s.b must be 0");
        Assert.equal(retVal, 3, "retVal must be 3");
    }

    /**
     * contract + external ==> Pass by copy
     */
    function contraExternMemFunc() public {
        ContraParamPassing c = new ContraParamPassing();

        ContraParamPassing.Struct1 memory s;
        uint256 retVal = c.externMemFunc(s);
        Assert.equal(s.a, 0, "s.a must be 0");
        Assert.equal(s.b, 0, "s.b must be 0");
        Assert.equal(retVal, 3, "retVal must be 3");
    }

    /**
     * contract + public ==> Pass by copy
     */
    function contraPubMemFunc() public {
        ContraParamPassing c = new ContraParamPassing();

        ContraParamPassing.Struct1 memory s;
        uint256 retVal = c.pubMemFunc(s);
        Assert.equal(s.a, 0, "s.a must be 0");
        Assert.equal(s.b, 0, "s.b must be 0");
        Assert.equal(retVal, 3, "retVal must be 3");
    }

    /**
     * contract + internal ==> not visible
     */
    // function contraInternMemFunc() public {
    //     ContraParamPassing c = new ContraParamPassing();

    //     ContraParamPassing.Struct1 memory s;
    //     uint256 retVal = c.internMemFunc(s);
    //     Assert.equal(s.a, 1, "s.a must be 1");
    //     Assert.equal(s.b, 2, "s.b must be 2");
    //     Assert.equal(retVal, 3, "retVal must be 3");
    // }

    /**
     * contract + private ==> not visible
     */
    // function contraPrivMemFunc() public {
    //     ContraParamPassing c = new ContraParamPassing();

    //     ContraParamPassing.Struct1 memory s;
    //     uint256 retVal = c.privMemFunc(s);
    //     Assert.equal(s.a, 0, "s.a must be 0");
    //     Assert.equal(s.b, 0, "s.b must be 0");
    //     Assert.equal(retVal, 3, "retVal must be 3");
    // }

    function libGasDiff() public {
        bytes memory b = new bytes(4096);
        b[0] = 0x01;

        // library

        uint256 internMemGasUsed = gasleft();
        uint256 retVal = LibParamPassing.internMemFunc(b);
        internMemGasUsed -= gasleft();
        Assert.equal(retVal, 3, "retVal must be 3");

        uint256 privMemGasUsed = gasleft();
        retVal = LibParamPassing.privMemFuncProxy(b);
        privMemGasUsed -= gasleft();
        Assert.equal(retVal, 3, "retVal must be 3");

        uint256 externMemGasUsed = gasleft();
        retVal = LibParamPassing.externMemFunc(b);
        externMemGasUsed -= gasleft();
        Assert.equal(retVal, 3, "retVal must be 3");

        uint256 externCallGasUsed = gasleft();
        retVal = LibParamPassing.externCallFunc(b);
        externCallGasUsed -= gasleft();
        Assert.equal(retVal, 3, "retVal must be 3");

        Assert.lesserThan(internMemGasUsed, uint256(500), "internMemGasUsed");
        // Assert.equal(internMemGasUsed, 152, "internMemGasUsed");
        Assert.lesserThan(privMemGasUsed, uint256(500), "privMemGasUsed");
        // Assert.equal(privMemGasUsed, 189, "privMemGasUsed");
        Assert.greaterThan(externMemGasUsed, uint256(5000), "externMemGasUsed");
        // Assert.equal(externMemGasUsed, 15145, "externMemGasUsed");
        Assert.greaterThan(externCallGasUsed, uint256(5000), "externCallGasUsed");
        // Assert.equal(externCallGasUsed, 11264, "externCallGasUsed");

        // contract
        ContraParamPassing c = new ContraParamPassing();

        externMemGasUsed = gasleft();
        retVal = c.externMemFunc(b);
        externMemGasUsed -= gasleft();
        Assert.equal(retVal, 3, "retVal must be 3");

        externCallGasUsed = gasleft();
        retVal = c.externCallFunc(b);
        externCallGasUsed -= gasleft();
        Assert.equal(retVal, 3, "retVal must be 3");

        uint256 pubCallGasUsed = gasleft();
        retVal = c.pubCallFunc(b);
        pubCallGasUsed -= gasleft();
        Assert.equal(retVal, 3, "retVal must be 3");

        Assert.greaterThan(externMemGasUsed, uint256(5000), "externMemGasUsed");
        // Assert.equal(externMemGasUsed, 12207, "externMemGasUsed");
        Assert.greaterThan(externCallGasUsed, uint256(5000), "externCallGasUsed");
        // Assert.equal(externCallGasUsed, 11298, "externCallGasUsed");
        Assert.greaterThan(pubCallGasUsed, uint256(5000), "pubCallGasUsed");
        // Assert.equal(pubCallGasUsed, 11277, "pubCallGasUsed");

        pubCallGasUsed = gasleft();
        retVal = c.pubCallFunc(m_b);
        pubCallGasUsed -= gasleft();
        Assert.equal(retVal, 3, "retVal must be 3");
        Assert.greaterThan(pubCallGasUsed, uint256(50000), "pubCallGasUsed");
        // Assert.equal(pubCallGasUsed, 281951, "pubCallGasUsed");

        uint256 internCallGasUsed = c.forwardInternCallFunc(b);
        Assert.lesserThan(internCallGasUsed, uint256(500), "internCallGasUsed");
        // Assert.equal(internCallGasUsed, 166, "internCallGasUsed");
    }

}
