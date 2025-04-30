
#include <string>

struct SipMsg 
{
    std::string getFromHeader(const std::string& from_usr, const std::string& from_ip) 
    {
        if (from_usr.empty() || from_ip.empty()) {
            LOG(WARNING) << "Empty parameters for getFromHeader";
        }
        return fmt::format("<sip:{}@{}>", from_usr, from_ip);
    }

    std::string getToHeader(const std::string& to_usr, const std::string& to_ip) 
    {
        if (to_usr.empty() || to_ip.empty()) {
            LOG(WARNING) << "Empty parameters for getToHeader";
        }
        return fmt::format("<sip:{}@{}>", to_usr, to_ip);
    }

    std::string getContactHeader(const std::string& contact_usr, const std::string& contact_ip) 
    {
        if (contact_usr.empty() || contact_ip.empty()) {
            LOG(WARNING) << "Empty parameters for getContactHeader";
        }
        return fmt::format("sip:{}@{}", contact_usr, contact_ip);
    }

    std::string getRequestURI(const std::string& sip_id, const std::string& addr_ip, int sip_port, int proto) 
    {
        if (sip_id.empty() || addr_ip.empty() || sip_port <= 0) {
            LOG(WARNING) << "Empty parameters for getRequestURI";
        }
        return fmt::format("sip:{}@{}:{};transport={}", sip_id, addr_ip, sip_port, proto == 1 ? "tcp" : "udp");
    }
};