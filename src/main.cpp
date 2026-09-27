#include <Geode/Geode.hpp>
#include <Geode/modify/CCHttpClient.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
    log::info("================================");
    log::info("CHERRY GDPS REDIRECT LOADED");
    log::info("================================");
}

class $modify(CherryCCHttpClient, CCHttpClient) {
    void send(CCHttpRequest* request) {
        if (request == nullptr) {
            CCHttpClient::send(request);
            return;
        }

        auto url = request->getUrl();

        if (url == nullptr) {
            CCHttpClient::send(request);
            return;
        }

        log::info("Cherry GDPS: REQUEST: {}", url);

        constexpr std::string_view official =
            "https://www.boomlings.com/database";

        constexpr std::string_view cherry =
            "https://playersbro.ps.fhgdps.com";

        std::string redirected(url);

        if (redirected.starts_with(official)) {
            redirected.replace(
                0,
                official.size(),
                cherry
            );

            request->setUrl(redirected.c_str());

            log::info(
                "Cherry GDPS: REDIRECTED: {}",
                redirected
            );
        }

        CCHttpClient::send(request);
    }
};
