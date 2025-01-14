#include "planning/planner/line_kick_path_planner.hpp"

#include <rj_geometry/util.hpp>

#include "control/trapezoidal_motion.hpp"
#include "escape_obstacles_path_planner.hpp"
#include "planning/primitives/create_path.hpp"
#include "planning/trajectory_utils.hpp"

using namespace std;
using namespace rj_geometry;

namespace planning {

Trajectory LineKickPathPlanner::plan(const PlanRequest& plan_request) {
    if (plan_request.play_state == PlayState::halt() ||
        plan_request.play_state == PlayState::stop()) {
        return Trajectory{};
    }

    const BallState& ball = plan_request.world_state->ball;

    if (!average_ball_vel_initialized_) {
        average_ball_vel_ = ball.velocity;
        average_ball_vel_initialized_ = true;
    } else {
        // Add the newest ball velocity measurement to the average velocity
        // estimate, but downweight the new value heavily
        //
        // e.g. new_avg_vel = (0.8 * avg_vel) + (0.2 * new_vel)
        //
        // TODO(Kevin): make this gain a ROS param like collect
        average_ball_vel_ = apply_low_pass_filter(average_ball_vel_, ball.velocity, 0.8);
    }

    // only process state transition if already started the planning
    if (has_created_plan) {
        process_state_transition();
    }
    switch (current_state_) {
        case INITIAL_APPROACH:
            prev_path_ = initial(plan_request);
            break;
        case FINAL_APPROACH:
            prev_path_ = final(plan_request);
            break;
    }
    prev_path_.stamp(RJ::now());
    has_created_plan = true;
    return prev_path_;
}

Trajectory LineKickPathPlanner::initial(const PlanRequest& plan_request) {
    // Getting ball info
    const BallState& ball = plan_request.world_state->ball;

    // Creating a modified plan_request to send to PathTargetPlanner
    PlanRequest modified_request = plan_request;

    // Where to navigate to
    auto distance_from_ball = kBallRadius + kRobotRadius + kAvoidBallBy;

    auto ball_position = ball.predict_at(RJ::now() + RJ::Seconds{kPredictIn}).position;

    auto goal_to_ball = (plan_request.motion_command.target.position - ball_position);
    auto offset_from_ball = distance_from_ball * goal_to_ball.normalized();

    // Updating the motion command
    LinearMotionInstant target{ball_position - offset_from_ball};

    MotionCommand modified_command{"path_target", target,
                                   FacePoint{plan_request.motion_command.target.position}};
    modified_request.motion_command = modified_command;

    // Getting the new path from PathTargetPlanner
    Trajectory path = path_target_.plan(modified_request);

    return path;
}

Trajectory LineKickPathPlanner::final(const PlanRequest& plan_request) {
    // remove ball from obstacles not needed?

    const BallState& ball = plan_request.world_state->ball;

    // Creating a modified plan_request to send to PathTargetPlanner
    PlanRequest modified_request = plan_request;

    // velocity
    auto goal_to_ball = (plan_request.motion_command.target.position - ball.position);
    auto vel = goal_to_ball.normalized() * kFinalRobotSpeed;

    LinearMotionInstant target{ball.position, vel};
    MotionCommand modified_command{"path_target", target,
                                   FacePoint{plan_request.motion_command.target.position}, true};
    modified_request.motion_command = modified_command;

    // Getting the new path from PathTargetPlanner
    Trajectory path = path_target_.plan(modified_request);

    return path;
}

void LineKickPathPlanner::process_state_transition() {
    if (current_state_ == INITIAL_APPROACH && (path_target_.is_done())) {
        current_state_ = FINAL_APPROACH;
    }
}

bool LineKickPathPlanner::is_done() const {
    // if ball is fast, assume we have kicked it correctly
    // (either way we can't go recapture it)
    return average_ball_vel_.mag() > kIsDoneBallVel;
}

}  // namespace planning
