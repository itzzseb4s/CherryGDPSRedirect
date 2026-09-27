#include <Geode/Geode.hpp>
#include <Geode/modify/CCHttpClient.hpp>

using namespace geode::prelude;

namespace {

    constexpr std::string_view OFFICIAL =
        "https://www.boomlings.com/database";

    constexpr std::string_view CHERRY =
        "https://playersbro.ps.fhgdps.com";

    class CherryResponseHandler : public CCObject {
    public:
        static CherryResponseHandler* get() {
            static auto instance = new CherryResponseHandler();
            return instance;
        }

        void onResponse(
            CCHttpClient* sender,
            CCHttpResponse* response
        ) {
            if (response == nullptr) {
                log::error(
                    "Cherry GDPS: RESPONSE IS NULL"
                );
                return;
            }

            auto request = response->getHttpRequest();

            if (request == nullptr) {
                log::error(
                    "Cherry GDPS: RESPONSE HAS NO REQUEST"
                );
                return;
            }

            auto url = request->getUrl();

            if (url == nullptr) {
                return;
            }

            // Only inspect the level search endpoint.
            if (
                std::string_view(url).find(
                    "playersbro.ps.fhgdps.com/getGJLevels21.php"
                ) == std::string_view::npos
            ) {
                return;
            }

            log::info(
                "================================"
            );

            log::info(
                "Cherry GDPS: RESPONSE RECEIVED"
            );

            log::info(
                "HTTP CODE: {}",
                response->getResponseCode()
            );

            log::info(
                "SUCCESS: {}",
                response->isSucceed()
            );

            if (!response->isSucceed()) {
                log::error(
                    "ERROR: {}",
                    response->getErrorBuffer()
                );

                log::info(
                    "================================"
                );

                return;
            }

            auto data = response->getResponseData();

            if (data == nullptr || data->empty()) {
                log::error(
                    "Cherry GDPS: EMPTY RESPONSE"
                );

                log::info(
                    "RESULT: EMPTY"
                );

                log::info(
                    "================================"
                );

                return;
            }

            std::string body(
                data->begin(),
                data->end()
            );

            log::info(
                "RESPONSE SIZE: {} bytes",
                body.size()
            );

            /*
             * Detect the common Geometry Dash server
             * responses.
             */

            if (body.empty()) {
                log::error(
                    "Cherry GDPS: RESPONSE IS EMPTY"
                );

                log::info(
                    "RESULT: EMPTY"
                );
            }
            else if (body == "-1") {
                log::error(
                    "Cherry GDPS: SERVER RETURNED -1"
                );

                log::info(
                    "RESULT: ERROR (-1)"
                );
            }
            else if (
                body.starts_with("##")
            ) {
                /*
                 * Responses beginning with ## are
                 * metadata/pagination style responses.
                 */
                log::info(
                    "Cherry GDPS: RESPONSE STARTS WITH ##"
                );

                log::info(
                    "RESULT: NO LEVEL DATA / METADATA"
                );

                log::info(
                    "RESPONSE: {}",
                    body
                );
            }
            else {
                /*
                 * A normal level response contains
                 * colon-separated level data.
                 */
                log::info(
                    "Cherry GDPS: RESPONSE CONTAINS DATA"
                );

                log::info(
                    "RESULT: LEVEL DATA DETECTED"
                );

                /*
                 * Don't dump an enormous response into
                 * the Geode log. Show only the beginning.
                 */
                constexpr size_t MAX_LOG_LENGTH = 500;

                if (body.size() > MAX_LOG_LENGTH) {
                    log::info(
                        "RESPONSE: {}...",
                        body.substr(
                            0,
                            MAX_LOG_LENGTH
                        )
                    );
                }
                else {
                    log::info(
                        "RESPONSE: {}",
                        body
                    );
                }
            }

            log::info(
                "================================"
            );
        }
    };

}

$on_mod(Loaded) {
    log::info(
        "================================"
    );

    log::info(
        "CHERRY GDPS REDIRECT LOADED"
    );

    log::info(
        "================================"
    );
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

        log::info(
            "Cherry GDPS: REQUEST: {}",
            url
        );

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

            /*
             * Prevent:
             *
             * https://playersbro.ps.fhgdps.com//getGJLevels21.php
             */
            if (
                redirected.starts_with(
                    "https://playersbro.ps.fhgdps.com//"
                )
            ) {
                redirected.replace(
                    34,
                    2,
                    "/"
                );
            }

            request->setUrl(
                redirected.c_str()
            );

            log::info(
                "Cherry GDPS: REDIRECTED: {}",
                redirected
            );
        }

        /*
         * Replace the game's response callback
         * temporarily so we can inspect the response.
         *
         * NOTE:
         * This diagnostic version intentionally focuses
         * on capturing the response. If the callback
         * replacement interferes with GD's processing,
         * we'll use a lower-level hook instead.
         */
        if (
            redirected.find(
                "playersbro.ps.fhgdps.com/getGJLevels21.php"
            ) != std::string::npos
        ) {
            request->setResponseCallback(
                CherryResponseHandler::get(),
                httpresponse_selector(
                    CherryResponseHandler::onResponse
                )
            );
        }

        CCHttpClient::send(request);
    }
};
