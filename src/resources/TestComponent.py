import py3dengine

class TestComponent(py3dengine.Component):
    def __init__(self):
        self.__theta = 0

    def update(self, dt):
        self.__theta = self.__theta + dt
        if self.__theta > 1.0:
            self.__theta = self.__theta - 1.0

            print("TestComponent::update ... dt was", str(dt))
