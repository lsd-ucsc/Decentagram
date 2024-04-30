// SPDX-License-Identifier: MIT
pragma solidity >=0.8.0 <0.9.0;


import {Asn1Decode} from "../../libs/asn1-decode/Asn1Decode.sol";
import {BytesUtils} from "../../libs/ens-contracts/BytesUtils.sol";
import {RSAVerify} from "../../libs/ens-contracts/RSAVerify.sol";


library LibRsaSha256 {

    using BytesUtils for bytes;
    using Asn1Decode for bytes;

    function verifyWithComponents(
        bytes memory modulus,
        bytes memory exponent,
        bytes32 hash,
        bytes memory sig
    )
        internal
        view
        returns (bool)
    {
        bytes memory expHash = RSAVerify.rsarecover(modulus, exponent, sig);
        require(expHash.length >= 32, "Invalid signature");
        return hash == expHash.readBytes32(expHash.length - 32);
    }

    /**
    * @dev Extracts modulus and exponent (respectively) from a DER-encoded RSA public key
    * @param key A DER-encoded RSA public key
    */
    function extractKeyComponents(bytes memory key)
        internal
        pure
        returns (bytes memory modulus, bytes memory exponent)
    {
        uint node;
        bytes32 oid;

        node = key.root();
        node = key.firstChildOf(node);

        // OID must be 1.2.840.113549.1.1.1 (rsaEncryption)
        oid = keccak256(key.bytesAt(key.firstChildOf(node)));
        require(oid == 0x3be606946d6f343b24d5ecdbd7e3370a5303ed54845f50f466a35f3bbeb46a45, "Invalid key");

        node = key.nextSiblingOf(node);
        node = key.rootOfBitStringAt(node);
        node = key.firstChildOf(node);
        modulus = key.uintBytesAt(node);
        node = key.nextSiblingOf(node);
        exponent = key.uintBytesAt(node);
    }

    function verifySignMsg(
        bytes memory key,
        bytes memory message,
        bytes memory sig
    )
        internal
        view
        returns (bool)
    {
        bytes memory m;
        bytes memory e;

        (m, e) = extractKeyComponents(key);

        return verifyWithComponents(m, e, sha256(message), sig);
    }

}
