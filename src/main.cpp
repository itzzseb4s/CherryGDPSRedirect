#include <Geode/Geode.hpp>
#include <Geode/modify/CCHttpClient.hpp>

using namespace geode::prelude;

namespace {

    constexpr std::string_view OFFICIAL =
        "https://www.boomlings.com/database";

    constexpr std::string_view CHERRY =
        "https://playersbro.ps.fhgdps.com";

    constexpr std::string_view LEVEL_ENDPOINT =
        "playersbro.ps.fhgdps.com/getGJLevels21.php";


    // ============================================================
    // RESPONSE HANDLER
    // ============================================================

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

            std::string_view urlView(url);

            // Solo analizar getGJLevels21.php
            if (
                urlView.find(LEVEL_ENDPOINT)
                == std::string_view::npos
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


            // ----------------------------------------------------
            // HTTP ERROR
            // ----------------------------------------------------

            if (!response->isSucceed()) {

                log::error(
                    "Cherry GDPS: REQUEST FAILED"
                );

                log::error(
                    "ERROR: {}",
                    response->getErrorBuffer()
                );

                log::info(
                    "================================"
                );

                return;
            }


            // ----------------------------------------------------
            // RESPONSE DATA
            // ----------------------------------------------------

            auto data = response->getResponseData();

            if (
                data == nullptr ||
                data->empty()
            ) {

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


            // ----------------------------------------------------
            // -1
            // ----------------------------------------------------

            if (body == "-1") {

                log::error(
                    "Cherry GDPS: SERVER RETURNED -1"
                );

                log::info(
                    "RESULT: ERROR (-1)"
                );
            }


            // ----------------------------------------------------
            // EMPTY / METADATA
            // ----------------------------------------------------

            else if (
                body == "#" ||
                body.starts_with("##")
            ) {

                log::warn(
                    "Cherry GDPS: NO LEVEL DATA"
                );

                log::info(
                    "RESULT: EMPTY / METADATA"
                );

                log::info(
                    "RESPONSE: {}",
                    body
                );
            }


            // ----------------------------------------------------
            // LEVEL DATA
            // ----------------------------------------------------

            else {

                log::info(
                    "Cherry GDPS: RESPONSE CONTAINS DATA"
                );

                log::info(
                    "RESULT: LEVEL DATA DETECTED"
                );


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


// ================================================================
// MOD LOADED
// ================================================================

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


// ================================================================
// HTTP REDIRECT
// ================================================================

class $modify(
    CherryCCHttpClient,
    CCHttpClient
) {

    void send(
        CCHttpRequest* request
    ) {

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


        std::string redirected(url);


        // --------------------------------------------------------
        // BOOMLINGS → CHERRY
        // --------------------------------------------------------

        if (
            redirected.starts_with(OFFICIAL)
        ) {

            redirected.replace(
                0,
                OFFICIAL.size(),
                CHERRY
            );


            // Evitar //
            constexpr std::string_view DOUBLE_SLASH =
                "https://playersbro.ps.fhgdps.com//";


            if (
                redirected.starts_with(DOUBLE_SLASH)
            ) {

                redirected.erase(
                    33,
                    1
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


        // --------------------------------------------------------
        // CAPTURAR getGJLevels21.php
        // --------------------------------------------------------

        if (
            redirected.find(
                "playersbro.ps.fhgdps.com/getGJLevels21.php"
            )
            != std::string::npos
        ) {

            request->setResponseCallback(
                CherryResponseHandler::get(),
                httpresponse_selector(
                    CherryResponseHandler::onResponse
                )
            );
        }


        // --------------------------------------------------------
        // ENVIAR PETICIÓN
        // --------------------------------------------------------

        CCHttpClient::send(request);
    }
};
