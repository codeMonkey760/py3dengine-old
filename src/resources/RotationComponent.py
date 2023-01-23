import py3dengine
from py3dmath import Vector3, Quaternion


class RotationComponent(py3dengine.Component):
    def __init__(self):
        self.speed = 0.0
        self.axis = Vector3(0, 0, 1)

    def update(self, dt):
        transform = self.get_owner().get_transform()

        displacement = Quaternion.FromAxisAndDegrees(self.axis, self.speed * dt)
        transform.rotate(displacement)

    def parse(self, values, resource_manager):
        self.speed = float(values['speed'])
        x = float(values['axis']['x'])
        y = float(values['axis']['y'])
        z = float(values['axis']['z'])

        self.axis = Vector3(x, y, z)
