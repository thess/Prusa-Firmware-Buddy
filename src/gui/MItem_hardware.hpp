#pragma once
#include "WindowMenuItems.hpp"
#include "i18n.h"
#include <configuration_store.hpp>

enum class HWCheckType {
    nozzle,
    model,
    firmware,
    gcode,
    compatibility
};

template <HWCheckType HWCheck>
class MI_HARDWARE_CHECK_t : public WI_SWITCH_t<3> {
    constexpr static const char *str_none = N_("None");
    constexpr static const char *str_warn = N_("Warn");
    constexpr static const char *str_strict = N_("Strict");

    size_t get_eeprom() {
        switch (HWCheck) {
        case HWCheckType::nozzle:
            return static_cast<size_t>(config_store().hw_check_nozzle.get());
        case HWCheckType::model:
            return static_cast<size_t>(config_store().hw_check_model.get());
        case HWCheckType::firmware:
            return static_cast<size_t>(config_store().hw_check_firmware.get());
        case HWCheckType::gcode:
            return static_cast<size_t>(config_store().hw_check_gcode.get());
        case HWCheckType::compatibility:
            return static_cast<size_t>(config_store().hw_check_compatibility.get());
        default:
            assert(false);
            return 1;
        }
    }

public:
    MI_HARDWARE_CHECK_t(string_view_utf8 label)
        : WI_SWITCH_t(get_eeprom(), label, nullptr, is_enabled_t::yes, is_hidden_t::no,
            string_view_utf8::MakeCPUFLASH((const uint8_t *)str_none),
            string_view_utf8::MakeCPUFLASH((const uint8_t *)str_warn),
            string_view_utf8::MakeCPUFLASH((const uint8_t *)str_strict)) {}

protected:
    void OnChange([[maybe_unused]] size_t old_index) override {
        switch (HWCheck) {
        case HWCheckType::nozzle:
            config_store().hw_check_nozzle.set(static_cast<HWCheckSeverity>(index));
            break;
        case HWCheckType::model:
            config_store().hw_check_model.set(static_cast<HWCheckSeverity>(index));
            break;
        case HWCheckType::firmware:
            config_store().hw_check_firmware.set(static_cast<HWCheckSeverity>(index));
            break;
        case HWCheckType::gcode:
            config_store().hw_check_gcode.set(static_cast<HWCheckSeverity>(index));
            break;
        case HWCheckType::compatibility:
            config_store().hw_check_compatibility.set(static_cast<HWCheckSeverity>(index));
            break;
        default:
            assert(false);
            break;
        }
    }
};

class MI_NOZZLE_DIAMETER : public WiSpinFlt {
    static constexpr const char *const label = N_("Nozzle Diameter");

    int tool_idx; ///< Configure this tool [indexed from 0]

    /**
     * @brief Get diameter value stored in eeprom.
     * @param tool_idx this tool [indexed from 0] (has to be parameter since internal variable is not yet inited when using this function)
     * @return diameter value
     */
    float get_eeprom(int tool_idx) const;

public:
    /**
     * @brief Construct menu item to configure nozzle diameter.
     * @param tool_idx this tool [indexed from 0]
     * @param with_toolchanger whether to hide this item if toolchanger is enabled
     */
    MI_NOZZLE_DIAMETER(int tool_idx = 0, is_hidden_t with_toolchanger = is_hidden_t::yes);
    virtual void OnClick() override;
};

class MI_HARDWARE_G_CODE_CHECKS : public WI_LABEL_t {
    static constexpr const char *const label = N_("G-Code Checks");

public:
    MI_HARDWARE_G_CODE_CHECKS();

protected:
    virtual void click(IWindowMenu &windowMenu) override;
};

class MI_NOZZLE_DIAMETER_CHECK : public MI_HARDWARE_CHECK_t<HWCheckType::nozzle> {
    static constexpr const char *const label = N_("Nozzle Diameter");

public:
    MI_NOZZLE_DIAMETER_CHECK()
        : MI_HARDWARE_CHECK_t(_(label)) {}
};

class MI_PRINTER_MODEL_CHECK : public MI_HARDWARE_CHECK_t<HWCheckType::model> {
    static constexpr const char *const label = N_("Printer Model");

public:
    MI_PRINTER_MODEL_CHECK()
        : MI_HARDWARE_CHECK_t(_(label)) {}
};

class MI_FIRMWARE_CHECK : public MI_HARDWARE_CHECK_t<HWCheckType::firmware> {
    static constexpr const char *const label = N_("Firmware Version");

public:
    MI_FIRMWARE_CHECK()
        : MI_HARDWARE_CHECK_t(_(label)) {}
};

class MI_GCODE_LEVEL_CHECK : public MI_HARDWARE_CHECK_t<HWCheckType::gcode> {
    static constexpr const char *const label = N_("G-Code Level");

public:
    MI_GCODE_LEVEL_CHECK()
        : MI_HARDWARE_CHECK_t(_(label)) {}
};

class MI_MK3_COMPATIBILITY_CHECK : public MI_HARDWARE_CHECK_t<HWCheckType::compatibility> {
    static constexpr const char *const label = N_("MK3 Compatibility");

public:
    MI_MK3_COMPATIBILITY_CHECK()
        : MI_HARDWARE_CHECK_t(_(label)) {}
};

class MI_NOZZLE_TYPE : public WI_SWITCH_t<2> {
    static constexpr const char *const label = N_("Nozzle Type");

    constexpr static const char *str_normal = N_("Normal");
    constexpr static const char *str_high_flow = N_("High Flow");

public:
    MI_NOZZLE_TYPE();

protected:
    virtual void OnChange(size_t old_index) override;
};

class MI_NOZZLE_SOCK : public WI_ICON_SWITCH_OFF_ON_t {
    static constexpr const char *const label = N_("Nextruder silicone sock");

public:
    MI_NOZZLE_SOCK();

protected:
    virtual void OnChange(size_t old_index) override;
};
