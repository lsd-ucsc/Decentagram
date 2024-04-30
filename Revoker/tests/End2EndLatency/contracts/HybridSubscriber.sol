// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;


import {Interface_PubSubService} from "./Interface_PubSubService.sol";


contract HybridSubscriber {

	event PubsubConfirmed(bytes data);
	event TransactionConfirmed(bytes data);

	address public m_eventMgrAddr = address(0);
	address public m_owner = address(0);

	constructor(address pubSubServiceAddr, address publisherAddr) payable {
		m_owner = msg.sender;
		m_eventMgrAddr = Interface_PubSubService(
			pubSubServiceAddr
		).subscribe{
			value: msg.value
		}(publisherAddr);
	}

	function onNotify(bytes memory data) external {
		require(m_eventMgrAddr != address(0), "Not subscribed");
		require(m_eventMgrAddr == msg.sender, "Unauthorized");
		emit PubsubConfirmed(data);
	}

	function onTransaction(bytes memory data) external {
		require(m_owner == msg.sender, "Unauthorized");
		emit TransactionConfirmed(data);
	}

}

