import math
import py3dengine


class RotationComponent(py3dengine.Component):
    def update(self, dt):
        owner = self.get_owner()
        if owner is None:
            print('I don\'t have an owner!')
            return

        transform = owner.get_transform()
        if transform is None:
            print('My owner doesn\'t have a transform!')
            return

        speed = math.pi / 2.0
        displacement = speed * dt
        fac = math.sin(displacement / 2.0)
        x = 0.0 * fac
        y = 0.0 * fac
        z = 1.0 * fac
        w = math.cos(displacement / 2.0)

        length = math.sqrt((x * x) + (y * y) + (z * z) + (w * w))
        x = x / length
        y = y / length
        z = z / length
        w = w / length

        transform.rotate(x, y, z, w)
