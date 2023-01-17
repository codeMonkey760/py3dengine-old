import py3dengine


class CameraComponent(py3dengine.Component):
    def __init__(self):
        self.fov_x_in_degrees = 0.0
        self.near_z = 0.0
        self.far_z = 0.0

    def parse(self, values):
        print(str(values.items()))

        if 'fov_x_in_degrees' in values.keys():
            self.fov_x_in_degrees = float(values['fov_x_in_degrees'])
        if 'near_z' in values.keys():
            self.near_z = float(values['near_z'])
        if 'far_z' in values.keys():
            self.far_z = float(values['far_z'])
