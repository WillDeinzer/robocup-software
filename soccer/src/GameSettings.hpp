#pragma once

#include <optional>
#include <rj_common/RefereeEnums.hpp>
#include <rj_msgs/msg/game_settings.hpp>

using GameSettingsMsg = rj_msgs::msg::GameSettings;

/**
 * Settings for the game, set by MainWindow to be consumed by the rest of the
 * soccer program. This includes playbooks and general settings.
 */
struct GameSettings {
    // Whether or not we're in simulation.
    bool simulation = true;

    // Whether external referee is allowed.
    // If this is set to true but no external referee is connected,
    // none will be used and the control panel can be used as always.
    bool use_external_referee = false;

    // Requests. These can be overridden by the referee if it's enabled
    bool requestBlueTeam = true;
    int requestGoalieID = 0;

    // One-off command. This should be set once, and acknowledged by setting it
    // back to nullopt.
    std::optional<RefereeModuleEnums::Command> requestRefCommand = std::nullopt;

    // Defend the plus-x direction in vision
    bool defendPlusX = true;

    bool use_our_half = true;
    bool use_their_half = true;

    bool paused = false;

    struct JoystickConfig {
        int manualID = -1;
        bool dampedTranslation = true;
        bool dampedRotation = true;
        bool useKickOnBreakBeam = false;
        bool useFieldOrientedDrive = false;
    };

    JoystickConfig joystick_config;

    /**
     * Copy from another struct, but don't clear optional values (RPCs) unless
     * there is a new value.
     * @param other
     */
    void update(const GameSettings& other) {
        std::optional<RefereeModuleEnums::Command> refCommand =
            other.requestRefCommand;
        if (refCommand == std::nullopt) {
            refCommand = requestRefCommand;
        }

        *this = other;
        requestRefCommand = refCommand;
    }

    /**
     * @brief Implicit conversion to GameSettingsMsg.
     * @return
     */
    operator GameSettingsMsg() const {
        GameSettingsMsg msg{};
        msg.simulation = simulation;
        msg.use_external_referee = use_external_referee;
        msg.request_blue_team = requestBlueTeam;
        msg.request_goalie_id = requestGoalieID;
        msg.defend_plus_x = defendPlusX;
        msg.use_our_half = use_our_half;
        msg.use_their_half = use_their_half;
        msg.paused = paused;

        return msg;
    }
};