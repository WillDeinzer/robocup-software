from abc import ABC, abstractmethod
from typing import Callable

import rj_gameplay.eval as eval
import argparse
import py_trees
import sys
import time
import numpy as np

import stp.skill as skill
import stp.role as role
import stp.action as action
from stp.skill.action_behavior import ActionBehavior
import stp.rc as rc

class IMark(skill.ISkill, ABC):
    ...

"""
A skill which marks a given opponent robot according to some heuristic cost function
"""
class Mark(IMark):

    def __init__(self,
            robot : rc.Robot = None,
            target_point : np.ndarray = np.array([0.0,0.0]),
            target_vel : np.ndarray = np.array([0.0,0.0]),
            face_angle : Optional[float] = None,
            face_point : Optional[np.ndarray] = None):

        # TODO: use mark_heuristic & CostBehavior to handle marking, rather than having tactic give target_point
        # argument: mark_heuristic: Callable[[np.array], float]
        # > self.mark_heuristic = mark_heuristic

        # workaround for non-working CostBehavior: 
        # initialize move action, update target point every tick (target point being opponent robot pos)

        self.__name__ = 'Mark Skill'
        self.robot = robot

        self.target_point = target_point
        if self.robot is not None:
            self.move = move.Move(self.robot.id, target_point, target_vel, face_angle, face_point)
        else:
            self.move = move.Move(self.robot, target_point, target_vel, face_angle, face_point)

        self.mark_behavior = ActionBehavior('Mark', self.move)
        self.root = self.mark_behavior
        self.root.setup_with_descendants()

    def tick(self, robot: rc.Robot, new_target: np.ndarray, world_state: rc.WorldState) -> None:
        self.robot = robot
        self.target_point = new_target
        actions = self.root.tick_once(robot, world_state)
        return actions
