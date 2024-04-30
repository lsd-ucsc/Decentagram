// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;


interface Interface_RevokerByVoting {
    function revokeVote(bytes32 enclaveId) external;
}

interface Interface_KeyRevokerByVoting {
    function revokeVote(address proposedKeyAddr) external;
}

contract VoteProxy {
    constructor() {
    }

    function enclaveRevokeVote(address revoker, bytes32 enclaveId) public {
        Interface_RevokerByVoting(revoker).revokeVote(enclaveId);
    }

    function keyRevokeVote(address revoker, address proposedKeyAddr) public {
        Interface_KeyRevokerByVoting(revoker).revokeVote(proposedKeyAddr);
    }
}
