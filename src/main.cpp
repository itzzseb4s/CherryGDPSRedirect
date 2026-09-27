#include <Geode/Geode.hpp>
#include <Geode/modify/CCHttpClient.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

namespace {

    constexpr std::string_view OFFICIAL =
        "https://www.boomlings.com/database";

    constexpr std::string_view CHERRY =
        "https://playersbro.ps.fhgdps.com";

    constexpr std::string_view LEVEL_ENDPOINT =
        "playersbro.ps.fhgdps.com/getGJLevels21.php";

    /*
     * ============================================================
     * CHERRY ACCOUNT
     * ============================================================
     *
     * Guardamos solamente:
     *
     *   cherry-username
     *   cherry-gjp2
     *
     * NO guardamos la contraseña.
     *
     * Para configurar estas dos variables puedes hacerlo desde
     * Geode settings / saved values posteriormente.
     */

    bool hasCherryAccount() {
        auto mod = Mod::get();

        auto username =
            mod->getSavedValue<std::string>(
                "cherry-username",
                ""
            );

        auto gjp2 =
            mod->getSavedValue<std::string>(
                "cherry-gjp2",
                ""
            );

        return !username.empty() && !gjp2.empty();
    }

    void loginToCherry() {

        auto mod = Mod::get();

        auto username =
            mod->getSavedValue<std::string>(
                "cherry-username",
                ""
            );

        auto gjp2 =
            mod->getSavedValue<std::string>(
                "cherry-gjp2",
                ""
            );

        if (username.empty() || gjp2.empty()) {

            log::warn(
                "Cherry GDPS: No Cherry account configured."
            );

            return;
        }

        auto account =
            GJAccountManager::get();

        if (account == nullptr) {

            log::error(
                "Cherry GDPS: GJAccountManager is NULL."
            );

            return;
        }

        log::info(
            "Cherry GDPS: Switching account..."
        );

        /*
         * Primero eliminamos la sesión actualmente activa.
         *
         * Esto NO elimina niveles locales ni progreso.
         * Solamente elimina la sesión de cuenta activa.
         */

        account->unlinkFromAccount();

        log::info(
            "Cherry GDPS: Original account unlinked."
        );

        /*
         * Iniciamos sesión usando el GJP2 guardado.
         *
         * Las peticiones de login pasarán por nuestro redirect
         * y terminarán en Cherry.
         */

        account->loginAccount(
            username,
            gjp2
        );

        log::info(
            "Cherry GDPS: Login request sent."
        );
    }

}

/*
 * ================================================================
 * RESPONSE CAPTURE
 * ================================================================
 */

class CherryResponseHandler : public CCObject {

public:

