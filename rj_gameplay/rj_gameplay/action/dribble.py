"""This module contains the interface and action for dribble."""

from abc import ABC, abstractmethod

import stp.role as role
import stp.action as action
import stp.rc as rc
from rj_msgs.msg import RobotIntent
from typing import Optional
from rj_msgs import msg


class IDribble(action.IAction, ABC):

    def is_done(self) -> bool:
        pass

class Dribble(IDribble):
    """
    Dribble action
    """
    def __init__(self, robot_id: int):
        self.robot_id = robot_id

    def tick(self, intent: msg.RobotIntent) -> msg.RobotIntent:
    	intent.dribbler_speed = 1.0
        intent.is_active = True
        return intent

    def is_done(self) -> bool:
        return False;
