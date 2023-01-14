import py3dengine


class TestComponent(py3dengine.Component):
    def __init__(self):
        self.__theta = 0

    def update(self, dt):
        self.__theta = self.__theta + dt
        if self.__theta > 1.0:
            self.__theta = self.__theta - 1.0

            owner = self.get_owner()
            print('owner is None:', owner is None)
            print('type(owner).__name__:', type(owner).__name__)
            print('hasattr(owner, \'get_name\': ', hasattr(owner, 'get_name'))
            print('callable(owner.get_name):', callable(owner.get_name))
            print('owner.get_name():', owner.get_name())