    static CherryResponseHandler* get() {

        static auto instance =
            new CherryResponseHandler();

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

        auto request =
            response->getHttpRequest();

        if (request == nullptr) {

            log::error(
                "Cherry GDPS: RESPONSE HAS NO REQUEST"
            );

            return;
        }

        auto url =
            request->getUrl();

        if (url == nullptr) {
            return;
        }

        std::string_view urlView(url);

        /*
         * Solo analizamos getGJLevels21.php.
         */

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

        /*
         * -1
         */

        if (body == "-1") {

            log::error(
                "Cherry GDPS: SERVER RETURNED -1"
            );

            log::info(
                "RESULT: ERROR (-1)"
            );
        }

        /*
         * Empty / metadata
         */

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

        /*
         * Level data
         */

        else {

            log::info(
                "Cherry GDPS: RESPONSE CONTAINS DATA"
            );

            log::info(
                "RESULT: LEVEL DATA DETECTED"
            );

            constexpr size_t MAX_LOG_LENGTH = 500;

            if (
                body.size() >
                MAX_LOG_LENGTH
            ) {

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


/*
 * ================================================================
 * MOD LOADED
 * ================================================================
 */

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

    /*
     * Comprobamos si existe una cuenta Cherry guardada.
     */

    if (hasCherryAccount()) {

        log::info(
            "Cherry GDPS: Cherry account found."
        );

        /*
         * IMPORTANTE:
         *
         * Esperamos un poco antes de tocar
         * GJAccountManager para darle tiempo
         * al juego a terminar su inicialización.
         */

        Loader::get()->queueInMainThread([] {

            loginToCherry();

        });

    }
    else {

        log::warn(
            "Cherry GDPS: No account configured."
        );

        log::warn(
            "Cherry GDPS: Account switching disabled."
        );
    }
}


/*
 * ================================================================
 * HTTP REDIRECT
 * ================================================================
 */

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

        log::info(
            "Cherry GDPS: REQUEST: {}",
            url
        );

        std::string redirected(url);

        /*
         * Redirect:
         *
         * https://www.boomlings.com/database/...
         *
         * ->
         *
         * https://playersbro.ps.fhgdps.com/...
         */

        if (
            redirected.starts_with(
                OFFICIAL
            )
        ) {

            redirected.replace(
                0,
                OFFICIAL.size(),
                CHERRY
            );

            /*
             * Evitamos //
             */

            constexpr std::string_view
                DOUBLE_SLASH =
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

        /*
         * Capturamos solamente la respuesta de
         * getGJLevels21.php.
         *
         * El callback original del juego sigue siendo
         * importante, por eso esta parte es solamente
         * para diagnóstico.
         */

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

        CCHttpClient::send(request);
    }
};#include <Geode/Geode.hpp>
#include <Geode/modify/CCHttpClient.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

namespace {

    constexpr std::string_view OFFICIAL =
        "https://www.boomlings.com/database";

    constexpr std::string_view CHERRY =
        "https://playersbro.ps.fhgdps.com";

    constexpr std::string_view LEVEL_ENDPOINT =
        "playersbro.ps.fhgdps.com/getGJLevels21.php";

    /*
     * ============================================================
     * CHERRY ACCOUNT
     * ============================================================
     *
     * Guardamos solamente:
     *
     *   cherry-username
     *   cherry-gjp2
     *
     * NO guardamos la contraseña.
     *
     * Para configurar estas dos variables puedes hacerlo desde
     * Geode settings / saved values posteriormente.
     */

    bool hasCherryAccount() {
        auto mod = Mod::get();

        auto username =
            mod->getSavedValue<std::string>(
                "cherry-username",
                ""
            );

        auto gjp2 =
            mod->getSavedValue<std::string>(
                "cherry-gjp2",
                ""
            );

        return !username.empty() && !gjp2.empty();
    }

    void loginToCherry() {

        auto mod = Mod::get();

        auto username =
            mod->getSavedValue<std::string>(
                "cherry-username",
                ""
            );

        auto gjp2 =
            mod->getSavedValue<std::string>(
                "cherry-gjp2",
                ""
            );

        if (username.empty() || gjp2.empty()) {

            log::warn(
                "Cherry GDPS: No Cherry account configured."
            );

            return;
        }

        auto account =
            GJAccountManager::get();

        if (account == nullptr) {

            log::error(
                "Cherry GDPS: GJAccountManager is NULL."
            );

            return;
        }

        log::info(
            "Cherry GDPS: Switching account..."
        );

        /*
         * Primero eliminamos la sesión actualmente activa.
         *
         * Esto NO elimina niveles locales ni progreso.
         * Solamente elimina la sesión de cuenta activa.
         */

        account->unlinkFromAccount();

        log::info(
            "Cherry GDPS: Original account unlinked."
        );

        /*
         * Iniciamos sesión usando el GJP2 guardado.
         *
         * Las peticiones de login pasarán por nuestro redirect
         * y terminarán en Cherry.
         */

        account->loginAccount(
            username,
            gjp2
        );

        log::info(
            "Cherry GDPS: Login request sent."
        );
    }

}

/*
 * ================================================================
 * RESPONSE CAPTURE
 * ================================================================
 */

class CherryResponseHandler : public CCObject {

public:

    static CherryResponseHandler* get() {

        static auto instance =
            new CherryResponseHandler();

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

        auto request =
            response->getHttpRequest();

        if (request == nullptr) {

            log::error(
                "Cherry GDPS: RESPONSE HAS NO REQUEST"
            );

            return;
        }

        auto url =
            request->getUrl();

        if (url == nullptr) {
            return;
        }

        std::string_view urlView(url);

        /*
         * Solo analizamos getGJLevels21.php.
         */

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

        /*
         * -1
         */

        if (body == "-1") {

            log::error(
                "Cherry GDPS: SERVER RETURNED -1"
            );

            log::info(
                "RESULT: ERROR (-1)"
            );
        }

        /*
         * Empty / metadata
         */

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

        /*
         * Level data
         */

        else {

            log::info(
                "Cherry GDPS: RESPONSE CONTAINS DATA"
            );

            log::info(
                "RESULT: LEVEL DATA DETECTED"
            );

            constexpr size_t MAX_LOG_LENGTH = 500;

            if (
                body.size() >
                MAX_LOG_LENGTH
            ) {

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


/*
 * ================================================================
 * MOD LOADED
 * ================================================================
 */

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

    /*
     * Comprobamos si existe una cuenta Cherry guardada.
     */

    if (hasCherryAccount()) {

        log::info(
            "Cherry GDPS: Cherry account found."
        );

        /*
         * IMPORTANTE:
         *
         * Esperamos un poco antes de tocar
         * GJAccountManager para darle tiempo
         * al juego a terminar su inicialización.
         */

        Loader::get()->queueInMainThread([] {

            loginToCherry();

        });

    }
    else {

        log::warn(
            "Cherry GDPS: No account configured."
        );

        log::warn(
            "Cherry GDPS: Account switching disabled."
        );
    }
}


/*
 * ================================================================
 * HTTP REDIRECT
 * ================================================================
 */

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

        log::info(
            "Cherry GDPS: REQUEST: {}",
            url
        );

        std::string redirected(url);

        /*
         * Redirect:
         *
         * https://www.boomlings.com/database/...
         *
         * ->
         *
         * https://playersbro.ps.fhgdps.com/...
         */

        if (
            redirected.starts_with(
                OFFICIAL
            )
        ) {

            redirected.replace(
                0,
                OFFICIAL.size(),
                CHERRY
            );

            /*
             * Evitamos //
             */

            constexpr std::string_view
                DOUBLE_SLASH =
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

        /*
         * Capturamos solamente la respuesta de
         * getGJLevels21.php.
         *
         * El callback original del juego sigue siendo
         * importante, por eso esta parte es solamente
         * para diagnóstico.
         */

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

        CCHttpClient::send(request);
    }
};
