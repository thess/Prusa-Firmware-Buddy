#include <gcode/gcode.h>

#include "M330.h"
#include <metric.h>
#include <metric_handlers.h>
#include <stdint.h>
#include <config_store/store_instance.hpp>

static const metric_handler_t *selected_handler = &metric_handler_syslog;

/** \addtogroup G-Codes
 * @{
 */

/**
 * M330: Select metrics handler
 *
 * ## Parameters
 *
 * - <handler> - Select `handler` for configuration (`SYSLOG` is selected by default)
 */

void PrusaGcodeSuite::M330() {
    bool handler_found = false;
    for (auto handlers = metric_get_handlers(); *handlers; handlers++) {
        const metric_handler_t *handler = *handlers;
        if (strcmp(handler->name, parser.string_arg) == 0) {
            selected_handler = handler;
            handler_found = true;
        }
    }
    if (handler_found) {
        SERIAL_ECHO_START();
        SERIAL_ECHOLNPAIR_F("Configuring handler ", parser.string_arg);
    } else {
        SERIAL_ERROR_MSG("Handler not found");
    }
}

/**
 * M331: Enable metric
 *
 * ## Parameters
 *
 * - <metric> - Enable `metric` for the currently selected `handler`
 */

void PrusaGcodeSuite::M331() {
    if (selected_handler == NULL) {
        SERIAL_ECHO_MSG("handler not set");
        return;
    }

    for (auto metric = metric_get_iterator_begin(), e = metric_get_iterator_end(); metric != e; metric++) {
        if (strcmp(metric->name, parser.string_arg) == 0) {
            // Syslog handler has to be allowed in settings
            if (selected_handler->identifier == METRIC_HANDLER_SYSLOG_ID) {
                const MetricsAllow metrics_allow = config_store().metrics_allow.get();
                if (metrics_allow != MetricsAllow::One && metrics_allow != MetricsAllow::All) {
                    SERIAL_ERROR_MSG("Net metrics are not allowed!");
                    return;
                }
            }

            metric_enable_for_handler(metric, selected_handler);
            SERIAL_ECHO_START();
            SERIAL_ECHOLNPAIR_F("Metric enabled: ", parser.string_arg);
            return;
        }
    }

    SERIAL_ERROR_START();
    SERIAL_ECHOLNPAIR("Metric not found: ", parser.string_arg);
}

/**
 * M332: Disable metric
 *
 * ## Parameters
 *
 * - <metric> - Disable `metric` for the currently selected `handler`
 */

void PrusaGcodeSuite::M332() {
    if (selected_handler == NULL) {
        SERIAL_ERROR_MSG("Handler not set");
        return;
    }

    for (auto metric = metric_get_iterator_begin(), e = metric_get_iterator_end(); metric != e; metric++) {
        if (strcmp(metric->name, parser.string_arg) == 0) {
            metric_disable_for_handler(metric, selected_handler);
            SERIAL_ECHO_START();
            SERIAL_ECHOLNPAIR_F("Metric disabled: ", parser.string_arg);
            return;
        }
    }

    SERIAL_ERROR_START();
    SERIAL_ECHOLNPAIR("Metric not found: ", parser.string_arg);
}

/**
 * M333: List all metrics and whether they are enabled for the currently selected `handler`.
 */

void PrusaGcodeSuite::M333() {
    if (selected_handler == NULL) {
        SERIAL_ERROR_MSG("Handler not set");
        return;
    }

    for (auto metric = metric_get_iterator_begin(), e = metric_get_iterator_end(); metric != e; metric++) {
        bool is_enabled = metric->enabled_handlers & (1 << selected_handler->identifier);
        SERIAL_ECHO_START();
        SERIAL_ECHOPGM(metric->name);
        SERIAL_ECHOPGM(is_enabled ? " 1" : " 0");
        SERIAL_EOL();
    }
}

/**
 * M334: Handler-specific configuration
 */

void PrusaGcodeSuite::M334() {
    if (selected_handler == NULL) {
        SERIAL_ERROR_MSG("Handler not set");
        return;
    }

    if (selected_handler->identifier == METRIC_HANDLER_SYSLOG_ID) {
        // Syslog handler has to be allowed in settings
        const MetricsAllow metrics_allow = config_store().metrics_allow.get();
        if (metrics_allow != MetricsAllow::One && metrics_allow != MetricsAllow::All) {
            SERIAL_ERROR_MSG("Syslog metrics are not allowed!");
            return;
        }

        char ipaddr[config_store_ns::metrics_host_size + 1];
        char format[10]; ///< Format string for sscanf that cannot do string size from parameter
        snprintf(format, std::size(format), "%%%us %%i", std::size(ipaddr) - 1);
        int port;
        int read = sscanf(parser.string_arg, format, ipaddr, &port);
        if (read == 2) {
            // Only one host allowed
            if (metrics_allow == MetricsAllow::One) {
                if (strcmp(ipaddr, config_store().metrics_host.get_c_str()) != 0
                    || port != config_store().metrics_port.get()) {
                    SERIAL_ERROR_MSG("This is not the one host and port allowed!");
                    return;
                }
            }
            metric_handler_syslog_configure(ipaddr, port);
            SERIAL_ECHO_START();
            SERIAL_ECHOLN("Syslog handler configured successfully");
        } else {
            metric_handler_syslog_configure("", 0);
            SERIAL_ECHO_START();
            SERIAL_ECHOLN("does not match '<address> <port>' pattern; disabling syslog handler");
        }
    } else {
        SERIAL_ERROR_MSG("Selected handler does not support configuration");
    }
}

/** @}*/
