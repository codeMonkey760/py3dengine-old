import py3dengine
from py3dmath import Vector3, Quaternion


class RotationComponent(py3dengine.Component):
    def __init__(self):
        self.speed = 0.0
        self.axis = Vector3(0, 0, 1)

    def update(self, dt):
        owner = self.get_owner()
        if owner is None:
            print('I don\'t have an owner!')
            return

        transform = owner.get_transform()
        if transform is None:
            print('My owner doesn\'t have a transform!')
            return

        displacement = Quaternion.FromAxisAndDegrees(self.axis, self.speed * dt)
        transform.rotate(displacement)

    def parse(self, values):
        if 'speed' in values.keys():
            self.speed = float(values['speed'])

        x = 0
        y = 0
        z = 0

        if 'axis' in values.keys():
            axis = values['axis']
            if 'x' in axis.keys():
                x = float(axis['x'])
            if 'y' in axis.keys():
                y = float(axis['y'])
            if 'z' in axis.keys():
                z = float(axis['z'])

        self.axis = Vector3(x, y, z)
