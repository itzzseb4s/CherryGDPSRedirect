#include <Geode/Geode.hpp>
#include <Geode/modify/HttpRequest.hpp>

using namespace geode::prelude;

namespace {
    constexpr char OFFICIAL_DATABASE[] =
        "https://www.boomlings.com/database";

    constexpr char CHERRY_DATABASE[] =
        "https://playersbro.ps.fhgdps.com";

    std::string redirectURL(char const* url) {
        if (url == nullptr) {
            return {};
        }

        std::string original(url);

        if (original.rfind(OFFICIAL_DATABASE, 0) != 0) {
            return original;
        }

        auto redirected = original;

        redirected.replace(
            0,
            std::string(OFFICIAL_DATABASE).size(),
            CHERRY_DATABASE
        );

        log::info(
            "Cherry GDPS redirect: {} -> {}",
            original,
            redirected
        );

        return redirected;
    }
}

class $modify(CherryHttpRequest, cocos2d::extension::CCHttpRequest) {
    void setUrl(char const* url) {
        auto redirected = redirectURL(url);

        CCHttpRequest::setUrl(redirected.c_str());
    }
};
