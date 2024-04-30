// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;

// This import is automatically injected by Remix
import "remix_tests.sol";


import {X509Parser} from "../../libs/x509-forest-of-trust/X509Parser.sol";


contract X509Parser_proxy{
	function toTimestampTest() external {

		// IAS Root Cert

		{
			uint256 gasUsed = gasleft();
			uint256 result = X509Parser.toTimestamp("161114153731Z");
			gasUsed -= gasleft();
			Assert.equal(result, 1479137851, "timestamp not equal");
			// Assert.equal(gasUsed, 8100, "gasUsed");
		}

		{
			uint256 gasUsed = gasleft();
			uint256 result = X509Parser.toTimestamp("220101000000Z");
			gasUsed -= gasleft();
			Assert.equal(result, 1640995200, "timestamp not equal");
			// Assert.equal(gasUsed, 8088, "gasUsed");
		}

		{
			uint256 gasUsed = gasleft();
			uint256 result = X509Parser.toTimestamp("20491231235959Z");
			gasUsed -= gasleft();
			Assert.equal(result, 2524607999, "timestamp not equal");
			// Assert.equal(gasUsed, 8552, "gasUsed");
		}

		{
			uint256 result = X509Parser.toTimestamp(hex"3136313131343135333733315A");
			Assert.equal(result, 1479137851, "timestamp not equal");
		}

		{
			uint256 result = X509Parser.toTimestamp(hex"3232303130313030303030305A");
			Assert.equal(result, 1640995200, "timestamp not equal");
		}

		{
			uint256 result = X509Parser.toTimestamp(hex"32303439313233313233353935395A");
			Assert.equal(result, 2524607999, "timestamp not equal");
		}

		// IAS Report Cert

		{
			uint256 result = X509Parser.toTimestamp(hex"3136313132323039333635385A");
			Assert.equal(result, 1479807418, "timestamp not equal");
		}

		{
			uint256 result = X509Parser.toTimestamp(hex"3236313132303039333635385A");
			Assert.equal(result, 1795167418, "timestamp not equal");
		}
	}
}
