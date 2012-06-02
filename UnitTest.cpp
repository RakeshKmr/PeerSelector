// Unit test file to test each method
#include "BitSwiftSelector.hpp"

int main(int argc, char* argv[])
{
    // Stores IP address and port number of preferred peers
    std::vector<ipPort_s> ip_port_list, ip_port_list1;
    ipPort_s listIpPort;

    // Object of BitSwiftSelector class to accees its interface
    BitSwiftSelector *objSelector = new BitSwiftSelector();

    if (objSelector)
    {

        // Test Add Peer method. one by one, pass IP address, port number and hash
        objSelector->addpeer("188.39.43.126", 2025, "0c1a100e92cf2649ac7a0a6875a48ee7c8bf551e");
        objSelector->addpeer("88.167.225.227", 19534, "0c1a100e92cf2649ac7a0a6875a48ee7c8bf551e");
        objSelector->addpeer("182.177.62.56", 13655, "0c1a100e92cf2649ac7a0a6875a48ee7c8bf551e");
        objSelector->addpeer("182.185.25.166", 1268, "0c1a100e92cf2649ac7a0a6875a48ee7c8bf551e");
        objSelector->addpeer("117.192.36.255", 2026, "0c1a100e92cf2649ac7a0a6875a48ee7c8bf551e");
        objSelector->addpeer("120.61.27.28", 1500, "0c1a100e92cf2649ac7a0a6875a48ee7c8bf551e");
        objSelector->addpeer("124.253.169.4", 6881, "0c1a100e92cf2649ac7a0a6875a48ee7c8bf551e");
        objSelector->addpeer("59.178.194.76", 1267, "0c1a100e92cf2649ac7a0a6875a48ee7c8bf551e");
        objSelector->addpeer("223.29.224.144", 1505, "0c1a100e92cf2649ac7a0a6875a48ee7c8bf551e");


        // Test addpeers method. Store all the IP addresses in a vecor and pass entire vector
        listIpPort.ipAddress = "59.92.192.38";
        listIpPort.port = 1501;
        ip_port_list.push_back(listIpPort);

        listIpPort.ipAddress = "219.64.190.135";
        listIpPort.port = 1502;
        ip_port_list.push_back(listIpPort);
            
        listIpPort.ipAddress = "193.105.7.54";
        listIpPort.port = 1502;
        ip_port_list.push_back(listIpPort);

        
        // add list of peers to database
        objSelector->addpeers(ip_port_list, "0c1a100e92cf2649ac7a0a6875a48ee7c8bf551e");

        int type = 0;
        
        //0/1/2 score/rtt/as-hop
        if (!strcmp(argv[1],"0"))
            type = 0;
        else if (!strcmp(argv[1],"1"))
            type = 1;
        else if (!strcmp(argv[1],"2"))
            type = 2;
        else if (!strcmp(argv[1],"3"))
            type = 3;

        // Print the sorted list of peers according to type
        objSelector->getpeers("0c1a100e92cf2649ac7a0a6875a48ee7c8bf551e", type, ip_port_list1);
        std::cout << "List of Peer according to policy type : " << type << std::endl;  
        for (std::vector<ipPort_s>::const_iterator j = ip_port_list1.begin(); j != ip_port_list1.end(); ++j)
        {
            std::cout<< j->ipAddress <<" : " <<j->port << std::endl;
        }


        // delete one peer from score-table
        if (!strcmp(argv[2],"1"))
            objSelector->deletepeer("182.177.62.56", 13655, "0c1a100e92cf2649ac7a0a6875a48ee7c8bf551e", 0);

        // wait for the user to end
        char a;
        std::cin.unsetf(std::ios_base::skipws);
        std::cin >> a;
    }
}
