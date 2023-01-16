import math
import py3dengine


class RotationComponent(py3dengine.Component):
    def __init__(self):
        self.speed = 0.0
        self.axis = [0.0, 0.0, 1.0]

    def update(self, dt):
        owner = self.get_owner()
        if owner is None:
            print('I don\'t have an owner!')
            return

        transform = owner.get_transform()
        if transform is None:
            print('My owner doesn\'t have a transform!')
            return

        displacement = self.speed * dt
        fac = math.sin(displacement / 2.0)
        x = self.axis[0] * fac
        y = self.axis[1] * fac
        z = self.axis[2] * fac
        w = math.cos(displacement / 2.0)

        length = math.sqrt((x * x) + (y * y) + (z * z) + (w * w))
        x = x / length
        y = y / length
        z = z / length
        w = w / length

        transform.rotate(x, y, z, w)

    def parse(self, values):
        if 'speed' in values.keys():
            self.speed = math.radians(float(values['speed']))

        if 'axis' in values.keys():
            axis = values['axis']
            if 'x' in axis.keys():
                self.axis[0] = float(axis['x'])
            if 'y' in axis.keys():
                self.axis[1] = float(axis['y'])
            if 'z' in axis.keys():
                self.axis[2] = float(axis['z'])
