# Game Module

This is the introduction of game module. The game layer should handle all business logic about game system, but not include the lower level graphic api, file parser, asset loading etc.

The game module using simple Entity-Component structure to design, no system is implemented at current stage.

## Terminology

### OnXXX vs XXX (e.g. OnUpdate vs Update)

The OnXXX method is designed for customization, while XXX method is used for system update. For example, OnUpdate used to define the per-frame custom behavior in component, but Update used to call the OnUpdate of each component.

### Frame Packet

The frame context to render the frame. We constuct this in OnRender method. This include all the command in different passes.

### Context

We can get context at Component and Scene by simply `GetContext`.

The context should store the service localator and other cross module/component/system helper.

### Event System

The event system is implemented which we could emit custom event or process system pre-defined input event.