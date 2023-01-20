import py3dengine


class CameraComponent(py3dengine.Component):
    def __init__(self):
        self.fov_x_in_degrees = 0.0
        self.near_z = 0.0
        self.far_z = 0.0

    def parse(self, values, resource_manager):
        self.fov_x_in_degrees = float(values['fov_x_in_degrees'])
        self.near_z = float(values['near_z'])
        self.far_z = float(values['far_z'])
