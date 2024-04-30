// SPDX-License-Identifier: MIT
pragma solidity >=0.4.17 <0.9.0;


import {IASReportCert} from "../../contracts/IASReportCert.sol";
import {X509CertNodes} from "../../contracts/X509CertNodes.sol";


contract IASReportCert_proxy {

    using IASReportCert for IASReportCert.IASReportCertObj;

    function loadCertTest(
        X509CertNodes.CertNodesObj calldata certNodes,
        bytes memory certDer,
        bytes memory caKeyMod,
        bytes memory caKeyExp
    )
        public
        view
        returns (IASReportCert.IASReportCertObj memory cert)
    {
        cert.loadCert(
            certNodes,
            certDer,
            caKeyMod,
            caKeyExp
        );
    }

}
