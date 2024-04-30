// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;


import {Interface_PubSubService} from "./Interface_PubSubService.sol";
import {Interface_EventManager} from "./Interface_EventManager.sol";


contract Oracle {

	event OnDataAvailable(bytes data);

	address public m_eventMangerAddr = address(0);

	constructor(address pubSubAddr) {
		m_eventMangerAddr = Interface_PubSubService(pubSubAddr).register();
	}

	function onDataAvailable(bytes memory data) external {
		Interface_EventManager(m_eventMangerAddr).notifySubscribers(data);
		emit OnDataAvailable(data);
	}
}

