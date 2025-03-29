class Component:
    """Base class for writing components"""
    def __init__(self):
        self.name = None
        self.owner = None
        self.is_enabled = True
        self.is_visible = True

    def enabled(self):
        """Determine if a Component is enabled"""
        return self.is_enabled

    def enable(self, enable):
        """Enable or disable a Component"""
        self.is_enabled = enable

    def visible(self):
        """Determine if a Component is visible"""
        return self.is_visible

    def make_visible(self, visible):
        """Make a Component visible or invisible"""
        self.is_visible = visible

    def start(self):
        """Start event handler"""
        pass

    def activate(self):
        """Activate event handler"""
        pass

    def update(self):
        """Update event handler"""
        pass

    def render(self):
        """Render event handler"""
        pass

    def deactivate(self):
        """Deactivate event handler"""
        pass

    def end(self):
        """End event handler"""
        pass

    def collide(self):
        """Handle per tick collision events"""
        pass

    def collider_enter(self):
        """Handle collision enter events"""
        pass

    def collider_exit(self):
        """Handle collision exit events"""
        pass

    def get_name(self):
        """Get component name"""
        return self.name

    def set_name(self, new_name):
        self.name = new_name

    def get_owner(self):
        """Get component owner"""
        return self.owner

    def set_owner(self, new_owner):
        # TODO: we might need to emit an attach / detach event pair here
        self.owner = new_owner

    def parse(self, parse_data, rm):
        """Parse json contents"""
        if 'name' not in parse_data:
            raise ValueError('Component must have a field called "name"')
        self.name = str(parse_data['name'])

        if 'enabled' in parse_data:
            self.is_enabled = bool(parse_data['enabled'])

        if 'visible' in parse_data:
            self.is_visible = bool(parse_data['visible'])