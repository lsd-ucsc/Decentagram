// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;


import {X509CertNodes} from "../../contracts/X509CertNodes.sol";


contract X509CertNodes_utils {

    function certNodesMem(
        X509CertNodes.CertNodesObj memory certNodes
    )
        public
        pure
        returns (uint256)
    {
        require(certNodes.tbs.version == 2, "version!=2");
        return 1;
    }

    function certNodesCall(
        X509CertNodes.CertNodesObj calldata certNodes
    )
        public
        pure
        returns (uint256)
    {
        require(certNodes.tbs.version == 2, "version!=2");
        return 1;
    }

}
