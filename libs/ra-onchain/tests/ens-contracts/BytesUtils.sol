// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";


import {BytesUtils} from "../../libs/ens-contracts/BytesUtils.sol";


contract BytesUtils_proxy {

    bytes constant TEST_INPUT_BYTES_128 =
        hex"11223344556677889900AABBCCDDEEFF"  // 16 bytes
        hex"21223344556677889900AABBCCDDEEFF"  // 32 bytes
        hex"31223344556677889900AABBCCDDEEFF"  // 48 bytes
        hex"41223344556677889900AABBCCDDEEFF"  // 64 bytes
        hex"51223344556677889900AABBCCDDEEFF"  // 80 bytes
        hex"61223344556677889900AABBCCDDEEFF"  // 96 bytes
        hex"71223344556677889900AABBCCDDEEFF"  // 112 bytes
        hex"81223344556677889900AABBCCDDEEFF"; // 128 bytes

    bytes constant TEST_INPUT_STR_128 =
        "1234567890ABCDEF"  // 16 bytes
        "2234567890ABCDEF"  // 32 bytes
        "3234567890ABCDEF"  // 48 bytes
        "4234567890ABCDEF"  // 64 bytes
        "5234567890ABCDEF"  // 80 bytes
        "6234567890ABCDEF"  // 96 bytes
        "7234567890ABCDEF"  // 112 bytes
        "8234567890ABCDEF"; // 128 bytes

    function testSubstringSafe() external {
        bytes memory b = TEST_INPUT_BYTES_128;

        // length aligned to 32 bytes
        {
            bytes memory expected =
                hex"31223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"41223344556677889900AABBCCDDEEFF"; // 32 bytes

            bytes memory actual = BytesUtils.substringSafe(b, 32, 32);
            Assert.equal(
                keccak256(actual),
                keccak256(expected),
                "substringSafe (aligned) mismatch"
            );
        }

        // length not aligned to 32 bytes
        {

            bytes memory expected =
                hex"31223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"41223344556677889900AABBCCDDEEFF"  // 32 bytes
                hex"51223344556677889900AABBCCDDEEFF"; // 48 bytes

            bytes memory actual = BytesUtils.substringSafe(b, 32, 48);
            Assert.equal(
                keccak256(actual),
                keccak256(expected),
                "substringSafe (non-aligned) mismatch"
            );
        }
    }

    function testSubstringFast() external {
        bytes memory b = TEST_INPUT_BYTES_128;

        // length aligned to 32 bytes
        {
            bytes memory expected =
                hex"31223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"41223344556677889900AABBCCDDEEFF"; // 32 bytes

            bytes memory actual = BytesUtils.substringFast(b, 32, 32);
            Assert.equal(
                keccak256(actual),
                keccak256(expected),
                "substringFast (aligned) mismatch"
            );
        }

        // length not aligned to 32 bytes
        {
            bytes memory expected =
                hex"31223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"41223344556677889900AABBCCDDEEFF"  // 32 bytes
                hex"51223344556677889900AABBCCDDEEFF"; // 48 bytes

            bytes memory actual = BytesUtils.substringFast(b, 32, 48);
            Assert.equal(
                keccak256(actual),
                keccak256(expected),
                "substringFast (non-aligned) mismatch"
            );
        }
    }

    function testSubstringUnsafe() external {

        bytes memory b = TEST_INPUT_BYTES_128;

        // length aligned to 32 bytes
        {
            bytes memory expected =
                hex"31223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"41223344556677889900AABBCCDDEEFF"; // 32 bytes

            bytes memory actual = BytesUtils.substringUnsafe(b, 32, 32);
            Assert.equal(
                keccak256(actual),
                keccak256(expected),
                "substringUnsafe (aligned) mismatch"
            );
        }

        // length not aligned to 32 bytes
        {

            bytes memory expected =
                hex"31223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"41223344556677889900AABBCCDDEEFF"  // 32 bytes
                hex"51223344556677889900AABBCCDDEEFF"; // 48 bytes

            bytes memory actual = BytesUtils.substringUnsafe(b, 32, 48);
            Assert.equal(
                keccak256(actual),
                keccak256(expected),
                "substringUnsafe (non-aligned) mismatch"
            );
        }
    }

    function testSubstrstringFast() external {
        bytes memory b = TEST_INPUT_STR_128;

        // length aligned to 32 bytes
        {
            string memory expected =
                "3234567890ABCDEF"  // 16 bytes
                "4234567890ABCDEF"; // 32 bytes

            string memory actual = BytesUtils.substrstringFast(b, 32, 32);
            Assert.equal(
                keccak256(bytes(actual)),
                keccak256(bytes(expected)),
                "substrstringFast (aligned) mismatch"
            );
        }

        // length not aligned to 32 bytes
        {
            string memory expected =
                "3234567890ABCDEF"  // 16 bytes
                "4234567890ABCDEF"  // 32 bytes
                "5234567890ABCDEF"; // 48 bytes

            string memory actual = BytesUtils.substrstringFast(b, 32, 48);
            Assert.equal(
                keccak256(bytes(actual)),
                keccak256(bytes(expected)),
                "substrstringFast (non-aligned) mismatch"
            );
        }
    }

    function concatTest() external {
        {
            bytes memory exp =
                hex"11223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"21223344556677889900AABBCCDDEEFF"  // 32 bytes
                hex"31223344556677889900AABBCCDDEEFF"  // 48 bytes
                hex"41223344556677889900AABBCCDDEEFF"; // 64 bytes
            bytes32 a =
                hex"11223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"21223344556677889900AABBCCDDEEFF"; // 32 bytes
            bytes32 b =
                hex"31223344556677889900AABBCCDDEEFF"  // 48 bytes
                hex"41223344556677889900AABBCCDDEEFF"; // 64 bytes

            bytes memory actual = abi.encodePacked(a, b);

            Assert.equal(
                actual.length,
                exp.length,
                "concat (bytes32, bytes32) length mismatch"
            );
            Assert.equal(
                keccak256(actual),
                keccak256(exp),
                "concat (bytes32, bytes32) mismatch"
            );
        }

        {
            bytes memory exp =
                hex"11223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"FFEEDDCCBBAA00998877665544332211"  // 32 bytes
                hex"1234567890ABCDEF1234567890ABCDEF"  // 48 bytes
                hex"FEDCBA0987654321FEDCBA0987654321"; // 64 bytes
            bytes32 a =
                hex"11223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"FFEEDDCCBBAA00998877665544332211"; // 32 bytes
            bytes32 b =
                hex"1234567890ABCDEF1234567890ABCDEF"  // 48 bytes
                hex"FEDCBA0987654321FEDCBA0987654321"; // 64 bytes

            bytes memory actual = abi.encodePacked(a, b);

            Assert.equal(
                actual.length,
                exp.length,
                "concat (bytes32, bytes32) length mismatch"
            );
            Assert.equal(
                keccak256(actual),
                keccak256(exp),
                "concat (bytes32, bytes32) mismatch"
            );
        }
    }

    function containsTest() external {
        {
            bytes memory concatList = hex"";
            bytes32 target =
                hex"11223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"21223344556677889900AABBCCDDEEFF"; // 32 bytes

            bool res = BytesUtils.contains(concatList, target);
            Assert.equal(res, false, "contains (empty list) mismatch");
        }
        {
            bytes memory concatList =
                hex"11223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"21223344556677889900AABBCCDDEEFF"; // 32 bytes

            bytes32 target =
                hex"11223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"21223344556677889900AABBCCDDEEFF"; // 32 bytes

            bool res = BytesUtils.contains(concatList, target);
            Assert.equal(res, true, "contains 1 mismatch");
        }
        {
            bytes memory concatList =
                hex"11223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"21223344556677889900AABBCCDDEEFF"; // 32 bytes

            bytes32 target =
                hex"11223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"FFEEDDCCBBAA00998877665544332211"; // 32 bytes

            bool res = BytesUtils.contains(concatList, target);
            Assert.equal(res, false, "contains 1 mismatch");
        }
        {
            bytes memory concatList =
                hex"11223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"FFEEDDCCBBAA00998877665544332211"  // 32 bytes
                hex"1234567890ABCDEF1234567890ABCDEF"  // 48 bytes
                hex"FEDCBA0987654321FEDCBA0987654321"; // 64 bytes

            bytes32 target =
                hex"11223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"FFEEDDCCBBAA00998877665544332211"; // 32 bytes

            bool res = BytesUtils.contains(concatList, target);
            Assert.equal(res, true, "contains 2 mismatch");
        }
        {
            bytes memory concatList =
                hex"11223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"FFEEDDCCBBAA00998877665544332211"  // 32 bytes
                hex"1234567890ABCDEF1234567890ABCDEF"  // 48 bytes
                hex"FEDCBA0987654321FEDCBA0987654321"; // 64 bytes

            bytes32 target =
                hex"1234567890ABCDEF1234567890ABCDEF"  // 16 bytes
                hex"FEDCBA0987654321FEDCBA0987654321"; // 32 bytes

            bool res = BytesUtils.contains(concatList, target);
            Assert.equal(res, true, "contains 2 mismatch");
        }
        {
            bytes memory concatList =
                hex"11223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"FFEEDDCCBBAA00998877665544332211"  // 32 bytes
                hex"1234567890ABCDEF1234567890ABCDEF"  // 48 bytes
                hex"FEDCBA0987654321FEDCBA0987654321"; // 64 bytes

            bytes32 target =
                hex"11223344556677889900AABBCCDDEEFF"  // 16 bytes
                hex"FEDCBA0987654321FEDCBA0987654321"; // 32 bytes

            bool res = BytesUtils.contains(concatList, target);
            Assert.equal(res, false, "contains 2 mismatch");
        }
    }

}
