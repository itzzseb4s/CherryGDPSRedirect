#include <Geode/Geode.hpp>
#include <Geode/modify/CCHttpRequest.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
    log::info("================================");
    log::info("CHERRY GDPS REDIRECT LOADED");
    log::info("================================");
}

class $modify(CherryCCHttpRequest, CCHttpRequest) {
    void setUrl(char const* url) {
        if (url == nullptr) {
            CCHttpRequest::setUrl(url);
            return;
        }

        log::info("Cherry GDPS: intercepted URL: {}", url);

        std::string original(url);

        constexpr std::string_view official =
            "https://www.boomlings.com/database";

        constexpr std::string_view cherry =
            "https://playersbro.ps.fhgdps.com";

        if (original.starts_with(official)) {
            original.replace(
                0,
                official.size(),
                cherry
            );

            log::info("Cherry GDPS: REDIRECTED TO: {}", original);
        }

        CCHttpRequest::setUrl(original.c_str());
    }
};
