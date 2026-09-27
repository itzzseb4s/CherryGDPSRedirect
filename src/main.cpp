#include <Geode/Geode.hpp>
#include <Geode/modify/CCHttpClient.hpp>

using namespace geode::prelude;

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

        constexpr std::string_view OFFICIAL =
            "https://www.boomlings.com/database";

        constexpr std::string_view CHERRY =
            "https://playersbro.ps.fhgdps.com";

        std::string originalUrl(url);
        std::string redirectedUrl(originalUrl);

        log::info(
            "Cherry GDPS: REQUEST: {}",
            originalUrl
        );

        if (redirectedUrl.starts_with(OFFICIAL)) {

            redirectedUrl.replace(
                0,
                OFFICIAL.size(),
                CHERRY
            );

            request->setUrl(
                redirectedUrl.c_str()
            );

            log::info(
                "Cherry GDPS: REDIRECTED: {}",
                redirectedUrl
            );
        }

        // Keep Geometry Dash's original callback.
        CCHttpClient::send(request);
    }
};

$on_mod(Loaded) {

    log::info(
        "================================"
    );

    log::info(
        "CHERRY GDPS REDIRECT LOADED"
    );

    log::info(
        "Cherry Mode: unlinking current account..."
    );

    auto accountManager = GJAccountManager::get();

    if (accountManager != nullptr) {
        accountManager->unlinkFromAccount();

        log::info(
            "Cherry Mode: account unlinked successfully"
        );
    }
    else {
        log::error(
            "Cherry Mode: GJAccountManager is NULL"
        );
    }

    log::info(
        "================================"
    );
}
