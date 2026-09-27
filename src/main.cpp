#include <Geode/Geode.hpp>
#include <Geode/modify/CCHttpClient.hpp>

using namespace geode::prelude;

namespace {

    constexpr std::string_view OFFICIAL =
        "https://www.boomlings.com/database";

    constexpr std::string_view CHERRY =
        "https://playersbro.ps.fhgdps.com";

    constexpr std::string_view LEVEL_ENDPOINT =
        "getGJLevels21.php";

    constexpr std::string_view LOGIN_ENDPOINT =
        "loginGJAccount.php";


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


            // ----------------------------------------------------
            // DETECT ENDPOINT
            // ----------------------------------------------------

            bool isLevelsRequest =
                urlView.find(LEVEL_ENDPOINT)
                != std::string_view::npos;

            bool isLoginRequest =
                urlView.find(LOGIN_ENDPOINT)
                != std::string_view::npos;


            // Ignore everything else
            if (
                !isLevelsRequest &&
                !isLoginRequest
            ) {
                return;
            }


            log::info(
                "================================"
            );

            log::info(
                "Cherry GDPS: RESPONSE RECEIVED"
            );


            if (isLoginRequest) {
                log::info(
                    "TYPE: ACCOUNT LOGIN"
                );
            }
            else if (isLevelsRequest) {
                log::info(
                    "TYPE: LEVEL SEARCH"
                );
            }


            // ----------------------------------------------------
            // HTTP INFORMATION
            // ----------------------------------------------------

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

            auto data =
                response->getResponseData();


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


            // ====================================================
            // LOGIN RESPONSE
            // ====================================================

            if (isLoginRequest) {

                if (body == "-1") {

                    log::error(
                        "Cherry GDPS: LOGIN REJECTED (-1)"
                    );

                    log::info(
                        "RESULT: LOGIN FAILED"
                    );

                }
                else {

                    /*
                     * IMPORTANT:
                     *
                     * Do not print the complete login response.
                     * It can contain account/session information.
                     */

                    log::info(
                        "Cherry GDPS: LOGIN RESPONSE RECEIVED"
                    );

                    log::info(
                        "RESULT: LOGIN RESPONSE IS NOT -1"
                    );

                    /*
                     * Show only a short prefix for diagnosis.
                     */

                    constexpr size_t MAX_LOGIN_LOG = 80;

                    if (body.size() > MAX_LOGIN_LOG) {

                        log::info(
                            "RESPONSE PREFIX: {}...",
                            body.substr(
                                0,
                                MAX_LOGIN_LOG
                            )
                        );

                    }
                    else {

                        log::info(
                            "RESPONSE PREFIX: {}",
                            body
                        );
                    }
                }

                log::info(
                    "================================"
                );

                return;
            }


            // ====================================================
            // LEVEL RESPONSE
            // ====================================================

            if (isLevelsRequest) {

                if (body == "-1") {

                    log::error(
                        "Cherry GDPS: SERVER RETURNED -1"
                    );

                    log::info(
                        "RESULT: LEVEL REQUEST FAILED"
                    );

                }

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

                else {

                    log::info(
                        "Cherry GDPS: RESPONSE CONTAINS DATA"
                    );

                    log::info(
                        "RESULT: LEVEL DATA DETECTED"
                    );


                    /*
                     * Only print the beginning.
                     * Level responses can be very large.
                     */

                    constexpr size_t MAX_LEVEL_LOG = 500;


                    if (
                        body.size() >
                        MAX_LEVEL_LOG
                    ) {

                        log::info(
                            "RESPONSE: {}...",
                            body.substr(
                                0,
                                MAX_LEVEL_LOG
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


        auto url =
            request->getUrl();


        if (url == nullptr) {

            CCHttpClient::send(request);

            return;
        }


        // --------------------------------------------------------
        // ORIGINAL URL
        // --------------------------------------------------------

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


            // ----------------------------------------------------
            // PREVENT DOUBLE SLASH
            // ----------------------------------------------------

            constexpr std::string_view DOUBLE_SLASH =
                "https://playersbro.ps.fhgdps.com//";


            if (
                redirected.starts_with(
                    DOUBLE_SLASH
                )
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
        // RESPONSE CAPTURE
        // --------------------------------------------------------

        bool isLevelsRequest =
            redirected.find(
                "getGJLevels21.php"
            )
            != std::string::npos;


        bool isLoginRequest =
            redirected.find(
                "loginGJAccount.php"
            )
            != std::string::npos;


        /*
         * Capture only:
         *
         * getGJLevels21.php
         * loginGJAccount.php
         *
         * We leave all other requests alone.
         */

        if (
            isLevelsRequest ||
            isLoginRequest
        ) {

            request->setResponseCallback(
                CherryResponseHandler::get(),

                httpresponse_selector(
                    CherryResponseHandler::onResponse
                )
            );
        }


        // --------------------------------------------------------
        // SEND REQUEST
        // --------------------------------------------------------

        CCHttpClient::send(request);
    }
};
