#pragma once

#include "step.h"

#include <http/types.h>

#include <cstring>
#include <optional>
#include <string_view>
#include <variant>
#include <option/xapi_key_auth.h>

#define AUTH_REALM "Printer API"

namespace nhttp::handler {

class RequestParser;

class StatusPage {
public:
    enum class CloseHandling {
        Close,
        /*
         * Close for error states.
         *
         * First closes the outward side, then eats all the data.
         */
        ErrorClose,
        KeepAlive,
    };

protected:
    const char *extra_content;
    http::Status status;
    CloseHandling close_handling;
    bool json_content;
    std::optional<uint32_t> etag = std::nullopt;

    // Note: This exists only so we can reuse the exact same code also for UnauthenticatedStatusPage, the only diffrerence
    // is we add the correct Authentication header into extra_hdrs.
    Step step_impl(std::string_view input, bool terminated_by_client, uint8_t *output, size_t output_size, const char *const *extra_hdrs);

public:
    StatusPage(http::Status status, const RequestParser &parser, const char *extra_content = "");
    StatusPage(http::Status status, CloseHandling close_handling, bool json_content, std::optional<uint32_t> etag = std::nullopt, const char *extra_content = "");
    virtual ~StatusPage() = default;

    bool want_read() const { return false; }
    bool want_write() const { return true; }
    virtual Step step(std::string_view input, bool terminated_by_client, uint8_t *output, size_t output_size);
};

#if XAPI_KEY_AUTH()
struct ApiKeyAuth {};
#endif
struct DigestAuth {
    uint64_t nonce {};
    bool nonce_stale {};
};
#if XAPI_KEY_AUTH()
using AuthMethod = std::variant<DigestAuth, ApiKeyAuth>;
#else
using AuthMethod = std::variant<DigestAuth>;
#endif
class UnauthenticatedStatusPage final : public StatusPage {
private:
    AuthMethod auth_method;
    Step step(std::string_view input, bool terminated_by_client, uint8_t *output, size_t output_size, DigestAuth digest_auth);
#if XAPI_KEY_AUTH()
    Step step(std::string_view input, bool terminated_by_client, uint8_t *output, size_t output_size, ApiKeyAuth api_key_auth);
#endif

public:
    UnauthenticatedStatusPage(const RequestParser &parser, AuthMethod auth_method);

    Step step(std::string_view input, bool terminated_by_client, uint8_t *output, size_t output_size) override;
};

} // namespace nhttp::handler
