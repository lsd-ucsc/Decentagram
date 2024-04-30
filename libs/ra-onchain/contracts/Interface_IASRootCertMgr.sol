// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;


interface Interface_IASRootCertMgr {

    function getNotAfter() external view returns (uint256);

    function requireValidity() external view;

    function getPubKey() external view returns (bytes memory, bytes memory);

}
